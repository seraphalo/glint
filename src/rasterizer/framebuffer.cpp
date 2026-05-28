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

void Framebuffer::setPixel(int x, int y, float z, uint32_t color)
{
    assert(x >= 0 && x < width_ && y >= 0 && y < height_);
    assert(z <= 1 && z >= 0);
    int ind = y * width_ + x;
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