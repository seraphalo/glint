#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "framebuffer.h"
#include "vec.h"

void drawTriangle(Framebuffer &fb,
                  Vec2 v0, float z0, Vec3 n0, uint32_t c0,
                  Vec2 v1, float z1, Vec3 n1, uint32_t c1,
                  Vec2 v2, float z2, Vec3 n2, uint32_t c2,
                  Vec3 light_dir);

#endif