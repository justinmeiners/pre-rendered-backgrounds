
#include "hint.h"

void HintBuffer_Init(HintBuffer* buffer)
{
    buffer->vaoGpuId = 0;
    buffer->vboGpuId = 0;
    buffer->iboGpuId = 0;
    
    buffer->indexCount = 0;
    buffer->vertCount = 0;
}

void HintBuffer_Clear(HintBuffer* buffer)
{
    buffer->indexCount = 0;
    buffer->vertCount = 0;
}

void HintBuffer_PackCircle(HintBuffer* buffer,
                           float radius,
                           Vec3 position,
                           Vec3 color,
                           int slices)
{
    
    /* starting item */
    
    buffer->verts[buffer->vertCount].pos = Vec3_Offset(position, radius, 0.0f, 0.0f);
    buffer->verts[buffer->vertCount].color = color;
    ++buffer->vertCount;
    
    for (int j = 0; j < slices; ++j)
    {
        float deg = (2 * M_PI) / (float)slices * (j + 1);
        
        buffer->verts[buffer->vertCount].pos = Vec3_Create(position.x + cosf(deg) * radius,
                                                       position.y + sinf(deg) * radius,
                                                       position.z);
        
        
        buffer->verts[buffer->vertCount].color = color;
        ++buffer->vertCount;
        
        
        /* index to last vertex */
        buffer->indicies[buffer->indexCount] = buffer->vertCount - 2;

        /* index to current vertex */
        buffer->indicies[buffer->indexCount + 1] = buffer->vertCount - 1;
        buffer->indexCount += 2;
    }
}

void HintBuffer_PackLine(HintBuffer* buffer,
                         const Vec3 start,
                         const Vec3 end,
                         const Vec3 color)
{

    buffer->verts[buffer->vertCount].pos = start;
    buffer->verts[buffer->vertCount].color = color;
    ++buffer->vertCount;
    
    buffer->verts[buffer->vertCount].pos = end;
    buffer->verts[buffer->vertCount].color = color;
    ++buffer->vertCount;
    
    buffer->indicies[buffer->indexCount] = buffer->vertCount - 2;
    buffer->indicies[buffer->indexCount + 1] = buffer->vertCount - 1;

    buffer->indexCount += 2;
}

void HintBuffer_PackAABB(HintBuffer* buffer,
                         AABB aabb,
                         Vec3 color)
{
    
    buffer->verts[buffer->vertCount].pos = Vec3_Create(aabb.min.x, aabb.min.y, aabb.min.z);
    buffer->verts[buffer->vertCount].color = color;
    ++buffer->vertCount;
    
    buffer->verts[buffer->vertCount].pos = Vec3_Create(aabb.max.x, aabb.min.y, aabb.min.z);
    buffer->verts[buffer->vertCount].color = color;
    ++buffer->vertCount;
    
    buffer->verts[buffer->vertCount].pos = Vec3_Create(aabb.max.x, aabb.max.y, aabb.min.z);
    buffer->verts[buffer->vertCount].color = color;
    ++buffer->vertCount;
    
    buffer->verts[buffer->vertCount].pos = Vec3_Create(aabb.min.x, aabb.max.y, aabb.min.z);
    buffer->verts[buffer->vertCount].color = color;
    ++buffer->vertCount;
    
    
    buffer->verts[buffer->vertCount].pos = Vec3_Create(aabb.min.x, aabb.min.y, aabb.max.z);
    buffer->verts[buffer->vertCount].color = color;
    ++buffer->vertCount;
    
    buffer->verts[buffer->vertCount].pos = Vec3_Create(aabb.max.x, aabb.min.y, aabb.max.z);
    buffer->verts[buffer->vertCount].color = color;
    ++buffer->vertCount;
    
    buffer->verts[buffer->vertCount].pos = Vec3_Create(aabb.max.x, aabb.max.y, aabb.max.z);
    buffer->verts[buffer->vertCount].color = color;
    ++buffer->vertCount;
    
    buffer->verts[buffer->vertCount].pos = Vec3_Create(aabb.min.x, aabb.max.y, aabb.max.z);
    buffer->verts[buffer->vertCount].color = color;
    ++buffer->vertCount;
    
    buffer->indicies[buffer->indexCount] = buffer->vertCount - 8;
    buffer->indicies[buffer->indexCount + 1] = buffer->vertCount - 7;
    buffer->indicies[buffer->indexCount + 2] = buffer->vertCount - 7;
    buffer->indicies[buffer->indexCount + 3] = buffer->vertCount - 6;
    buffer->indicies[buffer->indexCount + 4] = buffer->vertCount - 6;
    buffer->indicies[buffer->indexCount + 5] = buffer->vertCount - 5;
    buffer->indicies[buffer->indexCount + 6] = buffer->vertCount - 5;
    buffer->indicies[buffer->indexCount + 7] = buffer->vertCount - 8;
    
    buffer->indicies[buffer->indexCount + 8] = buffer->vertCount - 4;
    buffer->indicies[buffer->indexCount + 9] = buffer->vertCount - 3;
    buffer->indicies[buffer->indexCount + 10] = buffer->vertCount - 3;
    buffer->indicies[buffer->indexCount + 11] = buffer->vertCount - 2;
    buffer->indicies[buffer->indexCount + 12] = buffer->vertCount - 2;
    buffer->indicies[buffer->indexCount + 13] = buffer->vertCount - 1;
    buffer->indicies[buffer->indexCount + 14] = buffer->vertCount - 1;
    buffer->indicies[buffer->indexCount + 15] = buffer->vertCount - 4;
    buffer->indexCount += 16;

}


void Hintbuffer_PackSkel(HintBuffer* buffer,
                         const Skel* skel,
                         Vec3 position,
                         Vec3 boneColor,
                         Vec3 upColor)
{
    /*
    int i;
    for (i = 0; i < skel->jointCount; ++i)
    {
        const SkelJoint* joint = skel->joints + i;
        
        Vec3 up = Vec3_Scale(Quat_MultVec3(&joint->rotation, Vec3_Create(0.0f, 0.0f, 1.0f)), 0.5f);
        
        buffer->verts[buffer->vertCount].pos = Vec3_Add(position, joint->modelHead);
        buffer->verts[buffer->vertCount].color = boneColor;
        ++buffer->vertCount;
        
        buffer->verts[buffer->vertCount].pos = Vec3_Add(position, joint->modelTail);
        buffer->verts[buffer->vertCount].color = boneColor;
        ++buffer->vertCount;
        
        buffer->verts[buffer->vertCount].pos = Vec3_Add(position, Vec3_Add(joint->modelTail, up));
        buffer->verts[buffer->vertCount].color = upColor;
        ++buffer->vertCount;
        
        buffer->indicies[buffer->indexCount] = buffer->vertCount - 3;
        buffer->indicies[buffer->indexCount + 1] = buffer->vertCount - 2;
        buffer->indicies[buffer->indexCount + 2] = buffer->vertCount - 2;
        buffer->indicies[buffer->indexCount + 3] = buffer->vertCount - 1;

        buffer->indexCount += 4;
    }
     */
    
    int i;
    for (i = 0; i < skel->attachPointCount; ++i)
    {
        const SkelAttachPoint* point = skel->attachPoints + i;
        
        Vec3 up = Vec3_Scale(Quat_MultVec3(&point->modelRotation, Vec3_Create(0.0f, 1.0f, 0.0f)), 0.5f);
        
        buffer->verts[buffer->vertCount].pos = Vec3_Add(position, point->modelPosition);
        buffer->verts[buffer->vertCount].color = Vec3_Create(0.0f, 1.0f, 0.0f);
        ++buffer->vertCount;
        
        buffer->verts[buffer->vertCount].pos = Vec3_Add(position, Vec3_Add(point->modelPosition, up));
        buffer->verts[buffer->vertCount].color = Vec3_Create(0.0f, 1.0f, 0.0f);
        ++buffer->vertCount;
        
        buffer->indicies[buffer->indexCount] = buffer->vertCount - 2;
        buffer->indicies[buffer->indexCount + 1] = buffer->vertCount - 1;
        
        buffer->indexCount += 2;

    }
}

void HintBuffer_PackPath(HintBuffer* buffer,
                         const NavPath* path,
                         Vec3 color)
{
    buffer->verts[buffer->vertCount].pos = path->nodes[0].position;
    buffer->verts[buffer->vertCount].color = color;
    ++buffer->vertCount;
    
    int i;
    for (i = 0; i < path->nodeCount; ++i)
    {
        buffer->verts[buffer->vertCount].pos = path->nodes[i].position;
        buffer->verts[buffer->vertCount].color = color;
        ++buffer->vertCount;
        
        buffer->indicies[buffer->indexCount] = buffer->vertCount - 2;
        buffer->indicies[buffer->indexCount + 1] = buffer->vertCount - 1;
        buffer->indexCount += 2;
    }
}

void HintBuffer_PackNav(HintBuffer* buffer,
                        const NavMesh* mesh,
                        Vec3 color)
{
    for (int i = 0; i < mesh->polyCount; ++i)
    {
        const NavPoly* poly = mesh->polys + i;
                
        for (int j = 0; j < poly->edgeCount; ++j)
        {
            const NavEdge* edge = mesh->edges + poly->edgeStart + j;
            
            Vec3 va = mesh->vertices[edge->vertices[0]];
            Vec3 vb = mesh->vertices[edge->vertices[1]];
            
            buffer->verts[buffer->vertCount].pos = va;
            buffer->verts[buffer->vertCount].color = color;
            ++buffer->vertCount;
            
            buffer->verts[buffer->vertCount].pos = vb;
            buffer->verts[buffer->vertCount].color = color;
            ++buffer->vertCount;

            buffer->indicies[buffer->indexCount] = buffer->vertCount - 1;
            buffer->indicies[buffer->indexCount + 1] = buffer->vertCount - 2;
            buffer->indexCount += 2;
        }
    }
}

