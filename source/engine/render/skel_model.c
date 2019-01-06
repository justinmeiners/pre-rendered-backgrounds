
#include "skel_model.h"
#include <assert.h>
#include <string.h>
#include <limits.h>
#include "utils.h"

static int SkelModel_FromSKMESH(SkelModel* model, FILE* file)
{
    char lineBuffer[LINE_BUFFER_MAX];
    int readInfo = 0;
    
    int version = -1;
    int vertexCount = -1;
    int boneCount = -1;
    int uvChannelCount = -1;
    int weightCount = -1;
    
    int attachPointCount = 0;
    
    Vec3 origin = Vec3_Zero;
    
    while (!feof(file))
    {
        fgets(lineBuffer, LINE_BUFFER_MAX, file);
        char* ptr;
        char* command = strtok_r(lineBuffer, ":", &ptr);
        char* data = strtok_r(NULL, ":", &ptr);
        
        if (String_Prefix(command, "#", LINE_BUFFER_MAX))
        {
            // comment
        }
        else if (strstr(command, "version"))
        {
            version = atoi(data);
            
            if (version > 4)
            {
                return 0;
            }
        }
        else if (strstr(command, "origin"))
        {
            sscanf(data, " [%f, %f, %f]", &origin.x, &origin.y, &origin.z);
        }
        else if (strstr(command, "vertex_count"))
        {
            vertexCount = atoi(data);
        }
        else if (strstr(command, "bone_count"))
        {
            boneCount = atoi(data);
        }
        else if (strstr(command, "uv_channel_count"))
        {
            uvChannelCount = atoi(data);
        }
        else if (strstr(command, "weight_count"))
        {
            weightCount = atoi(data);
        }
        else if (strstr(command, "attach_point_count"))
        {
            attachPointCount = atoi(data);
        }
        else if (strstr(command, "bones"))
        {
            if (!readInfo)
            {
                if (vertexCount == -1 || uvChannelCount == -1 || boneCount == -1 || weightCount == -1)
                    return 0;
                
                Skel_Init(&model->skel, boneCount, attachPointCount);
                model->skel.origin = origin;
                
                SkelSkin_Init(&model->skin, vertexCount, weightCount);
                
                readInfo = 1;
            }
            
            for (int i = 0; i < boneCount; ++i)
            {
                SkelJoint* joint = &model->skel.joints[i];
                
                fgets(lineBuffer, LINE_BUFFER_MAX, file);
                
                if (version > 1)
                {
                    sscanf(lineBuffer, "( %s ), %hi, [%f, %f, %f]", joint->name, &joint->parent, &joint->tail.x, &joint->tail.y, &joint->tail.z);
                }
                else
                {
                    sscanf(lineBuffer, "( %s ), %hi, %f, %f, %f", joint->name, &joint->parent, &joint->tail.x, &joint->tail.y, &joint->tail.z);
                }
                
                joint->rotation = Quat_Identity;
                
            }
        }
        else if (strstr(command, "attach_points"))
        {
            for (int i = 0; i < attachPointCount; ++ i)
            {
                SkelAttachPoint* attachPoint = &model->skel.attachPoints[i];
                fgets(lineBuffer, LINE_BUFFER_MAX, file);
                
                sscanf(lineBuffer, "( %s ), %hi, [%f, %f, %f] [%f, %f, %f, %f]",
                       attachPoint->name,
                       &attachPoint->joint,
                       &attachPoint->offset.x,
                       &attachPoint->offset.y,
                       &attachPoint->offset.z,
                       &attachPoint->rotation.w,
                       &attachPoint->rotation.x,
                       &attachPoint->rotation.y,
                       &attachPoint->rotation.z);
            }
        }
        else if (strstr(command, "normals"))
        {
            for (int i = 0; i < vertexCount; ++i)
            {
                SkelSkinVert* vert = model->skin.verts + i;
                
                fgets(lineBuffer, LINE_BUFFER_MAX, file);
                sscanf(lineBuffer, "%f, %f, %f", &vert->normal.x, &vert->normal.y, &vert->normal.z);
            }
        }
        else if (strstr(command, "tangents"))
        {
            for (int i = 0; i < vertexCount; ++i)
            {
                SkelSkinVert* vert = model->skin.verts + i;
                
                fgets(lineBuffer, LINE_BUFFER_MAX, file);
                sscanf(lineBuffer, "%f, %f, %f", &vert->tangent.x, &vert->tangent.y, &vert->tangent.z);
            }
        }
        else if (strstr(command, "uv"))
        {
            Vec2 tempUv;
            for (int i = 0; i < vertexCount; ++i)
            {
                SkelSkinVert* vert = model->skin.verts + i;
                
                fgets(lineBuffer, LINE_BUFFER_MAX, file);
                sscanf(lineBuffer, "%f, %f", &tempUv.x, &tempUv.y);
                vert->uv.u = ceilf(USHRT_MAX * tempUv.x);
                vert->uv.v = ceilf(USHRT_MAX * tempUv.y);
            }
        }
        else if (strstr(command, "weights"))
        {
            int weightIndex = 0;
            int weightsPerGroup;
            
            for (int i = 0; i < vertexCount; ++i)
            {
                SkelSkinVert* vert = model->skin.verts + i;

                fgets(lineBuffer, LINE_BUFFER_MAX, file);
                sscanf(lineBuffer, "%i", &weightsPerGroup);
                
                if (weightsPerGroup > SKEL_WEIGHTS_PER_VERT)
                    printf("too many weights: %i\n", weightsPerGroup);

                if (weightsPerGroup <= 0)
                    printf("vertex not assigned to weight: %i\n", i);
                
                float influence = 0.0f;
                
                for (int j = 0; j < weightsPerGroup; ++j)
                {
                    short jointIndex = -1;
                    
                    fgets(lineBuffer, LINE_BUFFER_MAX, file);
                    sscanf(lineBuffer, "%hi, %f, %f, %f, %f",
                           &jointIndex,
                           &vert->weights[j].w,
                           &vert->weights[j].x,
                           &vert->weights[j].y,
                           &vert->weights[j].z);
                    
                    ++weightIndex;
                    
                    influence += vert->weights[j].w;
                    
                    assert(jointIndex != -1);

                    vert->weightJoints[j] = jointIndex;
                }
                
                if (fabs(influence - 1.0f) > V_EPSILON)
                    printf("low bone influence: %f\n", influence);
            }
        }
    }
    

    return 1;
    
error:

    return 0;
}

int SkelModel_FromPath(SkelModel* model, const char* path)
{
    FILE* file = fopen(path, "r");
    
    if (!file)
        return 0;
    
    int status = 0;
    
    if (strcmp(Filepath_Extension(path), "skmesh") == 0)
    {
        status = SkelModel_FromSKMESH(model, file);
    }
    
    fclose(file);
    
    if (!status)
        return 0;
    
    SkelAnimator_Init(&model->animator, &model->skel);
    Material_Init(&model->material);
    
    return 1;
}

int SkelModel_Copy(SkelModel* dest, const SkelModel* source)
{
    if (!dest || !source)
        return 0;
    
    if (!Skel_Copy(&dest->skel, &source->skel))
        return 0;
    
    if (!SkelSkin_Copy(&dest->skin, &source->skin))
        return 0;
    
    if (!SkelAnimator_Init(&dest->animator, &dest->skel))
        return 0;
    
    Material_Copy(&dest->material, &source->material);
    
    memcpy(dest->attachPointTable, source->attachPointTable, sizeof(source->attachPointTable));
    
    return 1;
}

void SkelModel_Shutdown(SkelModel* model)
{
    Skel_Shutdown(&model->skel);
    SkelSkin_Shutdown(&model->skin);
    SkelAnimator_Shutdown(&model->animator);
}

SkelAnimatorInfo SkelModel_Tick(SkelModel* model)
{
    assert(model);
    
    SkelAnimatorInfo info = SkelAnimator_Tick(&model->animator);
    Skel_Pose(&model->skel);
    
    return info;
}

const SkelAttachPoint* SkelModel_AttachPointAt(const SkelModel* model, int loc)
{
    int index = model->attachPointTable[loc];
    assert(index >= 0 && index < model->skel.attachPointCount);
    return model->skel.attachPoints + index;
}



