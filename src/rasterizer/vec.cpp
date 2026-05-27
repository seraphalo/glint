#include <algorithm>

#include "vec.h"

Vec3 Vec3::operator+(const Vec3 &o) const
{
    Vec3 result;
    result.x = x + o.x;
    result.y = y + o.y;
    result.z = z + o.z;
    return result;
}

Vec3 Vec3::operator-(const Vec3 &o) const
{
    Vec3 result;
    result.x = x - o.x;
    result.y = y - o.y;
    result.z = z - o.z;
    return result;
}

Vec3 Vec3::operator*(const float k) const
{
    Vec3 result;
    result.x = x * k;
    result.y = y * k;
    result.z = z * k;
    return result;
}

Vec3 Vec3::cross(const Vec3 &o) const
{
    Vec3 result;
    result.x = y * o.z - z * o.y;
    result.y = z * o.x - x * o.z;
    result.z = x * o.y - y * o.x;
    return result;
}

Vec3 Vec3::normalize() const
{
    Vec3 result;
    float length = std::sqrt(x * x + y * y + z * z);
    result.x = x / length;
    result.y = y / length;
    result.z = z / length;
    return result;
}

float Vec3::dot(const Vec3 &o) const
{
    return x * o.x + y * o.y + z * o.z;
}