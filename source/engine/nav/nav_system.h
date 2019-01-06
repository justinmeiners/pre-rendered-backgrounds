
#ifndef NAV_SYSTEM_H
#define NAV_SYSTEM_H

#include "nav.h"

typedef struct
{
    NavPoly* poly;
    Vec3 point;
    float distance;
} NavRaycastResult;

typedef struct
{
    NavMesh navMesh;
    NavSolver solver;
} NavSystem;

extern void NavSystem_Init(NavSystem* system);
extern void NavSystem_Shutdown(NavSystem* system);

extern int NavSystem_LoadNavMesh(NavSystem* system, const char* path);

extern int NavSystem_Raycast(const NavSystem* system, Ray3 ray, NavRaycastResult* hitInfo);

extern int NavSystem_LineIntersectsSolid(const NavSystem* system,
                                         Vec3 start,
                                         Vec3 end,
                                         float height);




#endif
