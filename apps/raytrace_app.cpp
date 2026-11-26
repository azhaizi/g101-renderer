#include "apps.hpp"
#include <string>
#include <cstdio>
#include "core/image.hpp"
#include "raytrace/integrator.hpp"
#include "scene_presets.hpp"

namespace g101 {
bool run_raytrace(const Args& args, bool pathtrace) {
    const std::string name  = args.get("scene", "cornell");
    const std::string asset = args.get("asset-dir", G101_DEFAULT_ASSET_DIR);

    auto sh = make_scene(name, asset);
    if (!sh.scene) { std::fprintf(stderr, "unknown scene: %s\n", name.c_str()); return false; }

    rt::RenderOptions opt;
    opt.width     = args.geti("width",  0);
    opt.height    = args.geti("height", 0);
    opt.spp       = args.geti("spp",    pathtrace ? 64 : 1);
    opt.max_depth = args.geti("depth",  5);
    opt.gamma     = args.getf("gamma",  pathtrace ? 0.6f : 1.0f);

    // 相机由 scene_presets 提供（cornell / bunny 各自不同）
    opt.eye        = sh.eye;
    opt.flip_x     = sh.flip_x;
    opt.positive_z = sh.positive_z;
    if (sh.gamma > 0.0f) opt.gamma = sh.gamma;

    // BVH 划分方式：--bvh naive|sah（默认 naive）
    if (args.get("bvh", "naive") == "sah")
        sh.scene->buildBVH(BVHAccel::SplitMethod::SAH);

    // 把递归深度传给 Scene（castRay 里按 depth > maxDepth 截断）
    sh.scene->maxDepth = opt.max_depth;
    Image img;
    rt::render(*sh.scene, opt, img);
    const std::string out = args.get("out", std::string("results/") + (pathtrace ? "pathtrace/" : "raytrace/") + name + ".png");
    return img.save_png(out);
}
}







