#pragma once
#include <memory>
#include <string>
#include "raytrace/scene.hpp"

namespace g101 {
struct SceneHandle {
    std::shared_ptr<Scene> scene;
    std::string name;
    // 每个场景自带相机约定，避免 raytrace_app 里按名字硬编码
    Vector3f eye{ -1, 5, 10 };
    bool  flip_x     = false;
    bool  positive_z = false;
    float gamma      = 0.6f;
};
SceneHandle make_scene(const std::string& name, const std::string& asset_dir);
}
