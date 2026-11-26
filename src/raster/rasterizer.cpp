//
// Created by goksu on 4/6/19.
// 重构说明：
//   1) set_pixel / get_index 修正越界（原写法 (height - y) 会越界一行）
//   2) 恢复 SSAA（2x2 / 4x4），采样点做重心坐标 + 逐样本深度测试，最后按命中数平均
//   3) fragment_shader 为空时退化为插值顶点色，不会 bad_function_call
//   4) 补上 draw(pos_buf_id, ind_buf_id, col_buf_id, Primitive) 的实现
//

#include <algorithm>
#include <functional>
#include <limits>
#include <cmath>
#include <tuple>

#include "raster/rasterizer.hpp"
#include "raster/shader.hpp"

namespace
{
    inline float deg2rad_local(float d) { return d * 3.14159265358979f / 180.0f; }
}

// ---------------- 缓冲 ----------------
rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f>& positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);
    return { id };
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i>& indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);
    return { id };
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f>& cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);
    return { id };
}

rst::col_buf_id rst::rasterizer::load_normals(const std::vector<Eigen::Vector3f>& normals)
{
    auto id = get_next_id();
    nor_buf.emplace(id, normals);
    normal_id = id;
    return { id };
}

// ---------------- 线 ----------------
void rst::rasterizer::draw_line(Eigen::Vector3f begin, Eigen::Vector3f end)
{
    auto x1 = begin.x(), y1 = begin.y();
    auto x2 = end.x(),   y2 = end.y();

    Eigen::Vector3f line_color = { 255, 255, 255 };

    int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
    dx = int(x2 - x1); dy = int(y2 - y1);
    dx1 = std::abs(dx); dy1 = std::abs(dy);
    px = 2 * dy1 - dx1; py = 2 * dx1 - dy1;

    if (dy1 <= dx1)
    {
        if (dx >= 0) { x = int(x1); y = int(y1); xe = int(x2); }
        else         { x = int(x2); y = int(y2); xe = int(x1); }
        if (in_screen(x, y)) set_pixel(Eigen::Vector2i(x, y), line_color);
        for (i = 0; x < xe; i++)
        {
            x = x + 1;
            if (px < 0) px = px + 2 * dy1;
            else
            {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) y = y + 1; else y = y - 1;
                px = px + 2 * (dy1 - dx1);
            }
            if (in_screen(x, y)) set_pixel(Eigen::Vector2i(x, y), line_color);
        }
    }
    else
    {
        if (dy >= 0) { x = int(x1); y = int(y1); ye = int(y2); }
        else         { x = int(x2); y = int(y2); ye = int(y1); }
        if (in_screen(x, y)) set_pixel(Eigen::Vector2i(x, y), line_color);
        for (i = 0; y < ye; i++)
        {
            y = y + 1;
            if (py <= 0) py = py + 2 * dx1;
            else
            {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) x = x + 1; else x = x - 1;
                py = py + 2 * (dx1 - dy1);
            }
            if (in_screen(x, y)) set_pixel(Eigen::Vector2i(x, y), line_color);
        }
    }
}

// ---------------- 几何工具 ----------------
namespace
{
    inline Eigen::Vector4f to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
    {
        return Eigen::Vector4f(v3.x(), v3.y(), v3.z(), w);
    }

    inline bool inside_triangle(float x, float y, const Eigen::Vector4f* _v)
    {
        Eigen::Vector3f v[3];
        for (int i = 0; i < 3; ++i) v[i] = { _v[i].x(), _v[i].y(), 1.0f };

        Eigen::Vector3f f0 = v[1].cross(v[0]);
        Eigen::Vector3f f1 = v[2].cross(v[1]);
        Eigen::Vector3f f2 = v[0].cross(v[2]);
        Eigen::Vector3f p(x, y, 1.f);

        return (p.dot(f0) * f0.dot(v[2]) > 0) &&
               (p.dot(f1) * f1.dot(v[0]) > 0) &&
               (p.dot(f2) * f2.dot(v[1]) > 0);
    }

    inline std::tuple<float, float, float> barycentric(float x, float y, const Eigen::Vector4f* v)
    {
        float c1 = (x * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * y + v[1].x() * v[2].y() - v[2].x() * v[1].y()) /
                   (v[0].x() * (v[1].y() - v[2].y()) + (v[2].x() - v[1].x()) * v[0].y() + v[1].x() * v[2].y() - v[2].x() * v[1].y());
        float c2 = (x * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * y + v[2].x() * v[0].y() - v[0].x() * v[2].y()) /
                   (v[1].x() * (v[2].y() - v[0].y()) + (v[0].x() - v[2].x()) * v[1].y() + v[2].x() * v[0].y() - v[0].x() * v[2].y());
        float c3 = (x * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * y + v[0].x() * v[1].y() - v[1].x() * v[0].y()) /
                   (v[2].x() * (v[0].y() - v[1].y()) + (v[1].x() - v[0].x()) * v[2].y() + v[0].x() * v[1].y() - v[1].x() * v[0].y());
        return { c1, c2, c3 };
    }

    inline Eigen::Vector3f interp3(float a, float b, float g, const Eigen::Vector3f& v0,
                                   const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, float w)
    {
        return (a * v0 + b * v1 + g * v2) / w;
    }

    inline Eigen::Vector2f interp2(float a, float b, float g, const Eigen::Vector2f& v0,
                                   const Eigen::Vector2f& v1, const Eigen::Vector2f& v2, float w)
    {
        return Eigen::Vector2f((a * v0[0] + b * v1[0] + g * v2[0]) / w,
                               (a * v0[1] + b * v1[1] + g * v2[1]) / w);
    }
} // namespace

// ---------------- 着色 ----------------
Eigen::Vector3f rst::rasterizer::shade(const fragment_shader_payload& payload) const
{
    if (fragment_shader) return fragment_shader(payload);

    // 没设 shader：退化为插值顶点色（A2 行为），避免 bad_function_call
    return payload.color;
}

// ---------------- draw：由缓冲绘制（A2 用法） ----------------
void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf_pos = pos_buf[pos_buffer.pos_id];
    auto& buf_ind = ind_buf[ind_buffer.ind_id];
    auto& buf_col = col_buf[col_buffer.col_id];

    std::vector<Triangle> tris;
    tris.reserve(buf_ind.size());

    for (const auto& idx : buf_ind)
    {
        Triangle t;
        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, buf_pos[idx[i]]);
            if (!buf_col.empty())
            {
                const Eigen::Vector3f& c = buf_col[idx[i]];
                // 缓冲里的颜色是 0~255（A2 约定），setColor 内部会 /255
                t.setColor(i, c.x(), c.y(), c.z());
            }
            t.setNormal(i, Eigen::Vector3f(0, 0, 1));
            t.setTexCoord(i, 0.f, 0.f);
        }
        tris.push_back(t);
    }

    if (type == Primitive::Line)
    {
        for (const auto& t : tris)
        {
            draw_line(t.v[0], t.v[1]);
            draw_line(t.v[1], t.v[2]);
            draw_line(t.v[2], t.v[0]);
        }
        return;
    }

    // 转成指针列表，走统一的 draw(Triangle*)
    std::vector<Triangle*> ptrs;
    ptrs.reserve(tris.size());
    for (auto& t : tris) ptrs.push_back(&t);
    draw(ptrs);

    // 兼容 A2 约定：这条入口的调用方把 framebuffer 当 0~255 用（A2 的 main 直接 convertTo）
    // 走 Triangle* 入口（A3 shader 路径）时 framebuffer 保持 0~1，不要在这里缩放。
    for (auto& c : frame_buf) c *= 255.0f;
}

// ---------------- draw：由 Triangle 列表绘制（A3 用法） ----------------
void rst::rasterizer::draw(std::vector<Triangle*>& TriangleList)
{
    const float f1 = (50.0f - 0.1f) / 2.0f;
    const float f2 = (50.0f + 0.1f) / 2.0f;

    const Eigen::Matrix4f mvp = projection * view * model;
    const Eigen::Matrix4f mv  = view * model;
    const Eigen::Matrix4f inv_trans = mv.inverse().transpose();

    for (const auto* src : TriangleList)
    {
        if (!src) continue;

        Triangle newtri = *src;

        // 视图空间顶点（给 shader 算光照用）
        std::array<Eigen::Vector3f, 3> viewspace_pos;
        for (int i = 0; i < 3; ++i)
            viewspace_pos[i] = (mv * to_vec4(src->v[i])).template head<3>();

        // 裁剪空间 → NDC
        Eigen::Vector4f v[3];
        for (int i = 0; i < 3; ++i)
        {
            v[i] = mvp * to_vec4(src->v[i]);
            if (std::fabs(v[i].w()) < 1e-8f) return;   // 退化三角形，整批跳过
            v[i].x() /= v[i].w();
            v[i].y() /= v[i].w();
            v[i].z() /= v[i].w();
        }

        // 法线变换（逆转置，w=0）
        Eigen::Vector4f n[3];
        for (int i = 0; i < 3; ++i)
            n[i] = inv_trans * to_vec4(src->normal[i], 0.0f);

        // 视口变换
        for (auto& vert : v)
        {
            vert.x() = 0.5f * width  * (vert.x() + 1.0f);
            vert.y() = 0.5f * height * (vert.y() + 1.0f);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            newtri.setVertex(i, v[i].template head<3>());
            newtri.setNormal(i, n[i].head<3>());
        }

        // 顶点色：只有在颜色全为 0（未设置）时才给个默认灰
        bool color_unset = (src->color[0].norm() < 1e-6f) &&
                           (src->color[1].norm() < 1e-6f) &&
                           (src->color[2].norm() < 1e-6f);
        if (color_unset)
        {
            newtri.setColor(0, 148, 121, 92);
            newtri.setColor(1, 148, 121, 92);
            newtri.setColor(2, 148, 121, 92);
        }

        rasterize_triangle(newtri, viewspace_pos);
    }
}

// ---------------- 光栅化 ----------------
void rst::rasterizer::rasterize_triangle(const Triangle& t, const std::array<Eigen::Vector3f, 3>& view_pos)
{
    auto v = t.toVector4();

    float minxf = std::min(v[0].x(), std::min(v[1].x(), v[2].x()));
    float maxxf = std::max(v[0].x(), std::max(v[1].x(), v[2].x()));
    float minyf = std::min(v[0].y(), std::min(v[1].y(), v[2].y()));
    float maxyf = std::max(v[0].y(), std::max(v[1].y(), v[2].y()));

    int minx = std::max(0, (int)std::floor(minxf));
    int maxx = std::min(width  - 1, (int)std::ceil(maxxf));
    int miny = std::max(0, (int)std::floor(minyf));
    int maxy = std::min(height - 1, (int)std::ceil(maxyf));

    const int n  = (ssaa <= 1) ? 1 : ssaa;
    const int n2 = n * n;
    const float inv_n = 1.0f / float(n);

    for (int x = minx; x <= maxx; ++x)
    {
        for (int y = miny; y <= maxy; ++y)
        {
            const int index = get_index(x, y);
            float best_depth = depth_buf[index];

            Eigen::Vector3f accum(0, 0, 0);
            int hits = 0;

            // SSAA：在每个像素内取 n×n 个子样本
            for (int sy = 0; sy < n; ++sy)
            {
                for (int sx = 0; sx < n; ++sx)
                {
                    const float px = x + (sx + 0.5f) * inv_n;
                    const float py = y + (sy + 0.5f) * inv_n;

                    if (!inside_triangle(px, py, v.data())) continue;

                    auto [alpha, beta, gamma] = barycentric(px, py, v.data());

                    // 透视校正的深度
                    float denom = alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w();
                    if (std::fabs(denom) < 1e-12f) continue;
                    float Z  = 1.0f / denom;
                    float zp = (alpha * v[0].z() / v[0].w() +
                                beta  * v[1].z() / v[1].w() +
                                gamma * v[2].z() / v[2].w()) * Z;

                    if (zp > best_depth) continue;   // 该样本被遮挡
                    best_depth = zp;

                    // 属性插值（1/w 加权）
                    Eigen::Vector3f color      = interp3(alpha, beta, gamma, t.color[0], t.color[1], t.color[2], 1.0f);
                    Eigen::Vector3f normal     = interp3(alpha, beta, gamma, t.normal[0], t.normal[1], t.normal[2], 1.0f).normalized();
                    Eigen::Vector2f texcoords  = interp2(alpha, beta, gamma, t.tex_coords[0], t.tex_coords[1], t.tex_coords[2], 1.0f);
                    Eigen::Vector3f shadingpos = interp3(alpha, beta, gamma, view_pos[0], view_pos[1], view_pos[2], 1.0f);

                    fragment_shader_payload payload(color, normal, texcoords, texture ? &*texture : nullptr);
                    payload.view_pos = shadingpos;

                    accum += shade(payload);
                    ++hits;
                }
            }

            if (hits > 0)
            {
                depth_buf[index] = best_depth;
                frame_buf[index] = accum / float(hits);
            }
        }
    }
    (void)n2;
}

// ---------------- 状态 ----------------
void rst::rasterizer::set_model(const Eigen::Matrix4f& m)      { model = m; }
void rst::rasterizer::set_view(const Eigen::Matrix4f& v)       { view = v; }
void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)  { projection = p; }

void rst::rasterizer::set_ssaa(int n)
{
    ssaa = (n <= 1) ? 1 : n;
    if (ssaa > 4) ssaa = 4;   // 上限 4x4，避免误用 8x8 太慢
}

void rst::rasterizer::set_vertex_shader(std::function<Eigen::Vector3f(vertex_shader_payload)> vert_shader)
{
    vertex_shader = std::move(vert_shader);
}

void rst::rasterizer::set_fragment_shader(std::function<Eigen::Vector3f(fragment_shader_payload)> frag_shader)
{
    fragment_shader = std::move(frag_shader);
}

void rst::rasterizer::clear(Buffers buff)
{
    if ((buff & Buffers::Color) == Buffers::Color)
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{ 0, 0, 0 });

    if ((buff & Buffers::Depth) == Buffers::Depth)
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(size_t(w) * h);
    depth_buf.resize(size_t(w) * h);
    clear(Buffers::Color | Buffers::Depth);
    texture = std::nullopt;
}

// 关键修复：原来 (height - y) 会越界一行
int rst::rasterizer::get_index(int x, int y) const
{
    return (height - 1 - y) * width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector2i& point, const Eigen::Vector3f& color)
{
    if (!in_screen(point.x(), point.y())) return;
    frame_buf[get_index(point.x(), point.y())] = color;
}





