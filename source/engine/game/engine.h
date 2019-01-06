
#ifndef ENGINE_H
#define ENGINE_H

#include "render_system.h"
#include "part_system.h"
#include "input_system.h"
#include "nav_system.h"
#include "gui_system.h"
#include "snd_system.h"
#include "scene_system.h"
#include "script_system.h"
#include "data_assets.h"

#include "utils.h"


typedef struct
{
    InputConfig inputConfig;
    short guiWidth;
    short guiHeight;
    short renderWidth;
    short renderHeight;
    float renderScaleFactor;
} EngineSettings;


typedef struct Engine
{
    RenderSystem renderSystem;
    PartSystem partSystem;
    NavSystem navSystem;
    InputSystem inputSystem;
    GuiSystem guiSystem;
    SndSystem soundSystem;
    ScriptSystem scriptSystem;
    
    SceneSystem sceneSystem;
    SceneView* currentView;
    
    int controlEnabled;

    char sceneFolder[MAX_OS_PATH];    
} Engine;

extern Engine g_engine;

extern int Engine_Init(Engine* engine,
                       Renderer* renderer,
                       SndDriver* sndDriver,
                       EngineSettings engineSettings);

extern void Engine_LoadScene(Engine* engine, const char* pathToFolder);
extern int Engine_LoadView(Engine* engine, const char* viewName);

extern void Engine_LoadAssets(Engine* engine);
extern void Engine_UnloadAssets(Engine* engine);

extern void Engine_Update(Engine* engine, const InputState* inputState);
extern void Engine_Render(Engine* engine);


extern int Engine_FindAsset(const AssetEntry* manifest, size_t count, const char* name);

#endif /* engine_h */
