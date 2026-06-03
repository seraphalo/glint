#include <SDL2/SDL.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "rasterizer/framebuffer.h"
#include "rasterizer/mat4.h"
#include "rasterizer/pipeline.h"
#include "rasterizer/triangle.h"
#include "rasterizer/vec.h"

struct Sphere
{
    // Object-space positions; for a unit sphere these double as object-space normals.
    std::vector<Vec3> vertices;
    std::vector<std::array<int, 3>> triangles;
};

Sphere generateSphere(int stacks, int slices)
{
    Sphere sphere;

    // (stacks+1) x (slices+1) grid of vertices in object space.
    for (int i = 0; i <= stacks; i++)
    {
        float theta = M_PI * i / stacks; // 0 = north pole, π = south pole
        float sin_t = std::sin(theta);
        float cos_t = std::cos(theta);
        for (int j = 0; j <= slices; j++)
        {
            float phi = 2.0f * M_PI * j / slices; // 0 → 2π around the equator
            float x = sin_t * std::cos(phi);
            float y = cos_t;
            float z = sin_t * std::sin(phi);
            sphere.vertices.push_back({x, y, z});
        }
    }

    // Two triangles per grid quad.
    int stride = slices + 1;
    for (int i = 0; i < stacks; i++)
    {
        for (int j = 0; j < slices; j++)
        {
            int a = i * stride + j;
            int b = (i + 1) * stride + j;
            int c = i * stride + (j + 1);
            int d = (i + 1) * stride + (j + 1);
            sphere.triangles.push_back({a, b, c});
            sphere.triangles.push_back({b, d, c});
        }
    }

    return sphere;
}

int main(int /*argc*/, char ** /*argv*/)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    constexpr int kWidth = 800;
    constexpr int kHeight = 600;

    SDL_Window *window = SDL_CreateWindow(
        "glint",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        kWidth, kHeight, 0);
    if (!window)
    {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        kWidth, kHeight);

    Framebuffer fb(kWidth, kHeight);

    // Fixed camera and projection — only the model matrix changes per frame
    Mat4 view = lookAt({0, 0, 3}, {0, 0, 0}, {0, 1, 0});
    Mat4 proj = perspective(M_PI / 3.0f, (float)kWidth / kHeight, 0.1f, 100.0f);

    Sphere sphere = generateSphere(16, 32);

    float yaw = 0.0f;

    Vec3 light_dir = Vec3{0, 1, 1}; // from above-right
    light_dir = light_dir.normalize();

    bool running = true;
    while (running)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                running = false;
            // Drag with left mouse button to rotate
            if (e.type == SDL_MOUSEMOTION && (e.motion.state & SDL_BUTTON_LMASK))
                yaw += e.motion.xrel * 0.01f;
        }

        fb.clear(0xFF202030u);

        Mat4 model = rotateY(yaw);
        Mat4 mvp = proj * view * model;

        // Precompute world-space normals and projected vertices once per vertex,
        // so shared vertices aren't transformed multiple times across triangles.
        std::vector<Vec3> world_normals;
        std::vector<ProjectedVertex> projected;
        world_normals.reserve(sphere.vertices.size());
        projected.reserve(sphere.vertices.size());
        for (const Vec3 &v : sphere.vertices)
        {
            // w = 0: normals are directions, not points — translation must not affect them.
            Vec4 n4 = {v.x, v.y, v.z, 0};
            n4 = model * n4;
            world_normals.push_back(Vec3{n4.x, n4.y, n4.z}.normalize());
            projected.push_back(projectVertex(v, mvp, kWidth, kHeight));
        }

        constexpr uint32_t sphere_color = packRGB(200, 200, 220);
        for (const auto &tri : sphere.triangles)
        {
            const ProjectedVertex &p0 = projected[tri[0]];
            const ProjectedVertex &p1 = projected[tri[1]];
            const ProjectedVertex &p2 = projected[tri[2]];
            if (p0.clipped || p1.clipped || p2.clipped)
                continue;

            drawTriangle(fb,
                         p0, world_normals[tri[0]], sphere_color,
                         p1, world_normals[tri[1]], sphere_color,
                         p2, world_normals[tri[2]], sphere_color,
                         light_dir);
        }
        fb.resolve();

        SDL_UpdateTexture(texture, nullptr, fb.data(), kWidth * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
