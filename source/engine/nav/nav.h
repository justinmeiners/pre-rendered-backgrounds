
#ifndef NAV_H
#define NAV_H

#include "nav_mesh.h"

#define PATH_MAX_NODES 128

typedef struct
{
    short polyIndex;
    short edgeIndex;
    Vec3 position;
} NavPathNode;

typedef struct
{
    int nodeCount;
    NavPathNode nodes[PATH_MAX_NODES];
} NavPath;

extern void NavPath_Init(NavPath* path);

extern void NavPath_AddNode(NavPath* path, Vec3 position, int edgeIndex, int polyIndex);
extern NavPathNode* NavPath_NodeAtIndex(NavPath* path, int index);

extern void NavPath_Clear(NavPath* path);
/* in and out cannot be the same path */
extern void NavPath_Reverse(const NavPath* inPath, NavPath* outPath);
extern int NavPath_NodeCount(NavPath* path);


struct NavSearchNode
{
    short polyIndex;
    short edgeIndex;
    float cost;
    
    // since the pool address is not stable, we need to use indicies
    int next;
    int parent;
};

typedef struct
{
    int head;
    struct NavSearchNode* pool;
    char* closed;
    
} NavSolver;

extern int NavSolver_Init(NavSolver* nav);
extern void NavSolver_Shutdown(NavSolver* nav);



extern void NavSolver_Prepare(NavSolver* nav,
                              const NavMesh* mesh);

/*
 pure mathematical - portal to portal path finding.
 A connectivity solution is found. Results are not smoothed,
 */

extern int NavSolver_Solve(NavSolver* nav,
                           const NavMesh* mesh,
                           Vec3 startPoint,
                           Vec3 endPoint,
                           const NavPoly* startPoly,
                           const NavPoly* endPoly,
                           NavPath* path);

/* create a nice smoothed version of the solved path */
extern void NavSolver_SmoothPath(const NavMesh* mesh,
                                 NavPath* path,
                                 Vec3 start,
                                 Vec3 dest,
                                 float radius);


#endif
