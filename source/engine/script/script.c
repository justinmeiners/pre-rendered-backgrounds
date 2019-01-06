#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "script.h"

#define LINE_BUFFER_MAX 1024
#define INT_LENGTH_MAX 32

void ScriptArg_CopyString(const ScriptArg* arg, char* dest)
{
    memcpy(dest, arg->stringValue, sizeof(char) * arg->stringLength);
    dest[arg->stringLength] = '\0';
}

static int ScriptCommand_Compare(const void* a, const void* b)
{
    const ScriptCommand* ca = a;
    const ScriptCommand* cb  = b;
    return strncmp(ca->name, cb->name, SCRIPT_ID_NAME_MAX);
}

static int ScriptEvent_Compare(const void* a, const void *b)
{
    const ScriptEvent* ea = a;
    const ScriptEvent* eb = b;
    return strncmp(ea->name, eb->name, SCRIPT_ID_NAME_MAX);
}

static ScriptEvent* Script_Find(const Script* script, const char* eventName)
{
    if (!eventName)
        return NULL;
    // create temporary key object for search
    ScriptEvent key;
    memcpy(key.name, eventName, SCRIPT_ID_NAME_MAX); 
    // binary search the sorted events
    return bsearch(&key, script->events, script->eventCount, sizeof(ScriptEvent), ScriptEvent_Compare);
}

typedef enum
{
    kScriptTokenId = 0,
    kScriptTokenInt,
    kScriptTokenString,
    kScriptTokenComma,
    kScriptTokenUnknown,
} TokenType;

typedef struct
{
    const char* string;
    size_t length;
    char character;
    TokenType type;
} ScriptToken;

static const ScriptArgType tokenTypeToArg[] = {
    kScriptArgId,
    kScriptArgInt,
    kScriptArgString,
};

static const char* const tokenToString[] = {
    "ID",
    "INT",
    "STRING",
    "COMMA",
    "UNKNOWN",
};

static const char* Script_ReadWhite(const char* c)
{
    while (*c != '\0' && isspace(*c))
        ++c;
    return c;
}

static const char* Script_ReadWhiteOnLine(const char* c)
{
    while (*c != '\0' && *c != '\n' && *c != '\r' && isspace(*c))
        ++c;
    return c;
}

static const char* Script_ReadLine(const char* c)
{
    while (*c != '\0' && *c != '\n')
        ++c;
    if (*c != '\0')
        ++c;
    return c;
}

static const char* Script_ReadString(const char* c)
{
    ++c;
    while (*c != '\0' && *c != '"')
        ++c;
    if (*c != '0')
        ++c; 
    return c;
}

static const char* Script_ReadInt(const char* c)
{
    while (*c != '\0' && isdigit(*c))
        ++c;
    return c;
}

static const char* Script_ReadId(const char* c)
{
    while (*c != '\0' && isalnum(*c))
        ++c;
    return c;
}

static const char* Script_GetToken(const char* c, ScriptToken* dest, int stayOnLine)
{
    // skip whitespace, comments, and events until we find a token
    
    if (stayOnLine)
    {
        c = Script_ReadWhiteOnLine(c);
    }
    else
    {
        c = Script_ReadWhite(c);
    }
    
    while (*c == ';' || *c == '#')
    {
        // skip comments until the end of the line
        c = Script_ReadLine(c); 
        // read whitespace after comment
        c = Script_ReadWhite(c);
    } 

    if (*c == ',')
    {
        // read comma
        dest->type = kScriptTokenComma;
        ++c;
    }
    else if (*c == '"')
    {
        // read string
        // calculate start and end
        const char* start = c + 1;
        c = Script_ReadString(c);
        const char* end = c - 1; 
        dest->string = start;
        dest->length = end - start;
        dest->type = kScriptTokenString;
    }
    else if (isdigit(*c))
    {
        // read number
        // calculate start and end
        const char* start = c;
        c = Script_ReadInt(c);
        dest->string = start;
        dest->length = c - start;
        dest->type = kScriptTokenInt;
    }
    else if (isalpha(*c)) 
    {
        // read ID
        // must start with a letter
        const char* start = c;
        c = Script_ReadId(c);
        dest->string = start;
        dest->length = c - start;
        dest->type = kScriptTokenId;
    }
    else
    {
        dest->type = kScriptTokenUnknown;
        dest->character = *c;
        ++c;
    }
    return c;
}

static ScriptState ScriptInterpreter_Next(ScriptInterpreter* interpreter)
{
    const Script* script = interpreter->scripts + interpreter->scriptIndex;
    const char* c = script->source + interpreter->pc;

    int line = interpreter->eventLineNumber;

    ScriptToken commaToken;
    ScriptToken commandToken;
    ScriptToken argTokens[SCRIPT_COMMAND_ARGS_MAX];

    // read command ID
    c = Script_GetToken(c, &commandToken, 0);
    if (commandToken.type != kScriptTokenId)
    {
        printf("syntax error %i: expected ID got, %s\n", line, tokenToString[commandToken.type]);
        return kScriptStateHalt; 
    }

    // read command arguments
    int argCount = 0;
    while (argCount < SCRIPT_COMMAND_ARGS_MAX)
    {
        // check argument is valid
        c = Script_GetToken(c, argTokens + argCount, 1);
        TokenType type = argTokens[argCount].type;
        
        if (type == kScriptTokenUnknown)
            break;
        
        int validArg = (type == kScriptTokenId || type == kScriptTokenInt || type == kScriptTokenString);
        if (!validArg)
        {
            printf("syntax error %i: invalid argument type: %s\n", line, tokenToString[type]);
            return kScriptStateHalt;
        }
        ++argCount;

        // comma indicates we have more arguments, otherwise this command is done
        const char* afterComma = Script_GetToken(c, &commaToken, 1);
        if (commaToken.type != kScriptTokenComma)
            break;

        c = afterComma;
        if (argCount == SCRIPT_COMMAND_ARGS_MAX)
        {
            printf("syntax error %i: too many arguments. max %i\n", line, SCRIPT_COMMAND_ARGS_MAX);
            return kScriptStateHalt;
        }
    }

    // binary search for command
    char keyBuffer[SCRIPT_ID_NAME_MAX];
    memcpy(keyBuffer, commandToken.string, commandToken.length);
    keyBuffer[commandToken.length] = '\0';

    ScriptCommand key;
    key.name = keyBuffer;

    const ScriptCommand* it = bsearch(&key, interpreter->commandTable, interpreter->commandCount, sizeof(ScriptCommand), ScriptCommand_Compare);

    // output error if not found
    if (!it || !it->name)
    {
        printf("%i error: unknown command: %.*s\n", line, (int)commandToken.length, commandToken.string); 
        return kScriptStateHalt;
    }

    // verify arguments
    if (argCount != it->argCount)
    {
        printf("error %i: %s expects %i arguments, %i given\n", line, it->name, it->argCount, argCount);
        return kScriptStateHalt;
    }

    ScriptArg args[SCRIPT_COMMAND_ARGS_MAX];
    char intConvertBuffer[INT_LENGTH_MAX];

    for (int i = 0; i < it->argCount; ++i)
    {
        const ScriptToken* tok = argTokens + i;

        if (tok->type != it->argTypes[i])
        {
            printf("error %i: %s got %s, expected %s, on argument %i\n", line, it->name, tokenToString[tok->type], tokenToString[it->argTypes[i]], i);
            return kScriptStateHalt;
        }
        args[i].type = tokenTypeToArg[tok->type];
        args[i].stringLength = tok->length;
        args[i].stringValue = tok->string;

        if (args[i].type == kScriptArgInt)
        {
            if (tok->length >= INT_LENGTH_MAX)
            {
                printf("error %i: integers can only have %i digits\n", line, INT_LENGTH_MAX);
            }

            memcpy(intConvertBuffer, tok->string, tok->length);
            intConvertBuffer[tok->length] = '\0';
            args[i].intValue = (int)strtol(intConvertBuffer, NULL, 10);
        } 
    }
    // modify pc before execution so ccommand can jump
    interpreter->pc = c - script->source;
    // execute command
    return it->execute(args, argCount, interpreter->context);
}

int Script_FromPath(Script* script, const char* path)
{
    FILE* file = fopen(path, "r");
    
    if (!file)
        return 0;

    // begin by finding all events and their position in the source buffer
    int eventCount = 0;
    int lineNumber = 0;
    char lineBuffer[LINE_BUFFER_MAX];

    while (!feof(file))
    {
        fgets(lineBuffer, LINE_BUFFER_MAX, file); 
        ++lineNumber;

        const char* c = lineBuffer;

        if (*c == '#')
        {
            // read ID
            ++c;
            if (!isalnum(*c))
            {
                printf("syntax error %i: event must start with name\n", lineNumber);
                fclose(file);
                return 0;
            }

            const char* start = c;
            c = Script_ReadId(c);
            size_t length = c - start;

            if (length == 0 || length >= SCRIPT_ID_NAME_MAX)
            {
                printf("%i syntax error: invalid event id\n", lineNumber);
                fclose(file);
                return 0;
            }

            ScriptEvent* event = script->events + eventCount;
            memcpy(event->name, start, length);
            event->name[length] = '\0';
            event->position = ftell(file);
            event->lineNumber = lineNumber;
            ++eventCount;
        } 
    }
    script->eventCount = eventCount;

    // sort events for binary search
    qsort(script->events, eventCount, sizeof(ScriptEvent), ScriptEvent_Compare);

    // allocate a buffer to store the source
    size_t sourceSize = ftell(file);  
    char* sourceBuffer = malloc(sourceSize);
    
    // failed buffer allocation
    if (!sourceBuffer)
    {
        fclose(file);
        return 0;
    }

    fseek(file, 0, SEEK_SET);

    // read the source into the buffer
    fread(sourceBuffer, 1, sourceSize, file);
    fclose(file);
    script->source = sourceBuffer;
    return 1;
}

void Script_Shutdown(Script* script)
{
    if (!script)
        return;
    free((void*)script->source);
}

int ScriptInterpreter_Init(ScriptInterpreter* interpreter, const ScriptCommand* commandTable, int commandCount)
{
    if (commandCount == 0)
        return 0;
    
    memset(interpreter->scripts, 0, sizeof(Script) * SCRIPT_COUNT);
    
    // allocate a copy of the table
    size_t tableSize = sizeof(ScriptCommand) * commandCount;
    ScriptCommand* tableCopy = malloc(tableSize);
    memcpy(tableCopy, commandTable, tableSize);
    // sort the commands for binary search
    qsort(tableCopy, commandCount, sizeof(ScriptCommand), ScriptCommand_Compare);

    interpreter->commandTable = tableCopy;
    interpreter->commandCount = commandCount;
    interpreter->state = kScriptStateHalt;
    interpreter->scriptIndex = 0;
    interpreter->pc = 0;
    return 1;
}

void ScriptInterpreter_Shutdown(ScriptInterpreter* interpreter)
{
    if (!interpreter)
        return;
    free((void*)interpreter->commandTable);
}

int ScriptInterpreter_Seek(ScriptInterpreter* interpreter, const char* eventName)
{
    // check to see if the event is any any of the sources
    const ScriptEvent* eventToFind = NULL;

    int i;
    for (i = 0; i < SCRIPT_COUNT; ++i)
    {
        const Script* script = interpreter->scripts + i;
        
        if (script->source == NULL)
            continue;
        
        eventToFind = Script_Find(script, eventName);
        if (eventToFind)
            break;
    }
    if (!eventToFind)
        return 0;

    // set the program counter to the event position
    interpreter->scriptIndex = i;
    interpreter->pc = eventToFind->position;
    interpreter->eventLineNumber = eventToFind->lineNumber;
    return 1;    
}

void ScriptInterpreter_Continue(ScriptInterpreter* interpreter)
{
    // if we are yielding, we cannot continue
    if (interpreter->state == kScriptStateYield)
        return;
    
    // otherwise continue
    interpreter->state = kScriptStateRun;

    while (interpreter->state == kScriptStateRun)
        interpreter->state = ScriptInterpreter_Next(interpreter);
}

void ScriptInterpreter_Run(ScriptInterpreter* interpreter)
{
    // if we are yielding, we cannot continue
    if (interpreter->state == kScriptStateYield)
        return;

    while (interpreter->state == kScriptStateRun)
        interpreter->state = ScriptInterpreter_Next(interpreter);
}



