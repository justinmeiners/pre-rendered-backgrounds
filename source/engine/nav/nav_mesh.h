
#ifndef NAV_MESH_H
#define NAV_MESH_H

#include "vec_math.h"
#include "geo_math.h"

typedef enum
{
    kNavEdgeFlagNone = 0,
    kNavEdgeFlagSolid = 1 << 0,
} NavEdgeFlag;


/*
 Nav edges store both positions and connectivity
 shared edges are duplicated providing a connection in each direction 
 */

typedef struct
{
    /* the index of the poly connected to */
    NavEdgeFlag flags;

    // -1 if no connection
    short neighborIndex;
    unsigned short vertices[2];
} NavEdge;

typedef struct
{
    // index into mesh edges
    unsigned short edgeStart;
    // how many edges in this poly?
    unsigned short edgeCount;

    // stores polygon normal and center
    Plane plane;
    
    unsigned short index;
} NavPoly;


typedef struct
{
    unsigned short vertexCount;
    unsigned short polyCount;
    unsigned short edgeCount;

    Vec3* vertices;
    NavPoly* polys;
    NavEdge* edges;
} NavMesh;

extern int NavMesh_Init(NavMesh* mesh,
                        unsigned short vertexCount,
                        unsigned short edgeCount,
                        unsigned short polyCount);

extern int NavMesh_FromPath(NavMesh* mesh, const char* path);

extern void NavMesh_Shutdown(NavMesh* mesh);



// returns the poly at the connection given by the index
extern NavPoly* NavMesh_GetPolyNeighbor(const NavMesh* mesh,
                                        const NavPoly* poly,
                                        int edgeIndex);


extern NavPoly* NavMesh_Raycast(const NavMesh* mesh, Ray3 ray, float* t);


/* this is for determining if a line crosses a nav mesh edge.
   This is useful for detecting intersections with solid edges */
extern int NavMesh_LineEdgeCast(const NavMesh* mesh,
                                const NavPoly* poly,
                                Vec2 p1,
                                Vec2 p2,
                                NavEdgeFlag queryFlags);


#endif
