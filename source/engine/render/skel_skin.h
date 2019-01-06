
#ifndef SKEL_SKIN_H
#define SKEL_SKIN_H

#include "vec_math.h"
#include "skel.h"


#define SKEL_WEIGHTS_PER_VERT 3

typedef struct
{
    unsigned short u;
    unsigned short v;
} SkelSkinVertUv;

typedef struct
{
    Vec3 normal;
    Vec3 tangent;
    SkelSkinVertUv uv;
    
    /* xyz - position relative to joint (vector from joint head), w - influence */
    Vec4 weights[SKEL_WEIGHTS_PER_VERT];
    
    /* must go last due to alignment */
    unsigned short weightJoints[SKEL_WEIGHTS_PER_VERT];
    unsigned short padding;
} SkelSkinVert;


/*
 SkelSkin transform is computationaly expensive on the CPU
 because it requires quaternion transforms for each vertex and normal for each bone
 it is influence by every frame. Fortunately the process is now
 offloaded to a vertex shader and computed in parallel. Bone transformation
 should still be done on the CPU to access bone states.
 */

typedef struct
{
    /* For Renderer */
    unsigned int vaoGpuId;
    unsigned int vboGpuId;

    SkelSkinVert* verts;
    
    unsigned int vertCount;
    unsigned int weightCount;
    
    int purgeable;
    
} SkelSkin;

/* certain use cases may not require normals, disabling saves computation and memory */
extern int SkelSkin_Init(SkelSkin* skin, unsigned int vertCount, unsigned int weightCount);
extern int SkelSkin_Copy(SkelSkin* dest, const SkelSkin* source);

extern void SkelSkin_Shutdown(SkelSkin* skin);

extern void SkelSkin_Purge(SkelSkin* skin);


#endif
