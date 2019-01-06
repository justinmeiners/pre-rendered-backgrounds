
#include "utils.h"
#include <string.h>
#include <ctype.h>

const int g_endian = 1;

char g_gamePath[MAX_OS_PATH] = "";
char g_savePath[MAX_OS_PATH] = "";


char* Filepath_Extension(const char* path)
{
    static char exten[8];
    char* j;
    char* i;
    
    i = (char*)path + strnlen(path, MAX_OS_PATH);
    j = (char*)exten + 7;
    
    exten[7] = '\0';
    
    while (i != path && *i != '.')
    {
        j--;
        i--;
        *j = *i;
    }
    ++j;
    
    return j;
}

void Filepath_Append(char* dest, const char* base, const char* sub)
{
    if (base == NULL || sub == NULL)
    {
        return;
    }
    
    size_t baseLength = strnlen(base, MAX_OS_PATH);
    
    if (baseLength < 1)
    {
        strncpy(dest, sub, MAX_OS_PATH);
    }
    else
    {
        if (base[baseLength - 1] == '/')
        {
            snprintf(dest, MAX_OS_PATH, "%s%s", base, sub);
        }
        else
        {
            snprintf(dest, MAX_OS_PATH, "%s/%s", base, sub);
        }
    }
}

void Filepath_SetDataPath(const char* path)
{
    strncpy(g_gamePath, path, MAX_OS_PATH);
}

char* Filepath_DataPath()
{
    return g_gamePath;
}

void Filepath_SetSavePath(const char* path)
{
    strncpy(g_savePath, path, MAX_OS_PATH);
}

char* Filepath_SavePath()
{
    return g_savePath;
}

int String_IsEmpty(const char* s)
{
    while (*s != '\0')
    {
        if (!isspace(*s))
        {
            return 0;
        }
        ++s;
    }
    return 1;
}

int String_Prefix(const char* str, const char* pre, size_t maxSize)
{
    return strncmp(pre, str, maxSize) == 0;
}

unsigned long String_Hash(const char* string)
{
    unsigned long hash = (unsigned long)*string;
    
    if (hash)
    {
        for (string += 1; *string != '\0'; ++string)
        {
            hash = (hash << 5) - hash + (unsigned long)*string;
        }
    }
    
    return hash;
}
