//
// Created by zhang on 2025/12/2.
//

#ifndef UNTITLED_DATA_STRUCTER_H
#define UNTITLED_DATA_STRUCTER_H
#include<opencv2/opencv.hpp>
struct Backward{
    bool success=false;
    std::string info;
};
struct Frame {
    int block_idx = 0;                          //在block中的位置
    int frame_idx = 0;                          //在原始帧中的索引
    double timestamp = 0.0;
    int sum_image = 0;
    int area = 0;
    cv::Point2f center;                     //图像中心
    cv::Point2f weight_center;              //重心
//    cv::Mat mask;
    cv::Mat image;
    cv::Mat jiuzheng_image;
    cv::Mat jiuzheng_mask;
};
// 椭圆结构体
struct Ellipse {
    float a;              // 长轴半径
    float b;              // 短轴半径
    float angle;          // 倾斜角度(度)
    double area;          // 面积
    double eccentricity;  // 离心率
    cv::Point2f center;   // 中心点
    cv::RotatedRect rect; // OpenCV椭圆矩形
    double coverage;      // 实际覆盖率
    int inlierCount;      // 内点数量
    double scaleFactor;   // 缩放因子
    Ellipse() : a(-1), b(-1), angle(-1), area(-1), eccentricity(-1),
                center(-1, -1), rect(), coverage(-1), inlierCount(-1),
                scaleFactor(-1) {}
    Ellipse(float a_val, float b_val, float angle_val, double area_val,
            double eccentricity_val, const cv::Point2f& center_val,
            const cv::RotatedRect& rect_val, double coverage_val,
            int inlierCount_val, double scaleFactor_val)
            : a(a_val), b(b_val), angle(angle_val), area(area_val),
              eccentricity(eccentricity_val), center(center_val), rect(rect_val),
              coverage(coverage_val), inlierCount(inlierCount_val),
              scaleFactor(scaleFactor_val) {}
};
struct Block {
    Frame frame_1;            //开始帧
    Frame frame_2;            //最大生长帧
    Frame frame_3;            //压力最大帧
    Frame frame_4;            //开始收缩帧
    Frame frame_5;            //结尾帧
    bool is_left;

    double time[4] = { 0 };                //4段时间: 毫秒

    cv::Mat mask;                 // 整个步态周期的最大活动区域
    std::vector<Frame> frames;

    double pressure[4] = { 0 };//4个阶段的平均压力
    double area[4] = { 0 };//4个阶段的平均面积
    cv::Mat mean_image[4];//4个阶段的每个点的平均压力
    cv::Mat max_image[4];//(4)  4个阶段的每个点的最大压力
    double up_lower;
    double left_right;

    //5
    std::vector<cv::Point2f> cop; //cop坐标
    cv::Point2f centers[4]; //4个阶段的平均压力的重心位置
    cv::Point2f mean_center;//平均重心
    float ori;//当前block旋转角度
    cv::RotatedRect rect;//当前脚形外接矩形
    cv::Point2f jiaodi_center; //较低正中心



    std::vector<cv::Mat> tmp_images;
    std::vector<cv::Mat> tmp_masks;

    cv::Mat kuodamask;
    cv::Mat kuodaimage;
    cv::Point2f buttom_center;
    bool operator==(const Block& other) const {
        cv::Mat comparison;
        cv::compare(mask, other.mask, comparison, cv::CMP_NE);
        return cv::countNonZero(comparison) == 0;
    }

    // 定义 !=
    bool operator!=(const Block& other) const {
        return !(*this == other);
    }
};
struct Danzu {
    cv::Mat image;                  //图像
    double jiaochang =0;
    double jiaokuan = 0;
    double zugongzhishu = 0;            // 足弓指数
    int zugongtype=-1;                  // 足弓类型： -1代表高足弓，0代表正常，1代表低足弓

    double pressure = 0;           //平均压力
    double area = 0;              //平均面积
    double nei_wai = 1;              //内外占比
    double up_back = 1;                 //前后占比


    int center_width = 1;               //重心左右偏移值
    int center_hight = 1;               //重心前后偏移值
    Ellipse ellipse;                //重心拟合椭圆
    std::vector<cv::Point2f> centers;    //重心           cv::Point2f c=venters[0]  c.x  c.y
    std::vector<cv::Vec2f> ffts;         //重心傅里叶变换   cv::Vec2f   v=ffts[0]     v[0]  v[1]


    cv::Mat kuoda_image;
    cv::Mat jiuzheng_image;
    cv::Point2f center;
    bool isvalid = 0;


    cv::Mat zugongimage; // debug 足弓指数图像;
    Danzu() {
        image = cv::Mat::zeros(40, 40, CV_32F);
        kuoda_image = cv::Mat::zeros(160, 160, CV_8U);
        jiuzheng_image = cv::Mat::zeros(160, 160, CV_32F);
        zugongimage = cv::Mat::zeros(160, 160, CV_8U);
        center=cv::Point2f(0,0);
    }


};
struct Lean {
    double left_up, left_back=-1;//左上比例，左下比例
    double right_up, right_back=-1;//右上比例,右下比例
    double left_right=-1;//左右倾斜状况
    double left_forward=-1;//左脚前后侧重
    double right_forward=-1;//右脚前后侧重
    int degree=0;
    Lean() {}
    Lean(double lu, double lb, double ru, double rb, double lr, double lf, double rf, int d) : left_up(lu), left_back(lb), right_up(ru), right_back(rb), left_right(lr), left_forward(lf), right_forward(rf), degree(d) {}
};
struct Jingtai_frame {  // 每帧的参数
    Danzu left;
    Danzu right;
    Lean lean;
};

struct Jingtai {
    Danzu left, right;
    Lean lean;                           // 偏移度
    double left_right = -1;              //左右足对比
    Ellipse ellipse;                     //重心拟合椭圆
    std::vector<cv::Point2f> centers;    //每帧重心
    std::vector<cv::Vec2f> ffts;
    std::vector<Jingtai_frame> frames_res;

    std::vector<cv::Mat> left_images;
    std::vector<cv::Mat> right_images;

    Backward backward;

};


struct Dongtai {
    std::vector<Block> lefts, rights; //每个block
    cv::Mat left_mean_image[4], right_mean_image[4];  //4个阶段平均图像(压力分布)
    cv::Mat left_mean_max_image[4], right_mean_max_image[4];  //4个阶段平均最大图像(压力分布)
    double left_mean_time[4] = { 0 }, right_mean_time[4] = { 0 };   //4个阶段平均时间
    double left_mean_pressures[4] = { 0 }, right_mean_pressures[4] = { 0 };//4个阶段平均压力
    double left_mean_areas[4] = { 0 }, right_mean_areas[4] = { 0 }; //4个阶段平均面积
    double left_mean_up_lower = -1, right_mean_up_lower = -1;//前后侧受力比例
    double left_mean_left_right = -1, right_mean_left_right = -1;//左右侧受力比例
//    std::vector<std::vector<cv::Point2f>> left_cops, right_cops; //每一个步态周期的cop
    double left_ori = -1, right_ori = -1;                 //左右脚的平均偏移角度
//    std::vector<cv::Point2f> left_cops,right_cops;      // 左脚的cop和右脚的cop

    std::vector<cv::Point2f> centers;            //每一帧的中心
    std::vector<cv::Point2f> cop;                // cop轨迹
    Ellipse cop_ellipse;                 // cop最小你和椭圆
    std::vector<double> speeds;              //每帧的瞬时速度曲线
    double buchang = 0;                     //步长
    double bukuan = 0;                      //步宽
    double shuangzhicheng = 0;              // 双支撑相比例
    double mean_speed = 0;                  //平均速度---每帧经过的像素点
    double bupin = 0;                       //步频
    cv::Mat quanzhouqitu;                 //全周期图

    double buchang_bianyi = 0;              //步长变异性
    double bukuan_bianyi = 0;
    double shuangzhicheng_bianyi = 0;


    Backward backward;
    int fangxiang = 0; // 前进方向


    // 构造函数
    Dongtai() {
        // 初始化 left_mean_image 和 right_mean_image 为 40×120 的全零矩阵
        for (int i = 0; i < 4; ++i) {
            left_mean_image[i] = cv::Mat::zeros(120, 40, CV_32FC1);
            right_mean_image[i] = cv::Mat::zeros(120, 40, CV_32FC1);
        }
    }
};
#endif //UNTITLED_DATA_STRUCTER_H
