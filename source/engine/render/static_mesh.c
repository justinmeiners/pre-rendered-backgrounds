
#include "static_mesh.h"
#include <assert.h>

int StaticMesh_Init(StaticMesh* mesh, int vertCount, short uvChannelCount)
{
    if (!mesh)
        return 0;
    
    mesh->vaoGpuId = 0;
    mesh->vboGpuId = 0;

    mesh->vertCount = vertCount;
    mesh->uvChannelCount = uvChannelCount;
    
    if (uvChannelCount > STATIC_MESH_UVS)
        return 0;
    
    if (vertCount > 0)
    {
        mesh->verts = calloc(sizeof(StaticMeshVert), mesh->vertCount);
        if (!mesh->verts)
            return 0;
    }

    mesh->purgeable = 1;
    
    return 1;
}

int StaticMesh_Copy(StaticMesh* dest, const StaticMesh* source)
{
    if (!dest || !source)
        return 0;
    
    dest->vaoGpuId = source->vaoGpuId;
    dest->vboGpuId = source->vboGpuId;
    
    dest->purgeable = source->purgeable;
    
    dest->vertCount = source->vertCount;
    dest->uvChannelCount = source->uvChannelCount;
    
    if (source->verts)
    {
        dest->verts = calloc(sizeof(StaticMeshVert), dest->vertCount);
        if (!dest->verts)
            return 0;
        
        memcpy(dest->verts, source->verts, sizeof(StaticMeshVert));
    }
    
    return 1;
}

void StaticMesh_Shutdown(StaticMesh* mesh)
{
    if (!mesh)
        return;
    
    StaticMesh_Purge(mesh);
}

void StaticMesh_Purge(StaticMesh* mesh)
{
    if (mesh->verts)
    {
        free(mesh->verts);
        mesh->verts = NULL;
    }
}



