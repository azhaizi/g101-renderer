#pragma once
#include <string>
#include "Eigen/Eigen"
#include "core/image.hpp"

class Texture {
public:
    explicit Texture(const std::string& path);

    int width  = 0;
    int height = 0;

    // 最近邻
    Eigen::Vector3f getColor(float u, float v) const;
    // 双线性
    Eigen::Vector3f getColorBilinear(float u, float v) const;

private:
    g101::Image m_image;
};