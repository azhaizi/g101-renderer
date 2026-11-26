#pragma once
#include "raytrace/scene.hpp"
#include "core/image.hpp"

namespace g101::rt {

struct RenderOptions {
    int   width     = 0;      // 0 = 用 scene 的
    int   height    = 0;
    int   spp       = 1;
    int   max_depth = 5;
    float gamma     = 1.0f;

    Vector3f eye      { -1, 5, 10 };   // 相机位置（Cornell 用 278,273,-800）
    bool     flip_x      = false;      // A7 Cornell 约定：x 取负
    bool     positive_z  = false;      // A7 Cornell 约定：z 取正
};

void render(const Scene& scene, const RenderOptions& opt, g101::Image& out);

} // namespace g101::rt