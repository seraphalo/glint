#include <algorithm>

#include "triangle.h"


void drawTriangle(Framebuffer &fb,
                  int x0, int y0,
                  int x1, int y1,
                  int x2, int y2,
                  uint32_t color) {
    int min_x = std::min({x0, x1, x2});
    int max_x = std::max({x0, x1, x2});
    int min_y = std::min({y0, y1, y2});
    int max_y = std::max({y0, y1, y2});

    min_x = std::max(min_x, 0);
    max_x = std::min(max_x, fb.width() - 1);
    min_y = std::max(min_y, 0);
    max_y = std::min(max_y, fb.height() - 1);

    auto edge = [&](int p_x, int p_y) {
        auto e01 = (x1 - x0) * (p_y - y0) - (y1 - y0) * (p_x - x0);
        auto e12 = (x2 - x1) * (p_y - y1) - (y2 - y1) * (p_x - x1);
        auto e20 = (x0 - x2) * (p_y - y2) - (y0 - y2) * (p_x - x2);
        return e01 >= 0 && e12 >= 0 && e20 >= 0;
    };

    for(int x = min_x; x <= max_x; x++) {
        for (int y = min_y; y <= max_y; y++) {
            if (edge(x, y))
                fb.setPixel(x, y, color);
        }
    }
}