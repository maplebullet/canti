#ifndef TORCH_PROCESS_H
#define TORCH_PROCESS_H

#include<opencv2/opencv.hpp>
//#include <utility>
#include <numeric>
#include "data_structer.h"
//using namespace cv;




std::vector<cv::Mat> detect_masks(const cv::Mat& bin_img, int threshold);

//转vector到Mat
void putText(cv::Mat& image, double value, int x, int y);
cv::Mat heatmap(const cv::Mat& image, int blue_limit = 50, int green_limit = 100, int yello_limit = 150);

//图像面积
int mat_area(const cv::Mat& binary_img);
//图像压力和
int mat_sum(const cv::Mat& img);
//图像的重心
cv::Point2f compute_weighted_centroid(const cv::Mat& img);

double distance_points(cv::Point point1, cv::Point point2, double kuan, double chang);
//多项式拟合
void polyfit(std::vector<double>& vec, int n = 4);

cv::Mat convert_vector_to_mat(const std::vector<uint8_t>& images, uchar high_limit = 50);
std::vector<uint8_t> convert_mat_to_vector(const cv::Mat& images);

cv::RotatedRect calcu_rect(const cv::Mat& ima);
std::pair<cv::Mat, float> cal_pianyi(const cv::Mat& image, int threshold);
std::pair<cv::Mat, float> jiuzheng(const cv::Mat& ima, cv::RotatedRect rect, float ori = 0);
cv::Mat jingtai_jiuzheng(const cv::Mat& ima);

//傅里叶分析
std::vector<cv::Vec2f> fft(std::vector<cv::Point2f>& coordinates);

double computeMinDistanceBetweenMasks(const cv::Mat& mask1, const cv::Mat& mask2);
std::pair<double, double> calculate_distance(cv::Point2f& center1, cv::Point2f& center2, float k, double kuan, double chang);

struct zuGongImage {
    cv::Mat image;
    cv::Mat label;
    zuGongImage(cv::Mat img, cv::Mat lab) : image(std::move(img)), label(std::move(lab)) {}
};
//脚趾识别--用了torch 比较耗时，所以单独拎出来
//zuGongImage recoglize(const cv::Mat& images, std::string model_path);



double mean_sum_image(std::vector<Frame> imgs, int start, int end);
double mean_area_image(std::vector<Frame> imgs, int start, int end);
double mean_area_image_with_resize(std::vector<Frame> imgs, int start, int end, int chang, int kuan);
cv::Mat mean_image_frame(std::vector<Frame> imgs, int start, int end);
cv::Mat max_image_frame(std::vector<Frame> imgs, int start, int end);
cv::Mat mean_image(std::vector<cv::Mat> imgs, int start, int end);
cv::Mat max_image(std::vector<cv::Mat> imgs, int start, int end);

//计算足弓指数



// 计算最小外接多边形面积
double minBoundingPolygonAreaCV(const std::vector<cv::Point2f>& points);
double cal_bianyi(const std::vector<double>& vec);


double trimmedMean_float(const std::vector<double>& data, float trim_ratio = 0.1f);
cv::Point2f distanceBasedTrimmedMean(std::vector<cv::Point2f>& points, float trim_ratio = 0.1f);

























//计算倾斜角度
Lean calculate_lean(const cv::Mat& image, int threshold = 20);


struct dim {
    cv::Mat left_image;
    cv::Mat right_image;
    cv::Rect left;
    cv::Rect right;
    dim() = default;
    dim(cv::Rect l, cv::Rect r, cv::Mat li, cv::Mat ri) :left(l), right(r), left_image(std::move(li)), right_image(std::move(ri)) {}
};
//计算脚长脚宽
dim calculate_dim(const cv::Mat& image, int threshold = 10);


#endif
