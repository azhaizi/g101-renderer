# games101作业6

## render更新
```
    Vector3f dir = normalize(Vector3f(x, y, -1) );
    framebuffer[m++] = scene.castRay(Ray(eye_pos , dir), 0);
```
在作业6中，我们需要针对castRay函数的变化对参数进行调整，Ray()函数可以帮我们生成一条需要的射线作为castRay的参数。

## getIntersection更新
```
    if(t_tmp<0)return inter;
    inter.happened = true;
    inter.coords = ray(t_tmp);
    inter.distance = t_tmp;
    inter.m = m;
    inter.normal = normal;
    inter.obj = this;
    return inter;
```
作业6中，已经为我们解出了作业5中的 t , u , v ，所以我们只需要关注如何更新相交信息即可。  
getIntersection是一个intersection函数，我们需要返回一个intersection，而函数中正好为我们写了一个"intersection inter;"一直没有用上，所以我们就为其赋值，并返回。

## IntersectP实现
```
inline bool Bounds3::IntersectP(const Ray& ray, const Vector3f& invDir,
    const std::array<int, 3>& dirIsNeg) const
{
    float tminx = ((*this)[dirIsNeg[0]].x - ray.origin.x) * invDir.x;
    float tmaxx = ((*this)[1-dirIsNeg[0]].x - ray.origin.x) * invDir.x;
    float tmin = std::min(tminx,tmaxx);
    float tmax = std::max(tminx,tmaxx);

    float tminy = ((*this)[dirIsNeg[1]].y - ray.origin.y) * invDir.y;
    float tmaxy = ((*this)[1-dirIsNeg[1]].y - ray.origin.y) * invDir.y;
    tmin = std::max(tmin,std::min(tminy,tmaxy));
    tmax = std::min(tmax,std::max(tminy,tmaxy));

    float tminz = ((*this)[dirIsNeg[2]].z - ray.origin.z) * invDir.z;
    float tmaxz = ((*this)[1-dirIsNeg[2]].z - ray.origin.z) * invDir.z;
    tmin = std::max(tmin,std::min(tminz,tmaxz));
    tmax = std::min(tmax,std::max(tminz,tmaxz));

    return tmax >= std::max(tmin,0.0f);
}
```
利用课上讲解的包围和判断方法即可。
(*this) 取的是上面的Bound3类。
```
inline const Vector3f& operator[](int i) const
{
    return (i == 0) ? pMin : pMax;
}
```
operater[] ,重载方括号，使其能写b[i]。
这段函数使 b[0]->pmix  ,
         b[1]->pmax  。

## getIntersection实现
```
Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    Intersection a;
    if(!node) return a;  //如果不存在结点，直接返回；

    std::array<int,3> dirIsNeg{ray.direction.x>0,ray.direction.y>0,ray.direction.z>0};
    if (!node->bounds.IntersectP(ray,ray.direction_inv,dirIsNeg)){
        return a;
    }     //如果不相交直接返回；

    if(node->left==nullptr && node->right==nullptr){
        return node->object->getIntersection(ray);
    }       //如果左右子结点都为空，返回子结点的物体与光线作用得到的交点；

    Intersection left = getIntersection(node->left,ray);    //递归查找
    Intersection right = getIntersection(node->right,ray);

    return (left.distance<right.distance) ? left : right;
}
```

## SAH(Surface Area Heuristic)
一条随机光线命中某个包围和的概率，正比与该盒子的表面积。
```
cost = Ctrav + AL / A * NL * Cisect + AR / A * NR * Cisect
```
- AL , AR : 左/右子结点包围和的表面积
- A : 父结点表面积
- NL , NR : 左右两侧的图元数
- Cisect : 测试一个图元的成本
- Ctrav : 测试一个内部结点的成本
- 面积越大，图元越多，说明被光线打到的概率越高、要测的越多，代价越大。  

注意：我们默认 Ctrav 和 Cisect 都为 1.0f 。

```
    const int nBuckets = 12;
    float bestcost = kInfinity;
    int bestAsix = -1,bestSplit = -1;

    Bounds3 centroidBounds;
    for(auto* o : objects) centroidBounds = 
        Union(centroidBounds,o->getBounds().Centroid());

    for(int axis=0;axis<3;axis++){
        Bounds3 buckets[nBuckets];
        int counts[nBuckets] = {0};

        for(auto* o : objects){      //把每个图元分进桶里
            Vector3f c = centroidBounds.Offset(o->getBounds().Centroid());
            float t = axis == 0 ? c.x : axis == 1 ? c.y : c.x ;
            int b = std::min(nBuckets-1,std::max(0,(int)(nBuckets*t)));
            buckets[b] = Union(buckets[b],o->getBounds());
            counts[b]++;
        }

        Bounds3 leftBounds[nBuckets];     //leftBounds[i]是0..i的桶的并集
        int leftCounts[nBuckets] = {0};
        Bounds3 acc;
        int cnt=0;
        for(int i=0;i<nBuckets;i++){
            acc = Union(acc,buckets[i]);
            cnt += counts[i];
            leftBounds[i] = acc;
            leftCounts[i] = cnt;
        }

        Bounds3 rightBounds[nBuckets];
        int rightCounts[nBuckets] = {0};
        acc=Bounds3();
        cnt=0;
        for(int i=0;i<nBuckets;i++){
            acc = Union(acc,buckets[i]);
            cnt += counts[i];
            rightBounds[i] = acc;
            rightCounts[i] = cnt;
        }
        float parentArea = bounds.SurfaceArea();
        if(parentArea<0.0f) continue;

        for(int i=0;i<nBuckets-1;i++){      //求cost
            int nL = leftCounts[i],nR = rightCounts[i+1];
            if(nL == 0 || nR ==0){
                continue;
            }
            float cost = 1.0f + (leftBounds[i].SurfaceArea() * nL + 
                rightBounds[i+1].SurfaceArea() * nR) * parentArea;
            
            if(cost<bestcost){
                bestAsix = axis;
                bestcost = cost;
                bestSplit = i;
            }
        }
    }

    if(bestAsix == -1){     //退化保护
        int dim = centroidBounds.maxExtent();
        std::sort(objects.begin(),objects.end(),[dim](Object* a,Object* b){
            return a->getBounds().Centroid()[dim] < b->getBounds().Centroid()[dim];
        });
        auto mid = objects.begin() + (objects.size() / 2);
        node->left = recursiveBuild(std::vector<Object*>(objects.begin(),mid));
        node->right = recursiveBuild(std::vector<Object*>(mid,objects.end()));
        node->bounds = Union(node->left->bounds,node->right->bounds);
        return node;
    }
    std::vector<Object*> leftshapes,rightshapes;  //按最优切点分组
    for(auto* o : objects){    
        Vector3f c = centroidBounds.Offset(o->getBounds().Centroid());
        float t = bestAsix == 0 ? c.x : bestAsix == 1 ? c.y : c.z;
        int b = std::min(nBuckets-1,std::max(0,(int)(nBuckets*t)));
        (b <= bestSplit ? leftshapes : rightshapes).push_back(o);
    }

    assert(!leftshapes.empty() && !rightshapes.empty());

    node->left = recursiveBuild(leftshapes);
    node->right = recursiveBuild(rightshapes);

    node->bounds = Union(node->left->bounds, node->right->bounds);
    return node;
```