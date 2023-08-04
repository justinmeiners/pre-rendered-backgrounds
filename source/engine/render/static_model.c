
#include "static_model.h"
#include "stretchy_buffer.h"
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"


/* indicies for a complete triangle */


typedef struct
{
    int v[3];
    int uv[3];
    int n[3];
} ObjFaceIndexSet;

/* Basic OBJ parser requires uvs, normals, and vertices */
static int StaticModel_FromObj(StaticModel* model, FILE* file)
{
    if (!model) return 0;
    
    /* setup buffers */
    char buffer[1024];
    memset(buffer, 0x0, sizeof(buffer));
    char textBuffer[64];
    memset(textBuffer, 0x0, sizeof(textBuffer));
    int err = 0;
    
    Vec3* vertReadBuffer = NULL;
    Vec3* normReadBuffer = NULL;
    Vec2* uvReadBuffer = NULL;
    ObjFaceIndexSet* faceReadBuffer = NULL;

    /* initialize with some starting values - any number would work this just
     reduces the amount of reallocs that will be called for most model loads
     
     */
    
    int startSize = 512;
    stb__sbmaybegrow(vertReadBuffer, startSize);
    stb__sbmaybegrow(normReadBuffer, startSize);
    stb__sbmaybegrow(uvReadBuffer, startSize);
    stb__sbmaybegrow(faceReadBuffer, startSize);
    
    while (!feof(file))
    {
        fgets(buffer, 1024, file);
        
        err = sscanf(buffer, "%s ", textBuffer);
        
        if (err == EOF)
        {
            goto error;
        }
        
        if (strcmp(textBuffer, "s") == 0)
        {
            /* smoothing groups - for noobs*/
        }
        else if (strcmp(textBuffer, "v") == 0)
        {
            /* parse vert */
            Vec3 vert;
            err = sscanf(buffer, "%s %f %f %f", textBuffer, &vert.x, &vert.y, &vert.z);
            stb_sb_push(vertReadBuffer, vert);
        }
        else if (strcmp(textBuffer, "vt") == 0)
        {
            /* parse uv */
            Vec2 uv;
            err = sscanf(buffer, "%s %f %f", textBuffer, &uv.x, &uv.y);
            stb_sb_push(uvReadBuffer, uv);
        }
        else if (strcmp(textBuffer, "vn") == 0)
        {
            /* parse normal */
            Vec3 norm;
            err = sscanf(buffer, "%s %f %f %f", textBuffer, &norm.x, &norm.y, &norm.z);
            stb_sb_push(normReadBuffer, norm);
        }
        else if (strcmp(textBuffer, "f") == 0)
        {
            /* one for each tri */
            ObjFaceIndexSet face;
            err = sscanf(buffer,
                         "%s %d/%d/%d %d/%d/%d %d/%d/%d",
                         textBuffer,
                         &face.v[0],
                         &face.uv[0],
                         &face.n[0],
                         &face.v[1],
                         &face.uv[1],
                         &face.n[1],
                         &face.v[2],
                         &face.uv[2],
                         &face.n[2]);
            
            stb_sb_push(faceReadBuffer, face);
        }
        
        if (err == EOF)
        {
            goto error;
        }
        
        textBuffer[0] = ' ';
    }
    
    int faceCount = stb_sb_count(faceReadBuffer);
    
    /* 3 verts in a tri */
    StaticMesh_Init(&model->mesh, faceCount * 3, 1);
    
    for (int i = 0; i < faceCount; ++i)
    {
        const ObjFaceIndexSet* indexSet = faceReadBuffer + i;
        
        int vertIndex = i * 3;
        for (int j = 0; j < 3; ++j)
        {
            StaticMeshVert* vert = model->mesh.verts + (vertIndex + j);
            
            /* oddly OBJ indicies start at 1 so we must subtract 1 from each */
            vert->pos = vertReadBuffer[indexSet->v[j] - 1];
            vert->normal = normReadBuffer[indexSet->n[j] - 1];
            
            Vec2 tempUv = uvReadBuffer[indexSet->uv[j] - 1];

            /* Store uvs in normalized 16 bit shorts */
            vert->uv[0].u = USHRT_MAX * tempUv.x;
            vert->uv[0].v = USHRT_MAX * tempUv.y;
        }
    }
    
    stb_sb_free(vertReadBuffer);
    stb_sb_free(normReadBuffer);
    stb_sb_free(uvReadBuffer);
    stb_sb_free(faceReadBuffer);
        
    return 1;
    
error:
    stb_sb_free(vertReadBuffer);
    stb_sb_free(normReadBuffer);
    stb_sb_free(uvReadBuffer);
    stb_sb_free(faceReadBuffer);
    
    return 0;
}

static int StaticModel_FromBMesh(StaticModel* mesh, FILE* file)
{
    int32_t version;
    int32_t vertCount;
    int32_t uvChannelCount;
    fread(&version, sizeof(int32_t), 1, file);
    version = End_ReadLittle32(version);
    
    if (version != 1)
    {
        return 0;
    }
    
    fread(&vertCount, sizeof(int32_t), 1, file);
    vertCount = End_ReadLittle32(vertCount);
    
    fread(&uvChannelCount, sizeof(int32_t), 1, file);
    uvChannelCount = End_ReadLittle32(uvChannelCount);

    StaticMesh_Init(&mesh->mesh, vertCount, uvChannelCount);

    int i;
    for (i = 0; i < vertCount; ++i)
    {
        fread(&mesh->mesh.verts[i].pos, sizeof(Vec3), 1, file);
        fread(&mesh->mesh.verts[i].normal, sizeof(Vec3), 1, file);

        Vec2 uv0;
        fread(&uv0, sizeof(Vec2), 1, file);
        mesh->mesh.verts[i].uv[0].u =  ceilf(USHRT_MAX * uv0.x);
        mesh->mesh.verts[i].uv[0].v =  ceilf(USHRT_MAX * uv0.y);

        Vec2 uv1;
        fread(&uv1, sizeof(Vec2), 1, file);
        mesh->mesh.verts[i].uv[1].u =  ceilf(USHRT_MAX * uv1.x);
        mesh->mesh.verts[i].uv[1].v =  ceilf(USHRT_MAX * uv1.y);
    }
    
    return 1;
}


static int StaticModel_FromMesh(StaticModel* mesh, FILE* file)
{
    char lineBuffer[LINE_BUFFER_MAX];

    int i = 0;
    int vertCount = -1;
    int uvChannels = -1;
    int currentChannel = 0;
    
    int version = -1;
    
    int readInfo = 0;
    
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
        if (strstr(command, "version"))
        {
             version = atoi(data);
        }
        else if (strstr(command, "vertex_count"))
        {
            vertCount = atoi(data);
        }
        else if (strstr(command, "uv_channel_count"))
        {
            uvChannels = atoi(data);
        }
        else if (strstr(command, "vertices"))
        {
            if (!readInfo)
            {
                if (vertCount == -1 || uvChannels == -1)
                {
                    return 0;
                }
                
                StaticMesh_Init(&mesh->mesh, vertCount, uvChannels);
                
                readInfo = 1;
            }
            
            for (i = 0; i < vertCount; ++i)
            {
                StaticMeshVert* vert = mesh->mesh.verts + i;
                
                fgets(lineBuffer, LINE_BUFFER_MAX, file);
                sscanf(lineBuffer, "%f, %f, %f", &vert->pos.x, &vert->pos.y, &vert->pos.z);
            }
        }
        else if (strstr(command, "normals"))
        {
            for (i = 0; i < vertCount; ++i)
            {
                StaticMeshVert* vert = mesh->mesh.verts + i;

                fgets(lineBuffer, LINE_BUFFER_MAX, file);
                sscanf(lineBuffer, "%f, %f, %f", &vert->normal.x, &vert->normal.y, &vert->normal.z);
            }
        }
        else if (strstr(command, "tangents"))
        {
            for (i = 0; i < vertCount; ++i)
            {

                fgets(lineBuffer, LINE_BUFFER_MAX, file);
                //sscanf(lineBuffer, "%f, %f, %f", &mesh->mesh.tangents[i].x, &mesh->mesh.tangents[i].y, &mesh->mesh.tangents[i].z);
            }
        }
        else if (strstr(command, "uv"))
        {
            Vec2 tempUv;

            for (i = 0; i < vertCount; ++i)
            {
                fgets(lineBuffer, LINE_BUFFER_MAX, file);
                
                sscanf(lineBuffer, "%f, %f", &tempUv.x, &tempUv.y);
        
                StaticMeshVert* vert = mesh->mesh.verts + i;
                
                /* Store uvs in normalized 16 bit shorts */
                vert->uv[currentChannel].u = ceilf(USHRT_MAX * tempUv.x);
                vert->uv[currentChannel].v = ceilf(USHRT_MAX * tempUv.y);
            }
            ++currentChannel;
        }
    }    
    return 1;
}

int StaticModel_FromPath(StaticModel* model, const char* path)
{
    FILE* file = fopen(path, "r");
    
    if (!file) return 0;
    
    int status = 0;
        
    if (strcmp(Filepath_Extension(path), "obj") == 0)
    {
        status = StaticModel_FromObj(model, file);
    }
    else if (strcmp(Filepath_Extension(path), "mesh") == 0)
    {
        status = StaticModel_FromMesh(model, file);
    }
    else if (strcmp(Filepath_Extension(path), "bmesh") == 0)
    {
        status = StaticModel_FromBMesh(model, file);
    }
    
    fclose(file);
    
    if (!status)
        return 0;
    
    Material_Init(&model->material);
    
    return 1;
}

int StaticModel_Copy(StaticModel* dest, const StaticModel* source)
{
    if (!dest || !source)
        return 0;
    
    if (!StaticMesh_Copy(&dest->mesh, &source->mesh))
        return 0;

    Material_Copy(&dest->material, &source->material);
        
    return 1;
}

void StaticModel_Shutdown(StaticModel* model)
{
    StaticMesh_Shutdown(&model->mesh);
}



