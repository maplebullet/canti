//
// Created by zhang on 2025/12/14.
//

#include "dongtai.h"
#include "utils.h"
#include "block.h"

using namespace cv;
void analysis_frames(Dongtai& dongtai, std::vector<cv::Mat>& frames) {
    for (const cv::Mat& image : frames) {
        dongtai.centers.push_back(compute_weighted_centroid(image));
    }
}

void block_calculate(std::vector<Block>& blocks) {
    for (Block& block : blocks) {
        int max_idx = 0;
        for (int i = 0; i < block.frames.size(); i++) {
            Frame& frame = block.frames[i];
            frame.block_idx = i;
            frame.sum_image = mat_sum(frame.image);
            frame.area = mat_area(frame.image);
            frame.center = compute_weighted_centroid(frame.image>0);
            frame.weight_center = compute_weighted_centroid(frame.image);

            if (frame.sum_image > block.frames[max_idx].sum_image) {
                max_idx = i;
            }
            block.cop.push_back(frame.weight_center);
        }
        block.frame_1 = block.frames[0];
        block.frame_5 = block.frames[block.frames.size() - 1];
        block.frame_3 = block.frames[max_idx];


        // 计算平均重心
        cv::Point2f sum(0.0, 0.0);
        for (Frame frame : block.frames) {
            sum.x += frame.weight_center.x;
            sum.y += frame.weight_center.y;
        }
        block.mean_center = cv::Point2f(sum.x / block.frames.size(), sum.y / block.frames.size());

        // 计算扩大化的mask
        cv::resize(block.mask, block.kuodamask, Size(block.mask.cols * 4, block.mask.rows * 4), 0, 0, INTER_BITS);
        cv::resize(block.frame_3.image, block.kuodaimage, Size(block.mask.cols * 4, block.mask.rows * 4), 0, 0, INTER_BITS);
    }
}

int cal_fangxiang(std::vector<Block>& blocks) {
    int fangxiang = 0;
    for (int i = 0; i < blocks.size() - 1; i++) {
        if (blocks[i].mean_center.y < blocks[i + 1].mean_center.y)
            fangxiang++;
        else
            fangxiang--;
    }
    return fangxiang >= 0;
}

void dongtai_frames_filter(std::vector<Frame>& frames){
    std::vector<int> areas;
    areas.reserve(frames.size());

    for(Frame& frame:frames){
        areas.push_back(mat_area(frame.image>50));
    }

    int i=0;
    for(;i<frames.size();i++)
        if(areas[i]>2)
            break;

    if(i>50){
        frames.erase(frames.begin(), frames.begin() + i-50);
    }
    for(i=0;i<frames.size();i++)
        frames[i].frame_idx=i;

}





void cal_shuangzhichengbianyi(Dongtai& dongtai, const std::vector<Block>& blocks) {
    std::vector<double> shuang;
    for (int i = 1; i < blocks.size() - 2; i += 2) {
        Frame start_frame = blocks[i].frame_1;
        Frame end_frame = blocks[i + 2].frame_1;
        double all_time = end_frame.timestamp - start_frame.timestamp;
        double shuangzhicheng = (blocks[i].frame_2.timestamp - blocks[i].frame_1.timestamp) + (blocks[i + 1].frame_2.timestamp - blocks[i + 1].frame_1.timestamp);
        shuang.push_back(shuangzhicheng / all_time);
    }
    //    for(int i=0;i<blocks.size()-1;i++){
    //        if(blocks[i+1].frame_1.frame_idx<blocks[i].frame_5.frame_idx){
    //            shuang.push_back((blocks[i].frame_5.timestamp-blocks[i+1].frame_1.timestamp));
    //        }else{
    //            shuang.push_back(0);
    //        }
    //    }
    dongtai.shuangzhicheng = std::accumulate(shuang.begin(), shuang.end(), 0.0) / shuang.size();
    dongtai.shuangzhicheng_bianyi = cal_bianyi(shuang);
}





void dongtai_filter(std::vector<Block>& blocks, int max_area) {
    for (int i = blocks.size() - 1; i >= 0; i--) {
        if (mat_area(blocks[i].mask) > max_area) {
            blocks.erase(blocks.begin() + i);
        }
    }

    for (int i = 0; i < blocks.size() - 1; i++) {
        if (blocks[i].frame_5.frame_idx > blocks[i + 1].frame_1.frame_idx) {
            for (int j = 0; j < blocks[i].frames.size(); j++) {
                if (blocks[i].frames[j].frame_idx == blocks[i + 1].frame_1.frame_idx) {
                    blocks[i].frame_4 = blocks[i].frames[j];
                    break;
                }
            }

            for (int j = 0; j < blocks[i + 1].frames.size(); j++) {
                if (blocks[i + 1].frames[j].frame_idx == blocks[i].frame_5.frame_idx) {
                    blocks[i + 1].frame_2 = blocks[i + 1].frames[j];
                }
            }
        }
        else {
            blocks[i].frame_4 = blocks[i].frame_5;
            blocks[i + 1].frame_2 = blocks[i + 1].frame_1;
        }

        if (blocks[i].frame_2.frame_idx > blocks[i].frame_4.frame_idx) {
            blocks[i].frame_2 = blocks[i].frame_4;
        }

        if (blocks[i].frame_3.frame_idx > blocks[i].frame_4.frame_idx) {
            blocks[i].frame_3 = blocks[i].frame_4;
        }

        if (blocks[i].frame_2.frame_idx > blocks[i].frame_3.frame_idx) {
            blocks[i].frame_3 = blocks[i].frame_2;
        }

    }
}
std::vector<cv::Mat> generate_frames_from_blocks(std::vector<Block>& blocks) {
    std::vector<cv::Mat> frames;
    if (!blocks.empty()) {
        int start_idx = blocks[0].frame_1.frame_idx;
        int end_idx = blocks[0].frame_5.frame_idx;
        for (const Block& block : blocks) {
            start_idx = std::min(start_idx, block.frame_1.frame_idx);
            end_idx = std::max(end_idx, block.frame_5.frame_idx);
        }
        for (int i = start_idx; i <= end_idx; i++) {
            cv::Mat image = cv::Mat::zeros(blocks[0].frames[0].image.rows, blocks[0].frames[0].image.cols, CV_8UC3);
            for (const Block& block : blocks) {
                if (block.frame_1.frame_idx <= i && block.frame_5.frame_idx >= i) {
                    for (const Frame& frame : block.frames) {
                        if (frame.frame_idx == i) {
                            frame.image.copyTo(image, frame.image>0);
                            break;
                        }
                    }
                }
            }
            frames.push_back(image);
        }
    }

    return frames;
}

void dongtai_analyse_all(Dongtai& dongtai,std::vector<Block>& blocks){

    // 每侧脚的偏移角度
    dongtai.left_ori = 0;
    for (Block& block : dongtai.lefts)
        dongtai.left_ori += block.ori;
    dongtai.left_ori /= dongtai.lefts.size();

    dongtai.right_ori = 0;
    for (Block& block : dongtai.rights)
        dongtai.right_ori += block.ori;
    dongtai.right_ori /= dongtai.rights.size();

}
void analysis_dongtai(Dongtai& dongtai) {
    //每侧脚平均图像
    for (int c = 0; c < 4; c++) {
        std::vector<cv::Mat> left, right;
        for (Block& block : dongtai.lefts)  left.push_back(block.mean_image[c]);
        for (Block& block : dongtai.rights) right.push_back(block.mean_image[c]);

        dongtai.left_mean_image[c] = mean_image(left, 0, left.size() - 1);
        dongtai.right_mean_image[c] = mean_image(right, 0, right.size() - 1);
        //show(heatmap(dongtai.left_mean_image[c]), "test");
    }

    //每侧脚平均最大图像
    for (int c = 0; c < 4; c++) {
        std::vector<cv::Mat> left, right;
        for (Block& block : dongtai.lefts)  left.push_back(block.max_image[c]);
        for (Block& block : dongtai.rights) right.push_back(block.max_image[c]);

        dongtai.left_mean_max_image[c] = mean_image(left, 0, left.size() - 1);
        dongtai.right_mean_max_image[c] = mean_image(right, 0, right.size() - 1);
    }

    //每侧脚平均时间
    for (int c = 0; c < 4; c++) {
        double left = 0, right = 0;
        for (Block& block : dongtai.lefts)  left += (block.time[c]);
        for (Block& block : dongtai.rights) right += (block.time[c]);

        dongtai.left_mean_time[c] = left * 1.0 / dongtai.lefts.size();
        dongtai.right_mean_time[c] = right * 1.0 / dongtai.rights.size();
    }

    //每侧脚平均压力
    for (int c = 0; c < 4; c++) {
        double left = 0, right = 0;
        for (Block& block : dongtai.lefts)  left += (block.pressure[c]);
        for (Block& block : dongtai.rights) right += (block.pressure[c]);

        dongtai.left_mean_pressures[c] = left * 1.0 / dongtai.lefts.size();
        dongtai.right_mean_pressures[c] = right * 1.0 / dongtai.rights.size();
    }

    //每侧脚平均面积
    for (int c = 0; c < 4; c++) {
        double left = 0, right = 0;
        for (Block& block : dongtai.lefts)  left += (block.area[c]);
        for (Block& block : dongtai.rights) right += (block.area[c]);

        dongtai.left_mean_areas[c] = left * 1.0 / dongtai.lefts.size();
        dongtai.right_mean_areas[c] = right * 1.0 / dongtai.rights.size();
    }

    //每侧脚平均前后侧
    std::vector<double> left_up_lowers, right_up_lowers;
    for (Block& block : dongtai.lefts) if (block.up_lower > 0) left_up_lowers.push_back(block.up_lower);
    for (Block& block : dongtai.rights)if (block.up_lower > 0) right_up_lowers.push_back(block.up_lower);
    dongtai.left_mean_up_lower = std::accumulate(left_up_lowers.begin(), left_up_lowers.end(), 0.0) / left_up_lowers.size();
    dongtai.right_mean_up_lower = std::accumulate(right_up_lowers.begin(), right_up_lowers.end(), 0.0) / right_up_lowers.size();

    //每侧脚平均左右侧
    std::vector<double> left_lrs, right_lrs;
    for (Block& block : dongtai.lefts) if (block.left_right > 0) left_lrs.push_back(block.left_right);
    for (Block& block : dongtai.rights)if (block.left_right > 0) right_lrs.push_back(block.left_right);

    dongtai.left_mean_left_right = std::accumulate(left_lrs.begin(), left_lrs.end(), 0.0) * 1.0 / left_lrs.size();
    dongtai.right_mean_left_right = std::accumulate(right_lrs.begin(), right_lrs.end(), 0.0) * 1.0 / right_lrs.size();
}

// 拟合直线
cv::Vec4f fit_line(const std::vector<cv::Point2d>& points) {
    cv::Vec4f line; // 存储直线参数 [vx, vy, x0, y0]
    if (points.size() < 2) {
        std::cerr << "Not enough points to fit a line!" << std::endl;
        return line;
    }
    cv::fitLine(points, line, cv::DIST_L2, 0, 0.01, 0.01);
    return line;
}
std::vector<cv::Point2f> get_tubao(cv::Mat& image) {
    std::vector<cv::Point2f> points;
    cv::findNonZero(image, points);
    if (points.empty()) {
        return points;
    }

    // 2. 计算凸包
    std::vector<cv::Point2f> hull;
    cv::convexHull(points, hull);
    return hull;
}
float classify(std::vector<Block>& blocks, Dongtai& result) {
    // 对所有重心拟合一条直线
    std::vector<cv::Point2d> centers;
    for (Block& block : blocks) {
        centers.push_back(compute_weighted_centroid(block.kuodamask));
    }

    cv::Vec4f line = fit_line(centers);
    float vx = line[0], vy = line[1], x0 = line[2], y0 = line[3];
    // 直线方程： y - y0 = (vy/vx) * (x - x0)
    float k = vy / vx; // 斜率
    float b = y0 - k * x0; // 截距

    for (Block& block : blocks) {
        block.buttom_center = findEdgeCenters(block.kuodamask, result.fangxiang);
    }


    // 绘图
    // 2. 创建可视化图像（黑色背景）
    cv::Mat visualization = cv::Mat::zeros(480, 160, CV_8UC3); // 与mask尺寸一致

    // 4. 绘制拟合直线（红色）
    cv::Point2f pt1, pt2;
    if (std::abs(vx) > 1e-5) {
        // 直线方程: (y - y0) / vy = (x - x0) / vx
        pt1 = cv::Point(0, y0 - x0 * vy / vx);
        pt2 = cv::Point(visualization.cols - 1, y0 + (visualization.cols - 1 - x0) * vy / vx);
    }
    else {
        // 垂直线（vx≈0）
        pt1 = cv::Point(x0, 0);
        pt2 = cv::Point(x0, visualization.rows - 1);
    }
    cv::line(visualization, pt1, pt2, cv::Scalar(0, 0, 255), 2); // 红色直线

    // 3. 绘制所有masks（绿色）
    for (Block& block : blocks) {
        cv::Mat coloredMask;
        cv::cvtColor(block.kuodamask, coloredMask, cv::COLOR_GRAY2BGR); // 转为彩色
        coloredMask.setTo(cv::Scalar(0, 255, 0), block.kuodamask);      // 非零像素设为绿色
        cv::add(visualization, coloredMask, visualization);  // 叠加到可视化图像

        cv::circle(visualization, block.buttom_center, 4, cv::Scalar(255, 0, 255), -1);
    }
    // 5. 显示结果
    //show(visualization, "test");



    for (int i = 0; i < blocks.size(); i++) {
        Block block = blocks[i];
        Point2d point = compute_weighted_centroid(block.kuodamask);
        // 根据点的位置，判断其在直线上方还是下方
        block.is_left = point.y >= k * point.x + b;
        if (block.is_left) {
            result.lefts.push_back(block);
        }
        else {
            result.rights.push_back(block);
        }
    }

    dongtai_analyse_all(result,blocks);

    result.lefts.clear();result.rights.clear();

    for (int i = 1; i < blocks.size()-1; i++) {
        Block block = blocks[i];
        Point2d point = compute_weighted_centroid(block.kuodamask);
        // 根据点的位置，判断其在直线上方还是下方
        block.is_left = point.y >= k * point.x + b;
        if (block.is_left) {
            result.lefts.push_back(block);
        }
        else {
            result.rights.push_back(block);
        }
    }

    return k;
}

void cal_result(std::vector<Block>& blocks, Dongtai& result, double kuan, double chang, float k) {
    // 计算相邻脚之间的 步长 步宽
    std::vector<double> buchang, bukuan;
    double suofang_x = kuan / 40;
    double suofang_y = chang / 120;
    for (int i = 0; i < blocks.size() - 1; i++) {
        //        std::pair<double,double> res=calculate_distance(blocks[i].mean_center,blocks[i+1].mean_center,k,kuan,chang);

        //        std::pair<double,double> res=calculate_distance(blocks[i].mean_center,blocks[i+1].buttom_center,k,kuan,chang);
        //        buchang.push_back(std::abs(res.first)/4);
        //        bukuan.push_back(std::abs(res.second)/4);

        double dy = (blocks[i].buttom_center.y - blocks[i + 1].buttom_center.y);
        buchang.push_back(std::abs(dy) * suofang_y / 4);

        double dx = (blocks[i].buttom_center.x - blocks[i + 1].buttom_center.x);
        bukuan.push_back(std::abs(dx) * suofang_x / 4);

    }

    // 计算步长平均值
    result.buchang = std::accumulate(buchang.begin(), buchang.end(), 0.0) / buchang.size();
    result.bukuan = std::accumulate(bukuan.begin(), bukuan.end(), 0.0) * 1.0 / bukuan.size();

    // 将变异性存入结果
    result.buchang_bianyi = cal_bianyi(buchang);
    result.bukuan_bianyi = cal_bianyi(bukuan);
    // 计算步宽平均值
    // 平均速度
//    double distance= -(calculate_distance(blocks[0].mean_center,blocks[blocks.size()-1].mean_center,k,kuan,chang).first);
    double distance = abs(blocks[0].mean_center.y - blocks[blocks.size() - 1].mean_center.y) * chang / 120;
    double time = blocks[blocks.size() - 1].frame_1.timestamp - blocks[0].frame_1.timestamp;
    result.mean_speed = distance / time;
    // 步频
    result.bupin = (blocks.size() - 1) * 1.0 / time;
    for (int i = 1; i < result.centers.size(); i++) {
        result.speeds.push_back(distance_points(result.centers[i], result.centers[i - 1], kuan, chang));
    }
    result.mean_speed = result.mean_speed;
    result.bupin = result.bupin;
}
