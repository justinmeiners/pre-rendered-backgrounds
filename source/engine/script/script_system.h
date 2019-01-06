
#ifndef SCRIPT_SYSTEM_H
#define SCRIPT_SYSTEM_H

#include "script.h"


typedef struct
{
    ScriptInterpreter interpreter;

    int yieldPath;
    
} ScriptSystem;

extern void ScriptSystem_Init(ScriptSystem* system);
extern int ScriptSystem_LoadScript(ScriptSystem* system, int scriptIndex, const char* path);
extern int ScriptSystem_RunEvent(ScriptSystem* system, const char* eventName);

extern void ScriptSystem_Update(ScriptSystem* system);
#endif 
