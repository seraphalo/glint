#include <SDL2/SDL.h>

#include <cstdint>
#include <cstdio>
#include <vector>

#include "rasterizer/framebuffer.h"
#include "rasterizer/triangle.h"
#include "rasterizer/mat4.h"
#include "rasterizer/pipeline.h"
#include "rasterizer/vec.h"

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
    fb.clear(0xFF202030u);
    fb.clearDepth();

    // Camera:
    Mat4 view = lookAt({0, 0, 3}, {0, 0, 0}, {0, 1, 0});
    Mat4 proj = perspective(M_PI / 3.0f, (float)kWidth / kHeight, 0.1f, 100.0f);
    Mat4 mvp = proj * view;
    // Red triangle — closer:
    Vec3 r0 = {-0.6f, -0.4f, -0.5f};
    Vec3 r1 = {0.6f, -0.4f, -0.5f};
    Vec3 r2 = {0.0f, 0.6f, -0.5f};
    // Blue triangle — further, offset to overlap:
    Vec3 b0 = {-0.3f, -0.6f, -1.5f};
    Vec3 b1 = {0.9f, -0.6f, -1.5f};
    Vec3 b2 = {0.3f, 0.4f, -1.5f};

    auto pr0 = projectVertex(r0, mvp, kWidth, kHeight);
    auto pr1 = projectVertex(r1, mvp, kWidth, kHeight);
    auto pr2 = projectVertex(r2, mvp, kWidth, kHeight);

    auto pb0 = projectVertex(b0, mvp, kWidth, kHeight);
    auto pb1 = projectVertex(b1, mvp, kWidth, kHeight);
    auto pb2 = projectVertex(b2, mvp, kWidth, kHeight);

    if (!pr0.clipped && !pr1.clipped && !pr2.clipped)
        drawTriangle(fb,
                     pr0.screen, pr0.z, packRGB(255, 80, 80),
                     pr1.screen, pr1.z, packRGB(255, 80, 80),
                     pr2.screen, pr2.z, packRGB(255, 80, 80));

    if (!pb0.clipped && !pb1.clipped && !pb2.clipped)
        drawTriangle(fb,
                     pb0.screen, pb0.z, packRGB(80, 80, 255),
                     pb1.screen, pb1.z, packRGB(80, 80, 255),
                     pb2.screen, pb2.z, packRGB(80, 80, 255));

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
        }

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
