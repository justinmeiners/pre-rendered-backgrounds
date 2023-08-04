
#include "actor.h"
#include "engine.h"
#include <string.h>

int ActorTypeEntry_Compare(const void* a, const void* b)
{
    const ActorTypeEntry* ea = a;
    const ActorTypeEntry* eb = b;
    
    return strncmp(ea->typeName, eb->typeName, SCENE_ELEMENT_NAME_MAX);
}

void Actor_Init(Actor* actor)
{
    memset(actor->name, 0, sizeof(char) * SCENE_ELEMENT_NAME_MAX);
    memset(actor->eventName, 0, sizeof(char) * SCRIPT_ID_NAME_MAX);
    
    actor->dead = 1;
    
    actor->type = kActorEmpty;
    actor->renderType = kActorRenderNone;

    NavPath_Init(&actor->path);
    
    actor->position = Vec3_Zero;
    actor->rotation = Quat_Identity;
    actor->bounds = AABB_CreateCentered(Vec3_Zero, Vec3_Create(1.0f, 1.0f, 1.0f));
    
    actor->onPath = 0;
    
    Mat4_Identity(&actor->worldMatrix);
    
    actor->item = NULL;
    
    actor->onTap = NULL;
    actor->onUpdate = NULL;
    actor->onTouch = NULL;
    actor->onKill = NULL;
    actor->onSpawn = NULL;
    actor->onUse = NULL;
    
    actor->tapEnabled = 0;
}

int Actor_StartPath(Actor* actor, Vec3 destination)
{
    Ray3 targetRay = Ray3_Create(Vec3_Add(destination, Vec3_Create(0.0f, 0.0f, 1.0f)), Vec3_Create(0.0f, 0.0f, -1.0f));
    
    const NavPoly* startPoly = actor->pathPoly;
    
    if (!startPoly)
        return 0;
    
    NavRaycastResult hitInfo;
    if (!NavSystem_Raycast(&g_engine.navSystem, targetRay, &hitInfo))
    {
        return 0;
    }
    
    actor->destinationPolyIndex = hitInfo.poly->index;
    actor->destination = destination;
    
    if (startPoly == hitInfo.poly)
    {
        actor->path.nodeCount = 0;
        NavPath_AddNode(&actor->path, destination, -1, -1);
    }
    else
    {
        NavSolver_Solve(&g_engine.navSystem.solver,
                        &g_engine.navSystem.navMesh,
                        actor->position,
                        destination,
                        startPoly,
                        hitInfo.poly,
                        &actor->path);
        
        NavSolver_SmoothPath(&g_engine.navSystem.navMesh, &actor->path, actor->position, destination, 0.25f);
    }
    
    actor->onPath = 1;
    actor->pathIndex = 0;
        
    return 1;
}

static void Player_OnUpdate(Actor* actor)
{
    Engine* engine = &g_engine;
    
    // Raycast from unit loctation to determine which nav polygon the unit is on
    Ray3 ray = Ray3_Create(Vec3_Add(actor->position, Vec3_Create(0.0f, 0.0f, 1.0f)), Vec3_Create(0.0f, 0.0f, -1.0f));
    
    NavRaycastResult hitInfo;
    if (NavSystem_Raycast(&engine->navSystem, ray, &hitInfo))
    {
        actor->position.z = hitInfo.point.z;
        actor->pathPoly = hitInfo.poly;
    }
    else
    {
        actor->pathPoly = NULL;
    }
    
    if (actor->onPath)
    {
        SkelAnimator_SetAnim(&actor->skelModel.animator, g_engine.renderSystem.anims + ANIM_ASTRONAUT_WALK, 0, 15);

        // move along path
        Vec3 dest = actor->path.nodes[actor->pathIndex].position;
        Vec3 dir = Vec3_Sub(dest, actor->position);
        
        float speed = 0.04f;
        
        float closeEnoughDist = 0.05f;
        
        if (Vec2_LengthSq(Vec2_FromVec3(dir)) > closeEnoughDist * closeEnoughDist)
        {
            actor->targetAngle = RAD_TO_DEG(atan2(dir.y, dir.x));
            
            dir = Vec3_Norm(dir);
            float z = actor->position.z;
            actor->position = Vec3_Add(actor->position, Vec3_Scale(dir, speed));
            actor->position.z = z;
        }
        else
        {
            actor->position = dest;
            
            if (actor->pathIndex + 1 < actor->path.nodeCount)
            {
                actor->pathIndex++;
            }
            else
            {
                actor->onPath = 0;
            }
        }
        
        SkelModel_Tick(&actor->skelModel);
    }
    else
    {
        if (actor->useTarget)
        {
            Vec2 dir = Vec2_Sub(Vec2_FromVec3(actor->useTarget->position), Vec2_FromVec3(actor->position));
            actor->targetAngle = DEG_TO_RAD(atan2f(dir.x, dir.y)) + 90.0f;
            SkelAnimator_SetAnim(&actor->skelModel.animator, g_engine.renderSystem.anims + ANIM_ASTRONAUT_PRESS, 0, 15);
            
            SkelAnimatorInfo info =  SkelModel_Tick(&actor->skelModel);
            
            if (info.marker != NULL)
            {
                if (actor->useTarget)
                    actor->useTarget->onUse(actor->useTarget, actor);
                actor->useTarget = NULL;
                
            }
        }
        else
        {
            SkelAnimator_SetAnim(&actor->skelModel.animator, g_engine.renderSystem.anims + ANIM_ASTRONAUT_IDLE, 0, 15);
            SkelModel_Tick(&actor->skelModel);

        }
        
    }
    
    float diff = Deg_Diff(actor->targetAngle, actor->angle);
    if (fabs(diff) > 3.5f)
    {
        diff = CLAMP(diff, -3.0f, 3.0f);
        actor->angle += diff;
    }

    actor->skelModel.skel.rotation = Quat_CreateAngle(actor->angle + 90.0f, 0.0f, 0.0f, 1.0f);
    
    actor->bounds = AABB_CreateCentered(actor->position, Vec3_Create(1.0f, 1.0f, 1.0f));
    
    const SkelAttachPoint* attachPoint = Skel_FindAttachPoint(&actor->skelModel.skel, "tool_attatch_point");
    Actor* item = actor->item;
    item->position = Vec3_Add(actor->position, attachPoint->modelPosition);
    item->rotation = attachPoint->modelRotation;
}

static void Weapon_OnUpdate(Actor* actor)
{
    actor->rotation = Quat_Mult(actor->rotation, Quat_CreateAngle(0.5f, 0.0f, 0.0f, 1.0f));
}

static void Weapon_OnSpawn(Actor* actor, int flags)
{
    actor->onUpdate = Weapon_OnUpdate;
    StaticModel_Copy(&actor->staticModel, g_engine.renderSystem.models + MODEL_WRENCH);
    actor->renderType = kActorRenderStaticModel;
    
    actor->staticModel.material.albedoMap = TEX_WRENCH_ALBEDO;
    actor->staticModel.material.specularMap = TEX_WRENCH_SPECULAR;
}

static void Player_OnSpawn(Actor* actor, int flags)
{
    actor->onUpdate = Player_OnUpdate;
    
    g_engine.sceneSystem.playerIndex = actor->index;
    actor->renderType = kActorRenderSkelModel;
    
    SkelModel_Copy(&actor->skelModel, g_engine.renderSystem.skelModels + SKEL_ASTRONAUT);
    
    actor->skelModel.material.albedoMap = TEX_ASTRONAUT_ALBEDO;
    actor->skelModel.material.normalMap = TEX_ASTRONAUT_NORMAL;
    actor->skelModel.material.specularMap = TEX_ASTRONAUT_SPECULAR;
    actor->skelModel.material.glossMap = TEX_ASTRONAUT_GLOSS;

    actor->item = SceneSystem_SpawnAt(&g_engine.sceneSystem, kActorWeapon, actor->position, actor->rotation, 0);
}

static void Button_OnUse(Actor* actor, Actor* user)
{
    if (!String_IsEmpty(actor->eventName))
        ScriptSystem_RunEvent(&g_engine.scriptSystem, actor->eventName);
}

static void Button_OnTap(Actor* actor, Ray3 ray)
{
    if (actor->pathPoly)
    {
        Actor* player = SceneSystem_Player(&g_engine.sceneSystem);
        Actor_StartPath(player, actor->position);
        player->useTarget = actor;
    }
}

static void Button_OnSpawn(Actor* actor, int flags)
{
    actor->onUse = Button_OnUse;
    actor->onTap = Button_OnTap;
    actor->tapEnabled = 1;
    
    Ray3 ray = Ray3_Create(AABB_Center(actor->bounds), Vec3_Create(0.0f, 0.0f, -1.0f));
    
    NavRaycastResult result;
    if (NavSystem_Raycast(&g_engine.navSystem, ray, &result))
    {
        actor->pathPoly = result.poly;
        actor->position = result.point;
    }
}

static void Trigger_OnTouch(Actor* actor, Actor* other)
{
    if (other->type == kActorPlayer)
    {
        if (!String_IsEmpty(actor->eventName))
            ScriptSystem_RunEvent(&g_engine.scriptSystem, actor->eventName);
    }
}

static void Trigger_OnSpawn(Actor* actor, int flags)
{
    actor->onTouch = Trigger_OnTouch;
}

static void ViewTrigger_OnTap(Actor* actor, Ray3 ray)
{
    if (actor->pathPoly)
    {
        Actor* player = SceneSystem_Player(&g_engine.sceneSystem);
        
        Actor_StartPath(player, actor->position);
    }
}

static void ViewTrigger_OnUpdate(Actor* actor)
{
    if (!String_IsEmpty(actor->viewName))
    {
        Actor* player = SceneSystem_Player(&g_engine.sceneSystem);

        if (player && AABB_IntersectsPoint(actor->bounds, player->position))
            Engine_LoadView(&g_engine, actor->viewName);
    }
}

static void ViewTrigger_OnSpawn(Actor* actor, int flags)
{
    actor->onUpdate = ViewTrigger_OnUpdate;
    
    if (actor->tapEnabled)
        actor->onTap = ViewTrigger_OnTap;
    
    Ray3 ray = Ray3_Create(AABB_Center(actor->bounds), Vec3_Create(0.0f, 0.0f, -1.0f));
    
    NavRaycastResult result;
    if (NavSystem_Raycast(&g_engine.navSystem, ray, &result))
    {
        actor->pathPoly = result.poly;
        actor->position = result.point;
    }
}

static void SceneStart_OnSpawn(Actor* actor, int flags)
{
    SceneSystem_SpawnAt(&g_engine.sceneSystem, kActorPlayer, actor->position, actor->rotation, 0);
}

static void Waypoint_OnSpawn(Actor* actor, int flags)
{
    Ray3 ray = Ray3_Create(actor->position, Vec3_Create(0.0f, 0.0f, -1.0f));

    NavRaycastResult result;
    if (NavSystem_Raycast(&g_engine.navSystem, ray, &result))
        actor->pathPoly = result.poly;
}

ActorTypeEntry g_actorTypeTable[] =
{
    {
        "player",
        kActorPlayer,
        Player_OnSpawn
    },
    {
        "weapon",
        kActorWeapon,
        Weapon_OnSpawn
    },
    {
        "trigger",
        kActorTrigger,
        Trigger_OnSpawn,
    },
    {
        "view_trigger",
        kActorViewTrigger,
        ViewTrigger_OnSpawn,
    },
    {
        "scene_start",
        kActorSceneStart,
        SceneStart_OnSpawn
        
    },
    {
        "button",
        kActorButton,
        Button_OnSpawn,
        
    },
    {
        "waypoint",
        kActorWaypoint,
        Waypoint_OnSpawn
        
    },
    {
        "",
        kActorNone,
        NULL,
        
    }
};
