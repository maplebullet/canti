#ifndef BLOCK_H
#define BLOCK_H
#include "utils.h"
#include "preprocess.h"
// #include "process.h"






void dongtai_get_blocks(const std::vector<Frame>& frames, std::vector<Block>& blocks, int min_area, double overlap_threshold, int num_limit);

cv::Point2f findEdgeCenters(const cv::Mat& mat, int fangxiang);
/*
 * 动态-分出5个时间点，4个阶段。 平均压力、平均面积、
 * 平均图像、最大图像、cop、平均重心
 * */
void analysis_block(std::vector<Block>& blocks, int chang, int kuan);

void show_blocks(std::vector<Block>& blocks, int delay = 50, bool save_mp4 = false);
void jingtai_filter(std::vector<Block>& blocks);

/*
 * 计算图像的旋转矩形和旋转角(倾斜角度)，保存纠正后的图像
 * 动态-计算纠正后图像的左右比例和前后比例
 * */
void jiuzheng_blocks(std::vector<Block>& blocks);
//四个区域的面积
void cal_up_lower_left_right(cv::Mat& img, int& up, int& lower, int& left, int& right);
bool block_cmp(Block b1, Block b2);
cv::Mat cal_quanzhouqi(const std::vector<Block>& blocks);

#endif
