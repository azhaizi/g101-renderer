//
// Created by LEI XU on 4/27/19.
//

#include "raster/texture.hpp"

Texture::Texture(const std::string& path)
{
    m_image = g101::Image::load(path);
    width   = m_image.width();
    height  = m_image.height();
}

static Eigen::Vector3f sample(const g101::Image& img, int x, int y)
{
    if (x < 0) x = 0; if (y < 0) y = 0;
    if (x >= img.width())  x = img.width()  - 1;
    if (y >= img.height()) y = img.height() - 1;
    const unsigned char* p = img.data() + (size_t(y) * img.width() + x) * 4;
    return Eigen::Vector3f(p[0] / 255.0f, p[1] / 255.0f, p[2] / 255.0f) * 255.0f;
}

Eigen::Vector3f Texture::getColor(float u, float v) const
{
    // 保持原框架的语义：v 需要翻转（纹理坐标原点在左下）
    int x = static_cast<int>(u * width);
    int y = static_cast<int>((1.0f - v) * height);
    return sample(m_image, x, y);
}

Eigen::Vector3f Texture::getColorBilinear(float u, float v) const
{
    float x = u * width  - 0.5f;
    float y = (1.0f - v) * height - 0.5f;
    int x0 = static_cast<int>(std::floor(x)), y0 = static_cast<int>(std::floor(y));
    float ax = x - x0, ay = y - y0;

    Eigen::Vector3f c00 = sample(m_image, x0,     y0);
    Eigen::Vector3f c10 = sample(m_image, x0 + 1, y0);
    Eigen::Vector3f c01 = sample(m_image, x0,     y0 + 1);
    Eigen::Vector3f c11 = sample(m_image, x0 + 1, y0 + 1);

    return (c00 * (1 - ax) + c10 * ax) * (1 - ay) +
           (c01 * (1 - ax) + c11 * ax) * ay;
}