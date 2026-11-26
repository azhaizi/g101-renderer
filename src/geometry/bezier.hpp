#pragma once
#include <vector>
#include <Eigen/Core>
#include "core/image.hpp"

namespace g101 {

using Vec2f = Eigen::Vector2f;

Vec2f de_casteljau(float t, const std::vector<Vec2f>& cps);

// 在 img 上画出曲线（3x3 邻域抗锯齿）
void draw_bezier(Image& img, const std::vector<Vec2f>& cps, int samples = 256);

} // namespace g101