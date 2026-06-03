#include <algorithm>

#include "triangle.h"

constexpr float k_a = 0.1f;
constexpr float k_d = 0.8f;
constexpr float k_s = 0.5f;
constexpr float shininess = 32;
constexpr uint32_t light_color = packRGB(100, 255, 100);

constexpr Vec2 kSampleOffsets[kSampleCount] = {
    {0.375f, 0.125f},
    {0.875f, 0.375f},
    {0.125f, 0.625f},
    {0.625f, 0.875f},
};

float interpolate(float weight0, float weight1, float weight2, float val0, float val1, float val2)
{
    return weight0 * val0 + weight1 * val1 + weight2 * val2;
}

Vec3 interpolate(float weight0, float weight1, float weight2, Vec3 vec0, Vec3 vec1, Vec3 vec2)
{
    return vec0 * weight0 + vec1 * weight1 + vec2 * weight2;
}

// Placeholder fragment shader. Phase 4A will replace this with a JIT-compiled function
// driven by user-written shader source; for now it's a hardcoded Phong model.
// View direction is approximated as {0, 0, 1} — see note in CLAUDE.md.
Vec3 phongShade(Vec3 normal, Vec3 albedo, Vec3 light_dir)
{
    Vec3 n = normal.normalize();
    float n_l_dot = n.dot(light_dir);
    float d_factor = k_d * std::max(0.f, n_l_dot);
    Vec3 R = n * 2 * n_l_dot - light_dir;
    float s_factor = k_s * std::pow(std::max(0.f, R.dot({0, 0, 1})), shininess);

    Vec3 light = unpackRGBf(light_color);
    // Albedo modulates ambient + diffuse; specular reflects light color directly.
    float ambient_diffuse = k_a + d_factor;
    Vec3 color = {
        albedo.x * ambient_diffuse * light.x + s_factor * light.x,
        albedo.y * ambient_diffuse * light.y + s_factor * light.y,
        albedo.z * ambient_diffuse * light.z + s_factor * light.z,
    };

    return {
        std::clamp(color.x, 0.f, 1.f),
        std::clamp(color.y, 0.f, 1.f),
        std::clamp(color.z, 0.f, 1.f),
    };
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

    Vec3 albedo0 = unpackRGBf(c0);
    Vec3 albedo1 = unpackRGBf(c1);
    Vec3 albedo2 = unpackRGBf(c2);

    // Each edge function returns twice the signed area of the sub-triangle
    // formed by that edge and P. All three positive means P is inside.
    auto edge = [&](float p_x, float p_y)
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
            bool coverage[kSampleCount]{};
            bool covered = false;
            for (size_t i = 0; i < kSampleCount; i++)
            {
                auto vals = edge(x + kSampleOffsets[i].x, y + kSampleOffsets[i].y);
                // >= 0 (not > 0) so pixels exactly on a shared edge get drawn by both
                // triangles — prevents visible cracks. The proper fix is the top-left rule.
                if (vals.e01 * sign >= 0 && vals.e12 * sign >= 0 && vals.e20 * sign >= 0)
                {
                    coverage[i] = true;
                    covered = true;
                }
            }
            if (!covered)
                continue;

            auto vals = edge(x + 0.5f, y + 0.5f);

            // Barycentric weights from sub-triangle areas (each weight opposite to its vertex).
            float weight2 = vals.e01 / area;
            float weight0 = vals.e12 / area;
            float weight1 = vals.e20 / area;

            // Perspective-correct interpolation: screen-space barycentric weights are
            // distorted by perspective, so for world-space quantities (normals, colors)
            // we interpolate (attr * 1/w) and divide by interpolated 1/w to undo the distortion.
            float inv_w_interp = interpolate(weight0, weight1, weight2, p0.inv_w, p1.inv_w, p2.inv_w);
            float pw0 = weight0 * p0.inv_w;
            float pw1 = weight1 * p1.inv_w;
            float pw2 = weight2 * p2.inv_w;

            Vec3 albedo = interpolate(pw0, pw1, pw2, albedo0, albedo1, albedo2) / inv_w_interp;
            Vec3 normal = interpolate(pw0, pw1, pw2, n0, n1, n2) / inv_w_interp;
            // z is already in NDC space (post-perspective-divide), which is naturally
            // linear in screen space — no perspective correction needed here.
            float z = interpolate(weight0, weight1, weight2, p0.z, p1.z, p2.z);

            Vec3 color = phongShade(normal, albedo, light_dir);
            for (size_t i = 0; i < kSampleCount; i++)
            {
                if (coverage[i])
                    fb.setSample(x, y, i, z, packRGBf(color));
            }
        }
    }
}
