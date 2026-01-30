//
// Created by zhang on 2025/12/22.
//

#include "zugong.h"
#include "utils.h"

int cal_zugongtype(double ratio){
    if (ratio < 0.21) {
        return -1;
    }
    else if (ratio < 0.26) {
        return 0;
    }
    else {
        return 1;
    }
}

double cal_bili(cv::Mat& image, double threshold) {
    cv::Mat k = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::Mat opening;
    cv::morphologyEx(image, opening, cv::MORPH_OPEN, k);
    cv::normalize(opening, opening, 0, 255, cv::NORM_MINMAX, CV_8U);

    cv::Mat mask = (opening > threshold) / 255;

    int row_start = -1;
    for (int i = 0; i < mask.rows; i++) {
        if (mask.at<uchar>(i, mask.cols / 2) == 1) {
            row_start = i;
            break;
        }
    }

    int row_end = mask.rows;
    for (int i = mask.rows - 1; i >= 0; i--) {
        if (mask.at<uchar>(i, mask.cols / 2) == 1) {
            row_end = i + 1;
            break;
        }
    }

    if (row_start == -1 || row_end == mask.rows) return 0.0;





    image(cv::Rect(0, 0, mask.cols, row_start)) = 0;

    mask = (image > threshold) / 255;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty()) {
        return 0.0;
    }

    std::vector<cv::Point> contour;
    for (std::vector<cv::Point>& c : contours)
        for (cv::Point& p : c)
            contour.push_back(p);
    cv::Rect boundingRect = cv::boundingRect(contour);
    //cv::RotatedRect rect = cv::minAreaRect(contour);
    //cv::Point2f box[4];
    //rect.points(box);

    //int col_start = static_cast<int>(box[0].x);
    //int col_end = static_cast<int>(box[2].x) + 1;
    int col_start = boundingRect.x;
    int col_end = boundingRect.x + boundingRect.width + 1;
    col_start = std::max(col_start, 1);
    col_end = std::min(col_end, mask.cols);


    int hight = row_end - row_start;

    int y0 = row_start;
    int y1 = row_start + static_cast<int>(hight / 3);
    int y2 = row_start + static_cast<int>(hight * 2 / 3);
    int y3 = row_end;

    std::vector<cv::Point> line0 = { cv::Point(col_start, y0), cv::Point(col_end, y0) };
    std::vector<cv::Point> line1 = { cv::Point(col_start, y1), cv::Point(col_end, y1) };
    std::vector<cv::Point> line2 = { cv::Point(col_start, y2), cv::Point(col_end, y2) };
    std::vector<cv::Point> line3 = { cv::Point(col_start, y3), cv::Point(col_end, y3) };

    cv::normalize(image(cv::Rect(0, y0, image.cols, y1 - y0 + 1)), image(cv::Rect(0, y0, image.cols, y1 - y0 + 1)), 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::normalize(image(cv::Rect(0, y1, image.cols, y2 - y1 + 1)), image(cv::Rect(0, y1, image.cols, y2 - y1 + 1)), 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::normalize(image(cv::Rect(0, y2, image.cols, y3 - y2 + 1)), image(cv::Rect(0, y2, image.cols, y3 - y2 + 1)), 0, 255, cv::NORM_MINMAX, CV_8U);

    mask = (image > threshold) / 255;
    try {
        int area1 = cv::sum(mask(cv::Rect(col_start, y0, col_end - col_start, y1 - y0)))[0] +
                    (cv::sum(mask(cv::Rect(col_start, y1, col_end - col_start, 1)))[0] * (hight % 3 / 3));

        int area2 = (cv::sum(mask(cv::Rect(col_start, y1, col_end - col_start, 1)))[0] * (1 - hight % 3 / 3)) +
                    cv::sum(mask(cv::Rect(col_start, y1 + 1, col_end - col_start, y2 - y1 - 1)))[0] +
                    (cv::sum(mask(cv::Rect(col_start, y2, col_end - col_start, 1)))[0] * (hight % 3 / 3));

        int area3 = (cv::sum(mask(cv::Rect(col_start, y2, col_end - col_start, 1)))[0] * (1 - hight % 3 / 3)) +
                    cv::sum(mask(cv::Rect(col_start, y2 + 1, col_end - col_start, y3 - y2 - 1)))[0];

        for (const auto& line : { line0, line1, line2, line3 }) {
            cv::line(mask, line[0], line[1], cv::Scalar(255, 0, 0), 2);
            cv::line(image, line[0], line[1], cv::Scalar(255, 0, 0), 2);
        }

        if (area1 + area2 + area3 == 0) {
            return 0.0;
        }
        //show(heatmap(image, 20),"test");

        return area2 * 1.0 / (area1 + area2 + area3);

    }
    catch (const std::exception& e) {
        cv::imwrite("bug.jpg", heatmap(image));
        return  0;
    }


}
std::pair<cv::Mat,double> calculare_ratio(const cv::Mat& im, const cv::String model_path, int threshold) {
    cv::Mat image = im;
    cv::Mat normalized; cv::normalize(image, normalized, 0, 1, cv::NORM_MINMAX, CV_32F);
    cv::Mat label = pred1(normalized, model_path);
    normalized.setTo(0, label == 1);
    cv::normalize(normalized, normalized, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::Mat filtered_image;
    bilateralFilter(normalized, filtered_image, 5, 50, 50);
    filtered_image.setTo(0, filtered_image < threshold);


    // 过滤掉脚趾部分
    cv::Mat bin_frame = filtered_image > 0;
    std::vector<cv::Mat> detected_masks = detect_masks(bin_frame, 5);
    cv::Mat mask = cv::Mat::zeros(bin_frame.size(), bin_frame.type());
    for(int i=0;i<2&&i<detected_masks.size();i++){
        cv::Mat& detected_mask=detected_masks[i];
        cv::Point2f cent= compute_weighted_centroid(detected_mask);
        if(cent.y>60)
            cv::bitwise_or(mask, detected_mask, mask);
    }
    filtered_image.setTo(0, mask==0);
    double ratio = cal_bili(filtered_image, 0);
    //show(heatmap(filtered_image), "test");
    return {filtered_image,ratio};
}

cv::Mat pred1(const cv::Mat& image, const std::string& model_path) {
    // 创建一个用于存储最大值索引的矩阵

    cv::Mat maxIndexMat(160, 160, CV_8U, cv::Scalar(0));

    //return image;
    cv::dnn::Net net = cv::dnn::readNetFromONNX(model_path);
    net.setInput(cv::dnn::blobFromImage(image, 1.0, cv::Size(160, 160), cv::Scalar(), false, false, CV_32F));


    // 运行推理
    cv::Mat label = net.forward();



    // 遍历每个像素
    for (int i = 0; i < 160; ++i) {
        for (int j = 0; j < 160; ++j) {
            // 遍历每个通道
            uchar maxChannelIndex = label.ptr<float>(0, 0, i)[j] < label.ptr<float>(0, 1, i)[j];
            // 将最大值的通道索引写入maxIndexMat
            maxIndexMat.at<uchar>(i, j) = maxChannelIndex;
        }
    }

    return maxIndexMat;

}