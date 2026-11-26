#include "apps.hpp"
#include "core/image.hpp"
#include "raster/rasterizer.hpp"
#include <vector>
#include <cmath>
#include <Eigen/Eigen>

namespace g101 {

static Eigen::Matrix4f model_m(float deg) {
    Eigen::Matrix4f m = Eigen::Matrix4f::Identity();
    float r = deg * 3.14159265358979f / 180.0f;
    m(0,0)=std::cos(r); m(0,2)=std::sin(r);
    m(2,0)=-std::sin(r); m(2,2)=std::cos(r);
    return m;
}
static Eigen::Matrix4f view_m(Eigen::Vector3f e) {
    Eigen::Matrix4f t = Eigen::Matrix4f::Identity();
    t(0,3)=-e.x(); t(1,3)=-e.y(); t(2,3)=-e.z();
    return t;
}
static Eigen::Matrix4f proj_m(float fov, float aspect, float n, float f) {
    float t = std::tan(fov * 3.14159265358979f / 180.0f / 2.0f) * std::fabs(n);
    float b = -t, r = t * aspect, l = -r;
    Eigen::Matrix4f persp = Eigen::Matrix4f::Identity();
    persp(0,0) = n; persp(1,1) = n;                 // ★ 修复：漏了这个导致顶点放大 10 倍
    persp(2,2) = n + f; persp(2,3) = -n * f; persp(3,2) = 1.0f; persp(3,3) = 0.0f;
    Eigen::Matrix4f scale = Eigen::Matrix4f::Identity();
    scale(0,0)=2/(r-l); scale(1,1)=2/(t-b); scale(2,2)=2/(n-f);
    Eigen::Matrix4f trans = Eigen::Matrix4f::Identity();
    trans(0,3)=-(r+l)/2; trans(1,3)=-(t+b)/2; trans(2,3)=-(n+f)/2;
    return trans * scale * persp;
}

bool run_raster(const Args& args) {
    const int W = args.geti("width", 700), H = args.geti("height", 700);

    rst::rasterizer r_(W, H);

    std::vector<Eigen::Vector3f> pos{
        {2, 0, -2}, {0, 2, -2}, {-2, 0, -2},
        {3.5f, -1, -5}, {2.5f, 1.5f, -5}, {-1, 0.5f, -5}
    };
    std::vector<Eigen::Vector3i> ind{ {0,1,2}, {3,4,5} };
    std::vector<Eigen::Vector3f> cols{
        {217,238,185}, {217,238,185}, {217,238,185},
        {185,217,238}, {185,217,238}, {185,217,238}
    };

    auto pos_id = r_.load_positions(pos);
    auto ind_id = r_.load_indices(ind);
    auto col_id = r_.load_colors(cols);

    r_.clear(rst::Buffers::Color | rst::Buffers::Depth);
    r_.set_model(model_m(args.getf("angle", 0.0f)));
    r_.set_view(view_m(Eigen::Vector3f(0, 0, 5)));
    r_.set_projection(proj_m(45.0f, float(W) / float(H), 0.1f, 50.0f));
    r_.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);

    Image img(W, H);
    auto& fb = r_.frame_buffer();
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            const Eigen::Vector3f& c = fb[(size_t)(H - 1 - y) * W + x];
            float rgb[3] = { c.x() / 255.0f, c.y() / 255.0f, c.z() / 255.0f };
            img.set_float(x, y, rgb);
        }
    return img.save_png(args.get("out", "results/raster/cube.png"));
}
}

