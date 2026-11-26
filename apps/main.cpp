#include <cstdio>
#include <string>
#include "apps.hpp"
#include "core/timer.hpp"

static void print_help() {
    std::printf(
        "usage: renderer --mode <raster|raytrace|pathtrace|bezier|rope> [options]\n"
        "  --scene <name>       cornell | bunny\n"
        "  --out <path>         output png (rope 模式为帧序列前缀)\n"
        "  --width/--height     override resolution\n"
        "  --spp <n>            samples per pixel (pathtrace 默认 64)\n"
        "  --depth <n>          max recursion depth (默认 5)\n"
        "  --gamma <g>          gamma (pathtrace 默认 0.6)\n"
        "  --bvh <naive|sah>    BVH 划分方式 (默认 naive)\n"
        "  --integrator <euler|verlet>   rope 积分器 (默认 euler)\n"
        "  --angle <deg>        raster 旋转角\n"
        "  --frames <n>         rope 帧数\n"
        "  --dt <s>             rope 每帧时长 (默认 1/60)\n"
        "  --substeps <n>       rope 每帧子步 (默认 64)\n"
        "  --asset-dir <dir>    assets root (default: G101_DEFAULT_ASSET_DIR)\n");
}

int main(int argc, char** argv) {
    g101::Args args(argc, argv);
    if (argc == 1 || args.has("help")) { print_help(); return 0; }

    const std::string mode = args.get("mode", "raytrace");
    g101::Timer t; t.start();

    bool ok = false;
    if      (mode == "raytrace")  ok = g101::run_raytrace(args, false);
    else if (mode == "pathtrace") ok = g101::run_raytrace(args, true);
    else if (mode == "raster")    ok = g101::run_raster(args);
    else if (mode == "bezier")    ok = g101::run_bezier(args);
    else if (mode == "rope")      ok = g101::run_rope(args);

    if (!ok) std::fprintf(stderr, "mode '%s' failed or unknown\n", mode.c_str());
    std::printf("[%s] %.1f ms\n", mode.c_str(), t.ms());
    return ok ? 0 : 1;
}
