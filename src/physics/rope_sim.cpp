#include "physics/rope_sim.hpp"
#include <cmath>
#include <vector>

namespace g101::phys {

Rope::Rope(float x0, float y0, float x1, float y1, int n, float mass, float k,
           const std::vector<int>& pinned)
{
    for (int i = 0; i < n; ++i) {
        float t = (n == 1) ? 0.f : float(i) / (n - 1);
        Mass m;
        m.position = Vec2(x0 + (x1 - x0) * t, y0 + (y1 - y0) * t);
        m.prev_position = m.position;
        m.mass = mass;
        m.pinned = false;
        masses.push_back(m);
    }
    for (int i : pinned) if (i >= 0 && i < n) masses[i].pinned = true;

    for (int i = 0; i + 1 < n; ++i) {
        Spring s;
        s.m1 = &masses[i]; s.m2 = &masses[i + 1];
        s.k = k;
        s.rest_length = (masses[i + 1].position - masses[i].position).norm();
        springs.push_back(s);
    }
}

void Rope::simulateEuler(float dt, Vec2 gravity)
{
    for (auto& m : masses) m.forces = gravity * m.mass;

    for (auto& s : springs) {
        Vec2 d = s.m1->position - s.m2->position;
        float len = d.norm();
        if (len < 1e-8f) continue;
        Vec2 f = -s.k * (d / len) * (len - s.rest_length);
        s.m1->forces += f;
        s.m2->forces -= f;
    }

    for (auto& m : masses) {
        if (m.pinned) continue;
        Vec2 a = m.forces / m.mass;
        m.velocity += a * dt;
        m.velocity = m.velocity * 0.999f;      // 阻尼
        m.position += m.velocity * dt;
    }
}

// Verlet 积分：position + (position - prev_position) * damping + a * dt^2
void Rope::simulateVerlet(float dt, Vec2 gravity)
{
    for (auto& m : masses) m.forces = gravity * m.mass;

    for (auto& s : springs)
    {
        Vec2  d   = s.m1->position - s.m2->position;
        float len = d.norm();
        if (len < 1e-8f) continue;
        Vec2 f = -s.k * (d / len) * (len - s.rest_length);
        s.m1->forces += f;
        s.m2->forces -= f;
    }

    const float damping = 0.999f;
    for (auto& m : masses)
    {
        if (m.pinned) continue;
        Vec2 x_prev = m.position;
        Vec2 a      = m.forces / m.mass;
        m.position  = m.position + (m.position - m.prev_position) * damping + a * dt * dt;
        m.prev_position = x_prev;
    }
}
} // namespace g101::phys
