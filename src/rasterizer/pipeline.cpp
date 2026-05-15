#include "pipeline.h"

ProjectedVertex projectVertex(const Vec3 &pos, const Mat4 &mvp, int width, int height)
{
    // Transform to clip space
    Vec4 clip = mvp * Vec4{pos.x, pos.y, pos.z, 1.0f};

    // w <= 0 means the point is behind the camera
    if (clip.w <= 0.0f)
        return {{0, 0}, 0.0f, true};

    // Perspective divide: clip space -> NDC [-1, 1]
    Vec3 ndc = {clip.x / clip.w, clip.y / clip.w, clip.z / clip.w};

    // Viewport transform: NDC -> screen pixels. Y flipped because screen y goes down.
    ProjectedVertex vertex;
    vertex.screen.x = (ndc.x + 1) * 0.5f * width;
    vertex.screen.y = (1 - ndc.y) * 0.5f * height;
    vertex.z = ndc.z * 0.5f + 0.5f; // remap [-1, 1] -> [0, 1] for depth buffer
    vertex.clipped = false;
    return vertex;
}
