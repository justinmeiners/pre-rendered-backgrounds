
#include "nav.h"
#include <stdlib.h>
#include "stretchy_buffer.h"
#include "utils.h"
#include <assert.h>


static float TriArea(const Vec3* a, const Vec3* b, const Vec3* c)
{
    const float ax = b->x - a->x;
    const float ay = b->y - a->y;
    const float bx = c->x - a->x;
    const float by = c->y - a->y;
    return bx * ay - ax * by;
}

static int VecEqual(const Vec3* a, const Vec3* b)
{
    static const float eq = 0.001f*0.001f;
    return ((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y)) < eq;
}

void NavPath_Init(NavPath* path)
{
    path->nodeCount = 0;
    memset(path->nodes, 0x0, sizeof(path->nodes));
}

void NavPath_AddNode(NavPath* path, Vec3 position, int edgeIndex, int polyIndex)
{
    assert(path->nodeCount + 1 < PATH_MAX_NODES);
    
    NavPathNode newNode;
    newNode.position = position;
    newNode.polyIndex = polyIndex;
    newNode.edgeIndex = edgeIndex;
    
    path->nodes[path->nodeCount] = newNode;
    ++path->nodeCount;
}

NavPathNode* NavPath_NodeAtIndex(NavPath* path, int index)
{
    assert(index < path->nodeCount);
    return &path->nodes[index];
}

void NavPath_Clear(NavPath* path)
{
    assert(path);
    path->nodeCount = 0;
}

int NavPath_NodeCount(NavPath* path)
{
    assert(path);
    return path->nodeCount;
}

void NavPath_Reverse(const NavPath* inPath, NavPath* outPath)
{
    assert(inPath);
    assert(outPath);
    
    outPath->nodeCount = inPath->nodeCount;
    
    for (int i = 0; i < inPath->nodeCount; i ++)
    {
        outPath->nodes[inPath->nodeCount - (i + 1)] = inPath->nodes[i];
    }
}


int NavSolver_Init(NavSolver* nav)
{
    nav->pool = NULL;
    nav->closed = NULL;
    
    return 1;
}

void NavSolver_Shutdown(NavSolver* nav)
{
    if (nav->pool)
        stb_sb_free(nav->pool);
    
    if (nav->closed)
        stb_sb_free(nav->closed);
    
    nav->pool = NULL;
    nav->closed = NULL;
}

void NavSolver_Prepare(NavSolver* nav,
                       const NavMesh* mesh)
{
    stb_sb_free(nav->pool);
    stb_sb_free(nav->closed);
    
    nav->pool = NULL;
    nav->closed = NULL;
    
    stb_sb_add(nav->pool, mesh->polyCount);
    stb_sb_add(nav->closed, mesh->polyCount);
}


/* A* path finding */
int NavSolver_Solve(NavSolver* nav,
                    const NavMesh* mesh,
                    Vec3 startPoint,
                    Vec3 endPoint,
                    const NavPoly* startPoly,
                    const NavPoly* endPoly,
                    NavPath* path)
{
    assert(nav);
    assert(mesh);
    
    if (!startPoly || !endPoly || mesh->polyCount < 1)
        return 0;
    
    NavPath_Clear(path);
    
    // reset count to 0
    stb__sbn(nav->pool) = 0;
    memset(nav->closed, 0, sizeof(char) * stb__sbn(nav->closed));
    
    assert(stb__sbn(nav->closed) == mesh->polyCount);
    
    struct NavSearchNode* first = stb_sb_add(nav->pool, 1);
    first->next = -1;
    first->parent = -1;
    first->polyIndex = startPoly->index;
    first->edgeIndex = -1;
    first->cost = 0.0f;

    nav->head = 0;
    
    while (nav->head != -1)
    {
        // pull the first (lowest cost) node from the list
        struct NavSearchNode* currentNode = nav->pool + nav->head;
        
        Vec3 currentPoint = startPoint;
        if (currentNode->edgeIndex != -1)
        {
            const NavEdge* edge = mesh->edges + currentNode->edgeIndex;
            currentPoint = Vec3_Lerp(mesh->vertices[edge->vertices[0]], mesh->vertices[edge->vertices[1]], 0.5f);
        }
        
        // move from open list to closed
        nav->head = currentNode->next;
        currentNode->next = -1;
        nav->closed[currentNode->polyIndex] = 1;

        // we found target!
        if (currentNode->polyIndex == endPoly->index)
        {
            // construct a path from end to start, leave off start
            NavPath temp;
            NavPath_Init(&temp);
            
            NavPath_AddNode(&temp, endPoint, -1, endPoly->index);
            
            const struct NavSearchNode* i = currentNode;
            
            while (i->parent != -1)
            {
                Vec3 point = startPoint;
                
                if (i->edgeIndex != -1)
                {
                    const NavEdge* edge = mesh->edges + currentNode->edgeIndex;
                    point = Vec3_Lerp(mesh->vertices[edge->vertices[0]], mesh->vertices[edge->vertices[1]], 0.5f);
                }
                
                NavPath_AddNode(&temp, Vec3_Zero, i->edgeIndex, i->polyIndex);
                i = nav->pool + i->parent;
            }
            
            NavPath_Reverse(&temp, path);
            
            return 1;
        }
        
        // traverse neighbor connections
        const NavPoly* currentPoly = mesh->polys + currentNode->polyIndex;
        
        for (int i = 0; i < currentPoly->edgeCount; ++i)
        {
            const NavEdge* edge = mesh->edges + currentPoly->edgeStart + i;
            
            if (edge->neighborIndex == -1)
                continue;
            
            const NavPoly* neighbor = mesh->polys + edge->neighborIndex;
            
            if (nav->closed[neighbor->index] == 1)
                continue; // this node has already been evaluated
            
            // find the node if it is already in the open list
            struct NavSearchNode* toInsert = NULL;
            
            if (nav->head != -1)
            {
                struct NavSearchNode* node = nav->pool + nav->head;
                struct NavSearchNode* previous = NULL;
                
                while (node)
                {
                    if (node->polyIndex == neighbor->index)
                    {
                        toInsert = node;
                        
                        if (previous)
                        {
                            previous->next = toInsert->next;
                        }
                        else
                        {
                            nav->head = toInsert->next;
                        }
                        break;
                    }
                    
                    previous = node;
                    
                    if (node->next != -1)
                    {
                        node = nav->pool + node->next;
                    }
                    else
                    {
                        node = NULL;
                    }
                }
            }

            // if we didn't find a slot, allocate from the pool
            if (!toInsert)
                toInsert = stb_sb_add(nav->pool, 1);
            
            Vec3 edgeCenter = Vec3_Lerp(mesh->vertices[edge->vertices[0]], mesh->vertices[edge->vertices[1]], 0.5f);
            
            float heuristic = Vec3_Dist(edgeCenter, endPoint) * 1.5f;
            toInsert->cost = currentNode->cost + Vec3_Dist(edgeCenter, currentPoint) + heuristic;

            toInsert->polyIndex = neighbor->index;
            toInsert->edgeIndex = currentPoly->edgeStart + i;
            toInsert->parent = (int)(currentNode - nav->pool);
            toInsert->next = -1;

            int toInsertIndex = (int)(toInsert - nav->pool);

            // sorted insertion
            if (nav->head == -1)
            {
                nav->head = toInsertIndex;
            }
            else
            {
                struct NavSearchNode* node = nav->pool + nav->head;
                struct NavSearchNode* previous = NULL;
                
                int inserted = 0;
                
                while (node)
                {
                    if (toInsert->cost < node->cost)
                    {
                        toInsert->next = (int)(node - nav->pool);
                        
                        if (previous)
                        {
                            previous->next = toInsertIndex;
                        }
                        else
                        {
                            nav->head = toInsertIndex;
                        }
                        inserted = 1;
                        break;
                    }
                    
                    previous = node;
                    
                    if (node->next != -1)
                    {
                        node = nav->pool + node->next;
                    }
                    else
                    {
                        node = NULL;
                    }
                }
                
                if (!inserted)
                {
                    assert(previous);
                    previous->next = toInsertIndex;
                }
            }
        }
    }

    return 0;
}


/* http://digestingduck.blogspot.com/2010/03/simple-stupid-funnel-algorithm.html */
/* http://www.koffeebird.com/2014/05/towards-modified-simple-stupid-funnel.html */

void NavSolver_SmoothPath(const NavMesh* mesh, NavPath* path, Vec3 start, Vec3 dest, float radius)
{
    Vec3 portals[path->nodeCount * 2 + 4];
    int pk = 0;
    
    portals[pk] = start;
    portals[pk+1] = start;
    pk += 2;
    
    for (int i = 0; i < path->nodeCount - 1; ++i)
    {
        // line edge intersection to find the correct edge
        const NavEdge* edge = mesh->edges + path->nodes[i].edgeIndex;
        
        Vec3 a = mesh->vertices[edge->vertices[0]];
        Vec3 b = mesh->vertices[edge->vertices[1]];

        Vec3 edgeVec = Vec3_Norm(Vec3_Sub(a, b));
        
        portals[pk] = Vec3_Add(b, Vec3_Scale(edgeVec, radius));
        portals[pk+1] = Vec3_Sub(a, Vec3_Scale(edgeVec, radius));
        
        pk += 2;
    }
    
    portals[pk] = dest;
    portals[pk + 1] = dest;
    pk+=2;
    
    int npts = 0;
    Vec3 portalApex, portalLeft, portalRight;
    int apexIndex = 0, leftIndex = 0, rightIndex = 0;
    
    portalApex = portals[0];
    portalLeft = portals[0];
    portalRight = portals[1];
    
    for (int i = 1; i <= path->nodeCount && npts < pk; ++i)
    {
        Vec3* left = portals + i * 2;
        Vec3* right = portals + i * 2 + 1;
        
        if (TriArea(&portalApex, &portalRight, right) <= 0.0f)
        {
            if (VecEqual(&portalApex, &portalRight) ||
                TriArea(&portalApex, &portalLeft, right) > 0.0f)
            {
                portalRight = *right;
                rightIndex = i;
            }
            else
            {
                path->nodes[npts].position = portalLeft;
                ++npts;
                
                portalApex = portalLeft;
                apexIndex = leftIndex;
                
                portalLeft = portalApex;
                portalRight = portalApex;
                leftIndex = apexIndex;
                rightIndex = apexIndex;
                
                i = apexIndex;
                continue;
            }
        }
        if (TriArea(&portalApex, &portalLeft, left) >= 0.0f)
        {
            if (VecEqual(&portalApex, &portalLeft) ||
                TriArea(&portalApex, &portalRight, left) < 0.0f)
            {
                portalLeft = *left;
                leftIndex = i;
            }
            else
            {
                path->nodes[npts].position = portalRight;
                ++npts;
                
                portalApex = portalRight;
                apexIndex = rightIndex;
                portalLeft = portalApex;
                portalRight = portalApex;
                leftIndex = apexIndex;
                rightIndex = apexIndex;
                
                i = apexIndex;
                continue;
            }
        }
    }
    
    path->nodes[npts].position = dest;
    npts++;
    path->nodeCount = npts;
}


