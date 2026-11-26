# games101作业7

## castRay实现
```cpp
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    Intersection inter = intersect(ray);
    if(!inter.happened) return Vector3f(0.0f);

    Vector3f hitPoint = inter.coords;
    Vector3f N = inter.normal;
    Material *m = inter.m;
    Vector3f wo = normalize(-ray.direction);

    Vector3f L_dir(0.0f),L_indir(0.0f);

    //直接光源

    Intersection lightInter;
    float lightpdf = 0.0f;
    sampleLight(lightInter,lightpdf);

    if(lightpdf > 0.0f){
        Vector3f x = lightInter.coords;
        Vector3f ws = normalize(x - hitPoint); 
        Vector3f d = x - hitPoint;
        float dist2 = dotProduct(d,d);   //求光源与点之间距离的平方

        Ray shadowRay(hitPoint+N*EPSILON,ws);   //一条从着色点打向光源的射线，检测是否有物体挡在中间
        Intersection shadowInter = intersect(shadowRay);
        bool blocked = shadowInter.happened && 
                        shadowInter.distance < std::sqrt(dist2) -EPSILON;

        if(!blocked){
            L_dir = lightInter.emit * m->eval(wo,ws,N) 
                    * dotProduct(ws,N) * dotProduct(-ws,lightInter.normal) 
                    / (dist2 * lightpdf) ;
        }
    }

    //间接光源

    if(get_random_float()<RussianRoulette){
        Vector3f wi = m->sample(wo,N);
        Ray r(hitPoint+N*EPSILON,wi);
        Intersection rInter = intersect(r);

        if(rInter.happened && !rInter.m->hasEmission()){
            float pdf = m->pdf(wo,wi,N);
            if(pdf>0.0f){
                L_indir = castRay(r,depth+1) * m->eval(wo,wi,N)
                        * dotProduct(wi,N) * dotProduct(wi,rInter.normal)
                        / (pdf * RussianRoulette);

            }
        }
    }
    return L_dir + L_indir;
}
```
该代码严格按照老师推导出来的公式执行，但是最终生成图像中光源区域却是纯黑。  
原因是当光源本身是被打到的物体时，没有返回其本身的材质。  
增加：
```cpp
    if(inter.m->hasEmission()){
        return inter.m->getEmission();
    }
```
即可正确返回。

## 多线程优化
### Renderer优化
```cpp
    unsigned hw = std::thread::hardware_concurrency();
    int nThreads = hw ? (int)hw : 4 ;
    
    auto worker  = [&](int Begin,int End){
        for (int j = Begin; j < End; ++j) {

            for (uint32_t i = 0; i < scene.width; ++i) {
                // generate primary ray direction
                float x = (2 * (i + 0.5) / (float)scene.width - 1) *
                        imageAspectRatio * scale;
                float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;

                Vector3f dir = normalize(Vector3f(-x, y, 1));

                int idx = j * scene.width + i;

                for (int k = 0; k < spp; k++){
                    framebuffer[idx] += scene.castRay(Ray(eye_pos, dir), 0) / spp ;  
                }
            }
            // UpdateProgress(j / (float)scene.height);
        }
    }; 

    std::vector<std::thread> pool;
    int chunk = ((int)scene.height + nThreads -1) / nThreads ; //向上取整

    for(int t = 0;t < nThreads;t++ ){
        int b = t * chunk;
        int e = std::min((int)scene.height,b + chunk);
        if(b<e) pool.emplace_back(worker,b,e);
    }

    for(auto& th:pool){
        th.join();
    }
    
    UpdateProgress(1.f);
```

将其作为一个函数通过多线程对象pool进行调用，实现多线程优化。

### Romdam优化
```cpp
inline float get_random_float()
{
    
    static thread_local std::mt19937 rng(std::random_device{}());
    static thread_local std::uniform_real_distribution<float> dist(0.f, 1.f); // distribution in range [1, 6]

    return dist(rng);
}
```
romdam 是性能炸弹，针对其进行优化使我的代码跑通时间从两三小时缩短到五分钟。

## Microfacet 材质实现
```cpp
Vector3f Material::eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N)
{
    if (!isMicrofacet) {
        float cosalpha = dotProduct(N, wo);
        return cosalpha > 0.0f ? Kd / M_PI : Vector3f(0.0f);
    }

    Vector3f V = normalize(wi);          // 表面 → 相机
    Vector3f L = normalize(wo);          // 表面 → 光源
    float NdotL = dotProduct(N, L);
    float NdotV = dotProduct(N, V);
    if (NdotL <= 0.0f || NdotV <= 0.0f) return Vector3f(0.0f);

    Vector3f Hv = L + V;
    float len = Hv.norm();
    if (len < 1e-6f) return Vector3f(0.0f);
    Vector3f H = Hv / len;

    float NdotH = std::max(0.0f, dotProduct(N, H));
    float HdotV = std::max(0.0f, dotProduct(H, V));

    float a = roughness * roughness, a2 = a * a;

    float demon = NdotH * NdotH * (a2 - 1.0f) + 1.0f;      
    float D = a2 / (M_PI * demon * demon);

    float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
    float G = (NdotV / (NdotV * (1.0f - k) + k)) *
              (NdotL / (NdotL * (1.0f - k) + k));

    Vector3f F = F0 + (Vector3f(1.0f) - F0) * std::pow(1.0f - HdotV, 5.0f);

    Vector3f spec = F * (D * G / (4.0f * NdotV * NdotL));    //镜面项
    Vector3f diff = Kd / M_PI * (Vector3f(1.0f) - F);    //漫反射

    return spec + diff;
}
```

针对 Material.hpp 中 eval 函数进行重实现。
利用：
> f = (F * G * D)/(4 * (n * l) * (n * v)) +  Kd / Pi * (1 - F)

其中：  

- D : 法线分布
> D(h) = a*a / (Pi * ((n * h) * (n * h) * (a * a - 1)+1) * 
> ((n * h) * (n * h) * (a * a - 1)+1) )
- F : 菲涅尔
> F(h,v) = F0 + (1-F0) * pow (1 - h * v, 5)
- G : 几何项
> G(x) = (n * x) / ((n * x)(1 - k) + k) , 
> k = (a+1) * (a+1) / 8 