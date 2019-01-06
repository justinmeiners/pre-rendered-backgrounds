
#ifndef SKEL_H
#define SKEL_H

#include "vec_math.h"

#define SKEL_JOINT_NAME_MAX 64
#define SKEL_ATTACH_POINTS_MAX 8

/* 
 In a skeleton file, only joint heads and tails are stored. This means that whatever the pose matrix is, is considered the identity.
 Bone rotations from the 3d editor to not come into play. All animations store rotations relative to pose positions.
 */

/* joint transform in local space and persistent joint data */
typedef struct
{
    char name[SKEL_JOINT_NAME_MAX];
    short parent;
    
    /* joint info in local space */
    Quat rotation;
    Vec3 tail; /* relative to head */
    
    /* joint pose in model space */
    Quat modelRotation;
    Vec3 modelHead;
    Vec3 modelTail;
} SkelJoint;


typedef struct
{
    char name[SKEL_JOINT_NAME_MAX];

    short joint;
    Vec3 offset;
    Quat rotation; /* offset rotation from bone rotation */
    
    /* attach point position in model space */
    Vec3 modelPosition;
    Quat modelRotation;
} SkelAttachPoint;



/*
Skel stores the current pose in local space, and the current pose in world space
this allows engine code to manipulate joint transformations in local space and apply
transformations directly to vertices from the world space skeleton.
*/

typedef struct
{
    SkelJoint* joints;
    /* data for GPU skinning */
    Quat* renderJointRotations;
    Vec3* renderJointOrigins;
    
    /* attach points for attaching weapons etc, offset from bones */
    SkelAttachPoint attachPoints[SKEL_ATTACH_POINTS_MAX];
    
    /* position of root bone head relative to skeleton */
    Vec3 origin;
    
    /* A vector offset from root bone position for animation
    The relative vector, rather than absolute position is helpful to allow the skeleton to be updated without changing animations
     */
    Vec3 offset;
    
    /* local rotation. This handled in the skeleton, rather than render matrix for attachments and collisions */
    Quat rotation;
    
    /* number of joints in skeleton */
    short jointCount;
    short attachPointCount;
} Skel;

extern int Skel_Init(Skel* skel, short jointCount, short attachPointCount);
extern int Skel_Copy(Skel* dest, const Skel* source);
extern void Skel_Shutdown(Skel* skel);

extern short Skel_FindJointIndex(const Skel* skel, const char* name);
extern const SkelJoint* Skel_FindJoint(const Skel* skel, const char* name);

extern short Skel_FindAttachPointIndex(const Skel* skel, const char* name);
extern const SkelAttachPoint* Skel_FindAttachPoint(const Skel* skel, const char* name);

extern short Skel_AddAttachPoint(Skel* skel,
                                 Vec3 offset,
                                 Quat rotation,
                                 int joint,
                                 const char* name);

/* updates world joints from local */
extern void Skel_Pose(Skel* skel);

#endif
