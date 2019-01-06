

#include "skel.h"
#include <assert.h>
#include "utils.h"


int Skel_Init(Skel* skel, short jointCount, short attachPointCount)
{
    if (!skel)
        return 0;
    
    if (attachPointCount > SKEL_ATTACH_POINTS_MAX)
    {
        printf("attachPointCount: %i exceeds maximum\n", attachPointCount);
        return 0;
    }
    
    memset(skel, 0, sizeof(Skel));
    skel->jointCount = jointCount;
    skel->attachPointCount = attachPointCount;
    
    skel->joints = malloc(sizeof(SkelJoint) * jointCount);
    if (!skel->joints)
        return 0;
    
    skel->renderJointRotations = malloc(sizeof(Quat) * jointCount);
    if (!skel->renderJointRotations)
    {
        free(skel->joints);
        return 0;
    }
    
    skel->renderJointOrigins = malloc(sizeof(Vec3) * jointCount);
    if (!skel->renderJointRotations)
    {
        free(skel->joints);
        free(skel->renderJointRotations);
        return 0;
    }
    
    for (int i = 0; i < jointCount; ++i)
    {
        skel->joints[i].modelRotation = Quat_Identity;
        skel->joints[i].rotation = Quat_Identity;
    }
    
    for (int i = 0; i < SKEL_ATTACH_POINTS_MAX; ++i)
    {
        skel->attachPoints[i].rotation = Quat_Identity;
        skel->attachPoints[i].modelRotation = Quat_Identity;
    }
    
    skel->origin = Vec3_Zero;
    skel->rotation = Quat_Identity;
    skel->offset = Vec3_Zero;
    
    return 1;

}

int Skel_Copy(Skel* dest, const Skel* source)
{
    if (!dest || ! source)
        return 0;
    
    dest->jointCount = source->jointCount;
    dest->attachPointCount = source->attachPointCount;
    
    dest->joints = malloc(sizeof(SkelJoint) * source->jointCount);
    if (!dest->joints)
        return 0;
    
    dest->renderJointRotations = malloc(sizeof(Quat) * source->jointCount);
    if (!dest->renderJointRotations)
    {
        free(dest->joints);
        return 0;
    }
    
    dest->renderJointOrigins = malloc(sizeof(Vec3) * source->jointCount);
    if (!dest->renderJointRotations)
    {
        free(dest->joints);
        free(dest->renderJointRotations);
        return 0;
    }
    
    memcpy(dest->joints, source->joints, source->jointCount * sizeof(SkelJoint));
    memcpy(dest->attachPoints, source->attachPoints, source->attachPointCount * sizeof(SkelAttachPoint));

    dest->origin = source->origin;
    dest->rotation = source->rotation;
    dest->offset = source->offset;
    return 1;
}

void Skel_Shutdown(Skel* skel)
{
    if (skel->joints)
        free(skel->joints);
    
    if (skel->renderJointOrigins)
        free(skel->renderJointOrigins);
    
    if (skel->renderJointRotations)
        free(skel->renderJointRotations);
}

/* posing transforms the world joints by the current local joint configuration */
void Skel_Pose(Skel* skel)
{
    assert(skel);
    
    Vec3 worldOrigin = Quat_MultVec3(&skel->rotation, Vec3_Add(skel->origin, skel->offset));
    
    for (int i = 0; i < skel->jointCount; ++i)
    {
        SkelJoint* joint = skel->joints + i;
        
        if (joint->parent < 0)
        {
            joint->modelHead = worldOrigin;
            joint->modelTail =  Vec3_Add(worldOrigin, joint->tail);
            joint->modelRotation = Quat_Mult(skel->rotation, joint->rotation);
        }
        else
        {
            const SkelJoint* parent = &skel->joints[joint->parent];
            
            joint->modelHead = parent->modelTail;
            joint->modelRotation = Quat_Mult(parent->modelRotation, joint->rotation);
            joint->modelTail = Vec3_Add(Quat_MultVec3(&joint->modelRotation, joint->tail), parent->modelTail);
        }
        
        skel->renderJointOrigins[i] = joint->modelHead;
        skel->renderJointRotations[i] = joint->modelRotation;
    }
    
    for (int i = 0; i < skel->attachPointCount; ++i)
    {
        SkelAttachPoint* point = skel->attachPoints + i;
        assert(point->joint >= 0 && point->joint < skel->jointCount);
        
        const SkelJoint* joint = skel->joints + point->joint;
        
        point->modelRotation = Quat_Mult(joint->modelRotation, point->rotation);
        point->modelPosition = Vec3_Add(Quat_MultVec3(&joint->modelRotation, point->offset), joint->modelTail);
    }
}

short Skel_FindJointIndex(const Skel* skel, const char* name)
{
    for (int i = 0; i < skel->jointCount; ++i)
    {
        if (strcmp(skel->joints[i].name, name) == 0)
        {
            return i;
        }
    }

    return -1;
}

const SkelJoint* Skel_FindJoint(const Skel* skel, const char* name)
{
    int index = Skel_FindJointIndex(skel, name);
    
    if (index == -1)
    {
        return NULL;
    }
    
    return skel->joints + index;
}

short Skel_FindAttachPointIndex(const Skel* skel, const char* name)
{
    for (int i = 0; i < skel->attachPointCount; ++i)
    {
        if (strcmp(skel->attachPoints[i].name, name) == 0)
        {
            return i;
        }
    }
    
    return -1;
}

const SkelAttachPoint* Skel_FindAttachPoint(const Skel* skel, const char* name)
{
    int index = Skel_FindAttachPointIndex(skel, name);
    
    if (index == -1)
    {
        return NULL;
    }
    
    return skel->attachPoints + index;
}


short Skel_AddAttachPoint(Skel* skel,
                         Vec3 offset,
                         Quat rotation,
                         int joint,
                         const char* name)
{
    SkelAttachPoint* point = skel->attachPoints + skel->attachPointCount;
    
    point->offset = offset;
    point->rotation = rotation;
    point->joint = joint;
    strcpy(point->name, name);
    
    ++skel->attachPointCount;
    
    return skel->attachPointCount - 1;
}

