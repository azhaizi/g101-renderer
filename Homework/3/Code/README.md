# games101作业3 

## 作业2的插值算法

```cpp
    if(depth_buf[index][i]>z_interpolated){
        depth_buf[index][i] = z_interpolated;
        sample_buf[index][i]=t.getColor();
    }
```
## 作业3的插值算法  
```cpp
    if(depth_buf[index]>zp){
        depth_buf[index]=zp;
        auto interpolated_color=interpolate(alpha,beta,gamma,
            t.color[0],t.color[1],t.color[2],1.0f);

        auto interpolated_normal=interpolate(alpha,beta,gamma,
            t.normal[0],t.normal[1],t.normal[2],1.0f);

        auto interpolated_texcoords=interpolate(alpha,beta,gamma,
            t.tex_coords[0],t.tex_coords[1],t.tex_coords[2],1.0f);

        auto interpolated_shadingcoords=interpolate(alpha,beta,gamma,
            view_pos[0],view_pos[1],view_pos[2],1.0f);

        fragment_shader_payload payload( interpolated_color, 
            interpolated_normal.normalized(), 
            interpolated_texcoords, texture ? &*texture : nullptr);

        payload.view_pos = interpolated_shadingcoords;
        auto pixel_color = fragment_shader(payload);
        set_pixel(Eigen::Vector2i(x,y),pixel_color);
    }
```  
从上面的代码对比可以看出，作业3与作业2代码接近，不同的是，作业3需要我们去获取额外的值来计算payload，再获取payload中的颜色量来设置像素颜色。 

## phone实现
```cpp
    Eigen::Vector3f result_color = {0, 0, 0};

    result_color+=la;

    for (auto& light : lights)
    {
        Eigen::Vector3f l=(light.position-point).normalized();
        Eigen::Vector3f v=(eye_pos-point).normalized();
        Eigen::Vector3f h=(l+v).normalized();
        float r=(light.position-point).squaredNorm();

        auto ld = kd.cwiseProduct(light.intensity/r)
            *std::max(0.0f,normal.dot(l)) ;
        auto ls = ks.cwiseProduct(light.intensity/r)*std::pow(std::max(0.0f,normal.dot(h)),p);

        result_color += ld+ls;
    }
```  
通过老师在课程中的讲解，我们知道个物体的成像是la(环境光)+ld(漫反射)+ls(高光)来实现的。  
我们需要计算每个光照造成的ld和ls，对其进行累加，这样我们就可以模拟出多个光照下的成图，得到一个近似的结果。  
la放在循环外是因为我们不需要对其进行光照下的累加，环境光可以看作是一个恒定的无死角光照，仍由其他光照变化，而它不变，所以我们将其放在循环外，只加一次。  

关于向量l和v的角平分线h，由于l和v已经进行过归一化，所以它们的长度相等，他们相加后的向量就是他们的角平分线。
.squareNorm()函数是求向量长度的平方。
cwiseProduct()函数求逐元素相乘的新的Vector3f。

## texture实现

```cpp
return_color = payload.texture->getColor(payload.tex_coords.x(),payload.tex_coords.y());
```
texture中要求我们将纹理颜色视为公式中的kd，通过对代码的观察，我们可以发现kd的值取自texture_color，而texture_color的值取自return_color，所以我们需要在return_color里获取纹理的颜色。

## bump实现
```cpp
float x=normal.x(),y=normal.y(),z=normal.z();
    Eigen::Vector3f t = {x*y/std::sqrt(x*x+z*z),std::sqrt(x*x+z*z),z*y/std::sqrt(x*x+z*z)};
    Eigen::Vector3f b = normal.cross(t);
    Eigen::Matrix3f TBN;
    TBN << t.x(),b.x(),normal.x(),  //将切线空间里的坐标变换到视空间
        t.y(),b.y(),normal.y(),
        t.z(),b.z(),normal.z();
    float u=payload.tex_coords.x(),v=payload.tex_coords.y();
    float w=payload.texture->width,h=payload.texture->height;

    float dU=kh*kn*(payload.texture->getColor(u+1.0f/w,v).norm()-payload.texture->getColor(u,v).norm());
    float dV=kn*kh*(payload.texture->getColor(u,v+1.0f/h).norm()-payload.texture->getColor(u,v).norm());

    Eigen::Vector3f ln={-dU,-dV,1};

    normal = (TBN*ln).normalized();
```
bump将法线按纹理颜色来掰歪，把纹理颜色当成高度。
du和dv求梯度，指那一小块像素在u方向和v方向上升高了多少，然后控制法线往那边偏移。


## displacement实现
```
float x=normal.x(),y=normal.y(),z=normal.z();
    Eigen::Vector3f t = {x*y/std::sqrt(x*x+z*z),std::sqrt(x*x+z*z),z*y/std::sqrt(x*x+z*z)};
    Eigen::Vector3f b = normal.cross(t);
    Eigen::Matrix3f TBN;
    TBN << t.x(),b.x(),normal.x(),
        t.y(),b.y(),normal.y(),
        t.z(),b.z(),normal.z();
    float u=payload.tex_coords.x(),v=payload.tex_coords.y();
    float w=payload.texture->width,h=payload.texture->height;
    float h0=payload.texture->getColor(u,v).norm();
    point += kn*normal*h0;

    float dU=kh*kn*(payload.texture->getColor(u+1.0f/w,v).norm()-h0);
    float dV=kn*kh*(payload.texture->getColor(u,v+1.0f/h).norm()-h0);

    Eigen::Vector3f ln={-dU,-dV,1};

    normal = (TBN*ln).normalized();
```
通过与bump实现的代码对比我们可以发现，displacement中我们将point加上一个值，这个值就是point沿着法线方向的偏移量，在后续的光栅化中，我们以更新后的point来进行计算。
## 提高项：双线性插值
```cpp
Eigen::Vector3f getColorBilinear(float u,float v){
        auto u_img = u * width;
        auto v_img = (1 - v) * height;

        int x = std::floor(u_img),y = std::floor(v_img);

        float fx=u_img-x,fy=v_img-y;

        auto fetch=[&](int cx ,int  cy){
            cx=std::clamp(cx,0,width-1);
            cy=std::clamp(cy,0,height-1);
            auto c = image_data.at<cv::Vec3b>(cy,cx);
            return Eigen::Vector3f(c[0],c[1],c[2]);
        };

        Eigen::Vector3f c00=fetch(x,y);
        Eigen::Vector3f c01=fetch(x,y+1);
        Eigen::Vector3f c10=fetch(x+1,y);
        Eigen::Vector3f c11=fetch(x+1,y+1);

        Eigen::Vector3f top = c00 * (1-fx) + c10 * fx;
        Eigen::Vector3f bottom = c01 * (1-fx) + c11 * fx;

        return top * (1-fy) + bottom * fy;
    }
```
我们通过观察可以发现，纹理颜色颜色的获取主要通过Texture.cpp里的getcolor来获取，所以我们在Texture.cpp里实现一个getcolorbilinear()来替换其他地方的getcolor函数即可。  
具体实现按照老师介绍的方法实现即可。