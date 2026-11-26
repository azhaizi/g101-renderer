#pragma once
#include <vector>
#include <Eigen/Core>

namespace g101::phys {
using Vec2 = Eigen::Vector2f;

struct Mass { Vec2 position{0,0}, prev_position{0,0}, velocity{0,0}, forces{0,0}; float mass = 1.f; bool pinned = false; };
struct Spring { Mass* m1; Mass* m2; float k; float rest_length; };

class Rope {
public:
    Rope(float x0, float y0, float x1, float y1, int n, float mass, float k,
         const std::vector<int>& pinned);

    void simulateEuler(float dt, Vec2 gravity);
    void simulateVerlet(float dt, Vec2 gravity);

    std::vector<Mass>   masses;
    std::vector<Spring> springs;
};
}
