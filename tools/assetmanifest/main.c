
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSET_NAME_MAX 128
#define PATH_MAX 512
#define LINE_MAX 1024

#define ASSET_TYPE_MAX 32
#define ASSET_ENTRY_MAX 512

typedef struct
{
    char name[ASSET_NAME_MAX];
    int count;
} AssetType;

typedef struct
{
    int type;
    char enumName[ASSET_NAME_MAX];
    
    int hasPath;
    char path[PATH_MAX];
    
} AssetEntry;

static int AssetEntry_Compare(const void* a, const void *b)
{
    const AssetEntry* ea = a;
    const AssetEntry* eb = b;
    return strncmp(ea->enumName, eb->enumName, ASSET_NAME_MAX);
}

int main(int argc, const char * argv[])
{
    char lineBuffer[LINE_MAX];
 
    int entryCount = 0;
    int typeCount = 0;
    
    AssetType types[ASSET_TYPE_MAX];
    AssetEntry entries[ASSET_ENTRY_MAX];
    memset(types, 0, sizeof(types));
    
    while (!feof(stdin))
    {
        fgets(lineBuffer, LINE_MAX, stdin);
        
        AssetEntry* entry = entries + entryCount;
        AssetType* type = types + typeCount;
        
        if (sscanf(lineBuffer, "%128[^:]: %128s %1024s", type->name, entry->enumName, entry->path) == 3)
        {
            entry->type = -1;
            
            for (int i = 0; i < typeCount; ++i)
            {
                if (strncmp(types[i].name, type->name, ASSET_NAME_MAX) == 0)
                {
                    entry->type = i;
                    break;
                }
            }
            
            if (entry->type == -1)
            {
                entry->type = typeCount;
                ++typeCount;
            }
            
            if (strcmp(entry->path, "NULL") == 0)
            {
                entry->hasPath = 0;
            }
            else
            {
                entry->hasPath = 1;
            }
            
            ++types[entry->type].count;
            ++entryCount;
        }
    }
    
    qsort(entries, entryCount, sizeof(AssetEntry), AssetEntry_Compare);
    
    printf("#ifndef ASSET_MANIFEST_H\n");
    printf("#define ASSET_MANIFEST_H\n\n");
    
    printf("#define ASSET_NAME_MAX %i\n\n", ASSET_NAME_MAX);

    printf("typedef struct\n{\n");
    printf("    int identifier;\n");
    printf("    const char* name;\n");
    printf("    const char* path;\n");
    printf("} AssetEntry;\n\n");
    
    for (int i = 0; i < typeCount; ++i)
    {
        printf("enum\n{\n");
        for (int j = 0; j < entryCount; ++j)
        {
            const AssetEntry* entry = entries + j;
            
            if (entry->type == i)
            {
                printf("    %s,\n", entry->enumName);
            }
        }
        printf("};\n\n");
    }
    
    
    for (int i = 0; i < typeCount; ++i)
    {
        const AssetType* type = types + i;
        
        printf("enum { Asset_%sCount=%i };\n\n", type->name, type->count);
        printf("static const AssetEntry Asset_%sManifest[] = {\n", type->name);
        
        for (int j = 0; j < entryCount; ++j)
        {
            const AssetEntry* entry = entries + j;
            
            if (entry->type == i)
            {
                if (entry->hasPath)
                {
                    printf("    %s, \"%s\", \"%s\",\n", entry->enumName, entry->enumName, entry->path);
                }
                else
                {
                    printf("    %s, \"%s\", NULL,\n", entry->enumName, entry->enumName);
                }
            }
        }
        
        printf("};\n\n");
    }
    
    printf("#endif\n");
    
    return 0;
}
