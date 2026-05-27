#include <algorithm>

#include "triangle.h"

constexpr float k_a = 0.1f;
constexpr float k_d = 0.8f;
constexpr float k_s = 0.5f;
constexpr float shininess = 32;
constexpr uint32_t light_color = packRGB(100, 255, 100);

float interpolate(float w0, float w1, float w2, float v0, float v1, float v2)
{
    return w0 * v0 + w1 * v1 + w2 * v2;
}

Vec3 interpolate(float w0, float w1, float w2, Vec3 n0, Vec3 n1, Vec3 n2)
{
    return n0 * w0 + n1 * w1 + n2 * w2;
}

struct EdgeVals
{
    int e01, e12, e20;
};

void drawTriangle(Framebuffer &fb,
                  Vec2 v0, float z0, Vec3 n0, uint32_t c0,
                  Vec2 v1, float z1, Vec3 n1, uint32_t c1,
                  Vec2 v2, float z2, Vec3 n2, uint32_t c2,
                  Vec3 light_dir)
{
    // Bounding box, clamped to framebuffer dimensions
    int min_x = std::max(std::min({v0.x, v1.x, v2.x}), 0);
    int max_x = std::min(std::max({v0.x, v1.x, v2.x}), fb.width() - 1);
    int min_y = std::max(std::min({v0.y, v1.y, v2.y}), 0);
    int max_y = std::min(std::max({v0.y, v1.y, v2.y}), fb.height() - 1);

    // Signed area via shoelace formula (2x actual area, same scale as edge functions).
    // Kept signed so that weights stay positive for both CW and CCW winding.
    int area = v0.x * (v1.y - v2.y) + v1.x * (v2.y - v0.y) + v2.x * (v0.y - v1.y);
    int sign = area > 0 ? 1 : -1;

    // Unpack vertex colors once outside the pixel loop
    auto [r0, g0, b0] = unpackRGBf(c0);
    auto [r1, g1, b1] = unpackRGBf(c1);
    auto [r2, g2, b2] = unpackRGBf(c2);

    auto [light_r, light_g, light_b] = unpackRGBf(light_color);

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
            if (vals.e01 * sign > 0 && vals.e12 * sign > 0 && vals.e20 * sign > 0)
            {
                // Each weight is the sub-triangle opposite to that vertex,
                // normalized by total area. Cast to float to avoid integer truncation.
                auto w2 = static_cast<float>(vals.e01) / area; // opposite v2
                auto w0 = static_cast<float>(vals.e12) / area; // opposite v0
                auto w1 = static_cast<float>(vals.e20) / area; // opposite v1

                float surface_r = interpolate(w0, w1, w2, r0, r1, r2);
                float surface_g = interpolate(w0, w1, w2, g0, g1, g2);
                float surface_b = interpolate(w0, w1, w2, b0, b1, b2);

                Vec3 n = interpolate(w0, w1, w2, n0, n1, n2);
                n = n.normalize();
                float z = interpolate(w0, w1, w2, z0, z1, z2);
                float n_l_dot = n.dot(light_dir);
                float d_factor = k_d * std::max(0.f, n_l_dot);
                Vec3 R = n * 2 * n_l_dot - light_dir;
                float s_factor = k_s * std::pow(std::max(0.f, R.dot(/* V = */ {0, 0, 1})), shininess);
                // float factor = k_a + d_factor + s_factor;
                float r = surface_r * k_a * light_r +
                          surface_r * d_factor * light_r +
                          s_factor * light_r;
                float g = surface_g * k_a * light_g +
                          surface_g * d_factor * light_g +
                          s_factor * light_g;
                float b = surface_b * k_a * light_b +
                          surface_b * d_factor * light_b +
                          s_factor * light_b;

                r = std::clamp(r, 0.f, 1.f);
                g = std::clamp(g, 0.f, 1.f);
                b = std::clamp(b, 0.f, 1.f);

                fb.setPixel({x, y}, z, packRGBf({r, g, b}));
            }
        }
    }
}
