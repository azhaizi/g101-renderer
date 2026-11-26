#include "apps.hpp"
#include "core/image.hpp"
#include "physics/rope_sim.hpp"
#include <cstdio>
#include <string>
#include <cmath>

namespace g101 {
bool run_rope(const Args& args) {
    const int frames   = args.geti("frames", 60);
    const float dt     = args.getf("dt", 1.0f / 60.0f);
    const int substeps = args.geti("substeps", 64);
    const std::string out = args.get("out", "results/physics/rope");

    // 注意 k 别太大：k=1000 + 1/60 秒 + 1 个 substep 会爆
    phys::Rope rope(0, 200, -400, 200, 16, 1.0f, 200.0f, {0, 15});
    Image canvas(800, 400);

    auto px = [](float x) { return x + 400.0f; };
    auto py = [](float y) { return 400.0f - y; };

    for (int f = 0; f < frames; ++f) {
        const bool use_verlet = (args.get("integrator", "euler") == "verlet");
        for (int s = 0; s < substeps; ++s)
        {
            if (use_verlet) rope.simulateVerlet(dt / float(substeps), phys::Vec2(0, -9.8f));
            else            rope.simulateEuler (dt / float(substeps), phys::Vec2(0, -9.8f));
        }

        canvas.fill(255, 255, 255);
        for (auto& sp : rope.springs)
            canvas.draw_line(px(sp.m1->position.x()), py(sp.m1->position.y()),
                             px(sp.m2->position.x()), py(sp.m2->position.y()), 30, 30, 30);
        for (auto& m : rope.masses) {
            float cx = px(m.position.x()), cy = py(m.position.y());
            if (!std::isfinite(cx) || !std::isfinite(cy)) continue;
            for (int dy = -2; dy <= 2; ++dy)
                for (int dx = -2; dx <= 2; ++dx)
                    if (dx*dx + dy*dy <= 4) canvas.set(int(cx)+dx, int(cy)+dy, 200, 40, 40);
        }
        char name[512];
        std::snprintf(name, sizeof(name), "%s_%04d.png", out.c_str(), f);
        if (!canvas.save_png(name)) return false;
    }
    return true;
}
}

