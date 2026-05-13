#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "framebuffer.h"

void drawTriangle(Framebuffer &fb,
                  int x0, int y0,
                  int x1, int y1,
                  int x2, int y2,
                  uint32_t color);

#endif