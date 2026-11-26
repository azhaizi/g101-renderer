#include "scene_presets.hpp"
#include <memory>
#include "raytrace/triangle.hpp"   // MeshTriangle
#include "raytrace/light.hpp"

namespace g101 {

// A6：bunny + 两盏灯
static SceneHandle make_cornell(const std::string& asset_dir) {
    auto scene = std::make_shared<Scene>(784, 784);
    const std::string d = asset_dir + "/models/cornellbox";

    Material* red   = new Material(DIFFUSE, Vector3f(0.0f)); red->Kd   = Vector3f(0.63f, 0.065f, 0.05f);
    Material* green = new Material(DIFFUSE, Vector3f(0.0f)); green->Kd = Vector3f(0.14f, 0.45f, 0.091f);
    Material* white = new Material(DIFFUSE, Vector3f(0.0f)); white->Kd = Vector3f(0.725f, 0.71f, 0.68f);
    Material* lightm = new Material(DIFFUSE, (8.0f * Vector3f(0.747f+0.058f, 0.747f+0.258f, 0.747f)
        + 15.6f * Vector3f(0.740f+0.287f, 0.740f+0.160f, 0.740f)
        + 18.4f * Vector3f(0.737f+0.642f, 0.737f+0.159f, 0.737f)));
    lightm->Kd = Vector3f(0.65f);
    lightm->isMicrofacet = false;

    MeshTriangle* floor   = new MeshTriangle(d + "/floor.obj",    white);
    MeshTriangle* shortbox= new MeshTriangle(d + "/shortbox.obj", white);
    MeshTriangle* tallbox = new MeshTriangle(d + "/tallbox.obj",  white);
    MeshTriangle* left    = new MeshTriangle(d + "/left.obj",     red);
    MeshTriangle* right   = new MeshTriangle(d + "/right.obj",    green);
    // 面光源用两个显式三角形（等价于 light.obj），避开 MeshTriangle 采样路径
    Triangle* l1 = new Triangle(Vector3f(213, 548.5f, 227), Vector3f(343, 548.5f, 227), Vector3f(343, 548.5f, 332), lightm);
    Triangle* l2 = new Triangle(Vector3f(213, 548.5f, 227), Vector3f(343, 548.5f, 332), Vector3f(213, 548.5f, 332), lightm);

    scene->Add(floor); scene->Add(shortbox); scene->Add(tallbox);
    scene->Add(left);  scene->Add(right);
    scene->Add(l1);    scene->Add(l2);
    scene->buildBVH();      // ★ 之前漏了这句：bvh 指针未初始化 → 黑图/偶发崩溃
    SceneHandle h;
    h.scene      = scene;
    h.name       = "cornell";
    h.eye        = Vector3f(278, 273, -800);   // Cornell 是厘米级坐标
    h.flip_x     = true;
    h.positive_z = true;
    h.gamma      = 0.6f;
    return h;
}

// A7：Cornell box（需要 models/cornellbox/*.obj）
// A6：bunny + 一块面光源（A7 的 castRay 只认 hasEmit 的物体，点光源不参与采样）
static SceneHandle make_bunny(const std::string& asset_dir) {
    auto scene = std::make_shared<Scene>(784, 784);
    scene->maxDepth = 5;

    Material* white = new Material(DIFFUSE, Vector3f(0.0f));
    white->Kd = Vector3f(0.73f, 0.72f, 0.70f);

    MeshTriangle* bunny = new MeshTriangle((asset_dir + "/bunny/bunny.obj").c_str(), white);
    scene->Add(bunny);

    Material* emit = new Material(DIFFUSE, Vector3f(25.0f));
    emit->Kd = Vector3f(0.0f);

    static Triangle l1(Vector3f(-0.30f, 0.55f, -0.30f), Vector3f(0.30f, 0.55f, -0.30f), Vector3f(0.30f, 0.55f, 0.30f), emit);
    static Triangle l2(Vector3f(-0.30f, 0.55f, -0.30f), Vector3f(0.30f, 0.55f,  0.30f), Vector3f(-0.30f, 0.55f, 0.30f), emit);
    scene->Add(&l1);
    scene->Add(&l2);

    scene->buildBVH();
    Bounds3 bb = bunny->getBounds();
    Vector3f center = (bb.pMin + bb.pMax) * 0.5f;
    float size = (bb.pMax - bb.pMin).norm();
    if (size < 1e-6f) size = 1.0f;

    SceneHandle h;
    h.scene      = scene;
    h.name       = "bunny";
    h.eye        = center + Vector3f(0.0f, 0.15f * size, 1.8f * size);
    h.flip_x     = false;
    h.positive_z = false;
    h.gamma      = 0.6f;

    scene->buildBVH();
    return h;
}

SceneHandle make_scene(const std::string& name, const std::string& asset_dir) {
    if (name == "cornell") return make_cornell(asset_dir);
    if (name == "bunny")   return make_bunny(asset_dir);
    return {};
}

} // namespace g101






