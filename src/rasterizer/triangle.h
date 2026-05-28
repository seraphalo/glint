#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "framebuffer.h"
#include "vec.h"
#include "pipeline.h"

void drawTriangle(Framebuffer &fb,
                  const ProjectedVertex &p0, Vec3 n0, uint32_t c0,
                  const ProjectedVertex &p1, Vec3 n1, uint32_t c1,
                  const ProjectedVertex &p2, Vec3 n2, uint32_t c2,
                  Vec3 light_dir);

#endif