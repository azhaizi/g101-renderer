//
// Created by goksu on 4/6/19.
// 重构：A2 的 SSAA + A3 的 shader/纹理 合并；修复 set_pixel 越界；补齐 draw(pos/ind/col) 实现
//

#pragma once

#include <Eigen/Eigen>

#include <functional>
#include <optional>
#include <map>
#include <vector>
#include <array>
#include <algorithm>
#include <limits>
#include <string>
#include <cstdint>

#include "core/global.hpp"
#include "raster/shader.hpp"
#include "raster/triangle.hpp"

using namespace Eigen;

namespace rst
{
    enum class Buffers
    {
        Color = 1,
        Depth = 2
    };

    inline Buffers operator|(Buffers a, Buffers b)
    {
        return Buffers((int)a | (int)b);
    }

    inline Buffers operator&(Buffers a, Buffers b)
    {
        return Buffers((int)a & (int)b);
    }

    enum class Primitive
    {
        Line,
        Triangle
    };

    /*
     * For the curious : The draw function takes two buffer id's as its arguments. These two structs
     * make sure that if you mix up with their orders, the compiler won't compile it.
     * Aka : Type safety
     * */
    struct pos_buf_id
    {
        int pos_id = 0;
    };

    struct ind_buf_id
    {
        int ind_id = 0;
    };

    struct col_buf_id
    {
        int col_id = 0;
    };

    class rasterizer
    {
    public:
        rasterizer(int w, int h);

        // ---------- 顶点 / 索引 / 颜色 / 法线缓冲 ----------
        pos_buf_id load_positions(const std::vector<Eigen::Vector3f>& positions);
        ind_buf_id load_indices(const std::vector<Eigen::Vector3i>& indices);
        col_buf_id load_colors(const std::vector<Eigen::Vector3f>& colors);
        col_buf_id load_normals(const std::vector<Eigen::Vector3f>& normals);

        // ---------- MVP ----------
        void set_model(const Eigen::Matrix4f& m);
        void set_view(const Eigen::Matrix4f& v);
        void set_projection(const Eigen::Matrix4f& p);

        // ---------- 纹理 ----------
        void set_texture(Texture tex) { texture = std::move(tex); }
        void set_texture(const std::string& path) { texture = Texture(path); }
        bool has_texture() const { return texture.has_value(); }

        // ---------- 着色器（不设置时退化为顶点色，不会崩溃） ----------
        void set_vertex_shader(std::function<Eigen::Vector3f(vertex_shader_payload)> vert_shader);
        void set_fragment_shader(std::function<Eigen::Vector3f(fragment_shader_payload)> frag_shader);
        bool has_fragment_shader() const { return static_cast<bool>(fragment_shader); }

        // ---------- 抗锯齿：1 = 关闭，2 = 2x2，4 = 4x4 ----------
        void set_ssaa(int n);
        int  get_ssaa() const { return ssaa; }

        // ---------- 像素 / 清屏 ----------
        void set_pixel(const Eigen::Vector2i& point, const Eigen::Vector3f& color);
        void clear(Buffers buff);

        // ---------- 绘制 ----------
        // ① 由 pos/ind/col 缓冲绘制（A2 用法，只吃顶点色）
        void draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type);
        // ② 由 Triangle 列表绘制（A3 用法，会走 vertex/fragment shader）
        void draw(std::vector<Triangle*>& TriangleList);

        std::vector<Eigen::Vector3f>& frame_buffer() { return frame_buf; }
        const std::vector<float>&      depth_buffer() const { return depth_buf; }

    private:
        void draw_line(Eigen::Vector3f begin, Eigen::Vector3f end);

        void rasterize_triangle(const Triangle& t, const std::array<Eigen::Vector3f, 3>& world_pos);

        // 对单个采样点求片元颜色（内部处理 shader 为空的情况）
        Eigen::Vector3f shade(const fragment_shader_payload& payload) const;

        // VERTEX SHADER -> MVP -> Clipping -> /.W -> VIEWPORT -> DRAWLINE/DRAWTRI -> FRAGSHADER

    private:
        Eigen::Matrix4f model      = Eigen::Matrix4f::Identity();
        Eigen::Matrix4f view       = Eigen::Matrix4f::Identity();
        Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

        int normal_id = -1;

        std::map<int, std::vector<Eigen::Vector3f>> pos_buf;
        std::map<int, std::vector<Eigen::Vector3i>> ind_buf;
        std::map<int, std::vector<Eigen::Vector3f>> col_buf;
        std::map<int, std::vector<Eigen::Vector3f>> nor_buf;

        std::optional<Texture> texture;

        std::function<Eigen::Vector3f(fragment_shader_payload)> fragment_shader;
        std::function<Eigen::Vector3f(vertex_shader_payload)>   vertex_shader;

        std::vector<Eigen::Vector3f> frame_buf;
        std::vector<float>           depth_buf;

        int get_index(int x, int y) const;
        bool in_screen(int x, int y) const { return x >= 0 && y >= 0 && x < width && y < height; }

        int width  = 0;
        int height = 0;
        int ssaa   = 2;   // 默认 2x2 抗锯齿；set_ssaa(1) 关闭

        int next_id = 0;
        int get_next_id() { return next_id++; }
    };
} // namespace rst

