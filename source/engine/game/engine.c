
#include "engine.h"
#include <time.h>
#include <string.h>

Engine g_engine;

// https://learnopengl.com/#!Advanced-OpenGL/Depth-testing
// http://www.songho.ca/opengl/gl_projectionmatrix.html
// http://www.songho.ca/opengl/gl_transform.html

int Engine_Init(Engine* engine,
                Renderer* renderer,
                SndDriver* sndDriver,
                EngineSettings engineSettings)
{
#if __APPLE__
    if (!renderer || !sndDriver)
        return 0;
#else
	if (!renderer)
        return 0;
#endif // __APPLE__
    SndSystem_Init(&engine->soundSystem, sndDriver);
    NavSystem_Init(&engine->navSystem);
    PartSystem_Init(&engine->partSystem);
    ScriptSystem_Init(&engine->scriptSystem);
    GuiSystem_Init(&engine->guiSystem, engineSettings.guiWidth, engineSettings.guiHeight);
    InputSystem_Init(&engine->inputSystem, engineSettings.inputConfig);
    RenderSystem_Init(&engine->renderSystem, engine, renderer, engineSettings.renderWidth, engineSettings.renderHeight);
    engine->renderSystem.scaleFactor = engineSettings.renderScaleFactor;
    engine->renderSystem.renderer->flushLoad(engine->renderSystem.renderer);
    
    engine->renderSystem.renderer->prepareBgBuffer(renderer, &engine->renderSystem.bgBuffer);
    
    engine->controlEnabled = 1;
    
    Engine_LoadAssets(engine);
    Engine_LoadScene(engine, "scenes/quarters");
    
    Engine_LoadView(engine, "hallway");
        
    return 1;
}

void Engine_LoadScene(Engine* engine, const char* pathToFolder)
{
    strncpy(engine->sceneFolder, pathToFolder, MAX_OS_PATH);
    
    char fullFolderPath[MAX_OS_PATH];
    Filepath_Append(fullFolderPath, Filepath_DataPath(), engine->sceneFolder);
    
    char navMeshPath[MAX_OS_PATH];
    Filepath_Append(navMeshPath, engine->sceneFolder, "level.nav");
    if (!NavSystem_LoadNavMesh(&engine->navSystem, navMeshPath))
    {
        printf("no nav mesh: %s\n", navMeshPath);
    }
    
    char scriptPath[MAX_OS_PATH];
    Filepath_Append(scriptPath, engine->sceneFolder, "level.script");
    if (!ScriptSystem_LoadScript(&engine->scriptSystem, SCRIPT_LEVEL, scriptPath))
    {
        printf("no script: %s\n", scriptPath);
    }
    
    char manifestPath[MAX_OS_PATH];
    Filepath_Append(manifestPath, fullFolderPath, "level.manifest");
    SceneSystem_Load(&engine->sceneSystem, manifestPath);
}

int Engine_LoadView(Engine* engine, const char* viewName)
{
    clock_t startTime = clock();
    SceneView* view = SceneSystem_FindView(&engine->sceneSystem, viewName);
    if (engine->currentView == view)
        return 0;
    
    engine->currentView = view;

    char filename[MAX_OS_PATH];
    sprintf(filename, "%s.png", viewName);
    char fullpath[MAX_OS_PATH];
    Filepath_Append(fullpath, engine->sceneFolder, filename);
    
    RenderSystem_LoadTexture(&engine->renderSystem, TEX_VIEW_BG, kTextureFlagNone, fullpath);
    
    sprintf(filename, "%s_depth.png", viewName);
    Filepath_Append(fullpath, engine->sceneFolder, filename);
    
    RenderSystem_LoadTexture(&engine->renderSystem, TEX_VIEW_BG_DEPTH, kTextureFlagNone, fullpath);
    
    engine->renderSystem.cam.near = view->near;
    engine->renderSystem.cam.far = view->far;
    engine->renderSystem.cam.fov = view->fov;
    engine->renderSystem.cam.position = view->position;
    
    Vec3 dir = Quat_MultVec3(&view->rotation, Vec3_Create(0.0f, 0.0f, -1.0f));
    engine->renderSystem.cam.target = Vec3_Add(view->position, dir);
    engine->renderSystem.cam.orientation = Vec3_Create(0.0f, 0.0f, 1.0f);
    
    engine->sceneSystem.activeLights[0] = SceneSystem_FindLight(&engine->sceneSystem, view->primaryLight);
    engine->sceneSystem.activeLights[1] = SceneSystem_FindLight(&engine->sceneSystem, view->secondaryLight);
    
    for (int i = 0; i < SCENE_ACTORS_MAX; ++i)
    {
        Actor* actor = engine->sceneSystem.actors + i;
        if (actor->dead) continue;
        
        if (actor->type == kActorViewTrigger)
        {
            if (strncmp(actor->viewName, viewName, SCENE_ELEMENT_NAME_MAX) == 0)
            {
                actor->tapEnabled = 0;
            }
            else
            {
                actor->tapEnabled = 1;
            }
        }
    }
    
    ScriptSystem_RunEvent(&engine->scriptSystem, viewName);
    
    sprintf(filename, "%s_cube.dds", viewName);
    Filepath_Append(fullpath, engine->sceneFolder, filename);
    RenderSystem_LoadTexture(&engine->renderSystem, TEX_VIEW_CUBE, 0, fullpath);

    clock_t endTime = clock();
    printf("view time: %f\n", (endTime - startTime) / (float)CLOCKS_PER_SEC);

    return 1;
}

static void Engine_RecieveInput(Engine* engine, const InputState* state, const InputState* last, InputInfo* info)
{
    if (!engine->controlEnabled)
        return;
    
    if (state->mouseButtons[kMouseButtonLeft].phase == kInputPhaseBegin && !info->mouseButtons[kMouseButtonLeft].handled)
    {
        Ray3 mouseRay;
        mouseRay.origin = engine->renderSystem.cam.position;
        
        Vec3 p2 = Frustum_Unproject(&engine->renderSystem.cam, Vec3_Create(state->mouseCursor.x, state->mouseCursor.y, 0.999f));
        mouseRay.dir = Vec3_Norm(Vec3_Sub(p2, engine->renderSystem.cam.position));
        
        
        // check for mouse click objects first
        Actor* bestActor = NULL;
        float bestDistance = 0.0f;
        
        for (int i = 0; i < SCENE_ACTORS_MAX; ++i)
        {
            Actor* actor = engine->sceneSystem.actors + i;
            if (actor->dead) { continue; }
        
            if (actor->onTap && actor->tapEnabled)
            {
                float t;
                if (AABB_IntersectsRay(actor->bounds, mouseRay, &t))
                {
                    if (t < bestDistance || !bestActor)
                    {
                        bestActor = actor;
                        bestDistance = t;
                    }
                    
                }
            }
        }
        
        if (bestActor)
        {
            bestActor->onTap(bestActor, mouseRay);
        }
        else
        {
            NavRaycastResult hitInfo;
            if (NavSystem_Raycast(&engine->navSystem, mouseRay, &hitInfo))
            {
                Actor* actor = SceneSystem_Player(&g_engine.sceneSystem);
                Actor_StartPath(actor, hitInfo.point);
            }
        }
    
        
        info->mouseButtons[kMouseButtonLeft].handled = 1;
    }
}

void Engine_Update(Engine* engine, const InputState* inputState)
{
    InputSystem_ProcessInput(&engine->inputSystem, inputState);
    
    HintBuffer_Clear(&engine->renderSystem.hintBuffer);

    const InputState* currentInput = &engine->inputSystem.current;
    const InputState* lastInput = &engine->inputSystem.last;
    
    Engine_RecieveInput(engine, currentInput, lastInput, &engine->inputSystem.info);

    /* update entities */
    for (int i = 0; i < SCENE_ACTORS_MAX; ++i)
    {
        Actor* actor = engine->sceneSystem.actors + i;
        if (actor->dead) { continue; }
        
        Mat4 rot;
        Mat4 translate = Mat4_CreateTranslate(actor->position);
        Quat_ToMatrix(actor->rotation, &rot);
        Mat4_Mult(&translate, &rot, &actor->worldMatrix);

        if (actor->onUpdate)
        {
            actor->onUpdate(actor);
        }
        
        for (int j = i + 1; j < SCENE_ACTORS_MAX; ++j)
        {
            Actor* other = engine->sceneSystem.actors + j;
            
            if (other->dead)
                continue;
            
            if (AABB_IntersectsAABB(actor->bounds, other->bounds))
            {
                if (actor->onTouch)
                    actor->onTouch(actor, other);
                
                if (other->onTouch)
                    other->onTouch(other, actor);
            }
        }
        
        if (actor->onPath)
            HintBuffer_PackPath(&engine->renderSystem.hintBuffer, &actor->path, Vec3_Create(0.0f, 0.0f, 1.0f));
        
        HintBuffer_PackAABB(&engine->renderSystem.hintBuffer, actor->bounds, Vec3_Create(0.7f, 0.7f, 0.7f));
    }
    
    HintBuffer_PackNav(&engine->renderSystem.hintBuffer, &engine->navSystem.navMesh, Vec3_Create(0.0f, 1.0f, 0.0f));
    ScriptSystem_Update(&engine->scriptSystem);
}

void Engine_Render(Engine* engine)
{
    Frustum_UpdateTransform(&engine->renderSystem.cam, engine->renderSystem.viewportWidth, engine->renderSystem.viewportHeight);
    RenderSystem_Render(&engine->renderSystem, &engine->renderSystem.cam, engine);
}


