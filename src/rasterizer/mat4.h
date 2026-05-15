#ifndef MAT4_H
#define MAT4_H

#include "vec.h"

struct Mat4
{
    float m[4][4];
    Mat4 operator*(const Mat4 &o) const;
    Vec4 operator*(const Vec4 &v) const;
    static Mat4 zero();
    static Mat4 identity();
};

Mat4 perspective(float fov_y, float aspect, float near, float far);
Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up);
Mat4 translate(Vec3 t);
Mat4 rotateY(float angle);
Mat4 scale(Vec3 s);

#endif