
#include "script_system.h"
#include "utils.h"
#include <string.h>

#include "engine.h"



static ScriptState ScriptCommand_Log(ScriptArg* args, int argCount, void* context)
{
    printf("%.*s\n", (int)args[0].stringLength, args[0].stringValue);
    return kScriptStateRun;
}

static ScriptState ScriptCommand_Jmp(ScriptArg* args, int argCount, void* context)
{
    char eventId[SCRIPT_ID_NAME_MAX];
    strncpy(eventId, args[0].stringValue, args[0].stringLength);
    
    ScriptSystem* system = context;
    ScriptInterpreter_Seek(&system->interpreter, eventId);
    return kScriptStateRun;
}

static ScriptState ScriptCommand_End(ScriptArg* args, int argCount, void* context)
{
    return kScriptStateHalt;
}

static ScriptState ScriptCommand_View(ScriptArg* args, int argCount, void* context)
{
    char viewName[SCRIPT_ID_NAME_MAX];
    ScriptArg_CopyString(args + 0, viewName);
    
    //ScriptSystem* system = context;
    
    if (Engine_LoadView(&g_engine, viewName))
        return kScriptStateRun;
    else
        return kScriptStateHalt;
}

static ScriptState ScriptCommand_Amb(ScriptArg* args, int argCount, void* context)
{
    char soundName[SCRIPT_ID_NAME_MAX];
    ScriptArg_CopyString(args + 0, soundName);

    int asset = Engine_FindAsset(Asset_soundManifest, Asset_soundCount, soundName);
    
    if (asset != -1)
    {
        SndSystem_SetAmbient(&g_engine.soundSystem, asset);
    }

    return kScriptStateRun;
}

static ScriptState ScriptCommand_Snd(ScriptArg* args, int argCount, void* context)
{
    char soundName[SCRIPT_ID_NAME_MAX];
    ScriptArg_CopyString(args + 0, soundName);
    
    int asset = Engine_FindAsset(Asset_soundManifest, Asset_soundCount, soundName);
    
    if (asset != -1)
    {
        SndSystem_PlaySound(&g_engine.soundSystem, asset);
    }
    
    return kScriptStateRun;
}

static ScriptState ScriptCommand_Kill(ScriptArg* args, int argCount, void* context)
{
    char actorName[SCRIPT_ID_NAME_MAX];
    ScriptArg_CopyString(args + 0, actorName);
    
    Actor* actor = SceneSystem_FindActor(&g_engine.sceneSystem, actorName);
    
    if (actor)
    {
        SceneSystem_Kill(&g_engine.sceneSystem, actor);
    }
    else
    {
        printf("missing actor: %s\n", actorName);
    }
    
    return kScriptStateRun;
}

static ScriptState ScriptCommand_Path(ScriptArg* args, int argCount, void* context)
{
    char actorName[SCRIPT_ID_NAME_MAX];
    ScriptArg_CopyString(args + 0, actorName);
    
    Actor* waypoint = SceneSystem_FindActor(&g_engine.sceneSystem, actorName);
    
    if (waypoint)
    {
        Actor* player = SceneSystem_Player(&g_engine.sceneSystem);
        Actor_StartPath(player, waypoint->position);
    }
    
    return kScriptStateRun;
}

static ScriptState ScriptCommand_WaitPath(ScriptArg* args, int argCount, void* context)
{
    Actor* player = SceneSystem_Player(&g_engine.sceneSystem);

    if (player->onPath)
    {
        ScriptSystem* system = context;
        system->yieldPath = 1;
        return kScriptStateYield;
    }
    
    return kScriptStateRun;
}

static ScriptState ScriptCommand_Control(ScriptArg* args, int argCount, void* context)
{
    g_engine.controlEnabled = args[0].intValue;
    return kScriptStateRun;
}

static const ScriptCommand commands[] = {
    {"LOG", ScriptCommand_Log, 1, {kScriptArgString, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"JMP", ScriptCommand_Jmp, 1, {kScriptArgId, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"END", ScriptCommand_End, 0, {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"VIEW", ScriptCommand_View, 1, {kScriptArgString, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"AMB", ScriptCommand_Amb, 1, {kScriptArgString, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"SND", ScriptCommand_Snd, 1, {kScriptArgString, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"KILL", ScriptCommand_Kill, 1, {kScriptArgString, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"PATH", ScriptCommand_Path, 1, {kScriptArgString, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"WPATH", ScriptCommand_WaitPath, 0, {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {"CTL", ScriptCommand_Control, 1, {kScriptArgInt, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
};

void ScriptSystem_Init(ScriptSystem* system)
{
    ScriptInterpreter_Init(&system->interpreter, commands, sizeof(commands) / sizeof(ScriptCommand));
    system->interpreter.context = system;
}

int ScriptSystem_LoadScript(ScriptSystem* system, int scriptIndex, const char* path)
{
    Script* script = system->interpreter.scripts + scriptIndex;
    
    if (path == NULL)
    {
        Script_Shutdown(script);
        return 0;
    }
    
    char fullPath[MAX_OS_PATH];
    Filepath_Append(fullPath, Filepath_DataPath(), path);
    
    if (Script_FromPath(script, fullPath))
    {
        return 1;
    }
    
    return 0;

}

int ScriptSystem_RunEvent(ScriptSystem* system, const char* eventName)
{
    if (ScriptInterpreter_Seek(&system->interpreter, eventName))
    {
        ScriptInterpreter_Continue(&system->interpreter);
        return 1;
    }

    return 0;
}


void ScriptSystem_Update(ScriptSystem* system)
{
    if (system->yieldPath)
    {
        Actor* player = SceneSystem_Player(&g_engine.sceneSystem);

        if (!player->onPath)
        {
            system->interpreter.state = kScriptStateRun;
            system->yieldPath = 0;
        }
    }
    
    ScriptInterpreter_Run(&system->interpreter);
}

