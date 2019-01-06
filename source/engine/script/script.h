#ifndef SCRIPT_H
#define SCRIPT_H

#include <stdio.h>

#define SCRIPT_COMMAND_ARGS_MAX 10
#define SCRIPT_ID_NAME_MAX 32
#define SCRIPT_EVENT_MAX 128
#define SCRIPT_COUNT 6

typedef enum
{
    kScriptArgId = 0,
    kScriptArgInt,
    kScriptArgString
} ScriptArgType;

typedef struct
{
    ScriptArgType type;
    int intValue;
    const char* stringValue;
    size_t stringLength;
} ScriptArg;

extern void ScriptArg_CopyString(const ScriptArg* arg, char* dest);

typedef enum
{
    kScriptStateHalt = 0,
    kScriptStateRun,
    kScriptStateYield,
} ScriptState;


/* this structure is used to define custom commands,
   a table of ScriptCommands is necessary to init
   the script interpreter. */
typedef struct
{
    const char* name;
    ScriptState (*execute)(ScriptArg* args, int argCount, void* context);
    int argCount;
    int argTypes[SCRIPT_COMMAND_ARGS_MAX];
} ScriptCommand;

typedef struct
{
    char name[SCRIPT_ID_NAME_MAX];
    int lineNumber;
    size_t position;
} ScriptEvent;

/* the event list is sorted for fast searching
   each script stores its own table of events
   so that scripts can be loaded/unloaded
    without requiring a resort
*/

typedef struct
{
    const char* source;
    int eventCount;
    ScriptEvent events[SCRIPT_EVENT_MAX];
} Script;

extern int Script_FromPath(Script* script, const char* path);
extern void Script_Shutdown(Script* script);

typedef struct
{
    ScriptState state;
    int scriptCount;
    Script scripts[SCRIPT_COUNT];

    int scriptIndex;
    size_t pc;
    int eventLineNumber;

    const ScriptCommand* commandTable;
    int commandCount;

    void* context;
} ScriptInterpreter;


extern int ScriptInterpreter_Init(ScriptInterpreter* interpreter, const ScriptCommand* commandTable, int commandCount);

extern void ScriptInterpreter_Shutdown(ScriptInterpreter* interpreter);

/* seeks to the event with the name provided. 
   this function will search all loaded scripts and return
   whether the even was found. */
extern int ScriptInterpreter_Seek(ScriptInterpreter* interpreter, const char* eventName);

/* moves the script from a halted state to a running state 
 this function will run commands, blocking until a command halts or yields */
extern void ScriptInterpreter_Continue(ScriptInterpreter* interpreter);

/* script execution
  this function will run commands, blocking until a command halts or yields */
extern void ScriptInterpreter_Run(ScriptInterpreter* interpreter);

#endif
