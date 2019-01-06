
#include "geo_math.h"

int Plane_IntersectRay(Plane plane, Ray3 ray, float* t)
{
    const float d = Vec3_Dot(plane.point, plane.normal);
    
    const float num = Vec3_Dot(plane.normal, ray.origin) - d;
    const float denom = Vec3_Dot(plane.normal, ray.dir);
    
    if (fabsf(denom) < 0.00001f)
    {
        return 0;
    }
    
    *t = -(num / denom);
    
    return 1;
}

AABB AABB_CreateAnchored(Vec3 center, Vec3 size, Vec3 anchor)
{
    Vec3 anchorOffset = Vec3_Mult(anchor, size);
    
    AABB a;
    a.min = Vec3_Sub(center, anchorOffset);
    a.max = Vec3_Sub(Vec3_Add(center, size), anchorOffset);
    return a;
}

AABB AABB_CreateCentered(Vec3 center, Vec3 size)
{
    return AABB_CreateAnchored(center, size, Vec3_Create(0.5f, 0.5f, 0.5f));
}

int AABB_IntersectsAABB(AABB a, AABB b)
{
    for (int i = 0; i < 3; ++i)
    {
        if (Vec3_Get(a.max, i) < Vec3_Get(b.min, i))
            return 0;
        
        if (Vec3_Get(a.min, i) > Vec3_Get(b.max, i))
            return 0;
    }
    
    return 1;
}

int AABB_IntersectsPoint(AABB a, Vec3 p)
{
    for (int i = 0; i < 3; ++i)
    {
        if (Vec3_Get(p, i) < Vec3_Get(a.min, i))
            return 0;
        
        if (Vec3_Get(p, i) > Vec3_Get(a.max, i))
            return 0;
    }
    return 1;
}

int AABB_IntersectsRay(AABB b, Ray3 ray, float* t)
{
    float tmin = -INFINITY, tmax = INFINITY;
    
    if (ray.dir.x != 0.0)
    {
        float tx1 = (b.min.x - ray.origin.x) / ray.dir.x;
        float tx2 = (b.max.x - ray.origin.x) / ray.dir.x;
        
        tmin = MAX(tmin, MIN(tx1, tx2));
        tmax = MIN(tmax, MAX(tx1, tx2));
    }
    
    if (ray.dir.y != 0.0)
    {
        float ty1 = (b.min.y - ray.origin.y) / ray.dir.y;
        float ty2 = (b.max.y - ray.origin.y) / ray.dir.y;
        
        tmin = MAX(tmin, MIN(ty1, ty2));
        tmax = MIN(tmax, MAX(ty1, ty2));
    }
    
    if (ray.dir.z != 0.0)
    {
        float tz1 = (b.min.z - ray.origin.z) / ray.dir.z;
        float tz2 = (b.max.z - ray.origin.z) / ray.dir.z;
        
        tmin = MAX(tmin, MIN(tz1, tz2));
        tmax = MIN(tmax, MAX(tz1, tz2));
    }
    
    *t = tmin;
    
    return tmax >= tmin;
}

int AABB_IntersectsPlane(AABB aabb, Plane plane)
{
    Vec3 p = aabb.min;
    
    if (plane.normal.x >= 0)
        p.x = aabb.max.x;

    if (plane.normal.y >= 0)
        p.y = aabb.max.y;

    if (plane.normal.z >= 0)
        p.z = aabb.max.z;
    
    if (Vec3_Dot(plane.normal, Vec3_Sub(p, plane.point)) < 0.0f)
        return 0;
    
    return 1;
}

Vec3 AABB_Center(AABB a)
{
    Vec3 r;
    r.x = (a.min.x + a.max.x) * 0.5f;
    r.y = (a.min.y + a.max.y) * 0.5f;
    r.z = (a.min.z + a.max.z) * 0.5f;
    return r;
}

int Sphere_IntersectsPoint(Sphere a, Vec3 p)
{
    float rSq = a.radius * a.radius;
    
    Vec3 diff = Vec3_Sub(a.origin, p);
    float distSq = Vec3_Dot(diff, diff);
    
    if (distSq < rSq)
        return 1;
    
    return 0;
}

int Sphere_IntersectsAABB(Sphere a, AABB b)
{
    Vec3 min = Vec3_Sub(b.min, a.origin);
    Vec3 max = Vec3_Sub(b.max, a.origin);
    
    for (int i = 0; i < 3; ++i)
    {
        if (Vec3_Get(max, i) < -a.radius)
            return 0;
        
        if (Vec3_Get(min, i) > a.radius)
            return 0;
    }
    
    return 1;
}

int Sphere_IntersectsSphere(Sphere a, Sphere b)
{
    float rSq = (a.radius + b.radius) * (a.radius + b.radius);
    
    Vec3 diff = Vec3_Sub(a.origin, b.origin);
    float distSq = Vec3_Dot(diff, diff);
    
    if (distSq < rSq)
        return 1;
    
    return 0;
}

int Sphere_IntersectsPlane(Sphere a, Plane p)
{
    if (Plane_SignedDist(p, a.origin) < a.radius)
        return 1;
    
    return 0;
}

int Sphere_IntersectsRay(Sphere a, Ray3 ray, float* t)
{
    float det;
    Vec3 p = Vec3_Sub(ray.origin, a.origin);
    float b = - Vec3_Dot(p, ray.dir);
    det = b * b - Vec3_Dot(p, p) + a.radius * a.radius;
    
    if (det < 0)
    {
        return 0;
    }
    
    det = sqrtf(det);
    float i1 = b - det;
    float i2 = b + det;
    
    if (i2 < 0)
    {
        return 0;
    }
    if (i1 < 0)
    {
        i1 = 0;
    }
    
    *t = i1;
    
    return 1;
}

Mat4 Mat4_CreateShadow(Plane plane, Vec3 lightPoint)
{
    Vec3 light = Vec3_Sub(lightPoint, plane.point);
    
    Mat4 m;
    float dot = Vec3_Dot(plane.normal, light);
    Mat4_Set(&m, 0, 0, dot - light.x * plane.normal.x);
    Mat4_Set(&m, 1, 0, 0.0f - light.x * plane.normal.y);
    Mat4_Set(&m, 2, 0, 0.0f - light.x * plane.normal.z);
    Mat4_Set(&m, 3, 0, 0.0f - light.x * plane.point.z);
    
    Mat4_Set(&m, 0, 1, 0.0f - light.y * plane.normal.x);
    Mat4_Set(&m, 1, 1, dot - light.y * plane.normal.y);
    Mat4_Set(&m, 2, 1, 0.0f - light.y * plane.normal.z);
    Mat4_Set(&m, 3, 1, 0.0f - light.y * plane.point.z);
    
    Mat4_Set(&m, 0, 2, 0.0f - light.z * plane.normal.x);
    Mat4_Set(&m, 1, 2, 0.0f - light.z * plane.normal.y);
    Mat4_Set(&m, 2, 2, dot - light.z * plane.normal.z);
    Mat4_Set(&m, 3, 2, 0.0f - light.z * plane.point.z);
    
    Mat4_Set(&m, 0, 3, 0.0f - plane.normal.x);
    Mat4_Set(&m, 1, 3, 0.0f - plane.normal.y);
    Mat4_Set(&m, 2, 3, 0.0f - plane.normal.z);
    Mat4_Set(&m, 3, 3, dot - plane.point.z);
    return m;
}

void Frustum_Init(Frustum* frustum, float fov, float near, float far)
{
    frustum->fov = fov;
    frustum->near = near;
    frustum->far = far;
    frustum->position = Vec3_Zero;
    frustum->target = Vec3_Create(0.0f, 1.0f, 0.0f);
    frustum->orientation = Vec3_Create(0.0f, 0.0f, 1.0f);
    
    frustum->right = Vec3_Create(1.0f, 0.0f, 0.0f);
    frustum->view = Vec3_Create(0.0f, 1.0f, 0.0f);
    frustum->up = Vec3_Create(0.0f, 0.0f, 1.0f);
    
    frustum->nearHeight = 0;
    
    Mat4_Identity(&frustum->projMatrix);
    Mat4_Identity(&frustum->viewMatrix);
}

void Frustum_UpdateTransform(Frustum* frustum, short viewWidth, short viewHeight)
{
    float aspectRatio = (float)viewWidth / (float)viewHeight;
    float nearHeight = frustum->near * tanf(frustum->fov * 0.5f);
    float nearWidth = nearHeight * aspectRatio;
    
    /* compute matrices */
    frustum->viewMatrix = Mat4_CreateLook(frustum->position, frustum->target, frustum->orientation);
    frustum->projMatrix = Mat4_CreateFrustum(-nearWidth, nearWidth, -nearHeight, nearHeight, frustum->near, frustum->far);
    
    /* determine orientation vectors */
    frustum->view = Vec3_Norm(Vec3_Sub(frustum->position, frustum->target));
    frustum->right = Vec3_Norm(Vec3_Cross(frustum->orientation, frustum->view));
    
    frustum->up = Vec3_Cross(frustum->view, frustum->right);
    
    Vec3 fc = Vec3_Sub(frustum->position, Vec3_Scale(frustum->view, frustum->far));
    Vec3 nc = Vec3_Sub(frustum->position, Vec3_Scale(frustum->view, frustum->near));
    
    Plane* planes = frustum->planes;
    
    planes[kFrustumPlaneNear].normal = Vec3_Negate(frustum->view);
    planes[kFrustumPlaneNear].point = nc;
    
    planes[kFrustumPlaneFar].normal = frustum->view;
    planes[kFrustumPlaneFar].point = fc;
    
    Vec3 aux, aux2, normal;
    
    aux2 = Vec3_Add(nc, Vec3_Scale(frustum->up, nearHeight));
    aux = Vec3_Norm(Vec3_Sub(aux2, frustum->position));
    normal = Vec3_Cross(aux, frustum->right);
    planes[kFrustumPlaneTop].normal = normal;
    planes[kFrustumPlaneTop].point = aux2;
    
    aux2 = Vec3_Sub(nc, Vec3_Scale(frustum->up, nearHeight));
    aux = Vec3_Norm(Vec3_Sub(aux2, frustum->position));
    aux = Vec3_Norm(aux);
    normal = Vec3_Cross(frustum->right, aux);
    planes[kFrustumPlaneBottom].normal = normal;
    planes[kFrustumPlaneBottom].point = aux2;
    
    aux2 = Vec3_Sub(nc, Vec3_Scale(frustum->right, nearWidth));
    aux = Vec3_Norm(Vec3_Sub(aux2, frustum->position));
    normal = Vec3_Cross(aux, frustum->up);
    planes[kFrustumPlaneLeft].normal = normal;
    planes[kFrustumPlaneLeft].point = aux2;
    
    aux2 = Vec3_Add(nc, Vec3_Scale(frustum->right, nearWidth));
    aux = Vec3_Norm(Vec3_Sub(aux2, frustum->position));
    normal = Vec3_Cross(frustum->up, aux);
    planes[kFrustumPlaneRight].normal = normal;
    planes[kFrustumPlaneRight].point = aux2;
    
    frustum->nearHeight = nearHeight;
}

Vec3 Frustum_Unproject(const Frustum* cam, Vec3 normalizedPoint)
{
    Mat4 mvp, inverseMvp;
    Mat4_Mult(&cam->projMatrix, &cam->viewMatrix, &mvp);
    Mat4_Inverse(&mvp, &inverseMvp);
    return Mat4_Unproject(&inverseMvp,  normalizedPoint);
}

const Mat4* Frustum_ViewMatrix(const Frustum* frustum)
{
    return &frustum->viewMatrix;
}

const Mat4* Frustum_ProjMatrix(const Frustum* frustum)
{
    return &frustum->projMatrix;
}

int Frustum_SphereVisible(const Frustum* frustum, Sphere sphere)
{
    for (int i = 0; i < 6; ++i)
    {
        if (Vec3_Dot(frustum->planes[i].normal, Vec3_Sub(sphere.origin, frustum->planes[i].point)) < -sphere.radius)
            return 0;
    }
    return 1;
}

int Frustum_PointVisible(const Frustum* frustum, Vec3 point)
{
    for (int i = 0; i < 6; ++i)
    {
        if (Vec3_Dot(frustum->planes[i].normal, Vec3_Sub(point, frustum->planes[i].point)) < 0)
            return 0;
    }
    return 1;
}

int Frustum_AabbVisible(const Frustum* frustum, AABB aabb)
{
    for (int i = 0; i < 6; ++i)
    {
        if (!AABB_IntersectsPlane(aabb, frustum->planes[i]))
            return 0;
    }
    return 1;
}

/*
 magic PNPOLY - Point Inclusion in Polygon Test
 W. Randolph Franklin (WRF) - http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
 */

int Geo_PointInPoly(int nvert, Vec2* verts, Vec2 test)
{
    int i, j, c = 0;
    for (i = 0, j = nvert - 1; i < nvert; j = i++)
    {
        if (((verts[i].y > test.y) != (verts[j].y > test.y)) &&
            (test.x < (verts[j].x - verts[i].x) * (test.y - verts[i].y) / (verts[j].y - verts[i].y) + verts[i].x))
        {
            c = !c;
        }
    }
    return c;
}

Vec3 Geo_BlendBarcentric(Vec3 p1, Vec3 p2, Vec3 p3, Vec2 i)
{
    Vec3 ret;
    float b0 = (p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) - (p2.y - p1.y);
    
    ret.x = ((p2.x - i.x) * (p3.y - i.y) * (p3.x - i.x) * (p2.y - i.y)) / b0;
    ret.y = ((p3.x - i.x) * (p1.y - i.y) * (p1.x - i.x) * (p3.y - i.y)) / b0;
    ret.z = ((p1.x - i.x) * (p2.y - i.y) * (p2.x - i.x) * (p1.y - i.y)) / b0;
    
    return ret;
}

int Geo_LineSegCross(Vec2 a, Vec2 b, Vec2 c, Vec2 d)
{
    float denominator = ((b.x - a.x) * (d.y - c.y)) - ((b.y - a.y) * (d.x - c.x));
    
    if (denominator == 0)
        return 0;
    
    float numerator1 = ((a.y - c.y) * (d.x - c.x)) - ((a.x - c.x) * (d.y - c.y));
    float numerator2 = ((a.y - c.y) * (b.x - a.x)) - ((a.x - c.x) * (b.y - a.y));
    
    if (numerator1 == 0 || numerator2 == 0)
        return 0;
    
    float r = numerator1 / denominator;
    float s = numerator2 / denominator;
    
    return (r > 0 && r < 1) && (s > 0 && s < 1);
}

