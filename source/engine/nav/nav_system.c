
#include "nav_system.h"
#include "utils.h"

void NavSystem_Init(NavSystem* system)
{
    NavSolver_Init(&system->solver);
}

void NavSystem_Shutdown(NavSystem* system)
{
    NavSolver_Shutdown(&system->solver);
    NavSystem_LoadNavMesh(system, NULL);
}

int NavSystem_LoadNavMesh(NavSystem* system, const char* path)
{
    if (path == NULL)
    {
        NavMesh_Shutdown(&system->navMesh);
        return 0;
    }
    
    char fullPath[MAX_OS_PATH];
    Filepath_Append(fullPath, Filepath_DataPath(), path);
    
    int result = NavMesh_FromPath(&system->navMesh, fullPath);
    
    if (result)
    {
        NavSolver_Prepare(&system->solver, &system->navMesh);
    }
    
    return 0;
}

int NavSystem_Raycast(const NavSystem* system, Ray3 ray, NavRaycastResult* hitInfo)
{
    if ((hitInfo->poly = NavMesh_Raycast(&system->navMesh, ray, &hitInfo->distance)))
    {
        hitInfo->point = Ray3_Slide(ray, hitInfo->distance);
        return 1;
    }
    
    return 0;
}

int NavSystem_LineIntersectsSolid(const NavSystem* system,
                             Vec3 start,
                             Vec3 end,
                             float height)
{
    Vec3 vec = Vec3_Sub(end, start);
    
    Ray3 r0 = Ray3_Create(start, Vec3_Norm(vec));
    
    Ray3 r1 = Ray3_Create(start, Vec3_Create(0.0f, 0.0f, -1.0f));
    Ray3 r2 = Ray3_Create(end, Vec3_Create(0.0f, 0.0f, -1.0f));
    
    // check if the line intersects with the ground 
    float t;
    if (NavMesh_Raycast(&system->navMesh, r0, &t))
    {
        if (t * t < Vec3_LengthSq(vec))
        {
            return 1;
        }
    }
    
    NavPoly* poly = NULL;
    
    poly = NavMesh_Raycast(&system->navMesh, r1, &t);
    
    if (poly == NULL)
    {
        poly = NavMesh_Raycast(&system->navMesh, r2, &t);
    }
    
    if (poly && NavMesh_LineEdgeCast(&system->navMesh, poly, Vec2_FromVec3(start), Vec2_FromVec3(end), kNavEdgeFlagSolid))
    {
        return 1;
    }
    
    return 0;
}
