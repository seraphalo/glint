#include <algorithm>

#include "triangle.h"

constexpr float k_a = 0.1f;
constexpr float k_d = 0.8f;
constexpr float k_s = 0.5f;
constexpr float shininess = 32;
constexpr uint32_t light_color = packRGB(100, 255, 100);

float interpolate(float weight0, float weight1, float weight2, float val0, float val1, float val2)
{
    return weight0 * val0 + weight1 * val1 + weight2 * val2;
}

Vec3 interpolate(float weight0, float weight1, float weight2, Vec3 vec0, Vec3 vec1, Vec3 vec2)
{
    return vec0 * weight0 + vec1 * weight1 + vec2 * weight2;
}

struct EdgeVals
{
    float e01, e12, e20;
};

void drawTriangle(Framebuffer &fb,
                  const ProjectedVertex &p0, Vec3 n0, uint32_t c0,
                  const ProjectedVertex &p1, Vec3 n1, uint32_t c1,
                  const ProjectedVertex &p2, Vec3 n2, uint32_t c2,
                  Vec3 light_dir)
{
    // Bounding box, clamped to framebuffer dimensions
    int min_x = std::max(std::floor(std::min({p0.screen.x, p1.screen.x, p2.screen.x})), 0.f);
    int max_x = std::min(std::ceil(std::max({p0.screen.x, p1.screen.x, p2.screen.x})), fb.width() - 1.f);
    int min_y = std::max(std::floor(std::min({p0.screen.y, p1.screen.y, p2.screen.y})), 0.f);
    int max_y = std::min(std::ceil(std::max({p0.screen.y, p1.screen.y, p2.screen.y})), fb.height() - 1.f);

    // Signed area via shoelace formula (2x actual area, same scale as edge functions).
    // Kept signed so that weights stay positive for both CW and CCW winding.
    float area = p0.screen.x * (p1.screen.y - p2.screen.y) +
                 p1.screen.x * (p2.screen.y - p0.screen.y) +
                 p2.screen.x * (p0.screen.y - p1.screen.y);
    // Skip degenerate and sliver triangles — dividing by tiny area amplifies float error
    // and the UV sphere produces slivers at the poles where vertices collapse.
    if (std::abs(area) < 1e-3f)
        return;
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
        auto e01 = (p1.screen.x - p0.screen.x) * (p_y - p0.screen.y) -
                   (p1.screen.y - p0.screen.y) * (p_x - p0.screen.x);
        auto e12 = (p2.screen.x - p1.screen.x) * (p_y - p1.screen.y) -
                   (p2.screen.y - p1.screen.y) * (p_x - p1.screen.x);
        auto e20 = (p0.screen.x - p2.screen.x) * (p_y - p2.screen.y) -
                   (p0.screen.y - p2.screen.y) * (p_x - p2.screen.x);
        return EdgeVals{e01, e12, e20};
    };

    for (int x = min_x; x <= max_x; x++)
    {
        for (int y = min_y; y <= max_y; y++)
        {
            auto vals = edge(x, y);
            // >= 0 (not > 0) so pixels exactly on a shared edge get drawn by both
            // triangles — prevents visible cracks. The proper fix is the top-left rule.
            if (vals.e01 * sign >= 0 && vals.e12 * sign >= 0 && vals.e20 * sign >= 0)
            {
                // Each weight is the sub-triangle opposite to that vertex,
                // normalized by total area. Cast to float to avoid integer truncation.
                auto weight2 = static_cast<float>(vals.e01) / area; // opposite v2
                auto weight0 = static_cast<float>(vals.e12) / area; // opposite v0
                auto weight1 = static_cast<float>(vals.e20) / area; // opposite v1

                // Perspective-correct interpolation: screen-space barycentric weights are
                // distorted by perspective, so for world-space quantities (normals, colors)
                // we interpolate (attr * 1/w) and divide by interpolated 1/w to undo the distortion.
                float inv_w_interp = interpolate(weight0, weight1, weight2, p0.inv_w, p1.inv_w, p2.inv_w);

                float surface_r = interpolate(weight0 * p0.inv_w, weight1 * p1.inv_w, weight2 * p2.inv_w,
                                              r0, r1, r2) /
                                  inv_w_interp;
                float surface_g = interpolate(weight0 * p0.inv_w, weight1 * p1.inv_w, weight2 * p2.inv_w,
                                              g0, g1, g2) /
                                  inv_w_interp;
                float surface_b = interpolate(weight0 * p0.inv_w, weight1 * p1.inv_w, weight2 * p2.inv_w,
                                              b0, b1, b2) /
                                  inv_w_interp;

                Vec3 n_interp = interpolate(weight0 * p0.inv_w, weight1 * p1.inv_w, weight2 * p2.inv_w,
                                            n0, n1, n2) /
                                inv_w_interp;

                n_interp = n_interp.normalize();
                // z is already in NDC space (post-perspective-divide), which is naturally
                // linear in screen space — no perspective correction needed here.
                float z_interp = interpolate(weight0, weight1, weight2, p0.z, p1.z, p2.z);
                float n_l_dot = n_interp.dot(light_dir);
                float d_factor = k_d * std::max(0.f, n_l_dot);
                Vec3 R = n_interp * 2 * n_l_dot - light_dir;
                // V approximated as {0, 0, 1}; correct V would require interpolating world-space position.
                float s_factor = k_s * std::pow(std::max(0.f, R.dot({0, 0, 1})), shininess);
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

                fb.setPixel(x, y, z_interp, packRGBf({r, g, b}));
            }
        }
    }
}
