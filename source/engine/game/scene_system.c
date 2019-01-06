
#include <stdlib.h>
#include "scene_system.h"
#include "json_utils.h"
#include "engine.h"

int SceneView_Compare(const void *a, const void *b)
{
    const SceneView* va = a;
    const SceneView* vb = b;
    return strncmp(va->name, vb->name, SCENE_ELEMENT_NAME_MAX);
}

int SceneLight_Compare(const void *a, const void *b)
{
    const SceneLight* la = a;
    const SceneLight* lb = b;
    return strncmp(la->name, lb->name, SCENE_ELEMENT_NAME_MAX);
}

int SceneNameEntry_Compare(const void* a, const void* b)
{
    const SceneNameEntry* ea = a;
    const SceneNameEntry* eb = b;
    return strncmp(ea->name, eb->name, SCENE_ELEMENT_NAME_MAX);
}

void SceneSystem_Load(SceneSystem* scene, const char* filepath)
{
    for (int i = 0; i < SCENE_ACTORS_MAX; ++i)
    {
        scene->actors[i].dead = 1;
        
        scene->actorNames[i].index = -1;
        scene->actorNames[i].name = NULL;
    }
    
    scene->actorNameCount = 0;
    scene->actorCount = 0;
    scene->lightCount = 0;
    scene->viewCount = 0;
    
    scene->playerIndex = -1;
    
    FILE* file = fopen(filepath, "r");
    
    fseek(file, 0, SEEK_END);
    size_t sourceSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* source = malloc(sourceSize);
    fread(source, sizeof(char), sourceSize, file);
    
    fclose(file);
    
    struct json_value_s* root = json_parse(source, sourceSize);
    struct json_object_s* rootObject = root->payload;
    struct json_object_element_s* dict = rootObject->start;
    
    while (dict)
    {
        if (strcmp("views", dict->name->string) == 0)
        {
            struct json_array_s* viewList = dict->value->payload;
            struct json_array_element_s* viewRoot= viewList->start;
            
            while (viewRoot)
            {
                struct json_object_s* viewObj = viewRoot->value->payload;
                struct json_object_element_s* viewElement = viewObj->start;
                
                SceneView* view = scene->views + scene->viewCount;
                
                while (viewElement)
                {
                    if (strcmp("name", viewElement->name->string) == 0)
                    {
                        json_value_get_string(viewElement->value, view->name, SCENE_ELEMENT_NAME_MAX);
                    }
                    else if (strcmp("fov", viewElement->name->string) == 0)
                    {
                        json_value_get_float(viewElement->value, &view->fov);
                    }
                    else if (strcmp("near", viewElement->name->string) == 0)
                    {
                        json_value_get_float(viewElement->value, &view->near);
                    }
                    else if (strcmp("far", viewElement->name->string) == 0)
                    {
                        json_value_get_float(viewElement->value, &view->far);
                    }
                    else if (strcmp("position", viewElement->name->string) == 0)
                    {
                        json_value_get_vec3(viewElement->value, &view->position);
                    }
                    else if (strcmp("rotation", viewElement->name->string) == 0)
                    {
                        json_value_get_quat(viewElement->value, &view->rotation);
                    }
                    else if (strcmp("light_primary", viewElement->name->string) == 0)
                    {
                        json_value_get_string(viewElement->value, view->primaryLight, SCENE_ELEMENT_NAME_MAX);
                    }
                    else if (strcmp("light_secondary", viewElement->name->string) == 0)
                    {
                        json_value_get_string(viewElement->value, view->secondaryLight, SCENE_ELEMENT_NAME_MAX);
                    }
                    
                    viewElement = viewElement->next;
                }
                
                ++scene->viewCount;
                viewRoot = viewRoot->next;
            }
        }
        else if (strcmp("actors", dict->name->string) == 0)
        {
            struct json_array_s* actorList = dict->value->payload;
            struct json_array_element_s* actorRoot = actorList->start;
            
            while (actorRoot)
            {
                struct json_object_s* actorObj = actorRoot->value->payload;
                struct json_object_element_s* actorElement = actorObj->start;
                
                Actor* actor = scene->actors + scene->actorCount;
                Actor_Init(actor);
                actor->dead = 0;
                
                while (actorElement)
                {
                    if (strcmp("name", actorElement->name->string) == 0)
                    {
                        json_value_get_string(actorElement->value, actor->name, SCENE_ELEMENT_NAME_MAX);
                        
                        if (strnlen(actor->name, SCENE_ELEMENT_NAME_MAX) > 0)
                        {
                            SceneNameEntry* entry =  scene->actorNames + scene->actorNameCount;
                            entry->name = actor->name;
                            entry->index = scene->actorCount;
                            ++scene->actorNameCount;
                        }
                    }
                    else if (strcmp("position", actorElement->name->string) == 0)
                    {
                        json_value_get_vec3(actorElement->value, &actor->position);
                    }
                    else if (strcmp("rotation", actorElement->name->string) == 0)
                    {
                        json_value_get_quat(actorElement->value, &actor->rotation);
                    }
                    else if (strcmp("bounds_min", actorElement->name->string) == 0)
                    {
                        json_value_get_vec3(actorElement->value, &actor->bounds.min);
                    }
                    else if (strcmp("bounds_max", actorElement->name->string) == 0)
                    {
                        json_value_get_vec3(actorElement->value, &actor->bounds.max);
                    }
                    else if (strcmp("view", actorElement->name->string) == 0)
                    {
                        json_value_get_string(actorElement->value, actor->viewName, SCRIPT_ID_NAME_MAX);
                    }
                    else if (strcmp("tap_enabled", actorElement->name->string) == 0)
                    {
                        json_value_get_int(actorElement->value, &actor->tapEnabled);
                    }
                    else if (strcmp("event", actorElement->name->string) == 0)
                    {
                        json_value_get_string(actorElement->value, actor->eventName, SCRIPT_ID_NAME_MAX);
                    }
                    else if (strcmp("type", actorElement->name->string) == 0)
                    {
                        char typeString[SCENE_ELEMENT_NAME_MAX];
                        json_value_get_string(actorElement->value, typeString, SCENE_ELEMENT_NAME_MAX);
                        
                        const ActorTypeEntry* it = g_actorTypeTable;
                        
                        while (it->type != kActorNone)
                        {
                            if (strcmp(typeString, it->typeName) == 0)
                            {
                                actor->type = it->type;
                                actor->onSpawn = it->onSpawn;
                                break;
                            }
                            ++it;
                        }
                    }
                    
                    actorElement = actorElement->next;
                }
                
                ++scene->actorCount;
                actorRoot = actorRoot->next;
            }
            
        }
        else if (strcmp("lights", dict->name->string) == 0)
        {
            struct json_array_s* lightList = dict->value->payload;
            struct json_array_element_s* lightRoot = lightList->start;
            
            while (lightRoot)
            {
                struct json_object_s* lightObj = lightRoot->value->payload;
                struct json_object_element_s* lightElement = lightObj->start;
                
                SceneLight* light = scene->lights + scene->lightCount;
                                
                while (lightElement)
                {
                    if (strcmp("name", lightElement->name->string) == 0)
                    {
                        json_value_get_string(lightElement->value, light->name, SCENE_ELEMENT_NAME_MAX);
                    }
                    else if (strcmp("position", lightElement->name->string) == 0)
                    {
                        json_value_get_vec3(lightElement->value, &light->position);
                    }
                    else if (strcmp("color", lightElement->name->string) == 0)
                    {
                        json_value_get_vec3(lightElement->value, &light->color);
                    }
                    
                    lightElement = lightElement->next;
                }
                
                ++scene->lightCount;
                lightRoot = lightRoot->next;
            }
            
        }

        dict = dict->next;
    }
    
    free(root);
    
    qsort(scene->actorNames, scene->actorCount, sizeof(SceneNameEntry), SceneNameEntry_Compare);
    qsort(scene->views, scene->viewCount, sizeof(SceneView), SceneView_Compare);
    qsort(scene->lights, scene->lightCount, sizeof(SceneLight), SceneLight_Compare);

    for (int i = 0; i < SCENE_ACTORS_MAX; ++i)
    {
        Actor* actor = scene->actors + i;
        actor->index = i;
        if (actor->dead == 1)
            continue;
        
        if (actor->onSpawn)
            actor->onSpawn(actor, 0);
    }
}

Actor* SceneSystem_SpawnAt(SceneSystem* scene,
                           ActorType type,
                           Vec3 position,
                           Quat rotation,
                           int flags)
{
    for (int i = 0; i < SCENE_ACTORS_MAX; ++i)
    {
        Actor* actor = scene->actors + i;
        
        if (!actor->dead) continue;
        
        Actor_Init(actor);
        
        const ActorTypeEntry* it = g_actorTypeTable;
        
        while (it->type != kActorNone)
        {
            if (it->type == type)
            {
                actor->type = it->type;
                actor->onSpawn = it->onSpawn;
                break;
            }
            ++it;
        }

        
        actor->dead = 0;
        actor->position = position;
        
        if (actor->onSpawn)
            actor->onSpawn(actor, flags);

        return actor;
    }
    
    return NULL;
}

void SceneSystem_Kill(SceneSystem* scene, Actor* actor)
{
    if (actor->onKill)
        actor->onKill(actor);
    
    actor->dead = 1;
}

SceneView* SceneSystem_FindView(SceneSystem* scene, const char* name)
{
    SceneView toFindKey;
    strncpy(toFindKey.name, name, SCENE_ELEMENT_NAME_MAX);
    
    SceneView* view = bsearch(&toFindKey, scene->views, scene->viewCount, sizeof(SceneView), SceneView_Compare);
    
    if (!view)
    {
        printf("no view: %s\n", name);
        return NULL;
    }
    
    return view;
}

SceneLight* SceneSystem_FindLight(SceneSystem* scene, const char* name)
{
    SceneLight toFindKey;
    strncpy(toFindKey.name, name, SCENE_ELEMENT_NAME_MAX);
    
    SceneLight* light = bsearch(&toFindKey, scene->lights, scene->lightCount, sizeof(SceneLight), SceneLight_Compare);
    
    if (!light)
    {
        printf("no light: %s\n", name);
        return NULL;
    }
    
    return light;
}

Actor* SceneSystem_FindActor(SceneSystem* scene, const char* name)
{
    SceneNameEntry key;
    key.name = (char*)name;
    
    SceneNameEntry* result = bsearch(&key, scene->actorNames, scene->actorNameCount, sizeof(SceneNameEntry), SceneNameEntry_Compare);

    if (!result || result->index == -1)
        return NULL;
    
    return scene->actors + result->index;
}

Actor* SceneSystem_Player(SceneSystem* scene)
{
    if (scene->playerIndex == -1)
        return NULL;
    
    return scene->actors + scene->playerIndex;
}


          
