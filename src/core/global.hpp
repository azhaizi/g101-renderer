#pragma once
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>
#include <limits>

// ---------- raster 需要 ----------
typedef unsigned char u08;
#define MY_PI 3.1415926
#define TWO_PI (2.0 * MY_PI)

// ---------- raytrace 需要 ----------
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

inline constexpr float EPSILON   = 1e-5f;
inline constexpr float kInfinity = std::numeric_limits<float>::max();

inline float clamp(const float& lo, const float& hi, const float& v)
{ return std::max(lo, std::min(hi, v)); }

inline bool solveQuadratic(const float& a, const float& b, const float& c, float& x0, float& x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) x0 = x1 = -0.5f * b / a;
    else {
        float q = (b > 0) ? -0.5f * (b + std::sqrt(discr)) : -0.5f * (b - std::sqrt(discr));
        x0 = q / a; x1 = c / q;
    }
    if (x0 > x1) std::swap(x0, x1);
    return true;
}

inline float get_random_float()
{
    static thread_local std::mt19937 rng(std::random_device{}());
    static thread_local std::uniform_real_distribution<float> dist(0.f, 1.f);
    return dist(rng);
}

inline void UpdateProgress(float progress)
{
    const int barWidth = 70;
    std::cout << "[";
    int pos = int(barWidth * progress);
    for (int i = 0; i < barWidth; ++i) std::cout << (i < pos ? "=" : (i == pos ? ">" : " "));
    std::cout << "] " << int(progress * 100.0f) << " %\r";
    std::cout.flush();
}