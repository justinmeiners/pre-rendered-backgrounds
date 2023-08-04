
#include "vec_math.h"
#if __APPLE__
#include <Accelerate/Accelerate.h>
#define APPLE_ACCELERATE 1
#endif // __APPLE__


Mat4 Mat4_CreateIdentity()
{
    Mat4 m;
    Mat4_Identity(&m);
    return m;
}

Mat4 Mat4_CreateTranslate(const Vec3 v)
{
    Mat4 m;
    Mat4_Identity(&m);
    
    Mat4_Set(&m, 3, 0, v.x);
    Mat4_Set(&m, 3, 1, v.y);
    Mat4_Set(&m, 3, 2, v.z);
    return m;
}

Mat4 Mat4_CreateLook(Vec3 eye, Vec3 target, Vec3 up)
{
    Vec3 forward, side, u;
    Mat4 m;
    Mat4_Identity(&m);
    
    forward = Vec3_Sub(target, eye);
    forward = Vec3_Norm(forward);
    side = Vec3_Cross(forward, up);
    side = Vec3_Norm(side);
    
    u = Vec3_Cross(side, forward);
    
    Mat4_Set(&m, 0, 0, side.x);
    Mat4_Set(&m, 1, 0, side.y);
    Mat4_Set(&m, 2, 0, side.z);
    
    Mat4_Set(&m, 0, 1, u.x);
    Mat4_Set(&m, 1, 1, u.y);
    Mat4_Set(&m, 2, 1, u.z);
    
    Mat4_Set(&m, 0, 2, -forward.x);
    Mat4_Set(&m, 1, 2, -forward.y);
    Mat4_Set(&m, 2, 2, -forward.z);
    
    Mat4 translate = Mat4_CreateTranslate(Vec3_Negate(eye));
    
    Mat4_Mult(&m, &translate, &m);
    return m;
}

Mat4 Mat4_CreateFrustum(float left, float right, float bottom, float top, float near, float far)
{
    Mat4 m;
    Mat4_Identity(&m);
    
    float invWidth = 1.0f / (right - left);
    float invHeight = 1.0f / (top - bottom);
    float invDepth = 1.0f / (far - near);
    
    Mat4_Set(&m, 0, 0, 2.0f * near * invWidth);
    Mat4_Set(&m, 1, 1, 2.0f * near * invHeight);
    
    Mat4_Set(&m, 2, 0, (right + left) * invWidth);
    Mat4_Set(&m, 2, 1, (top + bottom) * invHeight);
    Mat4_Set(&m, 2, 2, -(far + near) * invDepth);
    Mat4_Set(&m, 2, 3, -1.0f);
    
    Mat4_Set(&m, 3, 2, - 2.0f * near * far * invDepth);
    Mat4_Set(&m, 3, 3, 0.0f);
    return m;
}

Mat4 Mat4_CreateOrtho(float left,
                      float right,
                      float bottom,
                      float top,
                      float near,
                      float far)
{
    Mat4 m;
    Mat4_Identity(&m);
    
    const float deltaX = right - left;
    const float deltaY = top - bottom;
    const float deltaZ = far - near;
    
    if (deltaX == 0.0f || deltaY == 0.0f || deltaZ == 0.0f)
    {
        return m;
    }
    
    Mat4_Set(&m, 0, 0, 2.0f / deltaX);
    Mat4_Set(&m, 3, 0,  -(right + left) / deltaX);
    Mat4_Set(&m, 1, 1, 2.0f / deltaY);
    Mat4_Set(&m, 3, 1,  -(top + bottom) / deltaY);
    Mat4_Set(&m, 2, 2, -2.0f / deltaZ);
    Mat4_Set(&m, 3, 2,  -(near + far) / deltaZ);
    
    return m;
}

void Mat4_Identity(Mat4* m)
{
    m->m[0] = 1.0f;
    m->m[1] = 0.0f;
    m->m[2] = 0.0f;
    m->m[3] = 0.0f;
    
    m->m[4] = 0.0f;
    m->m[5] = 1.0f;
    m->m[6] = 0.0f;
    m->m[7] = 0.0f;
    
    m->m[8] = 0.0f;
    m->m[9] = 0.0f;
    m->m[10] = 1.0f;
    m->m[11] = 0.0f;
    
    m->m[12] = 0.0f;
    m->m[13] = 0.0f;
    m->m[14] = 0.0f;
    m->m[15] = 1.0f;
}

Vec3 Mat4_MultVec3(const Mat4* m, Vec3 v)
{
    Vec3 r;
    r.x = (v.x * m->m[0]) + (v.y * m->m[4]) + (v.z * m->m[8]) + m->m[12];
    r.y = (v.x * m->m[1]) + (v.y * m->m[5]) + (v.z * m->m[9]) + m->m[13];
    r.z = (v.x * m->m[2]) + (v.y * m->m[6]) + (v.z * m->m[10]) + m->m[14];
    return r;
}

Vec4 Mat4_MultVec4(const Mat4* m, Vec4 v)
{
    Vec4 r;
    r.x = (v.x * m->m[0]) + (v.y * m->m[4]) + (v.z * m->m[8]) + (v.w * m->m[12]);
    r.y = (v.x * m->m[1]) + (v.y * m->m[5]) + (v.z * m->m[9]) + (v.w * m->m[13]);
    r.z = (v.x * m->m[2]) + (v.y * m->m[6]) + (v.z * m->m[10]) + (v.w * m->m[14]);
    r.w = (v.x * m->m[3]) + (v.y * m->m[7]) + (v.z * m->m[11]) + (v.w * m->m[15]);
    return r;
}

void Mat4_Mult(const Mat4* a, const Mat4* b, Mat4* ab)
{
#if APPLE_ACCELERATE
    vDSP_mmul(b->m, 1, a->m, 1, ab->m, 1, 4, 4, 4);
#else
    Mat4 temp;
    
    int i;
    int j;
    int k;
    float n;
    for (i = 0; i < 4; ++i)
    {
        for (j = 0; j < 4; ++j)
        {
            n = 0;
            for (k = 0; k < 4; ++k)
            {
                n += Mat4_Get(b, i, k) * Mat4_Get(a, k, j);
            }
            Mat4_Set(&temp, i, j, n);
        }
    }
    
    Mat4_Copy(ab, &temp);
#endif
}

void Mat4_Transpose(Mat4* a)
{
    int i;
    int j;
    for (i = 0; i < 4; ++i)
    {
		for (j = i + 1; j < 4; ++j)
        {
            float save = Mat4_Get(a, i, j); ;
            Mat4_Set(a, i, j, Mat4_Get(a, j, i));
            Mat4_Set(a, j, i, save);
		}
	}
}

float Mat4_Det(const Mat4* m)
{
    return +Mat4_Get(m, 3, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 2, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 0, 3)
    - Mat4_Get(m, 3, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 0, 3) + Mat4_Get(m, 1, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 0, 3)
    
    + Mat4_Get(m, 2, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 1, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 0, 3)
    - Mat4_Get(m, 3, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 1, 3) + Mat4_Get(m, 2, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 1, 3)
    
    + Mat4_Get(m, 3, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 1, 3) - Mat4_Get(m, 0, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 1, 3)
    - Mat4_Get(m, 2, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 1, 3) + Mat4_Get(m, 0, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 1, 3)
    
    + Mat4_Get(m, 3, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 2, 3) - Mat4_Get(m, 1, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 2, 3)
    - Mat4_Get(m, 3, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 2, 3) + Mat4_Get(m, 0, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 2, 3)
    
    + Mat4_Get(m, 1, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 2, 3) - Mat4_Get(m, 0, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 2, 3)
    - Mat4_Get(m, 2, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 3, 3) + Mat4_Get(m, 1, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 3, 3)
    
    + Mat4_Get(m, 2, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 3, 3) - Mat4_Get(m, 0, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 3, 3)
    - Mat4_Get(m, 1, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 3, 3) + Mat4_Get(m, 0, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 3, 3);
}

void Mat4_Inverse(const Mat4* m, Mat4* r)
{
    Mat4_Set(r, 0, 0, +Mat4_Get(m, 2, 1) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 1, 3) - Mat4_Get(m, 3, 1) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 1, 3) + Mat4_Get(m, 3, 1) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 2, 3)
             - Mat4_Get(m, 1, 1) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 2, 3) - Mat4_Get(m, 2, 1) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 3, 3) + Mat4_Get(m, 1, 1) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 3, 3));
    
    Mat4_Set(r, 1, 0, +Mat4_Get(m, 3, 0) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 1, 3) - Mat4_Get(m, 2, 0) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 1, 3) - Mat4_Get(m, 3, 0) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 2, 3)
             + Mat4_Get(m, 1, 0) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 2, 3) + Mat4_Get(m, 2, 0) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 3, 3) - Mat4_Get(m, 1, 0) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 3, 3));
    
    Mat4_Set(r, 2, 0, +Mat4_Get(m, 2, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 1, 3) - Mat4_Get(m, 3, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 1, 3) + Mat4_Get(m, 3, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 2, 3)
             - Mat4_Get(m, 1, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 2, 3) - Mat4_Get(m, 2, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 3, 3) + Mat4_Get(m, 1, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 3, 3));
    
    Mat4_Set(r, 3, 0, +Mat4_Get(m, 3, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 1, 2) - Mat4_Get(m, 2, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 1, 2) - Mat4_Get(m, 3, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 2, 2)
             + Mat4_Get(m, 1, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 2, 2) + Mat4_Get(m, 2, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 3, 2) - Mat4_Get(m, 1, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 3, 2));
    
    Mat4_Set(r, 0, 1, +Mat4_Get(m, 3, 1) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 2, 1) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 3, 1) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 2, 3)
             + Mat4_Get(m, 0, 1) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 2, 3) + Mat4_Get(m, 2, 1) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 3, 3) - Mat4_Get(m, 0, 1) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 3, 3));
    
    Mat4_Set(r, 1, 1, +Mat4_Get(m, 2, 0) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 3, 0) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 0, 3) + Mat4_Get(m, 3, 0) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 2, 3)
             - Mat4_Get(m, 0, 0) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 2, 3) - Mat4_Get(m, 2, 0) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 3, 3) + Mat4_Get(m, 0, 0) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 3, 3));
    
    Mat4_Set(r, 2, 1, +Mat4_Get(m, 3, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 2, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 3, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 2, 3)
             + Mat4_Get(m, 0, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 2, 3) + Mat4_Get(m, 2, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 3, 3) - Mat4_Get(m, 0, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 3, 3));
    
    Mat4_Set(r, 3, 1, +Mat4_Get(m, 2, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 0, 2) - Mat4_Get(m, 3, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 0, 2) + Mat4_Get(m, 3, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 2, 2)
             - Mat4_Get(m, 0, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 2, 2) - Mat4_Get(m, 2, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 3, 2) + Mat4_Get(m, 0, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 3, 2));
    
    Mat4_Set(r, 0, 2, +Mat4_Get(m, 1, 1) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 3, 1) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 0, 3) + Mat4_Get(m, 3, 1) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 1, 3)
             - Mat4_Get(m, 0, 1) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 1, 3) - Mat4_Get(m, 1, 1) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 3, 3) + Mat4_Get(m, 0, 1) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 3, 3));
    
    Mat4_Set(r, 1, 2, +Mat4_Get(m, 3, 0) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 1, 0) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 3, 0) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 1, 3)
             + Mat4_Get(m, 0, 0) * Mat4_Get(m, 3, 2) * Mat4_Get(m, 1, 3) + Mat4_Get(m, 1, 0) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 3, 3) - Mat4_Get(m, 0, 0) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 3, 3));
    
    Mat4_Set(r, 2, 2, +Mat4_Get(m, 1, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 3, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 0, 3) + Mat4_Get(m, 3, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 1, 3)
             - Mat4_Get(m, 0, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 1, 3) - Mat4_Get(m, 1, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 3, 3) + Mat4_Get(m, 0, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 3, 3));
    
    Mat4_Set(r, 3, 2, +Mat4_Get(m, 3, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 0, 2) - Mat4_Get(m, 1, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 0, 2) - Mat4_Get(m, 3, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 1, 2)
             + Mat4_Get(m, 0, 0) * Mat4_Get(m, 3, 1) * Mat4_Get(m, 1, 2) + Mat4_Get(m, 1, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 3, 2) - Mat4_Get(m, 0, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 3, 2));
    
    Mat4_Set(r, 0, 3, +Mat4_Get(m, 2, 1) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 1, 1) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 2, 1) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 1, 3)
             + Mat4_Get(m, 0, 1) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 1, 3) + Mat4_Get(m, 1, 1) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 2, 3) - Mat4_Get(m, 0, 1) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 2, 3));
    
    Mat4_Set(r, 1, 3, +Mat4_Get(m, 1, 0) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 2, 0) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 0, 3) + Mat4_Get(m, 2, 0) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 1, 3)
             - Mat4_Get(m, 0, 0) * Mat4_Get(m, 2, 2) * Mat4_Get(m, 1, 3) - Mat4_Get(m, 1, 0) * Mat4_Get(m, 0, 2) * Mat4_Get(m, 2, 3) + Mat4_Get(m, 0, 0) * Mat4_Get(m, 1, 2) * Mat4_Get(m, 2, 3));
    
    Mat4_Set(r, 2, 3, +Mat4_Get(m, 2, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 1, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 0, 3) - Mat4_Get(m, 2, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 1, 3)
             + Mat4_Get(m, 0, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 1, 3) + Mat4_Get(m, 1, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 2, 3) - Mat4_Get(m, 0, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 2, 3));
    
    Mat4_Set(r, 3, 3, +Mat4_Get(m, 1, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 0, 2) - Mat4_Get(m, 2, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 0, 2) + Mat4_Get(m, 2, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 1, 2)
             - Mat4_Get(m, 0, 0) * Mat4_Get(m, 2, 1) * Mat4_Get(m, 1, 2) - Mat4_Get(m, 1, 0) * Mat4_Get(m, 0, 1) * Mat4_Get(m, 2, 2) + Mat4_Get(m, 0, 0) * Mat4_Get(m, 1, 1) * Mat4_Get(m, 2, 2));
    
    float det = Mat4_Det(m);
    
    int i;
    for (i = 0; i < 16; ++i)
    {
        r->m[i] = r->m[i] / det;
    }
}

void Mat4_Translate(Mat4* m, Vec3 v)
{
    Mat4_Set(m, 3, 0, Mat4_Get(m, 0, 0) * v.x + Mat4_Get(m, 1, 0) * v.y + Mat4_Get(m, 2, 0) * v.z + Mat4_Get(m, 3, 0));
    Mat4_Set(m, 3, 1, Mat4_Get(m, 0, 1) * v.x + Mat4_Get(m, 1, 1) * v.y + Mat4_Get(m, 2, 1) * v.z + Mat4_Get(m, 3, 1));
    Mat4_Set(m, 3, 2, Mat4_Get(m, 0, 2) * v.x + Mat4_Get(m, 1, 2) * v.y + Mat4_Get(m, 2, 2) * v.z + Mat4_Get(m, 3, 2));
    Mat4_Set(m, 3, 3, Mat4_Get(m, 0, 3) * v.x + Mat4_Get(m, 1, 3) * v.y + Mat4_Get(m, 2, 3) * v.z + Mat4_Get(m, 3, 3));
}

void Mat4_Scale(Mat4* m, Vec3 s)
{
    m->m[0] *= s.x;
    m->m[4] *= s.x;
    m->m[8] *= s.x;
    m->m[12] *= s.x;
    
    m->m[1] *= s.y;
    m->m[5] *= s.y;
    m->m[9] *= s.y;
    m->m[13] *= s.y;
    
    m->m[2] *= s.z;
    m->m[6] *= s.z;
    m->m[10] *= s.z;
    m->m[14] *= s.z;
}

Vec3 Mat4_Unproject(const Mat4* inverseMvp, Vec3 normalizePoint)
{
    Vec4 pointClip = Mat4_MultVec4(inverseMvp, Vec4_Create(normalizePoint.x, normalizePoint.y, normalizePoint.z, 1.0f));
    return Vec3_Scale(Vec3_Create(pointClip.x, pointClip.y, pointClip.z), 1.0f / pointClip.w);
}

Vec3 Mat4_Project(const Mat4* mvp, Vec3 point)
{
    Vec4 point3D = Mat4_MultVec4(mvp, Vec4_Create(point.x, point.y, point.z, 1.0f));
    return Vec3_Scale(Vec3_Create(point3D.x, point3D.y, point3D.z), 1.0f / point3D.w);
}

Quat Quat_CreateLook(Vec3 dir, Vec3 up)
{
    Quat r;
    Vec3 a = Vec3_Cross(up, dir);
    r.x = a.x;
    r.y = a.y;
    r.z = a.z;
    r.w = 1.0f + Vec3_Dot(up, dir);
    return Quat_Norm(r);
}

Quat Quat_Mult(Quat q0, Quat q1)
{
    Quat v;
    v.x = q0.w * q1.x + q0.x * q1.w + q0.y * q1.z - q0.z * q1.y;
    v.y = q0.w * q1.y + q0.y * q1.w + q0.z * q1.x - q0.x * q1.z;
    v.z = q0.w * q1.z + q0.z * q1.w + q0.x * q1.y - q0.y * q1.x;
    v.w = q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z;
    return v;
}

Vec3 Quat_MultVec3(const Quat* q, Vec3 v)
{
    Vec3 u = Vec3_Create(q->x, q->y, q->z);
    
    Vec3 t = Vec3_Scale(Vec3_Cross(u, v), 2.0f);
    
    Vec3 r = Vec3_Add(v, Vec3_Scale(t, q->w));
    r = Vec3_Add(r, Vec3_Cross(u, t));
    
    return r;
    
    /*
     Alternate implementation
     
     Quat vecQuat, resQuat;
     
     vecQuat.x = v.x;
     vecQuat.y = v.y;
     vecQuat.z = v.z;
     vecQuat.w = 0.0f;
     
     resQuat = Quat_Mult(vecQuat, Quat_Negate(*a));
     resQuat = Quat_Mult(*a, resQuat);
     
     return Vec3_Create(resQuat.x, resQuat.y, resQuat.z);
     */
}

Quat Quat_Norm(Quat q)
{
    Quat r = q;
    
    float mag2 = q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;
    
    if (mag2 != 0.0f && (fabsf(mag2 - 1.0f) > 0.000001))
    {
        float mag = 1.0f / sqrtf(mag2);
        r.w = q.w * mag;
        r.x = q.x * mag;
        r.y = q.y * mag;
        r.z = q.z * mag;
    }
    
    return r;
}

Quat Quat_Slerp(Quat q0, Quat q1, float t)
{
    float w1, w2;
    float cosTheta = Quat_Dot(q0, q1);
    float theta = acosf(cosTheta);
    float sinTheta = sinf(theta);
    
    if (sinTheta > 0.0001f)
    {
        w1 = (sinf((1.0f - t) * theta) / sinTheta);
        w2 = (sinf(t * theta) / sinTheta);
    }
    else
    {
        w1 = 1.0f - t;
        w2 = t;
    }
    q0 = Quat_Scale(q0, w1);
    q1 = Quat_Scale(q1, w2);
    
    return  Quat_Add(q0, q1);
}

void Quat_ToMatrix(Quat a, Mat4* dest)
{
    float x2 = 2.0f * a.x,  y2 = 2.0f * a.y,  z2 = 2.0f * a.z;
	
    float xy = x2 * a.y,  xz = x2 * a.z;
    float yy = y2 * a.y,  yw = y2 * a.w;
    float zw = z2 * a.w,  zz = z2 * a.z;
	
    Mat4_Set(dest, 0, 0, 1.0f - (yy + zz));
    Mat4_Set(dest, 1, 0, (xy - zw));
    Mat4_Set(dest, 2, 0, (xz + yw));
    Mat4_Set(dest, 3, 0, 0.0f);

    float xx = x2 * a.x,  xw = x2 * a.w,  yz = y2 * a.z;
    
    Mat4_Set(dest, 0, 1, (xy +  zw));
    Mat4_Set(dest, 1, 1, 1.0f - (xx + zz));
    Mat4_Set(dest, 2, 1, (yz - xw));
    Mat4_Set(dest, 3, 1, 0.0f);
	
    Mat4_Set(dest, 0, 2, (xz - yw));
    Mat4_Set(dest, 1, 2, (yz + xw));
    Mat4_Set(dest, 2, 2, 1.0f - ( xx + yy ));
    Mat4_Set(dest, 3, 2, 0.0f);
    
    Mat4_Set(dest, 0, 3, 0.0f);
    Mat4_Set(dest, 1, 3, 0.0f);
    Mat4_Set(dest, 2, 3, 0.0f);
    Mat4_Set(dest, 3, 3, 1.0f);
}

Quat Quat_CreateEuler(float pitch, float yaw, float roll)
{
    Quat q;
    float p = pitch * 0.5f;
    float y = yaw * 0.5f;
    float r = roll * 0.5f;
    
    float sinp = sinf(p);
    float siny = sinf(y);
    float sinr = sinf(r);
    float cosp = cosf(p);
    float cosy = cosf(y);
    float cosr = cosf(r);
    
    q.x = sinr * cosp * cosy - cosr * sinp * siny;
    q.y = cosr * sinp * cosy + sinr * cosp * siny;
    q.z = cosr * cosp * siny - sinr * sinp * cosy;
    q.w = cosr * cosp * cosy + sinr * sinp * siny;
    
    return Quat_Norm(q);
}

Quat Quat_CreateAngle(float angle, float x, float y, float z)
{
    float halfAngle = angle * M_PI / 360.0f;
    float sinA = sinf(halfAngle);
    
    Quat q;
	q.w = cosf(halfAngle);
	q.x = x * sinA;
	q.y = y * sinA;
	q.z = z * sinA;
	return q;
}

