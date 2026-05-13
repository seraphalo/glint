#include <SDL2/SDL.h>

#include <cstdint>
#include <cstdio>
#include <vector>

#include "rasterizer/framebuffer.h"
#include "rasterizer/triangle.h"

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
    drawTriangle(fb,
                 Vec2{400, 100}, packRGB(255, 0, 0),
                 Vec2{700, 500}, packRGB(0, 255, 0),
                 Vec2{100, 500}, packRGB(0, 0, 255));

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
