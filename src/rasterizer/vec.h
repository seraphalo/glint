#ifndef VEC_H
#define VEC_H

struct Vec2
{
    float x, y;
};

struct Vec3
{
    float x, y, z;
    Vec3 operator+(const Vec3 &o) const;
    Vec3 operator-(const Vec3 &o) const;
    Vec3 operator*(const float k) const;
    Vec3 operator/(const float k) const;
    Vec3 cross(const Vec3 &o) const;
    Vec3 normalize() const;
    float dot(const Vec3 &o) const;
};

struct Vec4
{
    float x, y, z, w;
};

#endif