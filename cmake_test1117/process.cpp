#include "process.h"
#include <string>
#include <numeric>
#include "preprocess.h"
#include "block.h"
#include "jingtai.h"
#include "tuoyuan.h"
#include "frame.h"
#include "dongtai.h"
#include <opencv2/core/utils/logger.hpp>
#include <qDebug>
using namespace cv;

frame_res generage_center(std::vector<float>& convertedPressure){
    if (convertedPressure.size() != 40*40) {
        return frame_res();
    }
    cv::Mat img(40, 40, CV_32F, convertedPressure.data());
    if(isvalidframe(img)){
        cv::Rect leftrect(0, 0, 20, 40), rightrect(20, 0, 20, 40);
        cv::Mat left_image = img(leftrect), right_image = img(rightrect);
        frame_res res;
        res.isvalid=true;
        res.center= compute_weighted_centroid(img>0);
        res.left_center= compute_weighted_centroid(left_image);
        res.right_center= compute_weighted_centroid(right_image);
        res.right_center.x+=20;
        return res;
    }
    return frame_res();

}
std::vector<cv::Point2f> generate_cop(const std::vector<cv::Mat>& frames) {
    std::vector<cv::Point2f> cop;
    for (const cv::Mat& ima : frames) {
        cv::Point2f point = compute_weighted_centroid(ima);
        cop.push_back(point);
    }
    return cop;
}


Jingtai jingtai_analyce(const cv::String filepath, const cv::String model_path, int chang, int kuan, int debug) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);
    Jingtai result;
    std::vector<Frame> frames = read_video_from_file(filepath, 40, 40, 500, 5);
    if (frames.size()<10){
        result.backward={false,"数据量不足"};
        return result;
    }
    filter_frames(frames, 20);
    if (frames.size()<10){
        result.backward={false,"数据无效/触碰边界"};
        return result;
    }

    jingtai_get_blocks(frames, result);
    jingtai_frames_calulate(result, model_path, chang, kuan);
    if(result.centers.empty()){
        result.backward={false,"未匹配到有效脚型"};
        return result;
    }
    jingtai_calculate(result);

    result.backward.success= true;
    return result;
}

Dongtai dongtai_analyce(const std::string filepath, double kuan, double chang, int debug) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);
    Dongtai result;
    std::vector<Frame> frames = read_video_from_file(filepath, 120, 40, 500, 3);
    dongtai_frames_filter(frames);
//    show_video(frames,50);
    if (frames.size()<20) {
        result.backward={false,"数据量不足"};
        return result;
    }

    std::vector<Block> blocks;
    dongtai_get_blocks(frames, blocks, 10, 0.5, 10);
    if(blocks.empty()){
        result.backward={false,"脚型数量不足"};
        return result;
    }

    block_calculate(blocks);
    dongtai_filter(blocks, 200);
//    show_blocks(blocks,50);
    result.fangxiang = cal_fangxiang(blocks);
    result.quanzhouqitu = cal_quanzhouqi(blocks);
    //    show(heatmap(result.quanzhouqitu),"test");


    //    show_blocks(blocks,100);
    if (blocks.size() < 4){
        result.backward={false,"脚型数量不足"};
        return result;
    }
    cal_shuangzhichengbianyi(result, blocks);
    analysis_block(blocks, chang, kuan);

    float k = classify(blocks, result);
    cal_result(blocks, result, kuan, chang, k);

    std::vector<cv::Mat> new_frames = generate_frames_from_blocks(blocks);
    result.cop = generate_cop(new_frames);

    analysis_frames(result, new_frames);// centers

    jiuzheng_blocks(result.lefts);
    jiuzheng_blocks(result.rights);
    //show_blocks(blocks, 20);

    analysis_dongtai(result);

    // 计算cop最小外接多边形面积
    result.cop_ellipse = tuoyuan(result.cop);
    //result.cop_area=minBoundingPolygonAreaCV(result.cop);
    qDebug() << "帧数:" << frames.size();
    qDebug() << "blocks数:" << blocks.size();
    result.backward.success = true;
    //    show(heatmap(result.cop),"cop");
    return result;
}

void test_image(const std::string filepath){
    std::vector<Frame> frames = read_video_from_file(filepath, 120, 40, 500, 3);
    // 二值化图像
    cv::Mat bin_frame = frames[0].image > 0;
    Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 3)); // 可以根据需要调整结构元素大小
    cv::dilate(bin_frame, bin_frame, kernel);
    // 检测 masks
    std::vector<cv::Mat> detected_masks = detect_masks(bin_frame, 5);
    return;
}
