#include <cassert>
#include "framebuffer.h"

Framebuffer::Framebuffer(int width, int height) : width_(width), height_(height),
                                                  pixels_(width * height),
                                                  depth_(width * height) {};

void Framebuffer::clear(uint32_t color)
{
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void Framebuffer::clearDepth()
{
    std::fill(depth_.begin(), depth_.end(), 1.0f);
}

void Framebuffer::setPixel(Vec2 pos, float z, uint32_t color)
{
    assert(pos.x >= 0 && pos.x < width_ && pos.y >= 0 && pos.y < height_);
    assert(z <= 1 && z >= 0);
    int ind = pos.y * width_ + pos.x;
    if (z < depth_[ind])
    {
        depth_[ind] = z;
        pixels_[ind] = color;
    }
}

const uint32_t *Framebuffer::data() const
{
    return pixels_.data();
}

int Framebuffer::width() const
{
    return width_;
}

int Framebuffer::height() const
{
    return height_;
}