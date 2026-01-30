//
// Created by zhang on 2025/12/6.
//

#ifndef QINGDAO_BUTAI_TUOYUAN_H
#define QINGDAO_BUTAI_TUOYUAN_H
#include "data_structer.h"
Ellipse tuoyuan(const std::vector<cv::Point2f>& points, double target_coverage = 0.95);
void visualizeEllipse(const std::vector<cv::Point2f>& points,
    const Ellipse& ellipse,
    const std::string& patternName,
    const std::string& windowName = "Ellipse Fitting");


#endif //QINGDAO_BUTAI_TUOYUAN_H
