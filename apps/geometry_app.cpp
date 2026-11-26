#include "apps.hpp"
#include "core/image.hpp"
#include "geometry/bezier.hpp"
#include <vector>

namespace g101 {
bool run_bezier(const Args& args) {
    const int W = args.geti("width", 700), H = args.geti("height", 700);
    Image img(W, H);
    img.fill(0, 0, 0);

    std::vector<Vec2f> cps{
        {100, 500}, {200, 100}, {350, 600}, {500, 150}, {600, 400}
    };
    draw_bezier(img, cps, args.geti("samples", 512));
    return img.save_png(args.get("out", "results/geometry/bezier_curve.png"));
}
}
