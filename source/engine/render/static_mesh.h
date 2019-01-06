
#ifndef MESH_H
#define MESH_H

#include "vec_math.h"
#include <stdlib.h>

/* 0 is color map uvs, 1 is lightmap */
#define STATIC_MESH_UVS 2


/*
 USHRT_MAX is 65535 which is much larger than any reasonable texture dimension.
 Therefore using an unsigned short to store texture coordinates should result in absolutely no quality loss,
 while reducing memory and bandwidth.
 */

typedef struct
{
    unsigned short u;
    unsigned short v;
} StaticMeshVertUv;

typedef struct
{
    Vec3 pos;
    Vec3 normal;
    
    StaticMeshVertUv uv[STATIC_MESH_UVS];
} StaticMeshVert;


typedef struct
{
    /* For Renderer */
    unsigned int vaoGpuId;
    unsigned int vboGpuId;
    
    unsigned int vertCount;
    unsigned short uvChannelCount;
    
    StaticMeshVert* verts;
    
    /* This flag allows the texture's RAM to be deleted after uploading to VRAM. Defaults to true if static, false if dynamic. */
    int purgeable;
    
} StaticMesh;

extern int StaticMesh_Init(StaticMesh* mesh, int vertCount, short uvChannelCount);
extern int StaticMesh_Copy(StaticMesh* dest, const StaticMesh* source);

extern void StaticMesh_Shutdown(StaticMesh* mesh);

extern void StaticMesh_Purge(StaticMesh* mesh);

#endif
