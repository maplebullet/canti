//
// Created by zhang on 2025/12/2.
//

#ifndef UNTITLED_JINGTAI_H
#define UNTITLED_JINGTAI_H
#include "process.h"
void jingtai_get_blocks(const std::vector<Frame>& frames, Jingtai& res);
void jingtai_frames_calulate(Jingtai& result, const cv::String& model_path, double chang, double kuan);
void jingtai_calculate(Jingtai& result);
cv::Rect cal_dim(const cv::Mat& im);
#endif //UNTITLED_JINGTAI_H
