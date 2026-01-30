#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <chrono>
#include <limits>
#include "tuoyuan.h"



// 计算点到椭圆的代数距离
double pointToEllipseDistance(const cv::Point2f& point, const cv::RotatedRect& ellipse) {
    cv::Point2f center = ellipse.center;
    float a = ellipse.size.width / 2.0f;
    float b = ellipse.size.height / 2.0f;
    float angle = ellipse.angle * CV_PI / 180.0f;

    // 将点转换到椭圆坐标系
    float x = point.x - center.x;
    float y = point.y - center.y;

    // 反向旋转（将椭圆旋转到坐标轴对齐）
    float cos_angle = cos(angle);
    float sin_angle = sin(angle);

    float x_rot = x * cos_angle + y * sin_angle;
    float y_rot = -x * sin_angle + y * cos_angle;

    // 计算代数距离
    double distance = (x_rot * x_rot) / (a * a) + (y_rot * y_rot) / (b * b) - 1.0;
    return distance;  // 返回有符号的距离
}

// 强制椭圆覆盖指定比例的点
std::pair<cv::RotatedRect, double> enforceCoverage(
    const cv::RotatedRect& initialEllipse,
    const std::vector<cv::Point2f>& points,
    double target_coverage) {

    if (points.empty()) {
        return { initialEllipse, 1.0 };
    }

    // 1. 计算所有点到初始椭圆的距离
    std::vector<double> distances;
    for (const auto& p : points) {
        distances.push_back(pointToEllipseDistance(p, initialEllipse));
    }

    // 2. 排序距离（从大到小）
    std::sort(distances.begin(), distances.end(),
        [](double a, double b) { return a > b; });

    // 3. 找到覆盖target_coverage比例点的最小缩放因子
    int target_index = static_cast<int>(points.size() * (1.0 - target_coverage));
    if (target_index >= static_cast<int>(distances.size())) {
        target_index = distances.size() - 1;
    }

    double max_distance = distances[target_index];

    // 4. 如果max_distance <= 0，说明已经覆盖了足够的点
    if (max_distance <= 0) {
        return { initialEllipse, 1.0 };
    }

    // 5. 计算缩放因子
    double scale_factor = sqrt(1.0 + max_distance);

    // 6. 缩放椭圆
    cv::RotatedRect scaledEllipse = initialEllipse;
    scaledEllipse.size.width *= scale_factor;
    scaledEllipse.size.height *= scale_factor;

    return { scaledEllipse, scale_factor };
}

// 计算椭圆的实际覆盖率
std::pair<int, double> calculateCoverage(const std::vector<cv::Point2f>& points,
    const cv::RotatedRect& ellipse) {
    int inliers = 0;
    for (const auto& p : points) {
        if (pointToEllipseDistance(p, ellipse) <= 0) {  // 代数距离<=0表示在椭圆内或边界上
            inliers++;
        }
    }
    double coverage = static_cast<double>(inliers) / points.size();
    return { inliers, coverage };
}

// 改进的椭圆拟合函数，处理单边密集的点
Ellipse tuoyuan_improved(const std::vector<cv::Point2f>& points, double target_coverage = 0.95) {
    if (points.size() < 5) {
        cv::RotatedRect ellipse = cv::fitEllipseDirect(points);
        auto coverage_info = calculateCoverage(points, ellipse);
        double scale_factor = 1.0;

        // 如果覆盖率不足，尝试强制缩放
        if (coverage_info.second < target_coverage) {
            auto result = enforceCoverage(ellipse, points, target_coverage);
            ellipse = result.first;
            scale_factor = result.second;
            coverage_info = calculateCoverage(points, ellipse);
        }

        float a = ellipse.size.width / 2.0f;
        float b = ellipse.size.height / 2.0f;
        float angle = ellipse.angle;

        if (a < b) {
            std::swap(a, b);
            angle += 90.0f;
            if (angle >= 180.0f) angle -= 180.0f;
        }

        double area = CV_PI * a * b;
        double eccentricity = 0;
        if (a > 0) {
            eccentricity = std::sqrt(std::max(0.0, 1.0 - (b * b) / (a * a)));
        }

        return Ellipse{ a, b, angle, area, eccentricity, ellipse.center, ellipse,
                       coverage_info.second, coverage_info.first, scale_factor };
    }

    int maxIterations = 100;
    int targetInliers = static_cast<int>(points.size() * target_coverage);

    // 1. 初始拟合
    cv::RotatedRect bestEllipse = cv::fitEllipseDirect(points);
    auto bestCoverageInfo = calculateCoverage(points, bestEllipse);
    double bestScaleFactor = 1.0;

    // 2. 如果初始拟合就满足覆盖率，直接返回
    if (bestCoverageInfo.second >= target_coverage) {
        // 计算椭圆参数
        float a = bestEllipse.size.width / 2.0f;
        float b = bestEllipse.size.height / 2.0f;
        float angle = bestEllipse.angle;

        if (a < b) {
            std::swap(a, b);
            angle += 90.0f;
            if (angle >= 180.0f) angle -= 180.0f;
        }

        double area = CV_PI * a * b;
        double eccentricity = 0;
        if (a > 0) {
            eccentricity = std::sqrt(std::max(0.0, 1.0 - (b * b) / (a * a)));
        }

        return Ellipse{ a, b, angle, area, eccentricity, bestEllipse.center, bestEllipse,
                       bestCoverageInfo.second, bestCoverageInfo.first, bestScaleFactor };
    }

    // 3. 使用RANSAC思想多次尝试
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int iter = 0; iter < maxIterations; ++iter) {
        // 随机选择5个点拟合椭圆
        std::vector<cv::Point2f> sample_points;
        for (int i = 0; i < 5; ++i) {
            int idx = std::uniform_int_distribution<>(0, points.size() - 1)(gen);
            sample_points.push_back(points[idx]);
        }

        cv::RotatedRect ellipse = cv::fitEllipseDirect(sample_points);

        // 计算所有点到这个椭圆的距离
        std::vector<std::pair<double, int>> distances;
        for (int i = 0; i < points.size(); ++i) {
            distances.push_back({ std::abs(pointToEllipseDistance(points[i], ellipse)), i });
        }

        // 排序，选择最近的targetInliers个点
        std::sort(distances.begin(), distances.end());

        std::vector<cv::Point2f> inliers;
        for (int i = 0; i < targetInliers; ++i) {
            inliers.push_back(points[distances[i].second]);
        }

        // 用内点重新拟合
        if (inliers.size() >= 5) {
            ellipse = cv::fitEllipseDirect(inliers);

            // 强制覆盖
            auto enforced = enforceCoverage(ellipse, points, target_coverage);
            ellipse = enforced.first;
            double scale_factor = enforced.second;

            // 计算覆盖率
            auto coverage_info = calculateCoverage(points, ellipse);

            // 如果这个椭圆更好，更新最佳椭圆
            if (coverage_info.second > bestCoverageInfo.second ||
                (coverage_info.second == bestCoverageInfo.second && scale_factor < bestScaleFactor)) {
                bestEllipse = ellipse;
                bestCoverageInfo = coverage_info;
                bestScaleFactor = scale_factor;

                // 如果达到目标覆盖率，提前结束
                if (coverage_info.second >= target_coverage) {
                    break;
                }
            }
        }
    }

    // 4. 用最佳椭圆重新计算参数
    float a = bestEllipse.size.width / 2.0f;
    float b = bestEllipse.size.height / 2.0f;
    float angle = bestEllipse.angle;

    if (a < b) {
        std::swap(a, b);
        angle += 90.0f;
        if (angle >= 180.0f) angle -= 180.0f;
    }

    double area = CV_PI * a * b;
    double eccentricity = 0;
    if (a > 0) {
        eccentricity = std::sqrt(std::max(0.0, 1.0 - (b * b) / (a * a)));
    }

    return Ellipse{ a, b, angle, area, eccentricity, bestEllipse.center, bestEllipse,
                   bestCoverageInfo.second, bestCoverageInfo.first, bestScaleFactor };
}

// 使用加权最小二乘拟合椭圆
cv::RotatedRect fitEllipseWeighted(const std::vector<cv::Point2f>& points,
    const std::vector<double>& weights) {
    if (points.size() < 5 || points.size() != weights.size()) {
        return cv::fitEllipseDirect(points);
    }

    // 计算加权中心
    double total_weight = 0.0;
    cv::Point2f weighted_center(0, 0);

    for (size_t i = 0; i < points.size(); ++i) {
        weighted_center.x += points[i].x * weights[i];
        weighted_center.y += points[i].y * weights[i];
        total_weight += weights[i];
    }

    if (total_weight > 0) {
        weighted_center.x /= total_weight;
        weighted_center.y /= total_weight;
    }

    // 对点进行中心化
    std::vector<cv::Point2f> centered_points = points;
    for (auto& p : centered_points) {
        p.x -= weighted_center.x;
        p.y -= weighted_center.y;
    }

    // 使用常规方法拟合椭圆
    cv::RotatedRect ellipse = cv::fitEllipseDirect(centered_points);

    // 将中心平移回来
    ellipse.center.x += weighted_center.x;
    ellipse.center.y += weighted_center.y;

    return ellipse;
}

// 改进的椭圆拟合函数，处理单边密集的情况
Ellipse tuoyuan(const std::vector<cv::Point2f>& points, double target_coverage) {
    if (points.size() < 5) {
        return Ellipse();
    }

    // 1. 检测点的密度分布
    std::vector<double> densities(points.size(), 0.0);
    double max_density = 0.0;

    // 计算每个点的局部密度
    for (size_t i = 0; i < points.size(); ++i) {
        double density = 0.0;
        for (size_t j = 0; j < points.size(); ++j) {
            if (i == j) continue;
            double dx = points[i].x - points[j].x;
            double dy = points[i].y - points[j].y;
            double distance = std::sqrt(dx * dx + dy * dy);

            // 使用高斯核函数计算密度
            if (distance < 50.0) {  // 50像素内的邻居
                density += std::exp(-distance * distance / 100.0);
            }
        }
        densities[i] = density;
        max_density = std::max(max_density, density);
    }

    // 归一化密度
    if (max_density > 0) {
        for (auto& d : densities) {
            d /= max_density;
        }
    }

    // 2. 使用密度加权的初始拟合
    std::vector<cv::Point2f> initial_inliers;
    double density_threshold = 0.3;  // 密度阈值
    double weight_sum = 0.0;

    for (size_t i = 0; i < points.size(); ++i) {
        if (densities[i] > density_threshold) {
            initial_inliers.push_back(points[i]);
            weight_sum += densities[i];
        }
    }

    // 如果高密度点太少，使用所有点
    if (initial_inliers.size() < 5) {
        initial_inliers = points;
        weight_sum = points.size();
    }

    // 3. 拟合初始椭圆
    cv::RotatedRect bestEllipse = cv::fitEllipseDirect(initial_inliers);
    auto bestCoverageInfo = calculateCoverage(points, bestEllipse);
    double bestScaleFactor = 1.0;

    // 4. 迭代优化
    int maxIterations = 50;
    int targetInliers = static_cast<int>(points.size() * target_coverage);

    for (int iter = 0; iter < maxIterations; ++iter) {
        // 计算所有点到当前椭圆的距离
        std::vector<std::pair<double, int>> distances;
        for (int i = 0; i < points.size(); ++i) {
            distances.push_back({ std::abs(pointToEllipseDistance(points[i], bestEllipse)), i });
        }

        // 排序，选择最近的targetInliers个点
        std::sort(distances.begin(), distances.end());

        std::vector<cv::Point2f> inliers;
        for (int i = 0; i < targetInliers; ++i) {
            inliers.push_back(points[distances[i].second]);
        }

        // 用内点重新拟合
        if (inliers.size() >= 5) {
            cv::RotatedRect newEllipse = cv::fitEllipseDirect(inliers);

            // 强制覆盖
            auto enforced = enforceCoverage(newEllipse, points, target_coverage);
            newEllipse = enforced.first;
            double scale_factor = enforced.second;

            // 计算覆盖率
            auto coverage_info = calculateCoverage(points, newEllipse);

            // 如果更好，更新
            if (coverage_info.second > bestCoverageInfo.second) {
                bestEllipse = newEllipse;
                bestCoverageInfo = coverage_info;
                bestScaleFactor = scale_factor;

                if (coverage_info.second >= target_coverage) {
                    break;
                }
            }
            else {
                // 没有改进，提前结束
                break;
            }
        }
        else {
            break;
        }
    }

    // 5. 提取椭圆参数
    float a = bestEllipse.size.width / 2.0f;
    float b = bestEllipse.size.height / 2.0f;
    float angle = bestEllipse.angle;

    if (a < b) {
        std::swap(a, b);
        angle += 90.0f;
        if (angle >= 180.0f) angle -= 180.0f;
    }

    double area = CV_PI * a * b;
    double eccentricity = 0;
    if (a > 0) {
        eccentricity = std::sqrt(std::max(0.0, 1.0 - (b * b) / (a * a)));
    }

    return Ellipse{ a, b, angle, area, eccentricity, bestEllipse.center, bestEllipse,
                   bestCoverageInfo.second, bestCoverageInfo.first, bestScaleFactor };
}

// 生成单边密集的点
std::vector<cv::Point2f> generateOneSidedDensePoints(int n) {
    std::vector<cv::Point2f> points;
    points.reserve(n);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dense_dist(0, 20);  // 密集区域
    std::uniform_real_distribution<> sparse_dist(0, 300);  // 稀疏区域

    // 在左侧生成密集点
    int dense_count = n * 0.7;  // 70%的点在左侧
    for (int i = 0; i < dense_count; ++i) {
        float x = 100 + dense_dist(gen);
        float y = 300 + dense_dist(gen);
        points.push_back(cv::Point2f(x, y));
    }

    // 在右侧生成稀疏点
    for (int i = dense_count; i < n; ++i) {
        float x = 300 + sparse_dist(gen);
        float y = 300 + sparse_dist(gen) - 150;  // 在y方向也有分布
        points.push_back(cv::Point2f(x, y));
    }

    return points;
}

// 可视化函数
void visualizeEllipseComparison(const std::vector<cv::Point2f>& points,
    const Ellipse& ellipse_original,
    const Ellipse& ellipse_improved,
    const std::string& windowName) {
    const int IMG_WIDTH = 1200;
    const int IMG_HEIGHT = 600;

    cv::Mat image(IMG_HEIGHT, IMG_WIDTH, CV_8UC3, cv::Scalar(240, 240, 240));

    // 计算边界
    float min_x = std::numeric_limits<float>::max();
    float max_x = std::numeric_limits<float>::lowest();
    float min_y = std::numeric_limits<float>::max();
    float max_y = std::numeric_limits<float>::lowest();

    for (const auto& p : points) {
        min_x = std::min(min_x, p.x);
        max_x = std::max(max_x, p.x);
        min_y = std::min(min_y, p.y);
        max_y = std::max(max_y, p.y);
    }

    // 添加边距
    float margin = 50.0f;
    min_x -= margin;
    max_x += margin;
    min_y -= margin;
    max_y += margin;

    // 计算缩放
    float width = max_x - min_x;
    float height = max_y - min_y;

    float scale_x = (IMG_WIDTH / 2.0f - 100) / width;
    float scale_y = (IMG_HEIGHT - 100) / height;
    float scale = std::min(scale_x, scale_y);

    // 转换函数
    auto transform = [&](float x, float y, int offset_x) -> cv::Point {
        int img_x = static_cast<int>((x - min_x) * scale) + offset_x;
        int img_y = static_cast<int>((y - min_y) * scale) + 50;
        return cv::Point(img_x, img_y);
    };

    // 绘制原始方法结果（左侧）
    cv::putText(image, "Original Method", cv::Point(50, 30),
        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);

    // 绘制点
    for (const auto& p : points) {
        cv::Point pt = transform(p.x, p.y, 50);
        double dist = pointToEllipseDistance(p, ellipse_original.rect);
        cv::Scalar color = (dist <= 0) ? cv::Scalar(0, 200, 0) : cv::Scalar(0, 0, 200);
        cv::circle(image, pt, 3, color, -1);
    }

    // 绘制椭圆
    cv::ellipse(image,
        cv::RotatedRect(
            transform(ellipse_original.center.x, ellipse_original.center.y, 50),
            cv::Size2f(ellipse_original.rect.size.width * scale,
                ellipse_original.rect.size.height * scale),
            ellipse_original.rect.angle),
        cv::Scalar(0, 0, 255), 2);

    // 绘制改进方法结果（右侧）
    cv::putText(image, "Improved Method", cv::Point(IMG_WIDTH / 2 + 50, 30),
        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);

    // 绘制点
    for (const auto& p : points) {
        cv::Point pt = transform(p.x, p.y, IMG_WIDTH / 2 + 50);
        double dist = pointToEllipseDistance(p, ellipse_improved.rect);
        cv::Scalar color = (dist <= 0) ? cv::Scalar(0, 200, 0) : cv::Scalar(0, 0, 200);
        cv::circle(image, pt, 3, color, -1);
    }

    // 绘制椭圆
    cv::ellipse(image,
        cv::RotatedRect(
            transform(ellipse_improved.center.x, ellipse_improved.center.y, IMG_WIDTH / 2 + 50),
            cv::Size2f(ellipse_improved.rect.size.width * scale,
                ellipse_improved.rect.size.height * scale),
            ellipse_improved.rect.angle),
        cv::Scalar(0, 0, 255), 2);

    // 添加信息
    std::string info_orig = cv::format("Coverage: %.1f%%, a=%.1f, b=%.1f",
        ellipse_original.coverage * 100,
        ellipse_original.a, ellipse_original.b);
    cv::putText(image, info_orig, cv::Point(50, IMG_HEIGHT - 20),
        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);

    std::string info_improved = cv::format("Coverage: %.1f%%, a=%.1f, b=%.1f",
        ellipse_improved.coverage * 100,
        ellipse_improved.a, ellipse_improved.b);
    cv::putText(image, info_improved, cv::Point(IMG_WIDTH / 2 + 50, IMG_HEIGHT - 20),
        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);

    cv::imshow(windowName, image);
    cv::waitKey(3000);
}

// 测试单边密集的点
void testOneSidedDense() {
    std::cout << "\n=== 测试单边密集的点 ===" << std::endl;

    // 生成点
    auto points = generateOneSidedDensePoints(200);

    std::cout << "生成 " << points.size() << " 个点" << std::endl;
    std::cout << "左侧密集区域: 140个点" << std::endl;
    std::cout << "右侧稀疏区域: 60个点" << std::endl;

    // 计算点分布
    float min_x = 1000, max_x = 0;
    float min_y = 1000, max_y = 0;

    for (const auto& p : points) {
        min_x = std::min(min_x, p.x);
        max_x = std::max(max_x, p.x);
        min_y = std::min(min_y, p.y);
        max_y = std::max(max_y, p.y);
    }

    std::cout << "点分布范围: x[" << min_x << ", " << max_x << "], y[" << min_y << ", " << max_y << "]" << std::endl;

    // 使用原始方法
    //std::cout << "\n--- 原始方法 ---" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    //Ellipse ellipse_orig = tuoyuan_improved(points, 0.6);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    //std::cout << "时间: " << duration.count() << "ms" << std::endl;
    //std::cout << "长轴: " << ellipse_orig.a << ", 短轴: " << ellipse_orig.b << std::endl;
    //std::cout << "角度: " << ellipse_orig.angle << "度" << std::endl;
    //std::cout << "中心: (" << ellipse_orig.center.x << ", " << ellipse_orig.center.y << ")" << std::endl;
    //std::cout << "覆盖率: " << ellipse_orig.coverage * 100 << "% ("
   //     << ellipse_orig.inlierCount << "/" << points.size() << ")" << std::endl;
    //std::cout << "离心率: " << ellipse_orig.eccentricity << std::endl;

    // 使用改进方法
    std::cout << "\n--- 改进方法 ---" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    Ellipse ellipse_improved = tuoyuan(points, 0.8);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "时间: " << duration.count() << "ms" << std::endl;
    std::cout << "长轴: " << ellipse_improved.a << ", 短轴: " << ellipse_improved.b << std::endl;
    std::cout << "角度: " << ellipse_improved.angle << "度" << std::endl;
    std::cout << "中心: (" << ellipse_improved.center.x << ", " << ellipse_improved.center.y << ")" << std::endl;
    std::cout << "覆盖率: " << ellipse_improved.coverage * 100 << "% ("
        << ellipse_improved.inlierCount << "/" << points.size() << ")" << std::endl;
    std::cout << "离心率: " << ellipse_improved.eccentricity << std::endl;

    // 可视化比较
    visualizeEllipse(points, ellipse_improved, "One-Sided Dense Points");
    //visualizeEllipseComparison(points, ellipse_orig, ellipse_improved, "One-Sided Dense Points");
}

// 测试各种点分布
void testVariousDistributions() {
    std::vector<std::pair<std::string, std::function<std::vector<cv::Point2f>(int)>>> test_cases = {
            {"单边密集", generateOneSidedDensePoints},
            {"聚类分布", [](int n) -> std::vector<cv::Point2f> {
                std::vector<cv::Point2f> points;
                std::random_device rd;
                std::mt19937 gen(rd());
                std::normal_distribution<> normal_dis(0, 1);

                // 两个聚类
                for (int i = 0; i < n; ++i) {
                    float x, y;
                    if (i % 3 == 0) {  // 第一个聚类
                        x = 100 + normal_dis(gen) * 30;
                        y = 100 + normal_dis(gen) * 30;
                    }
 else {  // 第二个聚类
  x = 400 + normal_dis(gen) * 30;
  y = 400 + normal_dis(gen) * 30;
}
points.push_back(cv::Point2f(x, y));
}
return points;
}},
{"线性分布", [](int n) -> std::vector<cv::Point2f> {
    std::vector<cv::Point2f> points;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> normal_dis(0, 1);

    for (int i = 0; i < n; ++i) {
        float x = 100 + i * 2.0f;
        float y = 300 + i + normal_dis(gen) * 20;
        points.push_back(cv::Point2f(x, y));
    }
    return points;
}},
{"弧形分布", [](int n) -> std::vector<cv::Point2f> {
    std::vector<cv::Point2f> points;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> normal_dis(0, 1);

    cv::Point2f center(300, 300);
    for (int i = 0; i < n; ++i) {
        float angle = i * 2.0f * CV_PI / n;
        float x = center.x + 100 * cos(angle) + normal_dis(gen) * 10;
        float y = center.y + 150 * sin(angle) + normal_dis(gen) * 10;
        points.push_back(cv::Point2f(x, y));
    }
    return points;
}}
    };

    for (const auto& [name, generator] : test_cases) {
        std::cout << "\n=== 测试: " << name << " ===" << std::endl;

        auto points = generator(200);

        // 原始方法
        auto start = std::chrono::high_resolution_clock::now();
        Ellipse ellipse_orig = tuoyuan_improved(points, 0.95);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration_orig = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // 改进方法
        start = std::chrono::high_resolution_clock::now();
        Ellipse ellipse_improved = tuoyuan(points, 0.95);
        end = std::chrono::high_resolution_clock::now();
        auto duration_improved = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "原始方法: 覆盖率=" << ellipse_orig.coverage * 100
            << "%, 时间=" << duration_orig.count() << "ms" << std::endl;
        std::cout << "改进方法: 覆盖率=" << ellipse_improved.coverage * 100
            << "%, 时间=" << duration_improved.count() << "ms" << std::endl;

        // 可视化比较
        visualizeEllipseComparison(points, ellipse_orig, ellipse_improved, name);
    }
}



// 自适应缩放可视化函数
void visualizeEllipse(const std::vector<cv::Point2f>& points,
    const Ellipse& ellipse,
    const std::string& patternName,
    const std::string& windowName) {

    // 1. 定义常量
    const int IMG_WIDTH = 800;   // 图像宽度
    const int IMG_HEIGHT = 600;  // 图像高度
    const int MARGIN = 40;       // 边距

    // 2. 计算数据范围
    float min_x = std::numeric_limits<float>::max();
    float max_x = std::numeric_limits<float>::lowest();
    float min_y = std::numeric_limits<float>::max();
    float max_y = std::numeric_limits<float>::lowest();

    for (const auto& p : points) {
        min_x = std::min(min_x, p.x);
        max_x = std::max(max_x, p.x);
        min_y = std::min(min_y, p.y);
        max_y = std::max(max_y, p.y);
    }

    // 3. 智能计算点密度
    int num_points = points.size();
    float data_width = max_x - min_x;
    float data_height = max_y - min_y;

    // 计算点密度
    float point_density = 0;
    if (data_width > 0 && data_height > 0) {
        point_density = num_points / (data_width * data_height);
    }

    // 4. 智能缩放策略
    float scale = 1.0f;
    float scale_factor = 1.0f;

    if (data_width > 0 && data_height > 0) {
        // 计算基本缩放比例
        float scale_x = (IMG_WIDTH - 2 * MARGIN) / data_width;
        float scale_y = (IMG_HEIGHT - 2 * MARGIN) / data_height;
        scale = std::min(scale_x, scale_y);

        // 根据点密度调整缩放
        if (point_density > 1.0f) {  // 高密度点
            // 增加缩放，让点之间有空隙
            scale_factor = std::max(1.0f, std::sqrt(std::sqrt(point_density)) * 0.8f);
        }
        else if (point_density < 0.01f) {  // 低密度点
            // 减小缩放，让点更紧凑
            scale_factor = std::max(0.3f, point_density * 100.0f);
        }
        else {
            scale_factor = 1.0f;
        }

        scale *= scale_factor;

        // 限制缩放范围
        scale = std::max(0.2f, std::min(scale, 150.0f));
    }

    // 5. 计算中心点
    float center_x = (min_x + max_x) / 2.0f;
    float center_y = (min_y + max_y) / 2.0f;

    // 6. 计算偏移，使图形居中
    float display_width = data_width * scale;
    float display_height = data_height * scale;
    float offset_x = (IMG_WIDTH - display_width) / 2.0f - min_x * scale;
    float offset_y = (IMG_HEIGHT - display_height) / 2.0f - min_y * scale;

    // 7. 坐标转换函数
    auto transformPoint = [&](const cv::Point2f& p) -> cv::Point2i {
        int x = static_cast<int>(p.x * scale + offset_x);
        int y = static_cast<int>(p.y * scale + offset_y);
        return cv::Point2i(x, y);
    };

    // 8. 创建图像
    cv::Mat image(IMG_HEIGHT, IMG_WIDTH, CV_8UC3, cv::Scalar(240, 240, 240));

    // 9. 绘制网格背景（只在缩放较小时显示网格）
    if (scale < 5.0f) {
        int grid_spacing = static_cast<int>(50.0f / scale);
        if (grid_spacing > 5) {
            // 计算网格起点
            int grid_start_x = static_cast<int>((-min_x) * scale + offset_x);
            int grid_start_y = static_cast<int>((-min_y) * scale + offset_y);

            // 绘制垂直线
            for (int x = grid_start_x; x < IMG_WIDTH; x += grid_spacing) {
                cv::line(image, cv::Point(x, 0), cv::Point(x, IMG_HEIGHT),
                    cv::Scalar(220, 220, 220), 1);
            }
            for (int x = grid_start_x; x >= 0; x -= grid_spacing) {
                cv::line(image, cv::Point(x, 0), cv::Point(x, IMG_HEIGHT),
                    cv::Scalar(220, 220, 220), 1);
            }

            // 绘制水平线
            for (int y = grid_start_y; y < IMG_HEIGHT; y += grid_spacing) {
                cv::line(image, cv::Point(0, y), cv::Point(IMG_WIDTH, y),
                    cv::Scalar(220, 220, 220), 1);
            }
            for (int y = grid_start_y; y >= 0; y -= grid_spacing) {
                cv::line(image, cv::Point(0, y), cv::Point(IMG_WIDTH, y),
                    cv::Scalar(220, 220, 220), 1);
            }
        }
    }

    // 10. 绘制坐标轴
    cv::Point2i origin = transformPoint(cv::Point2f(0, 0));
    if (origin.x >= 0 && origin.x < IMG_WIDTH) {
        cv::line(image, cv::Point(origin.x, 0), cv::Point(origin.x, IMG_HEIGHT),
            cv::Scalar(180, 180, 180), 2);
    }
    if (origin.y >= 0 && origin.y < IMG_HEIGHT) {
        cv::line(image, cv::Point(0, origin.y), cv::Point(IMG_WIDTH, origin.y),
            cv::Scalar(180, 180, 180), 2);
    }

    // 11. 智能绘制点
    for (const auto& p : points) {
        cv::Point2i img_point = transformPoint(p);

        // 计算点到椭圆的距离
        double dist = 0;
        if (ellipse.inlierCount > 0) {
            dist = pointToEllipseDistance(p, ellipse.rect);
        }

        // 智能确定点的大小
        int point_size = 2;  // 基础大小
        if (num_points < 50) {
            point_size = 4;  // 点少，显示大点
        }
        else if (num_points < 200) {
            point_size = 3;  // 中等数量
        }
        else if (num_points < 500) {
            point_size = 2;  // 点较多
        }
        else {
            point_size = 1;  // 点很多，显示小点
        }

        // 根据缩放调整点大小
        if (scale > 3.0f) {
            point_size = std::max(1, point_size - 1);
        }
        else if (scale < 0.5f) {
            point_size = point_size + 1;
        }

        // 内点用绿色，外点用红色
        cv::Scalar color = (dist <= 0) ? cv::Scalar(0, 200, 0) : cv::Scalar(0, 0, 200);
        cv::circle(image, img_point, point_size, color, -1);
    }

    // 12. 绘制拟合的椭圆
    if (ellipse.inlierCount > 0) {
        // 计算椭圆的顶点
        std::vector<cv::Point2f> ellipse_vertices(360);
        float angle_rad = ellipse.angle * CV_PI / 180.0f;
        float a = ellipse.a;
        float b = ellipse.b;
        cv::Point2f center = ellipse.center;

        for (int i = 0; i < 360; i++) {
            float t = i * CV_PI / 180.0f;
            float x = a * cos(t);
            float y = b * sin(t);

            // 旋转
            float x_rot = x * cos(angle_rad) - y * sin(angle_rad);
            float y_rot = x * sin(angle_rad) + y * cos(angle_rad);

            // 平移
            ellipse_vertices[i] = cv::Point2f(center.x + x_rot, center.y + y_rot);
        }

        // 绘制椭圆边界
        std::vector<cv::Point2i> transformed_vertices;
        for (const auto& v : ellipse_vertices) {
            transformed_vertices.push_back(transformPoint(v));
        }

        // 用多边形绘制椭圆
        cv::polylines(image, transformed_vertices, true, cv::Scalar(0, 0, 255), 2);

        // 绘制椭圆中心
        cv::Point2i center_pt = transformPoint(ellipse.center);
        cv::circle(image, center_pt, 3, cv::Scalar(0, 255, 0), -1);

        // 绘制长轴和短轴
        cv::Point2f major_end(
            ellipse.center.x + ellipse.a * cos(angle_rad),
            ellipse.center.y + ellipse.a * sin(angle_rad)
        );
        cv::Point2f minor_end(
            ellipse.center.x - ellipse.b * sin(angle_rad),
            ellipse.center.y + ellipse.b * cos(angle_rad)
        );

        cv::Point2i major_end_pt = transformPoint(major_end);
        cv::Point2i minor_end_pt = transformPoint(minor_end);

        cv::line(image, center_pt, major_end_pt, cv::Scalar(255, 0, 0), 1);
        cv::line(image, center_pt, minor_end_pt, cv::Scalar(0, 255, 0), 1);

        // 在轴端点添加标记
        cv::circle(image, major_end_pt, 2, cv::Scalar(255, 0, 0), -1);
        cv::circle(image, minor_end_pt, 2, cv::Scalar(0, 255, 0), -1);
    }

    // 13. 添加信息面板
    cv::Rect panel_rect(10, 10, 300, 140);
    cv::rectangle(image, panel_rect, cv::Scalar(255, 255, 255, 200), -1);
    cv::rectangle(image, panel_rect, cv::Scalar(100, 100, 100), 1);

    int y_offset = 35;
    cv::putText(image, "Pattern: " + patternName, cv::Point(20, y_offset),
        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    y_offset += 20;

    if (ellipse.inlierCount > 0) {
        std::string info1 = cv::format("Ellipse: a=%.1f, b=%.1f, angle=%.1f",
            ellipse.a, ellipse.b, ellipse.angle);
        cv::putText(image, info1, cv::Point(20, y_offset),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
        y_offset += 20;

        std::string info2 = cv::format("Center: (%.1f, %.1f)",
            ellipse.center.x, ellipse.center.y);
        cv::putText(image, info2, cv::Point(20, y_offset),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
        y_offset += 20;

        std::string info3 = cv::format("Points: %d, Inliers: %d",
            num_points, ellipse.inlierCount);
        cv::putText(image, info3, cv::Point(20, y_offset),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
        y_offset += 20;

        std::string info4 = cv::format("Coverage: %.1f%%", ellipse.coverage * 100);
        cv::Scalar coverage_color = (ellipse.coverage >= 0.95) ?
            cv::Scalar(0, 150, 0) : cv::Scalar(0, 0, 150);
        cv::putText(image, info4, cv::Point(20, y_offset),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, coverage_color, 1);
        y_offset += 20;

        std::string info5 = cv::format("Eccentricity: %.3f", ellipse.eccentricity);
        cv::putText(image, info5, cv::Point(20, y_offset),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);

        // 显示视图信息
        std::string view_info = cv::format("View: (%.1f,%.1f) to (%.1f,%.1f)",
            min_x, min_y, max_x, max_y);
        cv::putText(image, view_info, cv::Point(20, IMG_HEIGHT - 20),
            cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(100, 100, 100), 1);

        std::string scale_info = cv::format("Scale: %.2fx, Density: %.2f pts/unit",
            scale, point_density);
        cv::putText(image, scale_info, cv::Point(20, IMG_HEIGHT - 40),
            cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(100, 100, 100), 1);
    }
    else {
        cv::putText(image, "No ellipse fitted", cv::Point(20, y_offset),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
    }

    // 14. 添加图例
    int legend_x = IMG_WIDTH - 120;
    int legend_y = 10;

    cv::rectangle(image, cv::Rect(legend_x, legend_y, 110, 100),
        cv::Scalar(255, 255, 255, 200), -1);
    cv::rectangle(image, cv::Rect(legend_x, legend_y, 110, 100),
        cv::Scalar(100, 100, 100), 1);

    legend_y += 20;
    cv::circle(image, cv::Point(legend_x + 10, legend_y), 2, cv::Scalar(0, 200, 0), -1);
    cv::putText(image, "Inlier", cv::Point(legend_x + 20, legend_y + 3),
        cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
    legend_y += 20;

    cv::circle(image, cv::Point(legend_x + 10, legend_y), 2, cv::Scalar(0, 0, 200), -1);
    cv::putText(image, "Outlier", cv::Point(legend_x + 20, legend_y + 3),
        cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
    legend_y += 20;

    cv::line(image, cv::Point(legend_x + 5, legend_y), cv::Point(legend_x + 15, legend_y),
        cv::Scalar(0, 0, 255), 2);
    cv::putText(image, "Ellipse", cv::Point(legend_x + 20, legend_y + 3),
        cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
    legend_y += 20;

    cv::line(image, cv::Point(legend_x + 5, legend_y), cv::Point(legend_x + 15, legend_y),
        cv::Scalar(255, 0, 0), 1);
    cv::putText(image, "Major axis", cv::Point(legend_x + 20, legend_y + 3),
        cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 1);
    legend_y += 20;

    cv::line(image, cv::Point(legend_x + 5, legend_y), cv::Point(legend_x + 15, legend_y),
        cv::Scalar(0, 255, 0), 1);
    cv::putText(image, "Minor axis", cv::Point(legend_x + 20, legend_y + 3),
        cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 1);

    // 15. 显示图像
    cv::imshow(windowName, image);
    cv::waitKey(2000);  // 显示2秒
}


//int main() {
//    std::cout << "改进椭圆拟合算法测试" << std::endl;
//    std::cout << "处理单边密集点分布" << std::endl;
//    std::cout << "====================================" << std::endl;
//
//    testOneSidedDense();
//
//    char choice;
//    std::cout << "\n是否测试其他点分布? (y/n): ";
//    std::cin >> choice;
//
//    if (choice == 'y' || choice == 'Y') {
//        testVariousDistributions();
//    }
//
//    cv::destroyAllWindows();
//    return 0;
//}