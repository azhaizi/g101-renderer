#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

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

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    int num = control_points.size();
    if(num==1)return control_points[num-1];

    std::vector<cv::Point2f> elsePoint;

    for(int i=0;i<num-1;i++){
        elsePoint.push_back((1-t)*control_points[i]+t*control_points[i+1]); 
    }

    return cv::Point2f(recursive_bezier(elsePoint,t));

}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    float t = 0;
    
    while(t<=1){
        cv::Point2f Point = recursive_bezier(control_points,t);
        put_pixel(window,Point,cv::Vec3b(0,255,0));
        t+=0.001f;
    }

}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
            // naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
