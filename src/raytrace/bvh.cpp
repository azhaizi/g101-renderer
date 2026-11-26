#include <algorithm>
#include <cassert>
#include "BVH.hpp"
#include <chrono>
#include <cmath>
#include <limits>

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    if (primitives.empty())
        return;

    auto t0 = std::chrono::steady_clock::now();
    root = (splitMethod == SplitMethod::SAH) ? recursiveBuildSAH(primitives)
                                              : recursiveBuild(primitives);
    auto t1 = std::chrono::steady_clock::now();

    const double us = std::chrono::duration<double, std::micro>(t1 - t0).count();
    const double ms = us / 1000.0;

    if (ms >= 1.0)
        printf("BVH built: %d prims, %.2f ms\n", (int)primitives.size(), ms);
    else
        printf("BVH built: %d prims, %.1f us\n", (int)primitives.size(), us);
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        node->area = objects[0]->getArea();
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                       f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                       f2->getBounds().Centroid().y;
            });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                       f2->getBounds().Centroid().z;
            });
            break;
        }

        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        auto leftshapes = std::vector<Object*>(beginning, middling);
        auto rightshapes = std::vector<Object*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
    }

    return node;
}

Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // 递归遍历 BVH：先测包围盒，命中叶子再求物体精确交点
    Intersection a;
    if(!node) return a;

    std::array<int,3> dirIsNeg{ray.direction.x>0,ray.direction.y>0,ray.direction.z>0};
    if (!node->bounds.IntersectP(ray,ray.direction_inv,dirIsNeg)){
        return a;
    }

    if(node->left==nullptr && node->right==nullptr){
        return node->object->getIntersection(ray);
    }

    Intersection left = getIntersection(node->left,ray);
    Intersection right = getIntersection(node->right,ray);

    return (left.distance<right.distance) ? left : right;
}


void BVHAccel::getSample(BVHBuildNode* node, float p, Intersection &pos, float &pdf){
    if (!node) { pdf = 0.0f; return; }
    if (node->left == nullptr || node->right == nullptr) {
        if (!node->object) { pdf = 0.0f; return; }   // 防御：空叶子
        node->object->Sample(pos, pdf);
        pdf *= node->area;
        return;
    }
    if(p < node->left->area) getSample(node->left, p, pos, pdf);
    else getSample(node->right, p - node->left->area, pos, pdf);
}

void BVHAccel::Sample(Intersection &pos, float &pdf){
    if (!root) { pdf = 0.0f; return; }
    float p = std::sqrt(get_random_float()) * root->area;
    getSample(root, p, pos, pdf);
    pdf /= root->area;
}

// ---------------- SAH 构建（16 桶） ----------------
// 参考 PBRT 的桶式 SAH：对最长轴分 16 桶，前缀/后缀扫描求最小代价划分
BVHBuildNode* BVHAccel::recursiveBuildSAH(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    Bounds3 bounds;
    for (size_t i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());

    if (objects.size() == 1)
    {
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->area   = objects[0]->getArea();
        node->left   = nullptr;
        node->right  = nullptr;
        return node;
    }

    if (objects.size() == 2)
    {
        node->left   = recursiveBuildSAH({ objects[0] });
        node->right  = recursiveBuildSAH({ objects[1] });
        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area   = node->left->area + node->right->area;
        return node;
    }

    constexpr int NB = 16;

    Bounds3 centroidBounds;
    for (auto* o : objects)
        centroidBounds = Union(centroidBounds, o->getBounds().Centroid());
    const int dim = centroidBounds.maxExtent();

    struct BucketInfo { int count = 0; Bounds3 bounds; };
    BucketInfo buckets[NB];

    for (auto* o : objects)
    {
        const Vector3f ci = centroidBounds.Offset(o->getBounds().Centroid()); int b = (int)(NB * ci[dim]);
        b = std::min(NB - 1, std::max(0, b));
        buckets[b].count++;
        buckets[b].bounds = Union(buckets[b].bounds, o->getBounds());
    }

    int      countL[NB - 1];
    Bounds3  boundsL[NB - 1];
    int      countR[NB - 1];
    Bounds3  boundsR[NB - 1];

    {
        int c = 0; Bounds3 b;
        for (int i = 0; i < NB - 1; ++i) { c += buckets[i].count; b = Union(b, buckets[i].bounds); countL[i] = c; boundsL[i] = b; }
    }
    {
        int c = 0; Bounds3 b;
        for (int i = NB - 1; i >= 1; --i) { c += buckets[i].count; b = Union(b, buckets[i].bounds); countR[i - 1] = c; boundsR[i - 1] = b; }
    }

    const double totalArea = (bounds.SurfaceArea() > 0.0) ? bounds.SurfaceArea() : 1.0;
    double minCost = std::numeric_limits<double>::infinity();
    int    minSplit = 0;
    for (int i = 0; i < NB - 1; ++i)
    {
        double cost = 1.0 + (countL[i] * boundsL[i].SurfaceArea() +
                             countR[i] * boundsR[i].SurfaceArea()) / totalArea;
        if (cost < minCost) { minCost = cost; minSplit = i; }
    }

    std::vector<Object*> leftObjs, rightObjs;
    leftObjs.reserve(objects.size());
    rightObjs.reserve(objects.size());
    for (auto* o : objects)
    {
        const Vector3f ci = centroidBounds.Offset(o->getBounds().Centroid()); int b = (int)(NB * ci[dim]);
        b = std::min(NB - 1, std::max(0, b));
        if (b <= minSplit) leftObjs.push_back(o); else rightObjs.push_back(o);
    }

    // 划分退化（全部落到一侧）→ 退回中位数划分，保证不无限递归
    if (leftObjs.empty() || rightObjs.empty())
        return recursiveBuild(objects);

    node->left   = recursiveBuildSAH(leftObjs);
    node->right  = recursiveBuildSAH(rightObjs);
    node->bounds = Union(node->left->bounds, node->right->bounds);
    node->area   = node->left->area + node->right->area;
    return node;
}





