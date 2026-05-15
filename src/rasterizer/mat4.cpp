#include <cmath>
#include <cstddef>

#include "mat4.h"

Mat4 Mat4::operator*(const Mat4 &o) const
{
    Mat4 result;
    for (size_t i = 0; i < 4; i++)
        for (size_t j = 0; j < 4; j++)
        {
            result.m[i][j] = 0.f;
            for (size_t k = 0; k < 4; k++)
                result.m[i][j] += m[i][k] * o.m[k][j];
        }
    return result;
}

Vec4 Mat4::operator*(const Vec4 &v) const
{
    Vec4 result;
    auto dot = [&](int row_i)
    { return m[row_i][0] * v.x +
             m[row_i][1] * v.y +
             m[row_i][2] * v.z +
             m[row_i][3] * v.w; };

    result.x = dot(0);
    result.y = dot(1);
    result.z = dot(2);
    result.w = dot(3);

    return result;
}

Mat4 Mat4::zero()
{
    Mat4 zero;
    for (size_t i = 0; i < 4; i++)
        for (size_t j = 0; j < 4; j++)
            zero.m[i][j] = 0;
    return zero;
}

Mat4 Mat4::identity()
{
    Mat4 identity = Mat4::zero();
    for (size_t i = 0; i < 4; i++)
        identity.m[i][i] = 1;
    return identity;
}

// Right-handed perspective matrix. Maps camera-space frustum to NDC [-1, 1].
// m[3][2] = -1 puts -z into w, so the perspective divide (÷w) produces foreshortening.
// m[2][2] and m[2][3] remap z from [-near, -far] to [-1, 1].
Mat4 perspective(float fov_y, float aspect, float near, float far)
{
    Mat4 result = Mat4::zero();
    float f = 1.0f / std::tan(fov_y / 2.0f);

    result.m[0][0] = f / aspect;
    result.m[1][1] = f;
    result.m[2][2] = -(far + near) / (far - near);
    result.m[2][3] = -2.0f * far * near / (far - near);
    result.m[3][2] = -1.0f;
    return result;
}

// Builds a view matrix from camera position and target.
// Rows encode the camera's right/up/forward axes in world space.
// Last column encodes translation as dot products with eye.
Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up)
{
    Mat4 result = Mat4::identity();

    Vec3 forward = (center - eye).normalize();
    Vec3 right = forward.cross(up).normalize();
    Vec3 upp = right.cross(forward); // recomputed up — orthogonal to both axes

    // first row
    result.m[0][0] = right.x;
    result.m[0][1] = right.y;
    result.m[0][2] = right.z;
    result.m[0][3] = -right.dot(eye);

    // second row
    result.m[1][0] = upp.x;
    result.m[1][1] = upp.y;
    result.m[1][2] = upp.z;
    result.m[1][3] = -upp.dot(eye);

    // third row
    result.m[2][0] = -forward.x;
    result.m[2][1] = -forward.y;
    result.m[2][2] = -forward.z;
    result.m[2][3] = forward.dot(eye);

    return result;
}

Mat4 translate(Vec3 t)
{
    Mat4 result = Mat4::identity();
    result.m[0][3] = t.x;
    result.m[1][3] = t.y;
    result.m[2][3] = t.z;
    return result;
}

Mat4 rotateY(float angle)
{
    Mat4 result = Mat4::identity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    result.m[0][0] =  c;
    result.m[0][2] =  s;
    result.m[2][0] = -s;
    result.m[2][2] =  c;
    return result;
}

Mat4 scale(Vec3 s)
{
    Mat4 result = Mat4::identity();
    result.m[0][0] = s.x;
    result.m[1][1] = s.y;
    result.m[2][2] = s.z;
    return result;
}
