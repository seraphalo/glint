#include <cassert>
#include "framebuffer.h"

Framebuffer::Framebuffer(int width, int height) : width_(width), height_(height),
                                                  pixels_(width * height),
                                                  depth_(width * height * kSampleCount),
                                                  samples_(width * height * kSampleCount) {};

void Framebuffer::clear(uint32_t color)
{
    clearSample(color);
    clearDepth();
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void Framebuffer::clearDepth()
{
    std::fill(depth_.begin(), depth_.end(), 1.0f);
}

void Framebuffer::clearSample(uint32_t color)
{
    std::fill(samples_.begin(), samples_.end(), color);
}

void Framebuffer::setPixel(int x, int y, float z, uint32_t color)
{
    assert(x >= 0 && x < width_ && y >= 0 && y < height_);
    assert(z <= 1 && z >= 0);
    int ind = (y * width_ + x) * kSampleCount;
    if (z < depth_[ind])
    {
        depth_[ind] = z;
        pixels_[y * width_ + x] = color;
    }
}

void Framebuffer::setSample(int x, int y, int sample_idx, float z, uint32_t color)
{
    assert(x >= 0 && x < width_ && y >= 0 && y < height_);
    assert(z <= 1 && z >= 0);
    assert(0 <= sample_idx && sample_idx < kSampleCount);
    int ind = (y * width_ + x) * kSampleCount + sample_idx;
    if (z < depth_[ind])
    {
        depth_[ind] = z;
        samples_[ind] = color;
    }
}

void Framebuffer::resolve()
{
    for (size_t i = 0; i < pixels_.size(); i++)
    {
        int ind = i * kSampleCount;
        uint32_t r = 0;
        uint32_t g = 0;
        uint32_t b = 0;
        for (size_t j = 0; j < kSampleCount; j++)
        {
            auto [s_r, s_g, s_b] = unpackRGB(samples_[ind + j]);
            r += s_r;
            g += s_g;
            b += s_b;
        }
        pixels_[i] = packRGB(static_cast<uint8_t>(r / kSampleCount),
                             static_cast<uint8_t>(g / kSampleCount),
                             static_cast<uint8_t>(b / kSampleCount));
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