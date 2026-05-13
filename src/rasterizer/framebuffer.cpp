#include <cassert>
#include "framebuffer.h"

Framebuffer::Framebuffer(int width, int height) : width_(width), height_(height), pixels_(width * height) {};

void Framebuffer::clear(uint32_t color) {
    for(auto it = pixels_.begin(); it != pixels_.end(); ++it)
        *it = color;
}

void Framebuffer::setPixel(Vec2 pos, uint32_t color) {
    assert(pos.x >= 0 && pos.x < width_ && pos.y >= 0 && pos.y < height_);
    pixels_[pos.y * width_ + pos.x] = color;
}

const uint32_t* Framebuffer::data() const{
    return pixels_.data();
}


int Framebuffer::width() const {
    return width_;
}

int Framebuffer::height() const {
    return height_;
}