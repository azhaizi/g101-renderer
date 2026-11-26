#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace g101 {

class Image {
public:
    Image() = default;
    Image(int w, int h) : w_(w), h_(h), data_(size_t(w) * h * 4, 0) {}

    int width()  const { return w_; }
    int height() const { return h_; }

    void set(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void set_float(int x, int y, const float rgb[3]);
    void fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    // A2/A3 的 framebuffer 与 A5~A7 的 Vector3f 数组都走这里
    // flip_y：GAMES101 的 framebuffer y 朝上，PNG 朝下
    static Image from_rgb_float(const float* fb, int w, int h, int ch = 3, bool flip_y = true);

    template <class V>
    static Image from_vec3f(const V* fb, int w, int h, bool flip_y = true) {
        Image img(w, h);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) {
                const V& c = fb[(flip_y ? (h - 1 - y) : y) * w + x];
                const float rgb[3] = { float(c.x), float(c.y), float(c.z) };
                img.set_float(x, y, rgb);
            }
        return img;
    }

    bool save_png(const std::string& path) const;
    static Image load(const std::string& path);
    void draw_line(float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b);
    const uint8_t* data() const { return data_.data(); }
    uint8_t*       data()       { return data_.data(); }

private:
    int w_ = 0, h_ = 0;
    std::vector<uint8_t> data_;
};

} // namespace g101