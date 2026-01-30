//
// Created by zhang on 2025/12/22.
//

#ifndef UNTITLED_ZUGONG_H
#define UNTITLED_ZUGONG_H
#include<opencv2/opencv.hpp>
int cal_zugongtype(double ratio);
cv::Mat pred1(const cv::Mat& image, const cv::String& model_path);
std::pair<cv::Mat,double> calculare_ratio(const cv::Mat& im, const cv::String model_path, int threshold);
#endif //UNTITLED_ZUGONG_H
