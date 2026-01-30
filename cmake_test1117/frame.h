//
// Created by zhang on 2025/12/14.
//

#ifndef UNTITLED_FRAME_H
#define UNTITLED_FRAME_H
#include "data_structer.h"
bool isvalidframe(cv::Mat& image,int min_value=5,int min_area=70);
std::vector<Frame> read_video_from_file(const cv::String& filepath, int height, int width, float max_value, float min_value);
void filter_frames(std::vector<Frame>& frames, int min_area);
void clean_noise(cv::Mat& image);
#endif //UNTITLED_FRAME_H
