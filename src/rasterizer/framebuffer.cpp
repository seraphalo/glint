#include <cassert>
#include "framebuffer.h"

Framebuffer::Framebuffer(int width, int height) : width_(width), height_(height), pixels_(width * height) {};

void Framebuffer::clear(uint32_t color) {
    for(auto it = pixels_.begin(); it != pixels_.end(); ++it)
        *it = color;
}

void Framebuffer::setPixel(int x, int y, uint32_t color) {
    assert(x >= 0 && x < width_ && y >= 0 && y < height_);
    pixels_[y * width_ + x] = color;
}

const uint32_t* Framebuffer::data() const{
    return pixels_.data();
}