
#ifndef ACTOR_H
#define ACTOR_H

#include "nav.h"
#include "script.h"
#include "static_model.h"
#include "skel_model.h"

#define SCENE_ELEMENT_NAME_MAX 64


typedef enum
{
    kActorNone = 0,
    kActorEmpty,
    kActorPlayer,
    kActorWeapon,
    kActorTrigger,
    kActorViewTrigger,
    kActorSceneStart,
    kActorButton,
    kActorWaypoint,
    
} ActorType;

typedef enum
{
    kActorRenderNone = 0,
    kActorRenderSkelModel,
    kActorRenderStaticModel,
    
} ActorRenderType;

struct Engine;
struct Actor;

typedef struct
{
    char typeName[SCENE_ELEMENT_NAME_MAX];
    ActorType type;
    void (*onSpawn)(struct Actor* actor, int flags);
} ActorTypeEntry;

extern int ActorTypeEntry_Compare(const void* a, const void* b);


typedef struct Actor
{
    int index;
    char name[SCENE_ELEMENT_NAME_MAX];
    int dead;
    ActorType type;
    ActorRenderType renderType;
    
    AABB bounds;
    Vec3 position;
    Quat rotation;
    
    Mat4 worldMatrix;
    
    int pathIndex;
    float angle;
    float targetAngle;
    NavPath path;
    NavPoly* pathPoly;
    int destinationPolyIndex;
    int onPath;
    Vec3 destination;
    
    SkelModel skelModel;
    StaticModel staticModel;
    
    int tapEnabled;
    
    struct Actor* useTarget;
    struct Actor* item;
    
    char eventName[SCRIPT_EVENT_MAX];
    char viewName[SCRIPT_EVENT_MAX];

    void (*onSpawn)(struct Actor* actor, int flags);
    void (*onKill)(struct Actor* actor);
    
    void (*onUpdate)(struct Actor* actor);
    void (*onTouch)(struct Actor* actor, struct Actor* other);
    void (*onUse)(struct Actor* actor, struct Actor* user);
        
    void (*onTap)(struct Actor* actor, Ray3 ray);
} Actor;

extern ActorTypeEntry g_actorTypeTable[];

extern void Actor_Init(Actor* actor);
extern int Actor_StartPath(Actor* actor, Vec3 destination);

#endif
