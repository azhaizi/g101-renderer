# games作业101作业4

## bezier实现
```
void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    float t = 0;
    while(t<=1){
        cv::Point2f Point = recursive_bezier(control_points,t);
        put_pixel(window,Point,cv::Vec3b(0,255,0));
        t+=0.001f;
    }

}
```
根据文档说明，t 在 0 到 1 的范围内进行迭代，并在每次迭代中使 t 增加一个微小值。
所以我们先给 t 赋值 0 ，然后以 0.001f 的大小递增。
在循环里，我们的点使用 recursive_bezier() 函数来获取，再将得到的点设置到像素上。

## recursive_bezier实现
```
cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    int num = control_points.size();
    if(num==1)return control_points[num-1];

    std::vector<cv::Point2f> elsePoint;

    for(int i=0;i<num-1;i++){
        elsePoint.push_back((1-t)*control_points[i]+t*control_points[i+1]); 
    }

    return cv::Point2f(recursive_bezier(elsePoint,t));
}
```
在这个函数里，我们要实现一个递归算法来计算出每个点。  
for循环里，按给定的t进行辅助点的计算，一直向下递归，直到只剩最后一个点，那个点就是我们最后画出来的像素点。

## 提高项：线段的反走样
```cpp
static void put_pixel(cv::Mat &window , const cv::Point2f point , const cv::Vec3b color , float radius = 1.5f){
    int x0 = (int)std::floor(point.x);
    int y0 = (int)std::floor(point.y);

    for(int dx=-1;dx<=1;dx++){
        for(int dy=-1;dy<=1;dy++){
            int x=x0+dx,y=y0+dy;
            if(x>=window.cols||x<0||y<0||y>window.rows) continue;

            float cx=x+0.5f,cy=y+0.5f;
            float dist = std::sqrt((point.x-cx)*(point.x-cx)+(point.y-cy)*(point.y-cy));
            if(dist>=radius) continue;

            float w = (radius-dist)/radius;    //线性衰减；

            cv::Vec3b &px=window.at<cv::Vec3b>(y,x);
            for(int c=0;c<3;c++){
                px[c] = std::max(px[c] , cv::saturate_cast<uchar>(w*color[c]));
            }

        }
    }
}
```
我们增加一个函数，用于根据像素中心来计算周围像素的颜色。