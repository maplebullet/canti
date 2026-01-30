#include "preprocess.h"
#include "utils.h"


using namespace cv;

Frame deepCopyFrame(const Frame& src) {
    Frame dst;
    dst.timestamp = src.timestamp;
//    dst.mask = src.mask.clone();
    dst.image = src.image.clone();
    dst.jiuzheng_image = src.jiuzheng_image.clone();
    dst.jiuzheng_mask = src.jiuzheng_mask.clone();
    dst.sum_image = src.sum_image;
    dst.area = src.area;
    dst.center = src.center;
    dst.frame_idx = src.frame_idx;
    dst.weight_center = src.weight_center;
    dst.block_idx = src.block_idx;
    return dst;
}

void show(const cv::Mat& image, std::string title) {
    cv::namedWindow(title, 0);
    cv::imshow(title, image);
    cv::resizeWindow(title, image.cols, image.rows);
    waitKey(0);
}
cv::Mat heatmap(const cv::Mat& image, int blue_limit, int green_limit, int yellow_limit) {
    cv::Mat normal;
    normalize(image, normal, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::Mat output;
    int cols = 160;
    int rows = cols / image.cols * image.rows;
    cv::resize(normal, output, Size(cols, rows), 0, 0, INTER_BITS);
    //resize(image, output, Size(400, 400));
    // 使用双边滤波进行图像平滑处理
    //cv::Mat filteredImage;
    //cv::bilateralFilter(output, filteredImage, 8, 30, 30);
    cv::Mat heatmap = cv::Mat::zeros(output.rows, output.cols, CV_8UC3);
    for (int i = 0; i < heatmap.cols; ++i) {
        for (int j = 0; j < heatmap.rows; ++j) {
            int value = output.at<uchar>(cv::Point(i, j));
            uchar blue, green, red = value;
            if (value < blue_limit) {
                blue = 255;
                red = green = 255 - (value * 255 / blue_limit);
            }
            else if (value < green_limit) {
                red = 0;
                green = (value - blue_limit) * 255 / (green_limit - blue_limit);
                blue = 255 - green;
            }
            else if (value < yellow_limit) {
                blue = 0;
                green = 255;
                red = (value - green_limit) * 255 / (yellow_limit - green_limit);
            }
            else {
                red = 255;
                green = blue = 255 - (value - yellow_limit) * 255 / (255 - yellow_limit);
            }
            heatmap.at<cv::Vec3b>(cv::Point(i, j)) = cv::Vec3b(blue, green, red);
        }
    }

    return heatmap;
}

void show_video(const std::vector<Frame>& frames, int delay, const std::string avi_path) {
    if (!avi_path.empty()) {
        int cols = 160;
        int rows = cols / frames[0].image.cols * frames[0].image.rows;
        cv::Size frameSize(cols, rows);
        int codec = cv::VideoWriter::fourcc('X', 'V', 'I', 'D');

        // 创建 VideoWriter 对象
        cv::VideoWriter writer(avi_path, codec, 2, frameSize);

        if (!writer.isOpened()) {
            std::cerr << "Could not open the output video file for write!" << std::endl;
            return;
        }

        // 写入每一帧
        for (const auto& frame : frames) {
            cv::Mat image = heatmap(frame.image);
            writer.write(image);
        }

        writer.release();  // 释放视频文件
    }
    else {
        for (const auto& frame : frames) {

            // Show the frame
            cv::imshow("Video", heatmap(frame.image));

            // Wait for a delay (in milliseconds) before showing the next frame
            if (cv::waitKey(delay) >= 0) {
                break;  // Exit if any key is pressed
            }
        }

        // Close the window after the video is done
        cv::destroyAllWindows();
    }

}






