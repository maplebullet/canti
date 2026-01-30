//
// Created by zhang on 2025/12/14.
//

#include "frame.h"
#include "utils.h"
#include <fstream>
#include <regex>
bool isvalidframe(cv::Mat& image,int min_value,int min_area){
    image.setTo(0,image<min_value);
    cv::Rect leftrect(0, 0, 20, 40), rightrect(20, 0, 20, 40);
    cv::Mat left_image = image(leftrect), right_image = image(rightrect);
    if(mat_area(left_image)<min_area) return false;
    if(mat_area(right_image)<min_area) return false;
    return true;
}
std::vector<float> split(const std::string& str, char delimiter) {
    std::vector<float> tokens;
    if (str.length() < 10) return tokens;
    // 定义正则表达式，匹配方括号中的任意内容
    std::regex bracket_pattern(R"(\[.*?\]\s*)");

    std::string new_str = std::regex_replace(str, bracket_pattern, "");
    std::stringstream ss(new_str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(std::stof(token));
    }
    return tokens;
}
std::vector<Frame> read_video_from_file(const cv::String& filepath, int height, int width, float max_value, float min_value) {
    std::vector<Frame> frames;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open the file!" << std::endl;
        return frames;
    }

    std::string line;
    while (std::getline(file, line)) {
        Frame curr_frame;
        size_t end_bracket = line.find(']');
        if (end_bracket == std::string::npos) {
            std::cerr << "未找到时间戳" << std::endl;
        }
        else {
            std::string timestamp_str = line.substr(1, end_bracket - 1);

            // 将时间戳转换为微秒
            std::tm tm = {};
            std::istringstream timestamp_stream(timestamp_str);  // 这里修正为timestamp_stream
            char colon, dot;
            int hour, min, sec, ms;

            timestamp_stream >> hour >> colon >> min >> colon >> sec >> dot >> ms;

            if (colon != ':' || dot != '.') {
                std::cerr << "时间戳格式不对" << std::endl;
            }

            // 转换为秒
            curr_frame.timestamp = hour * 3600 + min * 60 + sec + ms / 1000.0;

        }

        std::istringstream iss(line);
        std::vector<float> pixel_values = split(line, ' ');

        // Check if the number of pixel values is exactly 1600
        if (pixel_values.size() != height * width) {
            std::cerr << pixel_values.size() << "!=" << height << "*" << width << std::endl;
            continue;
        }
        double sum_value = 0;
        for (float& value : pixel_values) {
            if (value < min_value) value = 0;
            if (value > max_value) {
                value = max_value;
            }
            sum_value += value;
        }
        if (sum_value > 5) {
            // Convert the pixel values into a 40x40 cv::Mat
            cv::Mat img(height, width, CV_32F, pixel_values.data());
            clean_noise(img);
            curr_frame.image = img.clone();
            frames.push_back(curr_frame);
//            std::cout<<sum_value<<std::endl;
        }
    }

    file.close();
    for (int i = 0; i < frames.size(); i++)
        frames[i].frame_idx = i;
    return frames;
}
bool is_valid_jingtai_image(cv::Mat& img,float threshold=10,int max_count=6){
    int height = img.rows;
    int width = img.cols;
    int count = 0;

    // 检查上边缘
    for (int x = 0; x < width; ++x) {
        if (img.at<float>(0, x) > threshold) {
            if (++count > max_count) {
                return false;
            }
        }
    }

    // 检查下边缘
    for (int x = 0; x < width; ++x) {
        if (img.at<float>(height - 1, x) > threshold) {
            if (++count > max_count) {
                return false;
            }
        }
    }

    // 检查左边缘
    for (int y = 1; y < height - 1; ++y) {
        if (img.at<float>(y, 0) > threshold) {
            if (++count > max_count) {
                return false;
            }
        }
    }

    // 检查右边缘
    for (int y = 1; y < height - 1; ++y) {
        if (img.at<float>(y, width - 1) > threshold) {
            if (++count > max_count) {
                return false;
            }
        }
    }

    // 检查中间竖线
    int mid_col = width / 2;
    for (int y = 0; y < height; ++y) {
        if (img.at<float>(y, mid_col) > threshold) {
            if (++count > max_count) {
                return false;
            }
        }
    }

    return true;
}
void filter_frames(std::vector<Frame>& frames, int min_area) {
    // 使用迭代器遍历 frames，这样可以在遍历过程中安全地删除元素
    auto it = frames.begin();
    while (it != frames.end()) {
        // 如果有效面积小于 min_area，将该帧删除
        if (mat_area(it->image) < min_area || !is_valid_jingtai_image(it->image)) {
            it = frames.erase(it); // 删除当前帧，并获取下一个帧的迭代器
        }
        else {
            ++it; // 保留该帧，移动到下一个帧
        }
    }
}

void clean_noise(cv::Mat& image){
    // 创建8邻域的卷积核
    cv::Mat kernel = cv::Mat::ones(3, 3, CV_32F);
    kernel.at<float>(1, 1) = 0;  // 中心点为0，不参与求和

    // 进行卷积
    cv::Mat convResult;
    filter2D(image, convResult, CV_32F, kernel);

    // 创建掩码：8邻域和是否为0
    cv::Mat mask = (convResult == 0);

    // 应用条件：如果8邻域都为0，中心点设为0
    image.setTo(0, mask);
}