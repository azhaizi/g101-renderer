# games101作业5

## render实现
```
    float x = (i+0.5f)*2.f/scene.width-1.f;     //将其变换到1*1*1的空间中
    float y = 1.f-2.f*(j+0.5f)/scene.height;
    x = x*scale*imageAspectRatio;   //变换到相机空间方向
    y = y*scale;
```
## rayTriangleIntersect实现
```
bool rayTriangleIntersect(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& orig,
    const Vector3f& dir, float& tnear, float& u, float& v)
{
    Vector3f e1 = v1-v0;
    Vector3f e2 = v2-v0;
    Vector3f s = orig-v0;
    Vector3f s1 =crossProduct(dir,e2);   //dir x e2;
    Vector3f s2 =crossProduct(s,e1);       //s x e1;
    float n = dotProduct(s1,e1);   //s1 * e1;
    tnear = dotProduct(s2,e2)/n;    //s2 * e2;
    u = dotProduct(s1,s)/n;    //s1 * s ;
    v = dotProduct(s2,dir)/n;    //s2 * dir;

    return tnear>0.0f && u>=0.0f && v>=0.0f && (u+v)<=1.0f;
}
```
Moller-Trumbore算法。
利用三角形重心坐标解出 t , u , v ，如果他们都大于 0 ，说明射线穿过了三角形。
注意， u , v 需要小于等于 1 ，因为 u , v 是三角形重心坐标的两个分量，要想在三角形内部，必须小于等于 1 。