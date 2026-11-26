// image.cpp
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#include "core/image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
// image.cpp 顶部
#include "core/image.hpp"
#include <cstring>    // memcpy
#include <cstdlib>    // abs
#include <cmath>
#include <filesystem>   // 顶部


namespace g101 {
static inline uint8_t to8(float v) {
    v = v < 0.f ? 0.f : (v > 1.f ? 1.f : v);
    return uint8_t(v * 255.f + 0.5f);
}
void Image::set(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (x < 0 || y < 0 || x >= w_ || y >= h_) return;
    size_t i = (size_t(y) * w_ + x) * 4;
    data_[i] = r; data_[i + 1] = g; data_[i + 2] = b; data_[i + 3] = a;
}
void Image::set_float(int x, int y, const float c[3]) { set(x, y, to8(c[0]), to8(c[1]), to8(c[2])); }
void Image::fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    for (size_t i = 0; i < data_.size(); i += 4) { data_[i]=r; data_[i+1]=g; data_[i+2]=b; data_[i+3]=a; }
}
Image Image::from_rgb_float(const float* fb, int w, int h, int ch, bool flip_y) {
    Image img(w, h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            int sy = flip_y ? (h - 1 - y) : y;
            img.set_float(x, y, fb + (size_t(sy) * w + x) * ch);
        }
    return img;
}

bool Image::save_png(const std::string& path) const {
    std::error_code ec;
    auto parent = std::filesystem::path(path).parent_path();
    if (!parent.empty()) std::filesystem::create_directories(parent, ec);
    return stbi_write_png(path.c_str(), w_, h_, 4, data_.data(), w_ * 4) != 0;
}
Image Image::load(const std::string& path) {
    int w = 0, h = 0, ch = 0;
    stbi_uc* pixels = stbi_load(path.c_str(), &w, &h, &ch, 4);   // 强制 RGBA
    if (!pixels) return Image();
    Image img(w, h);
    std::memcpy(img.data_.data(), pixels, size_t(w) * h * 4);
    stbi_image_free(pixels);
    return img;
}
void Image::draw_line(float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b)
{
    int ix0 = int(x0), iy0 = int(y0), ix1 = int(x1), iy1 = int(y1);
    int dx = std::abs(ix1 - ix0), dy = -std::abs(iy1 - iy0);
    int sx = ix0 < ix1 ? 1 : -1, sy = iy0 < iy1 ? 1 : -1, err = dx + dy;
    for (;;) {
        set(ix0, iy0, r, g, b);
        if (ix0 == ix1 && iy0 == iy1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; ix0 += sx; }
        if (e2 <= dx) { err += dx; iy0 += sy; }
    }
}
} // namespace g101