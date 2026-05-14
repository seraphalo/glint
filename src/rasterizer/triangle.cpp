#include <algorithm>

#include "triangle.h"

float interpolate(float w0, float w1, float w2, float v0, float v1, float v2)
{
    return w0 * v0 + w1 * v1 + w2 * v2;
}

struct EdgeVals
{
    int e01, e12, e20;
};

void drawTriangle(Framebuffer &fb,
                  Vec2 v0, float z0, uint32_t c0,
                  Vec2 v1, float z1, uint32_t c1,
                  Vec2 v2, float z2, uint32_t c2)
{
    // Bounding box, clamped to framebuffer dimensions
    int min_x = std::max(std::min({v0.x, v1.x, v2.x}), 0);
    int max_x = std::min(std::max({v0.x, v1.x, v2.x}), fb.width() - 1);
    int min_y = std::max(std::min({v0.y, v1.y, v2.y}), 0);
    int max_y = std::min(std::max({v0.y, v1.y, v2.y}), fb.height() - 1);

    // Signed area via shoelace formula (2x actual area, same scale as edge functions).
    // Kept signed so that weights stay positive for both CW and CCW winding.
    int area = v0.x * (v1.y - v2.y) + v1.x * (v2.y - v0.y) + v2.x * (v0.y - v1.y);

    // Unpack vertex colors once outside the pixel loop
    auto [r0, g0, b0] = unpackRGB(c0);
    auto [r1, g1, b1] = unpackRGB(c1);
    auto [r2, g2, b2] = unpackRGB(c2);

    // Each edge function returns twice the signed area of the sub-triangle
    // formed by that edge and P. All three positive means P is inside.
    auto edge = [&](int p_x, int p_y)
    {
        auto e01 = (v1.x - v0.x) * (p_y - v0.y) - (v1.y - v0.y) * (p_x - v0.x);
        auto e12 = (v2.x - v1.x) * (p_y - v1.y) - (v2.y - v1.y) * (p_x - v1.x);
        auto e20 = (v0.x - v2.x) * (p_y - v2.y) - (v0.y - v2.y) * (p_x - v2.x);
        return EdgeVals{e01, e12, e20};
    };

    for (int x = min_x; x <= max_x; x++)
    {
        for (int y = min_y; y <= max_y; y++)
        {
            auto vals = edge(x, y);
            if (vals.e01 > 0 && vals.e12 > 0 && vals.e20 > 0)
            {
                // Each weight is the sub-triangle opposite to that vertex,
                // normalized by total area. Cast to float to avoid integer truncation.
                auto w2 = static_cast<float>(vals.e01) / area; // opposite v2
                auto w0 = static_cast<float>(vals.e12) / area; // opposite v0
                auto w1 = static_cast<float>(vals.e20) / area; // opposite v1

                uint8_t r = static_cast<uint8_t>(interpolate(w0, w1, w2, r0, r1, r2));
                uint8_t g = static_cast<uint8_t>(interpolate(w0, w1, w2, g0, g1, g2));
                uint8_t b = static_cast<uint8_t>(interpolate(w0, w1, w2, b0, b1, b2));
                float z = interpolate(w0, w1, w2, z0, z1, z2);
                fb.setPixel({x, y}, z, packRGB(r, g, b));
            }
        }
    }
}
