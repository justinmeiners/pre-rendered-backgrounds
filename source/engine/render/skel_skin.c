
#include "skel_skin.h"
#include <assert.h>
#include <stdlib.h>

int SkelSkin_Init(SkelSkin* skin, unsigned int vertCount, unsigned int weightCount)
{
    if (!skin)
        return 0;
    
    skin->verts = NULL;
    skin->weightCount = weightCount;
    skin->vertCount = vertCount;
    
    if (skin->vertCount > 0)
    {
        skin->verts = calloc(sizeof(SkelSkinVert), vertCount);
        if (!skin->verts)
            return 0;
    }
    
    skin->vaoGpuId = 0;
    skin->vboGpuId = 0;
    
    skin->purgeable = 1;
    
    return 1;

}

int SkelSkin_Copy(SkelSkin* dest, const SkelSkin* source)
{
    assert(dest);
    assert(source);
    
    dest->vaoGpuId = source->vaoGpuId;
    dest->vboGpuId = source->vboGpuId;
    
    dest->purgeable = source->purgeable;
    
    dest->vertCount = source->vertCount;
    dest->weightCount = source->weightCount;
    
    if (source->verts)
    {
        dest->verts = calloc(source->vertCount, sizeof(SkelSkinVert));
        memcpy(dest->verts, source->verts, sizeof(SkelSkinVert) * source->vertCount);
    }
    
    return 1;
}

void SkelSkin_Shutdown(SkelSkin* skin)
{
    if (skin)
        SkelSkin_Purge(skin);
}

void SkelSkin_Purge(SkelSkin* skin)
{
    if (skin->verts)
    {
        free(skin->verts);
        skin->verts = NULL;
    }
}



