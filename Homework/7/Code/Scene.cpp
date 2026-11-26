//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    Intersection inter = intersect(ray);
    if(!inter.happened) return Vector3f(0.0f);

    if(inter.m->hasEmission()){
        return inter.m->getEmission();
    }

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
                        * dotProduct(wi,N) 
                        / (pdf * RussianRoulette) ;

            }
        }
    }
    return L_dir + L_indir;
}