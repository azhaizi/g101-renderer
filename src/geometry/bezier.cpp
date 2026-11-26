#include "geometry/bezier.hpp"
#include "core/image.hpp"
#include <vector>
#include <cmath>

namespace g101 {

Vec2f de_casteljau(float t, const std::vector<Vec2f>& cps) {
    std::vector<Vec2f> tmp = cps;
    for (int k = 1; k < (int)cps.size(); ++k)
        for (int i = 0; i + k < (int)cps.size(); ++i)
            tmp[i] = (1 - t) * tmp[i] + t * tmp[i + 1];
    return tmp[0];
}

static void splat(Image& img, float fx, float fy) {
    const int W = img.width(), H = img.height();
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx) {
            int x = int(fx) + dx, y = int(fy) + dy;
            if (x < 0 || y < 0 || x >= W || y >= H) continue;
            float w = (dx == 0 && dy == 0) ? 0.6f : ((dx == 0 || dy == 0) ? 0.25f : 0.10f);
            const uint8_t* cur = img.data() + (size_t(y) * W + x) * 4;
            float v = cur[0] / 255.f + w;
            if (v > 1.f) v = 1.f;
            float rgb[3] = { v, v, v };
            img.set_float(x, y, rgb);
        }
}

void draw_bezier(Image& img, const std::vector<Vec2f>& cps, int samples) {
    if (cps.size() < 2 || samples <= 0) return;
    for (int s = 0; s <= samples; ++s) {
        float t = float(s) / float(samples);
        Vec2f p = de_casteljau(t, cps);
        splat(img, p.x(), p.y());
    }
}

} // namespace g101