//
// Created by zhang on 2025/12/2.
//

#include "jingtai.h"
#include "data_structer.h"
#include "utils.h"
#include "block.h"
#include "tuoyuan.h"
#include "zugong.h"
void jingtai_get_blocks(const std::vector<Frame>& frames, Jingtai& res) {
    for (const auto& frame_idx : frames) {
        // 二值化图像
        Danzu left, right;
        cv::Rect leftrect(0, 0, 20, 40), rightrect(20, 0, 20, 40);
        cv::Mat left_image = frame_idx.image(leftrect), right_image = frame_idx.image(rightrect);
        cv::Mat left_bin_frame = left_image > 0, right_bin_frame = right_image > 0;
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)); // 可以根据需要调整结构元素大小
        cv::dilate(left_bin_frame, left_bin_frame, kernel);
        cv::dilate(right_bin_frame, right_bin_frame, kernel);

        // 检测 脚形区域
        // todo 检查这里是否有噪声点
        std::vector<cv::Mat> left_detected_masks = detect_masks(left_bin_frame, 5), right_detected_masks = detect_masks(right_bin_frame, 5);
        cv::Mat left_mask = cv::Mat::zeros(left_bin_frame.size(), left_bin_frame.type());
        cv::Mat right_mask = cv::Mat::zeros(right_bin_frame.size(), right_bin_frame.type());
        for(int i=0;i<2&&i<left_detected_masks.size();i++) {
            cv::Mat& detected_mask=left_detected_masks[i];
            cv::bitwise_or(left_mask, detected_mask, left_mask);
        }
        for(int i=0;i<2&&i<right_detected_masks.size();i++) {
            cv::Mat& detected_mask=right_detected_masks[i];
            cv::bitwise_or(right_mask, detected_mask, right_mask);
        }
        left.image = cv::Mat::zeros(frame_idx.image.size(), frame_idx.image.type());
        right.image = cv::Mat::zeros(frame_idx.image.size(), frame_idx.image.type());
        left_image.copyTo(left.image(leftrect), left_mask); right_image.copyTo(right.image(rightrect), right_mask);

        res.left_images.push_back(left.image);
        res.right_images.push_back(right.image);
        // 扩大图像
        resize(left.image, left.kuoda_image, cv::Size(160, 160), 0, 0, cv::INTER_BITS);
        resize(right.image, right.kuoda_image, cv::Size(160, 160), 0, 0, cv::INTER_BITS);
        left.kuoda_image*=(1.0* mat_sum(left.image)/mat_sum(left.kuoda_image));
        right.kuoda_image*=(1.0* mat_sum(right.image)/mat_sum(right.kuoda_image));
        left.jiuzheng_image = jingtai_jiuzheng(left.kuoda_image);
        right.jiuzheng_image = jingtai_jiuzheng(right.kuoda_image);
        left.jiuzheng_image*=(1.0* mat_sum(left.image)/mat_sum(left.jiuzheng_image));
        right.jiuzheng_image*=(1.0* mat_sum(right.image)/mat_sum(right.jiuzheng_image));


        Jingtai_frame jingtaiFrame{ left,right,Lean(0,0,0,0,0,0,0,0) };
        res.frames_res.push_back(jingtaiFrame);
    }
}
void jingtai_frames_calulate(Jingtai& result, const cv::String& model_path, double chang, double kuan) {
    //--------左侧-----------
    for (int i=result.frames_res.size()-1;i>=0;i--) {
        Jingtai_frame& frame=result.frames_res[i];

        cv::Mat tmp = frame.left.image;

        //脚长脚宽
        cv::Rect rect = cal_dim(frame.left.jiuzheng_image);
        frame.left.jiaochang = rect.height * chang / 160;
        frame.left.jiaokuan = rect.width * kuan / 160;


        //面积
        frame.left.area = mat_area(tmp);
        frame.left.isvalid =
                frame.left.area > 70
                && frame.left.jiaochang<32
                && frame.left.jiaochang>20
                && frame.left.jiaokuan<15
                ;
        if (!frame.left.isvalid) {
//            result.frames_res.erase(result.frames_res.begin()+i);
            continue;
        };

        //计算足弓指数
        cv::Mat merge_image = frame.left.jiuzheng_image.clone();
        std::pair<cv::Mat,double> pair= calculare_ratio(merge_image, model_path,10);
        frame.left.zugongimage=pair.first;
        frame.left.zugongzhishu = pair.second;
        frame.left.zugongtype= cal_zugongtype(frame.left.zugongzhishu);

        //压力
        frame.left.pressure = mat_sum(tmp) * 1.0;

        //重心
        frame.left.center = compute_weighted_centroid(tmp);


        //左右倾斜
        int left, right, up, lower;
        cal_up_lower_left_right(frame.left.jiuzheng_image, up, lower, left, right);
        if (right > 0) frame.left.nei_wai = (right * 1.0 / left);
        if (lower > 0) frame.left.up_back = (up * 1.0 / lower);
    }

    //--------右侧-----------
    for (int i=result.frames_res.size()-1;i>=0;i--) {
        Jingtai_frame& frame=result.frames_res[i];
        cv::Mat tmp = frame.right.image;

        //脚长脚宽
        cv::Rect rect = cal_dim(frame.right.jiuzheng_image);
        frame.right.jiaochang = rect.height * chang / 160;
        frame.right.jiaokuan = rect.width * kuan / 160;


        //面积
        frame.right.area = mat_area(tmp);
        frame.right.isvalid =
                frame.right.area > 70
                && frame.right.jiaochang<32
                && frame.right.jiaochang>20
                && frame.left.jiaokuan<15
                ;
        if (!frame.right.isvalid) {
//            result.frames_res.erase(result.frames_res.begin()+i);
            continue;
        }

        //计算足弓指数
        cv::Mat merge_image = frame.right.jiuzheng_image.clone();
        std::pair<cv::Mat,double> pair= calculare_ratio(merge_image, model_path,10);
        frame.right.zugongimage=pair.first;
        frame.right.zugongzhishu = pair.second;
        frame.right.zugongtype= cal_zugongtype(frame.right.zugongzhishu);

        //压力
        frame.right.pressure = mat_sum(tmp) * 1.0;

        //重心
        frame.right.center = compute_weighted_centroid(tmp);



        //左右倾斜
        int left, right, up, lower;
        cal_up_lower_left_right(frame.right.jiuzheng_image, up, lower, left, right);
        if (right > 0) frame.right.nei_wai = (left * 1.0 / right);
        if (lower > 0) frame.right.up_back = (up * 1.0 / lower);
    }

    // 综合
    for (Jingtai_frame& frame : result.frames_res){
        if(frame.left.isvalid && frame.right.isvalid){
            if(abs(frame.left.jiaochang-frame.right.jiaochang)>5){
                frame.left.isvalid=frame.right.isvalid=false;
                continue;
            }
            cv::Mat merge_image = cv::Mat::zeros(160, 160, CV_8U);
            frame.left.kuoda_image(cv::Rect(0, 0, 80, 160)).copyTo(merge_image(cv::Rect(0, 0, 80, 160)));
            frame.right.kuoda_image(cv::Rect(80, 0, 80, 160)).copyTo(merge_image(cv::Rect(80, 0, 80, 160)));
            frame.lean = calculate_lean(merge_image, 10);
            result.centers.push_back(compute_weighted_centroid(merge_image));
        }
    }
}

void jingtai_calculate(Jingtai& result) {
    //-----左侧
    {
        std::vector<double> pressures, nei_wais, up_backs, zugongs, areas, jiaochangs, jiaokuans;
        std::vector<cv::Mat> jiuzhengs;
        for (Jingtai_frame& frame : result.frames_res) {
            if (frame.left.isvalid) {
                pressures.push_back(frame.left.pressure);
                jiaochangs.push_back(frame.left.jiaochang);
                jiaokuans.push_back(frame.left.jiaokuan);
                zugongs.push_back(frame.left.zugongzhishu);
                areas.push_back(frame.left.area);
                nei_wais.push_back(frame.left.nei_wai);
                up_backs.push_back(frame.left.up_back);
                jiuzhengs.push_back(frame.left.jiuzheng_image);
                result.left.centers.push_back(frame.left.center);
            }
            
        }
        //脚长
        result.left.jiaochang = trimmedMean_float(jiaochangs);
        //脚宽
        result.left.jiaokuan = trimmedMean_float(jiaokuans);

        // 平均面积
        result.left.area = trimmedMean_float(areas);
        // 平均压力
        result.left.pressure = trimmedMean_float(pressures)/result.left.area;

        result.left.nei_wai = trimmedMean_float(nei_wais);
        result.left.up_back = trimmedMean_float(up_backs);

        //拟合椭圆
        result.left.ellipse = tuoyuan(result.left.centers,0.5);
        //visualizeEllipse(result.left.centers, result.left.ellipse, "left");

        //足弓指数
        result.left.zugongzhishu = trimmedMean_float(zugongs);
        result.left.zugongtype= cal_zugongtype(result.left.zugongzhishu);

        //傅里叶变换
        result.left.ffts = fft(result.left.centers);
        //平均图像
        std::vector<cv::Mat> images;
        result.left.image = mean_image(result.left_images, 0, result.left_images.size() - 1);
        // 重心宽度高度
        cv::Rect boundingBox = cv::boundingRect(result.left.centers);
        result.left.center_hight = boundingBox.height;
        result.left.center_width = boundingBox.width;

        //纠正图像
        result.left.jiuzheng_image= mean_image(jiuzhengs,0,jiuzhengs.size()-1);
    }

    //-----右侧
    {
        std::vector<double> pressures, nei_wais, up_backs,areas, zugongs, jiaochangs, jiaokuans;
        std::vector<cv::Mat> jiuzhengs;
        for (Jingtai_frame& frame : result.frames_res) {
            if (frame.right.isvalid) {
                pressures.push_back(frame.right.pressure);
                jiaochangs.push_back(frame.right.jiaochang);
                jiaokuans.push_back(frame.right.jiaokuan);
                zugongs.push_back(frame.right.zugongzhishu);
                areas.push_back(frame.right.area);
                nei_wais.push_back(frame.right.nei_wai);
                up_backs.push_back(frame.right.up_back);
                jiuzhengs.push_back(frame.right.jiuzheng_image);
                result.right.centers.push_back(frame.right.center);
            }
        }
        //脚长
        result.right.jiaochang = trimmedMean_float(jiaochangs);
        //脚宽
        result.right.jiaokuan = trimmedMean_float(jiaokuans);

        // 平均面积
        result.right.area = trimmedMean_float(areas);
        // 平均压力
        result.right.pressure = trimmedMean_float(pressures) / result.right.area;

        result.right.nei_wai = trimmedMean_float(nei_wais);
        result.right.up_back = trimmedMean_float(up_backs);

        //拟合椭圆
        result.right.ellipse = tuoyuan(result.right.centers,0.8);
        //visualizeEllipse(result.right.centers, result.right.ellipse, "right");

        //足弓指数
        result.right.zugongzhishu = trimmedMean_float(zugongs);
        result.right.zugongtype= cal_zugongtype(result.right.zugongzhishu);
        //傅里叶变换
        result.right.ffts = fft(result.right.centers);
        //平均图像
        std::vector<cv::Mat> images;
        result.right.image = mean_image(result.right_images, 0, result.right_images.size() - 1);
        // 重心宽度高度
        cv::Rect boundingBox = cv::boundingRect(result.right.centers);
        result.right.center_hight = boundingBox.height;
        result.right.center_width = boundingBox.width;

        result.right.jiuzheng_image= mean_image(jiuzhengs,0,jiuzhengs.size()-1);
    }

    // 综合
    {
        result.ellipse = tuoyuan(result.centers);
        result.ffts = fft(result.centers);
        if (result.left.pressure > 0 && result.right.pressure > 0)
            result.left_right = result.left.pressure / result.right.pressure;
        // lean
        std::vector<double> lus, lbs, rus, rbs, lrs, lfs, rfs;
        for (Jingtai_frame& frame : result.frames_res) {
            lus.push_back(frame.lean.left_up);
            lbs.push_back(frame.lean.left_back);
            lrs.push_back(frame.lean.left_right);
            lfs.push_back(frame.lean.left_forward);
            rus.push_back(frame.lean.right_up);
            rbs.push_back(frame.lean.right_back);
            rfs.push_back(frame.lean.right_forward);
        }
        result.lean.left_up = trimmedMean_float(lus);
        result.lean.left_back = trimmedMean_float(lbs);
        result.lean.left_right = trimmedMean_float(lrs);
        result.lean.left_forward = trimmedMean_float(lfs);
        result.lean.right_up = trimmedMean_float(rus);
        result.lean.right_back = trimmedMean_float(rbs);
        result.lean.right_forward =trimmedMean_float(rfs);

        double d = cv::max(cv::max(abs(result.lean.left_right), abs(result.lean.left_forward)), abs(result.lean.right_forward));
        if (d > 0.1) result.lean.degree = 0;
        else if (d > 0.08) result.lean.degree = 1;
        else result.lean.degree = 2;
    }
}

cv::Rect cal_dim(const cv::Mat& im) {
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(im>0, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (contours.empty()) {
        return {};
    }
    // 计算外接矩形
    std::vector<cv::Point> contour;
    for (std::vector<cv::Point>& c : contours)
        for (cv::Point& p : c)
            contour.push_back(p);
    cv::Rect boundingRect = cv::boundingRect(contour);
    return boundingRect;
}
