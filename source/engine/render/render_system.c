
#include "render_system.h"
#include "utils.h"
#include "engine.h"

int RenderSystem_Init(RenderSystem* system,
                      struct Engine* engine,
                      Renderer* renderer,
                      short renderWidth,
                      short renderHeight)
{
    if (system && renderer)
    {
        Frustum_Init(&system->cam, 78.0f, 5, 25.0f);
        system->cam.orientation = Vec3_Create(0.0f, 0.0f, 1.0f);
        
        system->viewportWidth = renderWidth;
        system->viewportHeight = renderHeight;
                
        system->scaleFactor = 1.0f;
                
        system->renderer = renderer;
        system->renderer->init(system->renderer);
        
        system->renderer->prepareHintBuffer(renderer, &engine->renderSystem.hintBuffer);
        system->renderer->prepareGuiBuffer(renderer, &engine->guiSystem.buffer);
        
        return 1;
    }
    
    return 0;
}

void RenderSystem_Shutdown(RenderSystem* system, struct Engine* engine)
{
    system->renderer->shutdown(system->renderer);
    system->renderer->cleanupGuiBuffer(system->renderer, &engine->guiSystem.buffer);
}


int RenderSystem_LoadTexture(RenderSystem* system, int texture, TextureFlags flags, const char* path)
{
    Texture* tex = system->textures + texture;
    
    if (path == NULL)
    {
        system->renderer->cleanupTexture(system->renderer, tex);
        Texture_Shutdown(tex);
        return 0;
    }
    
    char fullPath[MAX_OS_PATH];
    Filepath_Append(fullPath, Filepath_DataPath(), path);
    
    if (Texture_FromPath(tex, flags, fullPath))
    {
        if (tex->width > system->renderer->limits.maxTextureSize)
        {
            printf("invalid texture size\n");
            return 0;
        }
        
        return system->renderer->uploadTexture(system->renderer, tex);
    }
    else
    {
        return 0;
    }
}

int RenderSystem_LoadStaticModel(RenderSystem* system, int modelIndex, const char* path)
{
    StaticModel* model = system->models + modelIndex;

    if (path == NULL)
    {
        system->renderer->cleanupMesh(system->renderer, &model->mesh);
        StaticModel_Shutdown(model);
        return 0;
    }
    
    char fullPath[MAX_OS_PATH];
    Filepath_Append(fullPath, Filepath_DataPath(), path);
    
    if (StaticModel_FromPath(model, fullPath))
    {
        return system->renderer->uploadMesh(system->renderer, &model->mesh);
    }
    else
    {
        return 0;
    }
}

int RenderSystem_LoadAnim(RenderSystem* system, int animIndex, const char* path)
{
    SkelAnim* anim = system->anims + animIndex;
    
    if (path == NULL)
    {
        SkelAnim_Shutdown(anim);
        return 0;
    }
    
    char fullPath[MAX_OS_PATH];
    Filepath_Append(fullPath, Filepath_DataPath(), path);
    
    if (SkelAnim_FromPath(anim, fullPath))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int RenderSystem_LoadSkelModel(RenderSystem* system, int modelIndex, const char* path)
{
    SkelModel* model = system->skelModels + modelIndex;
    
    if (path == NULL)
    {
        system->renderer->cleanupSkelSkin(system->renderer, &model->skin);
        SkelModel_Shutdown(model);
        return 0;
    }
    
    char fullPath[MAX_OS_PATH];
    Filepath_Append(fullPath, Filepath_DataPath(), path);
    
    if (SkelModel_FromPath(model, fullPath))
    {
        if (model->skel.jointCount > system->renderer->limits.maxSkelJoints)
        {
            printf("invalid skeleton size\n");
            return 0;
        }
        
        return system->renderer->uploadSkelSkin(system->renderer, &model->skin);
    }
    else
    {
        return 0;
    }
}

static void RenderSystem_Cull(Renderer* renderer,
                              const Frustum* cam,
                              const Engine* engine,
                              RenderList* renderList)
{
    for (int i = 0; i < SCENE_ACTORS_MAX; ++i)
    {
        const Actor* actor = engine->sceneSystem.actors + i;
        
        if (actor->dead) continue;
        
        if (actor->renderType == kActorRenderStaticModel)
        {
            renderList->staticActors[renderList->staticActorCount] = i;
            ++renderList->staticActorCount;
        }
        else if (actor->renderType == kActorRenderSkelModel)
        {
            renderList->skelActors[renderList->skelActorCount] = i;
            ++renderList->skelActorCount;
        }
    }
}

void RenderSystem_Render(RenderSystem* system,
                         const Frustum* cam,
                         const struct Engine* engine)
{
    RenderList list;
    list.skelActorCount = 0;
    list.staticActorCount = 0;
    
    RenderSystem_Cull(system->renderer, cam, engine, &list);
    
    system->renderer->render(system->renderer, cam, engine, &list);
}
