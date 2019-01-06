
#ifndef RENDERER_H
#define RENDERER_H

#include "texture.h"
#include "geo_math.h"
#include "static_model.h"
#include "skel_model.h"
#include "gui_buffer.h"
#include "hint.h"
#include "scene_system.h"

#define RENDER_SYSTEM_LIGHTS_MAX 32

/*
 
 GL_POINTS,
 GL_LINE_STRIP,
 GL_LINE_LOOP,
 GL_LINES,
 GL_TRIANGLE_STRIP,
 GL_TRIANGLE_FAN, and
 GL_TRIANGLES
 */


struct RenderSystem;
struct Engine;


/* Render list allows culling, preprocessing, and sorting before rendering. */

typedef struct
{
    int skelActors[SCENE_ACTORS_MAX];
    int skelActorCount;
    
    int staticActors[SCENE_ACTORS_MAX];
    int staticActorCount;
    
} RenderList;

typedef struct
{
    unsigned short maxTextureSize;
    unsigned short maxSkelJoints;
} RendererLimits;

typedef struct
{
    unsigned int vaoGpuId;
    unsigned int vboGpuId;
    unsigned int iboGpuId;
    
} BgBuffer;


/* 
 The renderer completely abstracts rendering details from the rest of the game.
 This allows support of multiple OpenGL versions and ideally alternative graphics APIs.
 
 The Renderer needs to be aware of several systems including scene, gui, and particle data.
 */



typedef struct Renderer
{
    int (*init)(struct Renderer* renderer);
    void (*shutdown)(struct Renderer* renderer);
    
    void (*render)(struct Renderer* renderer,
                   const Frustum* cam,
                   const struct Engine* engine,
                   const RenderList* renderList);
    
    int (*uploadTexture)(struct Renderer* renderer, Texture* texture);

    int (*uploadMesh)(struct Renderer* renderer, StaticMesh* mesh);
    int (*uploadSkelSkin)(struct Renderer* renderer, SkelSkin* skin);
    
    int (*cleanupTexture)(struct Renderer* renderer, Texture* texture);
    int (*cleanupMesh)(struct Renderer* renderer, StaticMesh* mesh);
    int (*cleanupSkelSkin)(struct Renderer* renderer, SkelSkin* skin);

    int (*prepareGuiBuffer)(struct Renderer* renderer, GuiBuffer* buffer);
    int (*cleanupGuiBuffer)(struct Renderer* renderer, GuiBuffer* buffer);
    
    int (*prepareHintBuffer)(struct Renderer* renderer, HintBuffer* buffer);
    int (*cleanupHintBuffer)(struct Renderer* renderer, HintBuffer* buffer);

    int (*prepareBgBuffer)(struct Renderer* renderer, BgBuffer* buffer);

    void (*flushLoad)(struct Renderer* renderer);
    
    RendererLimits limits;

    int debug;
    void* context;
    
} Renderer;

#endif
