#include "raytrace/integrator.hpp"
#include <cmath>
#include <algorithm>
#include <cstdio>

namespace g101::rt {

static inline float clamp01(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }

void render(const Scene& scene, const RenderOptions& opt, g101::Image& out)
{
    const int W = opt.width  > 0 ? opt.width  : scene.width;
    const int H = opt.height > 0 ? opt.height : scene.height;
    const int spp = opt.spp > 0 ? opt.spp : 1;

    out = g101::Image(W, H);

    const float scale          = std::tan(scene.fov * 0.5f / 180.0f * 3.14159265358979f);
    const float aspect         = float(W) / float(H);
    const Vector3f eye_pos = opt.eye;      // A6 原始机位；可用 options 覆盖

#if defined(G101_HAS_OPENMP)
#pragma omp parallel for schedule(dynamic, 1)
#endif
    for (int j = 0; j < H; ++j) {
        for (int i = 0; i < W; ++i) {
            Vector3f color(0.f);
            for (int s = 0; s < spp; ++s) {
                // 亚像素抖动（spp=1 时退化为像素中心）
                float jx = (spp == 1) ? 0.5f : (float(rand()) / RAND_MAX);
                float jy = (spp == 1) ? 0.5f : (float(rand()) / RAND_MAX);

                float x = (2.f * (i + jx) / W - 1.f) * aspect * scale;
float y = (1.f - 2.f * (j + jy) / H) * scale;
if (opt.flip_x)     x = -x;
float z = opt.positive_z ? 1.f : -1.f;
Vector3f dir = normalize(Vector3f(x, y, z));
                color += scene.castRay(Ray(eye_pos, dir), 0);
            }
            color = color / float(spp);

            // gamma（A7 输出用 pow(...,0.6)）
            if (opt.gamma != 1.0f) {
                color.x = std::pow(clamp01(color.x), opt.gamma);
                color.y = std::pow(clamp01(color.y), opt.gamma);
                color.z = std::pow(clamp01(color.z), opt.gamma);
            }

            float rgb[3] = { clamp01(color.x), clamp01(color.y), clamp01(color.z) };
            out.set_float(i, j, rgb);       // y 已经是从上往下，无需 flip
        }
    }
}

} // namespace g101::rt