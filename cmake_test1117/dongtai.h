//
// Created by zhang on 2025/12/14.
//

#ifndef UNTITLED_DONGTAI_H
#define UNTITLED_DONGTAI_H
#include "data_structer.h"
/*
 * 根据根据blocks清除 前面的无用帧 和 尾后的无用帧
 * 记录result的每帧重心和每帧速度
 * */
std::vector<cv::Mat> generate_frames_from_blocks(std::vector<Block>& blocks);

void analysis_frames(Dongtai& dongtai, std::vector<cv::Mat>& frames);

/*
 * 动态-去掉
 * -- 超过 max_frames 帧数的block，
 * -- 面积大于 max_area 的block
 * */
void dongtai_filter(std::vector<Block>& blocks, int max_area = 100);

/*
 * 计算每侧脚的 平均图像、平均最大图像、步态平均时间、平均前后侧、
 * 平均左右侧、cops、平均偏移角度
 * */
void analysis_dongtai(Dongtai& dongtai);
/*
 * 将blocks 分为左右脚
 * */
float classify(std::vector<Block>& blocks, Dongtai& result);

/*
 * 计算 步长 步宽 步速 步频
 * **/
void cal_result(std::vector<Block>& blocks, Dongtai& dongtai, double kuan, double chang, float k);


void cal_shuangzhichengbianyi(Dongtai& dongtai, const std::vector<Block>& blocks);


void block_calculate(std::vector<Block>& blocks);

int cal_fangxiang(std::vector<Block>& blocks);

void dongtai_frames_filter(std::vector<Frame>& frames);

void dongtai_analyse_all(Dongtai& dongtai,std::vector<Block>& blocks);
#endif //UNTITLED_DONGTAI_H
