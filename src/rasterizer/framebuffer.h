#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <cstdint>
#include <vector>

#include "vec.h"

constexpr uint32_t packRGB(uint8_t r, uint8_t g, uint8_t b)
{
    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

struct RGB
{
    uint8_t r, g, b;
};

constexpr RGB unpackRGB(uint32_t color)
{
    return {
        static_cast<uint8_t>((color >> 16) & 0xFF),
        static_cast<uint8_t>((color >> 8) & 0xFF),
        static_cast<uint8_t>(color & 0xFF)};
}

constexpr Vec3 unpackRGBf(uint32_t color)
{
    auto [r, g, b] = unpackRGB(color);
    return {r / 255.f, g / 255.f, b / 255.f};
}

constexpr uint32_t packRGBf(Vec3 color)
{
    return packRGB(
        static_cast<uint8_t>(color.x * 255.f),
        static_cast<uint8_t>(color.y * 255.f),
        static_cast<uint8_t>(color.z * 255.f));
}

class Framebuffer
{
public:
    Framebuffer(int width, int height);
    void clear(uint32_t color);
    void clearDepth();
    void setPixel(Vec2 pos, float z, uint32_t color);
    const uint32_t *data() const;
    int width() const;
    int height() const;

private:
    int width_;
    int height_;
    std::vector<uint32_t> pixels_;
    std::vector<float> depth_;
};

#endif