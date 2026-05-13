#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <cstdint>
#include <vector>

#include "vec.h"

inline uint32_t packRGB(uint8_t r, uint8_t g, uint8_t b)
{
    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

struct RGB
{
    uint8_t r, g, b;
};

inline RGB unpackRGB(uint32_t color)
{
    return {
        static_cast<uint8_t>((color >> 16) & 0xFF),
        static_cast<uint8_t>((color >> 8) & 0xFF),
        static_cast<uint8_t>(color & 0xFF)};
}

class Framebuffer
{
public:
    Framebuffer(int width, int height);
    void clear(uint32_t color);
    void setPixel(Vec2 pos, uint32_t color);
    const uint32_t *data() const;
    int width() const;
    int height() const;

private:
    int width_;
    int height_;
    std::vector<uint32_t> pixels_;
};

#endif