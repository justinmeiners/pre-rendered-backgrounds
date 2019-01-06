

#ifndef GUI_BUFFER_H
#define GUI_BUFFER_H

#include "vec_math.h"

/*
 The process of constructing vertices for the GUI used to be in the renderer,
 however, it started to make more sense to have it separate to reduce the amount
 of code in the renderer, and for a separation of concerns. I figure that both the 
 mesh and skel skin are already designed specifically for rendering, so the gui buffer
 is analagous to those data structure.
 
 */

/* The gui rendering uses multitexturing to batch together all gui drawing to a single draw call */ 

#define GUI_VERTS_MAX 512


typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} GuiVertColor;

typedef struct
{
    unsigned short u;
    unsigned short v;
} GuiVertUv;

typedef struct
{
    Vec2 pos;
    GuiVertUv uv;
    
    GuiVertColor color;
    /* indicates which gui texture to use - must be last due to alignment */
    char atlasId;
} GuiVert;

typedef struct
{
    unsigned int vaoGpuId;
    unsigned int vboGpuId;
    unsigned int iboGpuId;
    
    unsigned short indicies[GUI_VERTS_MAX];
    GuiVert verts[GUI_VERTS_MAX];
    
    unsigned short vertCount;
    unsigned short indexCount;

} GuiBuffer;

extern void GuiBuffer_Init(GuiBuffer* buffer);

extern void GuiBuffer_Clear(GuiBuffer* buffer);


#endif
