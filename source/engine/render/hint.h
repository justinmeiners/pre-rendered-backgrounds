
#ifndef HINT_BUFFER_H
#define HINT_BUFFER_H

#include "geo_math.h"
#include "skel.h"
#include "nav.h"

/* See Gui Buffer for design info.
 
 The hint buffer is for rendering all the lines/circles for 3D gameplay interface.*/


#define HINT_VERTS_MAX 4096

typedef struct
{
    Vec3 pos;
    Vec3 color;
} HintVert;

typedef struct
{
    unsigned int vaoGpuId;
    unsigned int vboGpuId;
    unsigned int iboGpuId;
    
    unsigned short indicies[HINT_VERTS_MAX];
    HintVert verts[HINT_VERTS_MAX];
    
    unsigned short indexCount;
    unsigned short vertCount;
    
} HintBuffer;

extern void HintBuffer_Init(HintBuffer* buffer);
extern void HintBuffer_Clear(HintBuffer* buffer);

extern void HintBuffer_PackCircle(HintBuffer* buffer,
                                  float radius,
                                  Vec3 position,
                                  Vec3 color,
                                  int slices);

extern void HintBuffer_PackLine(HintBuffer* buffer,
                                Vec3 start,
                                Vec3 end,
                                Vec3 color);

extern void HintBuffer_PackAABB(HintBuffer* buffer,
                                AABB aabb,
                                Vec3 color);


extern void Hintbuffer_PackSkel(HintBuffer* buffer,
                                const Skel* skel,
                                Vec3 position,
                                Vec3 boneColor,
                                Vec3 upColor);

extern void HintBuffer_PackPath(HintBuffer* buffer,
                                const NavPath* path,
                                Vec3 color);

extern void HintBuffer_PackNav(HintBuffer* buffer,
                               const NavMesh* mesh,
                               Vec3 color);

#endif
