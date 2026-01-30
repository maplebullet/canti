#ifndef PROCESS_H
#define PROCESS_H
#include "data_structer.h"
#include"block.h"
//using namespace std;

//计算单帧图像
struct frame_res{
    bool isvalid=false;
    cv::Point2f left_center,right_center,center;
    frame_res(){
        left_center=right_center=center={0,0};
    }
};
frame_res generage_center(std::vector<float>& convertedPressure);

// 生成cop
std::vector<cv::Point2f> generate_cop(const std::vector<cv::Mat>& frames);

//  ---------------------------------------------------静态分析--------------------------------------------

Jingtai jingtai_analyce(const cv::String filepath, const cv::String model_path, int chang = 40, int kuan = 40, int debug = 0);



//  ---------------------------------------------------动态分析-------------------------------------------- 

Dongtai dongtai_analyce(const std::string filepath, double kuan = 84, double chang = 300, int debug = 0);
#endif



// ---------------------------debug---------------------
void test_image(const std::string filepath);