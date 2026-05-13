#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "framebuffer.h"
#include "vec.h"

void drawTriangle(Framebuffer &fb,
                  Vec2 v0, uint32_t c0,
                  Vec2 v1, uint32_t c1,
                  Vec2 v2, uint32_t c2);

#endif