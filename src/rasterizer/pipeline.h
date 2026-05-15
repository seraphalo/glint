#ifndef PIPELINE_H
#define PIPELINE_H

#include "vec.h"
#include "mat4.h"

struct ProjectedVertex
{
    Vec2 screen;
    float z;
    bool clipped;
};

ProjectedVertex projectVertex(const Vec3 &pos, const Mat4 &mvp, int width, int height);

#endif