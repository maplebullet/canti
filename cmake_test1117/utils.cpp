#include "utils.h"
#include "preprocess.h"
using namespace cv;
using namespace std;



void putText(cv::Mat& image, double value, int x, int y) {
    // 将小数转换为字符串
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << value;
    std::string myText = stream.str();

    // 设置文字参数
    cv::Point org(x, y); // 文字起始位置
    int fontFace = FONT_HERSHEY_SIMPLEX; // 字体类型
    double fontScale = 0.3; // 字体大小
    Scalar color(0, 0, 0); // 文字颜色，这里是绿色
    int thickness = 1; // 文字线条粗细

    // 在图像上打印文字
    putText(image, myText, org, fontFace, fontScale, color, thickness);
}


int mat_area(const cv::Mat& img) {
    return cv::countNonZero(img);
}

int mat_sum(const cv::Mat& img) {
    return cv::sum(img)[0];
}
bool cmp_Mat_by_area(cv::Mat& m1, cv::Mat& m2) {
    return  cv::countNonZero(m1) > cv::countNonZero(m2);
}
void dongtai_merge_masks(std::vector<cv::Mat>& masks,int start_row,int end_row){
    if(masks.size()<2) return;
    int mask0start=0,mask0end=0;
    for(int i=0;i<40;i++){
        if(mask0start==0 && (masks[0].at<uchar>(start_row,i)>0 || masks[0].at<uchar>(end_row,i)>0)){
            mask0start=i;
        }
        if(mask0end==0 && (masks[0].at<uchar>(start_row,39-i)>0 || masks[0].at<uchar>(end_row,39-i)>0)){
            mask0end=39-i;
        }
    }

    for(int i=masks.size()-1;i>0;i--){
        for(int j=mask0start;j<=mask0end;j++){
            if((masks[i].at<uchar>(start_row,j)>0 || masks[i].at<uchar>(end_row,j)>0)){
                cv::bitwise_or(masks[0], masks[i], masks[0]);
                masks.erase(masks.begin()+i);
                break;
            }

        }
    }
}
// 计算每个block的mask
std::vector<cv::Mat> detect_masks(const cv::Mat& bin_img, int threshold) {
    cv::Mat labels;
    int num_labels = cv::connectedComponents(bin_img, labels, 4, CV_32S);

    std::vector<cv::Mat> masks;
    std::vector<int> component_areas(num_labels, 0);

    for (int y = 0; y < labels.rows; ++y) {
        for (int x = 0; x < labels.cols; ++x) {
            int label = labels.at<int>(y, x);
            if (label > 0) {
                component_areas[label]++;
            }
        }
    }

    // Extract masks with area >= threshold
    for (int label = 1; label < num_labels; ++label) {
        if (component_areas[label] >= threshold) {
            masks.push_back((labels == label));
        }
    }
    sort(masks.begin(), masks.end(), cmp_Mat_by_area);

    if(bin_img.rows==120){
        dongtai_merge_masks(masks,40,42);
        dongtai_merge_masks(masks,80,82);
    }
    return masks;
}

// 高斯消去法解线性方程组 A * x = B
std::vector<double> gaussElimination(std::vector<std::vector<double>>& A, std::vector<double>& B) {
    int n = A.size();

    // 执行消去
    for (int i = 0; i < n; ++i) {
        // 寻找列中最大的主元
        double maxEl = abs(A[i][i]);
        int maxRow = i;
        for (int k = i + 1; k < n; ++k) {
            if (abs(A[k][i]) > maxEl) {
                maxEl = abs(A[k][i]);
                maxRow = k;
            }
        }

        // 交换最大行和当前行
        for (int k = i; k < n; ++k) {
            swap(A[maxRow][k], A[i][k]);
        }
        swap(B[maxRow], B[i]);

        // 消元过程
        for (int k = i + 1; k < n; ++k) {
            double c = -A[k][i] / A[i][i];
            for (int j = i; j < n; ++j) {
                if (i == j) {
                    A[k][j] = 0;
                }
                else {
                    A[k][j] += c * A[i][j];
                }
            }
            B[k] += c * B[i];
        }
    }

    // 反向替代求解
    std::vector<double> x(n);
    for (int i = n - 1; i >= 0; i--) {
        x[i] = B[i] / A[i][i];
        for (int k = i - 1; k >= 0; k--) {
            B[k] -= A[k][i] * x[i];
        }
    }

    return x;
}
// 定义一个拟合函数，n 是多项式的阶数
std::vector<double> polyfit(const std::vector<double>& y, int n) {
    std::vector<double>x;
    for (int i = 0; i < y.size(); i++) x.push_back(i);

    int N = x.size();
    // 创建 Vandermonde 矩阵
    std::vector<std::vector<double>> A(n + 1, std::vector<double>(n + 1, 0));
    std::vector<double> B(n + 1, 0);

    for (int i = 0; i <= n; ++i) {
        for (int j = 0; j <= n; ++j) {
            for (int k = 0; k < N; ++k) {
                A[i][j] += pow(x[k], i + j);
            }
        }
        for (int k = 0; k < N; ++k) {
            B[i] += pow(x[k], i) * y[k];
        }
    }

    // 用高斯消去法解线性方程组
    std::vector<double> coefficients = gaussElimination(A, B);

    return coefficients;
}


cv::Point2f compute_weighted_centroid(const cv::Mat& img) {
    if (mat_sum(img) < 2) return cv::Point2f(-1, -1);
    // 计算图像的几何矩
    cv::Moments m = cv::moments(img, false);  // 不二值化，直接使用像素值计算质量加权矩

    // 检查 m.m00 是否为 0（表示图像中全为0，没有有效质量）
    if (m.m00 == 0) {
        return cv::Point2f(-1, -1); // 返回无效的坐标
    }

    // 计算质量加权重心坐标 (x_c, y_c)
    float center_x = m.m10 / m.m00 + 0.5;
    float center_y = m.m01 / m.m00 + 0.5;

    return cv::Point2f(center_x, center_y);
}
//----------------------utils---------------------------------------


/*
limit:最高值，超过limit都会被替换为当前最大值
*/
cv::Mat convert_vector_to_mat(const std::vector<uint8_t> images, uchar high_limit) {
    uchar max = 0;
    cv::Mat image(40, 40, CV_8U);
    for (int y = 0; y < 40; y++) {
        for (int x = 0; x < 40; x++) {
            int value = images[y * 40 + x];
            if (value > high_limit) {
                value = max;
            }
            else {
                max = max > value ? max : value;
            }
            image.at<uchar>(y, x) = static_cast<uchar>(value);
        }
    }
    return image;
}
std::vector<uint8_t> convert_mat_to_vector(const cv::Mat& images) {
    std::vector<uint8_t> data;
    for (int y = 0; y < 40; y++) {
        for (int x = 0; x < 40; x++) {
            data.push_back(images.at<uchar>(y, x));
        }
    }
    return data;
}




//---------------------main--------------------------------------------

//zuGongImage recoglize(const cv::Mat& im, std::string model_path) {
//    cv::Mat image = im;
//    cv::Mat normalized; cv::normalize(image, normalized, 0, 1, cv::NORM_MINMAX, CV_32F);
//    cv::Mat label = pred1(normalized, model_path);
//    //show(label.mul(200), "hh");
//    cv::normalize(image, image, 0, 255, cv::NORM_MINMAX, CV_8U);
//    zuGongImage r(image, label);
//    return r;
//}
/**
* 图像的平移和旋转
*/
cv::Mat rotate(const cv::Mat& image, cv::Point dxdy, int ori = 0) {
    int cols = image.cols, rows = image.rows;
    int dx = cols / 2 - dxdy.x;
    int dy = rows / 2 - dxdy.y;

    // 平移和旋转的组合矩阵
    cv::Mat M = cv::getRotationMatrix2D(dxdy, ori, 1);
    M.at<double>(0, 2) += dx;  // 在旋转矩阵中加入平移
    M.at<double>(1, 2) += dy;

    // 进行一次仿射变换，减少插值次数
    cv::Mat rotated_image;
    cv::warpAffine(image, rotated_image, M, image.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT);
    //cv::warpAffine(image, rotated_image, M, image.size(), cv::INTER_CUBIC, cv::BORDER_CONSTANT, cv::Scalar(0));
    //show(heatmap(image), "test");
    //show(heatmap(rotated_image), "test");

    return rotated_image;
}
/**
* 根据脚的左上角，右上角，最下方三个点纠正脚的轻微偏移
*/
std::pair<cv::Mat, float> cal_pianyi(const cv::Mat& image, int threshold) {
    cv::Point lefttop, righttop, buttom;
    int cols = image.cols, rows = image.rows;
    cv::Mat colmax, rowmax;

    cv::reduce(image, rowmax, 1, REDUCE_MAX);
    cv::reduce(image(Rect(0, 0, cols, rows / 2)), colmax, 0, REDUCE_MAX);



    for (int left = 0; left < cols / 2; left++)
    {
        if (colmax.at<uchar>(left) > threshold) {
            int top = 0, buttom = rows / 2;
            for (; top < rows / 2; top++) {
                if (image.at <uchar>(cv::Point(left, top)) > threshold) {
                    break;
                }
            }
            for (; buttom >= 0; buttom--) {
                if (image.at <uchar>(cv::Point(left, buttom)) > threshold) {
                    break;
                }
            }
            lefttop = cv::Point(left, (top + buttom) / 2);
            break;
        }
    }
    for (int right = cols - 1; right >= cols / 2; right--) {
        if (colmax.at<uchar>(right) > threshold) {
            int top = 0, buttom = rows / 2;
            for (; top < rows / 2; top++) {
                if (image.at <uchar>(cv::Point(right, top)) > threshold) {
                    break;
                }
            }
            for (; buttom >= 0; buttom--) {
                if (image.at <uchar>(cv::Point(right, buttom)) > threshold) {
                    break;
                }
            }
            righttop = cv::Point(right, (top + buttom) / 2);
            break;
        }
    }
    for (int up = rows - 1; up > rows / 2; up--) {
        if (rowmax.at<uchar>(up) > threshold) {
            int left = 0, right = cols - 1;
            for (; left < cols; left++) {
                if (image.at <uchar>(cv::Point(left, up)) > threshold) {
                    break;
                }
            }
            for (; right >= 0; right--) {
                if (image.at <uchar>(cv::Point(right, up)) > threshold) {
                    break;
                }
            }
            buttom = cv::Point((left + right) / 2, up);
            break;
        }
    }
    //line(image, lefttop, buttom, 200, 1, 4);
    //line(image, righttop, buttom, 200, 1, 4);

    int delta_x = buttom.x - lefttop.x;
    int delta_y = buttom.y - lefttop.y;
    double angle_radians = std::atan2(delta_y, delta_x);
    double angle_1 = angle_radians * 180 / CV_PI;

    delta_x = buttom.x - righttop.x;
    delta_y = buttom.y - righttop.y;
    angle_radians = std::atan2(delta_y, delta_x);
    double angle_2 = angle_radians * 180 / CV_PI;

    double ori = (angle_2 + angle_1) / 2 - 90;
    cv::Mat r_image = rotate(image, cv::Point(image.cols / 2, image.rows / 2), ori);
    return std::pair<cv::Mat, float>(r_image, ori);
}
/**
* 旋转平移脚型至图像中部
*/
cv::RotatedRect calcu_rect(const cv::Mat& ima) {
    cv::Mat image = ima > 0;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (contours.size() == 0) {
        return cv::RotatedRect(cv::Point2f(0, 0), cv::Size2f(0, 0), 0.0);
    }

    std::vector<cv::Point> contour;
    for (std::vector<cv::Point>& c : contours)
        for (cv::Point& p : c)
            contour.push_back(p);

    cv::RotatedRect rect = cv::minAreaRect(contour);
    return rect;
}
std::pair<cv::Mat, float> jiuzheng(const cv::Mat& ima, cv::RotatedRect rect, float ori) {
    // 调整角度
    if (ori != 0) {
        cv::Mat image = rotate(ima.clone(), rect.center, ori);
        return std::pair<cv::Mat, float>(image, ori);
    }
    if (rect.size.width < rect.size.height) {
        ori = rect.angle;
    }
    else {
        ori = rect.angle - 90;
    }
    cv::Mat image = rotate(ima.clone(), rect.center, ori);
    return std::pair<cv::Mat, float>(image, ori);
}
cv::Mat jingtai_jiuzheng(const cv::Mat& ima) {
    cv::Mat image = ima.clone();

    // 寻找最小外接矩形
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(image>1, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (contours.size() == 0) {
        return image;
    }

    std::vector<cv::Point> contour;
    for (std::vector<cv::Point>& c : contours)
        for (cv::Point& p : c)
            contour.push_back(p);
    cv::RotatedRect rect = cv::minAreaRect(contour);



    // 调整角度
    float ori;
    if (rect.size.width < rect.size.height) {
        ori = rect.angle;
    }
    else {
        ori = rect.angle - 90;
    }
    image = rotate(image, rect.center, ori);

    return image;
}






Lean calculate_lean(const cv::Mat& im, int threshold) {
    double left_right = 0;//左右倾斜状况
    double left_forward = 0;//左脚前后侧重
    double right_forward = 0;//右脚前后侧重

    double left_up = 0, left_back = 0, right_up = 0, right_back = 0;
    cv::Mat image;
    cv::normalize(im, image, 0, 255, cv::NORM_MINMAX, CV_8U);


    Rect leftrect(0, 0, 80, 160), rightrect(80, 0, 80, 160);
    cv::Mat left_image = image(leftrect); cv::Mat right_image = image(rightrect);
    cv::Mat left_mask = (left_image > 0) / 255, right_mask = (right_image > 0) / 255;
    int left_area = cv::sum(left_mask)[0], right_area = cv::sum(right_mask)[0];
    int all_area = left_area + right_area;
    left_right = (left_area * 1.0 / all_area - 0.5) * 2;


    std::vector<std::vector<cv::Point>> lcontours;
    cv::findContours(left_image, lcontours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (lcontours.size() > 0) {
        std::vector<cv::Point> contour;
        for (std::vector<cv::Point>& c : lcontours)
            for (cv::Point& p : c)
                contour.push_back(p);
        cv::RotatedRect rect = cv::minAreaRect(contour);
        cv::Point left_centor = rect.center;
        left_up = cv::sum(left_mask(Rect(0, 0, left_mask.cols, left_centor.y)))[0] * 1.0 / all_area;
        left_back = cv::sum(left_mask(Rect(0, left_centor.y, left_mask.cols, left_mask.rows - left_centor.y)))[0] * 1.0 / all_area;
    }

    std::vector<std::vector<cv::Point>> rcontours;
    cv::findContours(right_image, rcontours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (rcontours.size() > 0) {
        std::vector<cv::Point> contour;
        for (std::vector<cv::Point>& c : rcontours)
            for (cv::Point& p : c)
                contour.push_back(p);
        cv::RotatedRect rect = cv::minAreaRect(contour);
        cv::Point right_centor = rect.center;
        right_up = cv::sum(right_mask(Rect(0, 0, right_mask.cols, right_centor.y)))[0] * 1.0 / all_area;
        right_back = cv::sum(right_mask(Rect(0, right_centor.y, right_mask.cols, right_mask.rows - right_centor.y)))[0] * 1.0 / all_area;
    }

    left_forward = left_up - left_back;
    right_forward = right_up - right_back;
    double d = max(max(abs(left_right), abs(left_forward)), abs(right_forward));
    int degree;
    if (d > 0.1) degree = 0;
    else if (d > 0.08) degree = 1;
    else degree = 2;
    Lean r(left_up, left_back, right_up, right_back, left_right, left_forward, right_forward, degree);
    return r;
}


double mean_sum_image(std::vector<Frame> imgs, int start, int end) {
    int result = 0.0;
    for (int i = start; i <= end; i++) {
        result += imgs[i].sum_image;
    }
    return result * 1.0 / (end - start + 1);
}
double mean_area_image(std::vector<Frame> imgs, int start, int end) {
    int result = 0.0;
    for (int i = start; i <= end; i++) {
        result += imgs[i].area;
    }
    return result * 1.0 / (end - start + 1);
}
double mean_area_image_with_resize(std::vector<Frame> imgs, int start, int end, int chang, int kuan) {
    int result = 0.0;
    for (int i = start; i <= end; i++) {
        cv::Mat output;
        cv::resize(imgs[i].image, output, Size(kuan, chang), 0, 0, INTER_NEAREST);
        result += mat_area(output);
    }
    return result * 1.0 / (end - start + 1);
}

cv::Mat mean_image_frame(std::vector<Frame> imgs, int start, int end) {
    cv::Mat result = cv::Mat::zeros(cv::Size(imgs.at(0).image.cols, imgs.at(0).image.rows), CV_32FC1);
    for (int i = start; i <= end; i++) {
        cv::add(result, imgs[i].image, result, noArray(), CV_32FC1);
    }
    return result / (end - start + 1);
}
cv::Mat max_image_frame(std::vector<Frame> imgs, int start, int end) {
    cv::Mat result = cv::Mat::zeros(cv::Size(imgs.at(0).image.cols, imgs.at(0).image.rows), CV_32FC1);
    for (int i = start; i <= end; i++) {
        cv::max(result, imgs[i].image, result);
    }
    return result;
}
cv::Mat mean_image(std::vector<cv::Mat> imgs, int start, int end) {
    cv::Mat result = cv::Mat::zeros(cv::Size(imgs.at(0).cols, imgs.at(0).rows), CV_32FC1);
    for (int i = start; i <= end; i++) {
        cv::add(result, imgs[i], result, noArray(), CV_32FC1);
    }
    return result / (end - start + 1);
}
cv::Mat max_image(std::vector<cv::Mat> imgs, int start, int end) {
    cv::Mat result = cv::Mat::zeros(cv::Size(imgs.at(0).cols, imgs.at(0).rows), CV_32FC1);
    for (int i = start; i <= end; i++) {
        cv::max(result, imgs[i], result);
    }
    return result;
}

std::vector<cv::Vec2f> fft(std::vector<cv::Point2f>& coordinates) {
    // 将坐标转换为 Mat 类型，用于傅里叶变换
    cv::Mat inputSignal(coordinates.size(), 1, CV_32FC2);
    for (size_t i = 0; i < coordinates.size(); ++i) {
        inputSignal.at<cv::Vec2f>(i)[0] = coordinates[i].x; // 实部为 x 坐标
        inputSignal.at<cv::Vec2f>(i)[1] = coordinates[i].y; // 虚部为 y 坐标
    }

    // 傅里叶变换结果
    cv::Mat dftResult;
    dft(inputSignal, dftResult, DFT_COMPLEX_OUTPUT);

    std::vector<cv::Vec2f> vecs;
    for (int i = 0; i < dftResult.rows; ++i) {
        vecs.push_back(dftResult.at<cv::Vec2f>(i));
    }
    return vecs;
}
double computeMinDistanceBetweenMasks(const cv::Mat& mask1, const cv::Mat& mask2) {
    // 确保两个mask尺寸相同
    CV_Assert(mask1.size() == mask2.size());

    // 获取mask1的前景点坐标
    std::vector<cv::Point> points1;
    cv::findNonZero(mask1, points1);

    // 获取mask2的前景点坐标
    std::vector<cv::Point> points2;
    cv::findNonZero(mask2, points2);

    // 如果任一mask没有前景点，返回无效值
    if (points1.empty() || points2.empty()) {
        return 0;
    }

    // 暴力搜索最小距离
    double minDist_x = std::numeric_limits<double>::max();
    for (const auto& pt1 : points1) {
        for (const auto& pt2 : points2) {
            double dx = abs(pt1.x - pt2.x);
            minDist_x = min(dx, minDist_x);
        }
    }

    return minDist_x;
}
std::pair<double, double> calculate_distance(cv::Point2f& center1, cv::Point2f& center2, float k, double kuan, double chang) {
    double dx = (center2.x - center1.x) * kuan;
    double dy = (center2.y - center1.y) * chang;
    double norm = std::sqrt(1 + k * k);

    double d_along = (dx * 1 + dy * k) / norm;  // 沿直线方向
    double d_perp = (dx * (-k) + dy * 1) / norm; // 垂直直线方向

    return { d_along, d_perp };
}

double distance_points(cv::Point point1, cv::Point point2, double kuan, double chang) {
    return std::sqrt(
        kuan * (point1.x - point2.x) * kuan * (point1.x - point2.x)
        + chang * (point1.y - point2.y) * chang * (point1.y - point2.y)
    );
}

double minBoundingPolygonAreaCV(const std::vector<cv::Point2f>& points) {
    if (points.size() < 3) return 0.0;

    // 计算凸包
    std::vector<cv::Point2f> hull;
    cv::convexHull(points, hull);

    // 计算凸包面积
    return cv::contourArea(hull);
}
double cal_bianyi(const std::vector<double>& vec) {
    double mean = std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
    double sq_sum = std::inner_product(vec.begin(), vec.end(), vec.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / vec.size() - mean * mean);
    return stdev / mean;
}

// 基础截断均值计算
double trimmedMean_float(const vector<double>& data, float trim_ratio) {
    if (data.empty()) {
        return 0.0f;
    }

    // 复制数据并排序
    vector<double> sorted_data = data;
    sort(sorted_data.begin(), sorted_data.end());

    int n = sorted_data.size();

    // 计算需要截断的数据点数量
    int k = static_cast<int>(n * trim_ratio);

    // 调整k值，确保至少保留2个数据点
    if (k * 2 >= n) {
        k = max(1, n / 4);  // 至少保留一半数据
    }

    // 计算截断后的和
    float sum = 0.0f;
    int count = 0;
    for (int i = k; i < n - k; i++) {
        sum += sorted_data[i];
        count++;
    }

    if (count == 0) {
        // 如果全部被截断，返回中位数
        return sorted_data[n / 2];
    }

    return sum / count;
}

// 基于距离的截断均值（更适用于点云数据）
Point2f distanceBasedTrimmedMean(vector<Point2f>& points, float trim_ratio) {
    if (points.empty()) {
        return Point2f(0, 0);
    }

    // 1. 计算初始中心（使用中位数）
    vector<float> x_vals, y_vals;
    for (const auto& p : points) {
        x_vals.push_back(p.x);
        y_vals.push_back(p.y);
    }

    sort(x_vals.begin(), x_vals.end());
    sort(y_vals.begin(), y_vals.end());

    Point2f initial_center(x_vals[x_vals.size() / 2], y_vals[y_vals.size() / 2]);

    // 2. 计算每个点到中心的距离
    vector<pair<float, Point2f>> distances;
    for (const auto& p : points) {
        float dx = p.x - initial_center.x;
        float dy = p.y - initial_center.y;
        float dist = sqrt(dx * dx + dy * dy);
        distances.emplace_back(dist, p);
    }

    // 3. 按距离排序
    sort(distances.begin(), distances.end(),
         [](const pair<float, Point2f>& a, const pair<float, Point2f>& b) {
             return a.first < b.first;
         });

    // 4. 截断
    int n = distances.size();
    int k = static_cast<int>(n * trim_ratio);

    if (k * 2 >= n) {
        k = max(1, n / 4);
    }

    // 5. 计算截断后的均值
    float sum_x = 0.0f, sum_y = 0.0f;
    int count = 0;

    points.clear();
    for (int i = k; i < n - k; i++) {
        sum_x += distances[i].second.x;
        sum_y += distances[i].second.y;
        count++;
        points.push_back(distances[i].second);
    }

    if (count == 0) {
        return initial_center;
    }



    return Point2f(sum_x / count, sum_y / count);
}

