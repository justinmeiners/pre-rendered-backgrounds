
#ifndef SCENE_DEF_H
#define SCENE_DEF_H

#include "geo_math.h"
#include "actor.h"
#include "nav_system.h"


typedef struct
{
    char name[SCENE_ELEMENT_NAME_MAX];
    
    char primaryLight[SCENE_ELEMENT_NAME_MAX];
    char secondaryLight[SCENE_ELEMENT_NAME_MAX];

    float fov;
    float near;
    float far;
    Vec3 position;
    Quat rotation;
} SceneView;

extern int SceneView_Compare(const void *a, const void *b);

typedef struct
{
    char name[SCENE_ELEMENT_NAME_MAX];
    Vec3 color;
    Vec3 position;
} SceneLight;

extern int SceneLight_Compare(const void *a, const void *b);

typedef struct
{
    char* name;
    int index;
} SceneNameEntry;

extern int SceneNameEntry_Compare(const void* a, const void* b);


#define SCENE_VIEWS_MAX 32
#define SCENE_LIGHTS_MAX 32
#define SCENE_ACTORS_MAX 256

#define SCENE_LIGHTS_PER_VIEW 2

typedef struct
{
    int viewCount;
    SceneView views[SCENE_VIEWS_MAX];
    
    int lightCount;
    SceneLight lights[SCENE_LIGHTS_MAX];
    
    int actorCount;
    Actor actors[SCENE_ACTORS_MAX];
    
    int playerIndex;
    
    SceneView* currentView;
    SceneLight* activeLights[SCENE_LIGHTS_PER_VIEW];
    
    int actorNameCount;
    SceneNameEntry actorNames[SCENE_ACTORS_MAX];
    
} SceneSystem;

extern void SceneSystem_Load(SceneSystem* scene, const char* filepath);


extern Actor* SceneSystem_SpawnAt(SceneSystem* scene,
                                  ActorType type,
                                  Vec3 position,
                                  Quat rotation,
                                  int flags);

extern void SceneSystem_Kill(SceneSystem* scene, Actor* actor);

extern SceneView* SceneSystem_FindView(SceneSystem* scene, const char* name);
extern SceneLight* SceneSystem_FindLight(SceneSystem* scene, const char* name);
extern Actor* SceneSystem_FindActor(SceneSystem* scene, const char* name);

extern Actor* SceneSystem_Player(SceneSystem* scene);

#endif
