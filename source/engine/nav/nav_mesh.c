
#include "nav_mesh.h"
#include <stdlib.h>
#include "utils.h"
#include <string.h>
#include <assert.h>


int NavMesh_Init(NavMesh* mesh,
                 unsigned short vertexCount,
                 unsigned short edgeCount,
                 unsigned short polyCount)
{
    mesh->vertexCount = vertexCount;
    mesh->polyCount = polyCount;
    mesh->edgeCount = edgeCount;
    
    mesh->vertices = malloc(sizeof(Vec3) * vertexCount);
    if (!mesh->vertices)
        return 0;
    
    mesh->polys = malloc(sizeof(NavPoly) * polyCount);
    if (!mesh->polys)
    {
        free(mesh->vertices);
        return 0;
    }
    mesh->edges = malloc(sizeof(NavEdge) * edgeCount);
    if (!mesh->edges)
    {
        free(mesh->vertices);
        free(mesh->polys);
        return 0;
    }
    
    return 1;
}

void NavMesh_Shutdown(NavMesh* mesh)
{
    if (mesh->vertices)
        free(mesh->vertices);
    if (mesh->polys)
        free(mesh->polys);
    if (mesh->edges)
        free(mesh->edges);
}

NavPoly* NavMesh_GetPolyNeighbor(const NavMesh* mesh,
                                 const NavPoly* poly,
                                 int edgeIndex)
{
    assert(edgeIndex >= 0);
    assert(edgeIndex < poly->edgeCount);
    
    const NavEdge* edge = mesh->edges + poly->edgeStart + edgeIndex;
    
    if (edge->neighborIndex == -1)
        return NULL;
    
    return mesh->polys + edge->neighborIndex;
}

NavPoly* NavMesh_Raycast(const NavMesh* mesh, Ray3 ray, float* r)
{
    float min = HUGE_VALF;
    NavPoly* result = NULL;
    float t;
    
    for (int i = 0; i < mesh->polyCount; ++i)
    {
        NavPoly* poly = mesh->polys + i;
        
        if (Plane_IntersectRay(poly->plane, ray, &t))
        {
            if (t < 0.0f)
            {
                continue;
            }
            
            if (t < min)
            {
                Vec3 intersection = Ray3_Slide(ray, t);
                
                const NavEdge* originEdge = mesh->edges + poly->edgeStart;
                Vec3 origin = mesh->vertices[originEdge->vertices[0]];
                
                // get vector in polygon
                Vec3 rightVec = Vec3_Sub(mesh->vertices[originEdge->vertices[1]], origin);
                rightVec = Vec3_Norm(rightVec);
                
                // get up vector
                Vec3 upVec = Vec3_Norm(Vec3_Cross(rightVec, poly->plane.normal));
                
                // transform intersection into vector space
                Vec2 intersection2;
                intersection2.x = Vec3_Dot(Vec3_Sub(intersection, origin), rightVec);
                intersection2.y = Vec3_Dot(Vec3_Sub(intersection, origin), upVec);
                
                // project polygon edge points into face local vector space
                
                Vec2 verts[poly->edgeCount];
                for (int j = 0; j < poly->edgeCount; ++j)
                {
                    const NavEdge* edge = mesh->edges + poly->edgeStart + j;
                                        
                    Vec3 pa = Vec3_Sub(mesh->vertices[edge->vertices[0]], origin);

                    verts[j].x = Vec3_Dot(pa, rightVec);
                    verts[j].y = Vec3_Dot(pa, upVec);
                }
                
                if (Geo_PointInPoly(poly->edgeCount, verts, intersection2))
                {
                    min = t;
                    result = poly;
                }
            }
        }
    }
    
    if (result)
    {
        *r = min;
    }
    
    return result;
}

int NavMesh_LineEdgeCast(const NavMesh* mesh,
                         const NavPoly* poly,
                         Vec2 p1,
                         Vec2 p2,
                         NavEdgeFlag queryFlags)
{
    for (int i = 0; i < poly->edgeCount; ++i)
    {
        const NavEdge* edge = mesh->edges + poly->edgeStart + i;

        // skip edges that are not marked solid
        if (queryFlags != 0)
        {
            if (!(edge->flags & queryFlags))
                continue;
        }
        
        Vec3 pa = mesh->vertices[edge->vertices[0]];
        Vec3 pb = mesh->vertices[edge->vertices[1]];
        
        if (Geo_LineSegCross(p1, p2, Vec2_FromVec3(pa), Vec2_FromVec3(pb)))
            return 1;
    }
    
    return 0;
}

static int NavMesh_FromNAV(NavMesh* mesh, FILE* file)
{
    int vertCount = -1;
    short polyCount = -1;
    int edgeCount = -1;
    int readInfo = 0;
    
    char lineBuffer[LINE_BUFFER_MAX];
    memset(lineBuffer, 0x0, sizeof(lineBuffer));
    
    while (!feof(file))
    {
        fgets(lineBuffer, LINE_BUFFER_MAX, file);
        char* ptr;
        char* command = strtok_r(lineBuffer, ":", &ptr);
        char* data = strtok_r(NULL, ":", &ptr);
        
        if (strstr(command, "version"))
        {
            assert(atoi(data) == 1);
        }
        else if (strstr(command, "vertex_count"))
        {
            vertCount = atoi(data);
        }
        else if (strstr(command, "edge_count"))
        {
            edgeCount = atoi(data);
        }
        else if (strstr(command, "poly_count"))
        {
            polyCount = atoi(data);
        }
        else if (strstr(command, "vertices"))
        {
            if (!readInfo)
            {
                if (edgeCount == -1 || vertCount == -1 || polyCount == -1)
                {
                    return 0;
                }
                NavMesh_Init(mesh, vertCount, edgeCount, polyCount);
                readInfo = 0;
            }
            
            for (int i = 0; i < vertCount; ++i)
            {
                fgets(lineBuffer, LINE_BUFFER_MAX, file);
                
                if (sscanf(lineBuffer, "%f, %f, %f", &mesh->vertices[i].x, &mesh->vertices[i].y, &mesh->vertices[i].z) != 3)
                {
                    return 0;
                }
            }
        }
        else if (strstr(command, "edges"))
        {
            for (int i = 0; i < edgeCount; ++i)
            {                fgets(lineBuffer, LINE_BUFFER_MAX, file);

                
                int solid = 0;
                if (sscanf(lineBuffer,
                           "%hd, %hu, %hu, %i",
                           &mesh->edges[i].neighborIndex,
                           &mesh->edges[i].vertices[0],
                           &mesh->edges[i].vertices[1],
                           &solid) != 4)
                {
                    return 0;
                }
                
                mesh->edges[i].flags = kNavEdgeFlagNone;

                if (solid)
                    mesh->edges[i].flags |= kNavEdgeFlagSolid;
            }
        }
        else if (strstr(command, "polys"))
        {
            for (int i = 0; i < polyCount; ++i)
            {
                mesh->polys[i].index = i;
                
                fgets(lineBuffer, LINE_BUFFER_MAX, file);
                sscanf(lineBuffer, "%hu, %hu",
                       &mesh->polys[i].edgeStart,
                       &mesh->polys[i].edgeCount);
                
                fgets(lineBuffer, LINE_BUFFER_MAX, file);
                sscanf(lineBuffer, "%f, %f, %f",
                       &mesh->polys[i].plane.normal.x,
                       &mesh->polys[i].plane.normal.y,
                       &mesh->polys[i].plane.normal.z);
                
                Vec3 center = Vec3_Zero;
                
                int edgeCount = mesh->polys[i].edgeCount;
                int edgeStart = mesh->polys[i].edgeStart;

                for (int j = 0; j < edgeCount; ++j)
                {
                    Vec3 pa = mesh->vertices[mesh->edges[edgeStart + j].vertices[0]];
                    Vec3 pb = mesh->vertices[mesh->edges[edgeStart + j].vertices[1]];
                    
                    center = Vec3_Add(center, Vec3_Lerp(pa, pb, 0.5f));
                }
                
                center.x /= (float)edgeCount;
                center.y /= (float)edgeCount;
                center.z /= (float)edgeCount;
                
                mesh->polys[i].plane.point = center;
            }
        }
    }

    // swap the vertex order of each edge so they make a loop in sequence
    // this is important for constructing polygons during raycasting
    for (int i = 0; i < polyCount; ++i)
    {
        const NavPoly* poly = mesh->polys + i;
        
        NavEdge* current = mesh->edges + poly->edgeStart;
        NavEdge* next = current + 1;
        
        if (current->vertices[0] == next->vertices[0] || current->vertices[0] == next->vertices[1])
        {
            unsigned short temp = current->vertices[0];
            current->vertices[0] = current->vertices[1];
            current->vertices[1] = temp;
        }

        for (int k = 1; k < poly->edgeCount; ++k)
        {
            next = mesh->edges + poly->edgeStart + k;
            
            if (current->vertices[1] == next->vertices[1])
            {
                unsigned short temp = next->vertices[0];
                next->vertices[0] = next->vertices[1];
                next->vertices[1] = temp;
            }
            
            assert(current->vertices[1] == next->vertices[0]);
            current = next;
        }        
    }
    
    return 1;
}

int NavMesh_FromPath(NavMesh* mesh, const char* path)
{
    FILE* file = fopen(path, "r");
    
    if (!file) return 0;
    
    int status = 0;
    
    if (strcmp(Filepath_Extension(path), "nav") == 0)
    {
        status = NavMesh_FromNAV(mesh, file);
    }
    
    fclose(file);
    
    if (!status)
        return 0;
    
    return 1;
}

