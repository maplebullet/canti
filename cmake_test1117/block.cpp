#include "block.h"
#include <string>
#include <numeric>
#include "utils.h"
#include "preprocess.h"

using namespace cv;


// 在底边附近寻找实际有像素的线段端点
cv::Point2f findActualBottomCenter(const std::vector<cv::Point>& contour,
    const cv::Point2f& bottom1, const cv::Point2f& bottom2) {
    // 确定底边的y坐标范围（取较高的y值，因为底边是y最大的边）
    float bottomY = std::max(bottom1.y, bottom2.y);

    // 容忍范围（像素）
    const float tolerance = 3.0f;
    float minY = bottomY - tolerance;
    float maxY = bottomY + tolerance;

    // 收集在底边附近的所有轮廓点
    std::vector<cv::Point> bottomPoints;
    for (const auto& point : contour) {
        if (point.y >= minY && point.y <= maxY) {
            bottomPoints.push_back(point);
        }
    }

    if (bottomPoints.empty()) {
        // 如果没有找到点，返回原底边中心
        return (bottom1 + bottom2) * 0.5f;
    }

    // 找到x坐标最小和最大的点（实际接触的两端）
    auto compareX = [](const cv::Point& a, const cv::Point& b) {
        return a.x < b.x;
    };

    auto minXPoint = *std::min_element(bottomPoints.begin(), bottomPoints.end(), compareX);
    auto maxXPoint = *std::max_element(bottomPoints.begin(), bottomPoints.end(), compareX);

    // 计算实际接触线段的中点
    cv::Point2f actualCenter;
    actualCenter.x = (minXPoint.x + maxXPoint.x) / 2.0f;
    actualCenter.y = bottomY; // 使用底边的y坐标

    return actualCenter;
}
// 计算计算最小外接矩形投影极值点----计算脚底中心
cv::Point2f findEdgeCenters(const cv::Mat& mat, int fangxiang) {
    std::vector<cv::Point> points;

    // 2. 查找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mat.clone(), contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        return { 0, 0 };
    }

    // 3. 计算最小外接矩形
    cv::RotatedRect minRect = cv::minAreaRect(contours[0]);
    cv::Point2f rectPoints[4];
    minRect.points(rectPoints);

    // 4. 明确识别顶边和底边（基于y坐标）
    // 先找到y坐标最小和最大的两个点（顶边和底边）
    auto compareY = [](const cv::Point2f& a, const cv::Point2f& b) {
        return a.y < b.y;
    };

    // 对顶点按y坐标排序
    std::sort(rectPoints, rectPoints + 4, compareY);

    // 5. 确定顶边和底边
    // 顶边由y坐标最小的两个点组成
    if (fangxiang) {
        return findActualBottomCenter(contours[0], rectPoints[0], rectPoints[1]);
    }
    else {
        return findActualBottomCenter(contours[0], rectPoints[2], rectPoints[3]);
    }
    cv::Point2f topEdgeCenter = (rectPoints[0] + rectPoints[1]) * 0.5f;

    // 底边由y坐标最大的两个点组成
    cv::Point2f bottomEdgeCenter = (rectPoints[2] + rectPoints[3]) * 0.5f;

    // 6. 根据参数返回对应的中心点
    return fangxiang ? topEdgeCenter : bottomEdgeCenter;
}


// Function to calculate the intersection area of two binary masks
cv::Mat intersection(const cv::Mat& mask1, const cv::Mat& mask2) {
    cv::Mat inter;
    cv::bitwise_and(mask1, mask2, inter);
    return inter;
}

// Function to calculate the overlap ratio (intersection over smaller area)
double overlap_ratio(const cv::Mat& mask1, const cv::Mat& mask2) {
    cv::Mat inter = intersection(mask1, mask2);
    double inter_area = mat_area(inter);
    double min_area = std::min(mat_area(mask1), mat_area(mask2));
    return inter_area / min_area;
}

bool compareLength(const Block& a, const Block& b) {
    return a.frames.size() > b.frames.size();
}
void jingtai_filter(std::vector<Block>& blocks) {
    for (Block& block : blocks) {
        for (int i = block.frames.size() - 1; i >= 0; --i) {
            cv::Mat inter = intersection(block.frames[i].image>0, block.mask);
            if (1.0 * mat_area(inter) / mat_area(block.mask) > 0.3) {
                cv::Point center = compute_weighted_centroid(block.frames[i].image);
                block.cop.push_back(center);
            }
            else {
                block.frames.erase(block.frames.begin() + i);
            }
        }
    }
    if (blocks.size() < 2) return;
    std::nth_element(blocks.begin(), blocks.begin() + 2, blocks.end(), compareLength);
    blocks.resize(2);
    if (compute_weighted_centroid(blocks[0].mask).x > compute_weighted_centroid(blocks[1].mask).x) {
        swap(blocks[0], blocks[1]);
    }
}

void jiuzheng_blocks(std::vector<Block>& blocks) {
    for (Block& block : blocks) {
        //计算旋转中心和旋转角
        int rows = block.mask.rows, cols = block.mask.cols;
        cv::Mat convertedMat;
        block.mask.convertTo(convertedMat, CV_8U, 255, 0);
        cv::Mat output; cv::resize(convertedMat, output, Size(160, 160 * rows / cols), 0, 0, cv::INTER_LINEAR);
        cv::RotatedRect rect = calcu_rect(output);
        std::pair<cv::Mat, float> result1 = jiuzheng(output, rect);
        std::pair<cv::Mat, float> result2 = cal_pianyi(result1.first, 0);
        block.ori = result1.second + result2.second;
        block.rect = calcu_rect(block.mask);

        //动态-纠正平均图像和最大图像
        if (block.mean_image[0].cols > 0) {
            for (int i = 0; i < 4; i++) {
                block.mean_image[i] = jiuzheng(block.mean_image[i], block.rect, block.ori).first;
                block.max_image[i] = jiuzheng(block.max_image[i], block.rect, block.ori).first;
            }
//            for (cv::Point2f& center : block.cop) {
//                int cols = block.mean_image[0].cols, rows = block.mean_image[0].rows;
//                int dx = cols / 2 - block.rect.center.x;
//                int dy = rows / 2 - block.rect.center.y;
//                center.x += dx;
//                center.y += dy;
//            }
        }

        // 纠正 原始图像
        for (int i = 0; i < block.frames.size(); i++) {
            cv::Mat normalized;
            cv::normalize(block.frames[i].image, normalized, 0, 1, cv::NORM_MINMAX);
            cv::Mat convertedMat;
            normalized.convertTo(convertedMat, CV_8U, 255, 0);
            cv::Mat output; cv::resize(convertedMat, output, Size(160, 160 * rows / cols), 0, 0, cv::INTER_LINEAR);
            std::pair<cv::Mat, float> result = jiuzheng(output, rect, block.ori);
            cv::Mat image = result.first;
            block.frames[i].jiuzheng_image = image.clone();
            block.frames[i].jiuzheng_mask = (image > 0);
        }

        int left, right, up, lower;
        if (block.mean_image[0].cols > 0) {
            cal_up_lower_left_right(block.frames[block.frame_3.block_idx].jiuzheng_image, up, lower, left, right);
            if (right == 0) block.left_right = -1;
            else            block.left_right = left * 1.0 / right;
            if (lower == 0) block.up_lower = -1;
            else            block.up_lower = up * 1.0 / lower;
        }

    }
}

void cal_up_lower_left_right(cv::Mat& img, int& up, int& lower, int& left, int& right) {
    // 寻找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(img>0, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (contours.size() == 0) {
        return;
    }
    std::vector<cv::Point> contour;
    for (std::vector<cv::Point>& c : contours)
        for (cv::Point& p : c)
            contour.push_back(p);
    // 获取最大轮廓的外接矩形
    cv::Rect boundingBox = cv::boundingRect(contour);

    // 提取外接矩形区域的图像
    cv::Mat roi = img(boundingBox);

    // 划分为上下左右四个部分
    int midX = roi.cols / 2;
    int midY = roi.rows / 2;
    if (midX < 2 || midY < 2) {
        up = 0;
        lower = 0;
        left = 0;
        right = 0;
    }
    else {
        // 左半部分
        cv::Mat leftPart = roi(cv::Rect(0, 0, midX, roi.rows));
        // 右半部分
        cv::Mat rightPart = roi(cv::Rect(midX, 0, roi.cols - midX, roi.rows));
        // 上半部分
        cv::Mat upperPart = roi(cv::Rect(0, 0, roi.cols, midY));
        // 下半部分
        cv::Mat lowerPart = roi(cv::Rect(0, midY, roi.cols, roi.rows - midY));
        up = cv::sum(upperPart)[0];
        lower = cv::sum(lowerPart)[0];
        left = cv::sum(leftPart)[0];
        right = cv::sum(rightPart)[0];
    }

}
// Function to process each frame and track the evolution of blocks



void dongtai_get_blocks(const std::vector<Frame>& frames, std::vector<Block>& blocks, int min_area, double overlap_threshold, int num_limit) {
    std::vector<Block> active_blocks;
    for (int frame_idx = 0; frame_idx < frames.size(); frame_idx++) {
        // 二值化图像
        cv::Mat bin_frame = frames[frame_idx].image > 0;
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 3)); // 可以根据需要调整结构元素大小
        cv::dilate(bin_frame, bin_frame, kernel);
        // 检测 masks
        std::vector<cv::Mat> detected_masks = detect_masks(bin_frame, 5);
        if (detected_masks.empty()) continue;

        for (cv::Mat detected_mask : detected_masks) {
            bool is_new_block = true;
            // 对每个mask，找到对应的activate_block就合并入
            cv::threshold(detected_mask, detected_mask, 0, 1, cv::THRESH_BINARY);
            for (auto& active_block : active_blocks) {
                if (overlap_ratio(detected_mask, active_block.mask) >= overlap_threshold) {
                    cv::bitwise_or(active_block.mask, detected_mask, active_block.mask);
                    active_block.frame_5=frames[frame_idx];
                    is_new_block = false;
                    break;
                }
            }

            // 找不到activate_block，视为新的activate_block
            if (is_new_block) {
                Block new_block;
                new_block.mask = detected_mask.clone();
                new_block.frame_1=new_block.frame_5=frames[frame_idx];
                active_blocks.push_back(new_block);
            }
        }


        // End blocks that are no longer present
        for (auto it = active_blocks.begin(); it != active_blocks.end();) {
            bool is_active = false;
            for (cv::Mat detected_mask : detected_masks) {
                cv::threshold(detected_mask, detected_mask, 0, 1, cv::THRESH_BINARY);
                if (overlap_ratio(detected_mask, it->mask) >= overlap_threshold) {
                    is_active = true;
                    break;
                }
            }
            if (!is_active) {
                blocks.push_back(*it);
                it = active_blocks.erase(it);
            }
            else {
                ++it;
            }
        }
    }
    //结束收尾
    for (const Block& active_block : active_blocks)
        blocks.push_back(active_block);

    //    //test
    //    for(Block& block:blocks){
    //        show(heatmap(block.mask),"test");
    //    }

    //补充完整
    for (Block& block : blocks) {
        for(int i=block.frame_1.frame_idx;i<=block.frame_5.frame_idx;i++){
            Frame newframe= deepCopyFrame(frames[i]);
            newframe.image.release();
            frames[i].image.copyTo(newframe.image,block.mask);
            newframe.block_idx=i-block.frame_1.frame_idx;
            block.frames.push_back(newframe);
        }
    }


    //前向拓展
    for (Block& block : blocks) {
        for (int i = block.frames[0].frame_idx - 1; i >= 0; i--) {
            cv::Mat testmask=block.mask*200;
            cv::Mat bin_frame = frames[i].image > 10;
            // 检测 masks
            std::vector<cv::Mat> detected_masks = detect_masks(bin_frame, 0);
            if (detected_masks.empty()) break;

            int flag = 1;
            for (cv::Mat detected_mask : detected_masks) {
                cv::threshold(detected_mask, detected_mask, 0, 1, cv::THRESH_BINARY);
                if (overlap_ratio(detected_mask, block.mask) >= overlap_threshold) {
                    Frame frame = deepCopyFrame(frames[i]);
                    frame.image.release();
                    frames[i].image.copyTo(frame.image, detected_mask);
//                    frame.mask = detected_mask.clone();
                    block.frames.insert(block.frames.begin(), frame);
                    flag = 0;
                    break;
                }
            }
            if (flag) break;
        }
    }

    //test
//    for(Block& block:blocks){
//        show(heatmap(block.mask),"test");
//    }

    //后处理
    for (Block& block : blocks) {
        for (int i = (int)block.frames.size() - 1; i >= 0; i--) {
            if (mat_area(block.frames[i].image) < 3) {
                block.frames.erase(block.frames.begin() + i);
            }
        }
    }
    // 过滤
    std::vector<Block> filtered_blocks;
    for (Block& block : blocks) {
        if (block.frames.size() >= num_limit) {
            int count = 0;
            for (Frame& frame : block.frames) {
                if (mat_area(frame.image) > min_area)
                    count++;
            }

            if (count > 5) {
                filtered_blocks.push_back(block);
            }
        }
    }
    blocks = std::move(filtered_blocks);

    std::sort(blocks.begin(), blocks.end(),
              [](const Block& a, const Block& b) {
                  if (a.frames.empty() || b.frames.empty()) return false;
                  return a.frames[0].frame_idx < b.frames[0].frame_idx;
              });
}

void analysis_block(std::vector<Block>& blocks, int chang, int kuan) {
    for (int i = 1; i < blocks.size() - 1; i++) {
        Block& block = blocks[i];
        //4段时间
        block.time[0] = block.frame_2.timestamp - block.frame_1.timestamp;
        block.time[1] = block.frame_3.timestamp - block.frame_2.timestamp;
        block.time[2] = block.frame_4.timestamp - block.frame_3.timestamp;
        block.time[3] = block.frame_5.timestamp - block.frame_4.timestamp;


        //平均面积
        block.area[0] = mean_area_image_with_resize(block.frames, 0, block.frame_2.block_idx, chang, kuan);
        block.area[1] = mean_area_image_with_resize(block.frames, block.frame_2.block_idx,
            block.frame_3.block_idx, chang, kuan);
        block.area[2] = mean_area_image_with_resize(block.frames, block.frame_3.block_idx,
            block.frame_4.block_idx, chang, kuan);
        block.area[3] = mean_area_image_with_resize(block.frames, block.frame_4.block_idx,
            block.frame_5.block_idx, chang, kuan);

        //平均压力
        block.pressure[0] = mean_sum_image(block.frames, 0, block.frame_2.block_idx) / block.area[0];
        block.pressure[1] = mean_sum_image(block.frames, block.frame_2.block_idx, block.frame_3.block_idx) / block.area[1];
        block.pressure[2] = mean_sum_image(block.frames, block.frame_3.block_idx, block.frame_4.block_idx) / block.area[2];
        block.pressure[3] = mean_sum_image(block.frames, block.frame_4.block_idx, block.frame_5.block_idx) / block.area[3];

        //平均图像
        block.mean_image[0] = mean_image_frame(block.frames, 0, block.frame_2.block_idx);
        block.mean_image[1] = mean_image_frame(block.frames, block.frame_2.block_idx, block.frame_3.block_idx);
        block.mean_image[2] = mean_image_frame(block.frames, block.frame_3.block_idx, block.frame_4.block_idx);
        block.mean_image[3] = mean_image_frame(block.frames, block.frame_4.block_idx, block.frame_5.block_idx);

        //最大图像
        block.max_image[0] = max_image_frame(block.frames, 0, block.frame_2.block_idx);
        block.max_image[1] = max_image_frame(block.frames, block.frame_2.block_idx, block.frame_3.block_idx);
        block.max_image[2] = max_image_frame(block.frames, block.frame_3.block_idx, block.frame_4.block_idx);
        block.max_image[3] = max_image_frame(block.frames, block.frame_4.block_idx, block.frame_5.block_idx);
    }
}


void show_blocks(std::vector<Block>& blocks, int delay, bool save_mp4) {
    if (save_mp4) {
        for (int i = 0; i < blocks.size(); i++) {
            char buffer[50];
            std::sprintf(buffer, "video_%d.avi", i + 1);
            std::string str(buffer);
            show_video(blocks[i].frames, delay, str);
        }
    }
    for (Block& block : blocks) {
        show_video(block.frames, delay);
        cv::waitKey(1000);
    }
}

cv::Mat cal_quanzhouqi(const std::vector<Block>& blocks) {
    // 绘图
    // 2. 创建可视化图像（黑色背景）
    cv::Mat visualization = cv::Mat::zeros(480, 160, CV_32F); // 与mask尺寸一致
    // 3. 绘制所有masks（绿色）
    for (const Block& block : blocks) {
        cv::Mat roi;
        cv::Mat block_maximg= max_image_frame(block.frames,0,block.frames.size()-1);
        cv::resize(block_maximg,block_maximg,Size(160, 480), 0, 0, cv::INTER_LINEAR);
        block_maximg.copyTo(roi, block_maximg>0);// 非零像素设为绿色
        cv::add(visualization, roi, visualization);  // 叠加到可视化图像
    }
    return visualization;
}

