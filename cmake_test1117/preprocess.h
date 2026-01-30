#ifndef PREPROCESS_H
#define PREPROCESS_H
#include<opencv2/opencv.hpp>
#include "utils.h"


Frame deepCopyFrame(const Frame& src);

/*
   0 - blue_limit  白到蓝
   blue_limit - green_limit 蓝到绿
   green_limit - yellow_limit 绿到黄
   yellow_limit - 255         黄到红
*/
cv::Mat heatmap(const cv::Mat& image, int blue_limit, int green_limit, int yellow_limit);
void show(const cv::Mat& image, std::string title);
void show_video(const std::vector<Frame>& frames, int delay = 30, std::string avi_path = "");
std::vector<float> split(const std::string& str, char delimiter);
void filter_frames(std::vector<Frame>& frames, int min_area);
std::vector<Frame> read_video_from_file(const cv::String& filepath, int height, int width, float max_value, float min_value);

#endif
