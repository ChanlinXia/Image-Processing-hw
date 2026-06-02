#include "imageprocesser.h"
#include <unordered_set>

ImageProcesser::ImageProcesser() {}


/*************************************************
*   展示图片，调试用
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void ImageProcesser::show(const grayEigen& image,const std::string& title)const{
    cv::Mat cv_image;
    cv::eigen2cv(image,cv_image);

    cv::imshow(title, cv_image);
    cv::waitKey(0);
}


/*************************************************
*   卷积，基于cv
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::convolution(cv::Mat& image,cv::Mat& kernel,cv::Mat& rlt)const {
    if(image.empty()) {
        return "[ImgaeProcesser][convolution-cv] original image is empty!";
    }
    if(kernel.empty()) {
        return "[ImgaeProcesser][convolution-cv] kernel is empty!";
    }
    cv::Mat gray_origin;
    getGray(image,gray_origin);

    cv::cvtColor(image,gray_origin,cv::COLOR_RGB2GRAY);
    cv::Mat result_float;
    cv::filter2D(gray_origin, result_float, CV_32F, kernel);
    // cv::normalize(result_float, result_cv_, 0, 255, cv::NORM_MINMAX);
    // result_cv_.convertTo(result_cv_, CV_8U);
    cv::convertScaleAbs(result_float, rlt);

    return "ok";
}

/*************************************************
*   卷积，基于cv
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::convolution(cv::Mat& image,cv::Mat& kernel)const {
        if(image.empty()) {
            return "[ImgaeProcesser][convolution-cv] original image is empty!";
        }
        if(kernel.empty()) {
            return "[ImgaeProcesser][convolution-cv] kernel is empty!";
        }
        cv::Mat gray_origin;
        getGray(image,gray_origin);

        cv::cvtColor(image,gray_origin,cv::COLOR_RGB2GRAY);
        // std::cout << "Image size: " << gray_origin.size() << std::endl;
        cv::Mat result_float;
        cv::filter2D(gray_origin, result_float, CV_32F, kernel);
        // cv::normalize(result_float, result_cv_, 0, 255, cv::NORM_MINMAX);
        // result_cv_.convertTo(result_cv_, CV_8U);
        cv::convertScaleAbs(result_float, image);

    return "ok";
}

/*************************************************
*   卷积，基于Eigen
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::convolution(intensityEigen& image,intensityEigen& kernel,grayEigen& rlt) const{
    int rows = image.rows();
    int cols = image.cols();
    int kRows = kernel.rows();
    int kCols = kernel.cols();

    if(rows == 0|| cols ==0) return "[ImgaeProcesser][convolution-Eigen] original image is empty!";
    if(kRows == 0 || kCols == 0) return "[ImgaeProcesser][convolution-cv] kernel is empty!";

    int padRows = kRows/2;
    int padCols = kCols/2;

    intensityEigen padded(rows + 2 * padRows, cols + 2 * padCols);
    padded.setZero();
    padded.block(padRows,padCols,rows,cols) = image;

    // cv::Mat intensity_origin;
    // cv::eigen2cv(image_container_.intensity_matrix,intensity_origin);

    for(int i =0;i<rows;++i){
        for(int j = 0;j<cols;++j){
            intensityEigen window = padded.block(i, j, kRows, kCols);
            double sum = (window.array() * kernel.array()).sum();

            rlt(i, j) = static_cast<uchar>(std::max(0.0, std::min(255.0, abs(sum))));
        }
    }

    return "ok";
}

/*************************************************
*   中值滤波，基于Eigen
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::medianFilter(intensityEigen& image,int kernel_size,grayEigen& rlt) const{
    int rows = image.rows();
    int cols = image.cols();
    if(rows == 0 || cols == 0) return "[ImgaeProcesser][medianFilter-Eigen] original or the rlt image is empty!";

    if(kernel_size % 2 == 0) { // must be odd
        kernel_size++;
        qDebug() << "Kernel size adjusted to odd:" << kernel_size;
    }

    int pad = kernel_size / 2;

    // 创建 padded 矩阵并填充边界(镜像)
    intensityEigen padded(rows + 2 * pad, cols + 2 * pad);
    padded.block(pad, pad, rows, cols) = image;


    for(int i = 0; i < pad; ++i) {
        for(int j = 0; j < pad; ++j) {
            // 左上角
            padded(i, j) = image(pad - i - 1, pad - j - 1);
            // 右上角
            padded(i, cols + pad + j) = image(pad - i - 1, cols - j - 1);
            // 左下角
            padded(rows + pad + i, j) = image(rows - i - 1, pad - j - 1);
            // 右下角
            padded(rows + pad + i, cols + pad + j) = image(rows - i - 1, cols - j - 1);
        }
    }
    // 填充上下边缘
    for(int i = 0; i < pad; ++i) {
        for(int j = 0; j < cols; ++j) {
            padded(i, pad + j) = image(pad - i - 1, j);  // 上边缘
            padded(rows + pad + i, pad + j) = image(rows - i - 1, j);  // 下边缘
        }
    }
    // 填充左右边缘
    for(int i = 0; i < rows; ++i) {
        for(int j = 0; j < pad; ++j) {
            padded(pad + i, j) = image(i, pad - j - 1);  // 左边缘
            padded(pad + i, cols + pad + j) = image(i, cols - j - 1);  // 右边缘
        }
    }

    rlt.resize(rows, cols);

    int windowSize = kernel_size * kernel_size;
    std::vector<double> window(windowSize);

    for(int i = 0; i < rows; ++i) {
        for(int j = 0; j < cols; ++j) {
            // 提取窗口内的所有像素
            int idx = 0;
            for(int ki = 0; ki < kernel_size; ++ki) {
                for(int kj = 0; kj < kernel_size; ++kj) {
                    window[idx++] = padded(i + ki, j + kj);
                }
            }

            // 排序并取中值
            std::nth_element(window.begin(),
                             window.begin() + windowSize / 2,
                             window.end());

            rlt(i, j) = static_cast<uchar>(window[windowSize / 2]);
        }
    }

    return "ok";
}

/*************************************************
*   中值滤波，基于Eigen
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::medianFilter(cv::Mat& image,int kernel_size,cv::Mat& rlt) const{
    QString debug_info = "ok";
    if(image.empty() || rlt.empty()) return "[ImgaeProcesser][medianFilter-cv] original or the rlt image is empty!";
    if(kernel_size % 2 != 1) {
        kernel_size++;
        debug_info = "Kernel size adjusted to odd";
    }
    cv::Mat result;
    cv::medianBlur(image, rlt, kernel_size);
    return debug_info;
}

/*************************************************
*   获得灰度图
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void ImageProcesser::getGray(cv::Mat& image,cv::Mat& gray) const{
    if(image.channels() == 1) gray = image;
    else cv::cvtColor(image,gray,cv::COLOR_BGR2GRAY);
}


/*************************************************
*   通过threshold返回mask，借用opencv方法
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::threshold(cv::Mat& image,int thershold,cv::Mat& mask,bool is_upper) const{
    int rows = image.rows;
    int cols = image.cols;
    if( rows == 0 || cols==0) return "[ImgaeProcesser][threshold-cv] the image is empty";
    if(mask.empty())    mask= cv::Mat::zeros(rows,cols,CV_8UC1);

    cv::Mat gray_image;
    getGray(image,gray_image);

    // qDebug() << "start to hist" << Qt::endl;
    // int histSize = 256;
    // float range[] = {0, 256};
    // const float* histRange = {range};
    // cv::Mat hist;
    // cv::calcHist(&gray_image, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

    // qDebug() << "start to mask" << Qt::endl;
    if( is_upper) cv::threshold(gray_image, mask, thershold, 255, cv::THRESH_BINARY_INV);
    else cv::threshold(gray_image, mask, thershold, 255, cv::THRESH_BINARY);

    return "ok";
}

/*************************************************
*   通过threshold返回mask，基于Eigen库
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::threshold(grayEigen& image,int threshold,grayEigen& mask,bool is_upper) const{
    int rows = image.rows();
    int cols = image.cols();
    if( rows == 0) return "[ImgaeProcesser][threshold-Eigen] the image is empty";

    if(mask.rows() != rows || mask.cols() != cols)     mask = grayEigen::Zero(rows,cols);

    for(int i=0;i<rows;++i){
        for(int j=0;j<cols;++j){
            auto brightness = image(i,j);
            if(is_upper){
                if(brightness <= threshold)mask(i,j) = BINARY_ONE;
                else mask(i,j) = BINARY_ZERO;
            }
            else{
                if(brightness <= threshold)mask(i,j) = BINARY_ZERO;
                else mask(i,j) = BINARY_ONE;
            }
        }
    }

    return "ok";
}

/*************************************************
*   获取SE cv
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::genSEKernel(cv::Mat& kernel,SE_TYPE se_type,int kernel_size)const{
    int center = kernel_size / 2;
    cv::Point centerPoint(center, center);
    switch(se_type){
    case ImageProcesser::SE_TYPE::Rect:
        kernel = cv::Mat::ones(kernel_size, kernel_size, CV_8U) ;
        break;
    case ImageProcesser::SE_TYPE::Cross:
        kernel = cv::Mat::zeros(kernel_size, kernel_size, CV_8U);

        // 中心行和中心列设为1
        kernel.row(center).setTo(1);  // 或者用 1，根据你的数据类型
        kernel.col(center).setTo(1);
        break;
    case ImageProcesser::SE_TYPE::Ellipse:
    // case ImageProcesser::SE_TYPE::Ellipse:
        // 椭圆形/圆形核
        kernel = cv::Mat::zeros(kernel_size, kernel_size, CV_8U);
        cv::circle(kernel, centerPoint, center, cv::Scalar(255), -1);  // -1 表示填充
        break;
    case ImageProcesser::SE_TYPE::Specific:
        kernel = cv::Mat::zeros(kernel_size, kernel_size, CV_8U);
        // int center = kernel_size / 2;
        for (int i = 0; i < kernel_size; i++) {
            for (int j = 0; j < kernel_size; j++) {
                if (abs(i - center) + abs(j - center) <= center) {
                    kernel.at<uchar>(i, j) = 255;
                }
            }
        }

        break;
    default: qDebug() << "please input a correct kernel type" <<Qt::endl;
        break;
    }
    return "ok";
}

/*************************************************
*   获取se Eigen
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::genSEKernel(grayEigen& kernel,SE_TYPE se_type,int kernel_size)const{
    kernel = grayEigen::Zero(kernel_size, kernel_size);
    switch(se_type) {
    case ImageProcesser::SE_TYPE::Rect: {
        kernel.setConstant(1);  // 全设为1
        break;
    }
    case ImageProcesser::SE_TYPE::Cross: {
        kernel.setZero();
        int center = kernel_size / 2;
        kernel.row(center).setConstant(1);
        kernel.col(center).setConstant(1);
        break;
    }
    case ImageProcesser::SE_TYPE::Ellipse: {
        kernel.setZero();
        int center = kernel_size / 2;
        int radius = kernel_size / 2;
        int radius_sq = radius * radius;

        for (int i = 0; i < kernel_size; i++) {
            for (int j = 0; j < kernel_size; j++) {
                int dx = i - center;
                int dy = j - center;
                if (dx * dx + dy * dy <= radius_sq) {
                    kernel(i, j) = 1;
                }
            }
        }
        break;
    }
    case ImageProcesser::SE_TYPE::Specific: {
        kernel.setZero();
        int center = kernel_size / 2;
        for (int i = 0; i < kernel_size; i++) {
            for (int j = 0; j < kernel_size; j++) {
                if (abs(i - center) + abs(j - center) <= center) {
                    kernel(i, j) = 1;
                }
            }
        }
        break;
    }
    default:
        qDebug() << "please input a correct kernel type" << Qt::endl;
        kernel.setConstant(1);
        break;
    }
    return "ok";
}

/*************************************************
*   计算膨胀 cv
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::morphologicalDilation(cv::Mat& image,const cv::Mat& se,cv::Mat& rlt)const{
    if (image.empty()) {
        qDebug() << "Input image is empty" << Qt::endl;
        return "error: empty image";
    }

    if (se.empty()) {
        qDebug() << "Structuring element is empty" << Qt::endl;
        return "error: empty SE";
    }
    cv::dilate(image, rlt, se);
    return "ok";
}

/*************************************************
*   计算膨胀 Eigen
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::morphologicalDilation(grayEigen& image,const grayEigen& se,grayEigen& rlt)const{

    if (image.size() == 0) {
        qDebug() << "Input image is empty" << Qt::endl;
        return "error: empty image";
    }

    if (se.size() == 0) {
        qDebug() << "Structuring element is empty" << Qt::endl;
        return "error: empty SE";
    }

    // 获取图像尺寸
    int rows = image.rows();
    int cols = image.cols();

    // 获取结构元素尺寸
    int se_rows = se.rows();
    int se_cols = se.cols();

    // 计算锚点（中心点）
    int anchor_y = se_rows / 2;
    int anchor_x = se_cols / 2;

    // 初始化结果矩阵（全0，与输入同尺寸）
    rlt.resize(rows, cols);
    rlt.setZero();

    // 遍历图像每个像素
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // 当前像素邻域的最大值
            uchar max_val = 0;

            // 遍历结构元素
            for (int m = 0; m < se_rows; m++) {
                for (int n = 0; n < se_cols; n++) {
                    // 如果结构元素在该位置有效（非0）
                    if (se(m, n) != 0) {
                        // 计算对应图像上的坐标
                        int img_i = i + (m - anchor_y);
                        int img_j = j + (n - anchor_x);

                        // 边界检查（超出边界的像素视为0）
                        if (img_i >= 0 && img_i < rows && img_j >= 0 && img_j < cols) {
                            uchar val = image(img_i, img_j);
                            if (val > max_val) {
                                max_val = val;
                            }
                        }
                    }
                }
            }

            // 将最大值赋给结果
            rlt(i, j) = max_val;
        }
    }

    return "ok";
}

/*************************************************
*   计算腐蚀 cv
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::morphologicalErosion(cv::Mat& image, const cv::Mat& se, cv::Mat& rlt) const {
    if (image.empty()) {
        qDebug() << "Input image is empty" << Qt::endl;
        return "error: empty image";
    }

    if (se.empty()) {
        qDebug() << "Structuring element is empty" << Qt::endl;
        return "error: empty SE";
    }

    // 使用 OpenCV 内置函数
    cv::erode(image, rlt, se);

    return "ok";
}

/*************************************************
*   计算腐蚀 Eigen
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::morphologicalErosion(grayEigen& image, const grayEigen& se, grayEigen& rlt) const {
    if (image.size() == 0) {
        qDebug() << "Input image is empty" << Qt::endl;
        return "error: empty image";
    }

    if (se.size() == 0) {
        qDebug() << "Structuring element is empty" << Qt::endl;
        return "error: empty SE";
    }

    int rows = image.rows();
    int cols = image.cols();
    int se_rows = se.rows();
    int se_cols = se.cols();
    int anchor_y = se_rows / 2;
    int anchor_x = se_cols / 2;

    // 收集结构元素的有效偏移
    std::vector<std::pair<int, int>> offsets;
    for (int m = 0; m < se_rows; m++) {
        for (int n = 0; n < se_cols; n++) {
            if (se(m, n) != 0) {
                offsets.push_back({m - anchor_y, n - anchor_x});
            }
        }
    }

    // 初始化结果矩阵
    rlt.resize(rows, cols);
    rlt.setZero();

    // 遍历每个像素
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            bool all_foreground = true;

            // 检查结构元素覆盖的所有位置是否都是前景
            for (const auto& offset : offsets) {
                int img_i = i  + offset.first;
                int img_j = j  + offset.second;


                // // // 边界检查（超出边界的视为背景）
                if (img_i < 0 || img_i >= rows || img_j < 0 || img_j >= cols) {
                    // all_foreground = false;
                    continue; // 对于边界外选择跳过，这是和opencv结果一致的关键
                }

                if (image(img_i, img_j) == 0) {  // 遇到背景
                    all_foreground = false;
                    break;
                }
            }

            // 只有完全被结构元素覆盖且所有对应位置都是前景，结果才为前景
            rlt(i, j) = all_foreground ? 255 : 0;
        }
    }

    return "ok";
}

/*************************************************
*   开运算 cv
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::morphologicalOepning(cv::Mat& image, const cv::Mat& se, cv::Mat& rlt) const {
    if (image.empty()) {
        qDebug() << "Input image is empty" << Qt::endl;
        return "error: empty image";
    }

    if (se.empty()) {
        qDebug() << "Structuring element is empty" << Qt::endl;
        return "error: empty SE";
    }

    // 方法1：使用 OpenCV 内置函数
    cv::morphologyEx(image, rlt, cv::MORPH_OPEN, se);

    return "ok";
}

/*************************************************
*   开运算 Eigen
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::morphologicalOepning(grayEigen& image, const grayEigen& se, grayEigen& rlt) const {
    if (image.size() == 0) {
        qDebug() << "Input image is empty" << Qt::endl;
        return "error: empty image";
    }

    if (se.size() == 0) {
        qDebug() << "Structuring element is empty" << Qt::endl;
        return "error: empty SE";
    }

    // 先腐蚀
    grayEigen temp;
    QString result = morphologicalErosion(image, se, temp);
    if (result != "ok") {
        return result;
    }

    // 再膨胀
    result = morphologicalDilation(temp, se, rlt);

    return result;
}

/*************************************************
*   闭运算 cv
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::morphologicalClosing(cv::Mat& image, const cv::Mat& se, cv::Mat& rlt) const {
    if (image.empty()) {
        qDebug() << "Input image is empty" << Qt::endl;
        return "error: empty image";
    }

    if (se.empty()) {
        qDebug() << "Structuring element is empty" << Qt::endl;
        return "error: empty SE";
    }

    cv::morphologyEx(image, rlt, cv::MORPH_CLOSE, se);
    return "ok";
}

/*************************************************
*   闭运算 eigen
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::morphologicalClosing(grayEigen& image, const grayEigen& se, grayEigen& rlt) const {
    if (image.size() == 0) {
        qDebug() << "Input image is empty" << Qt::endl;
        return "error: empty image";
    }

    if (se.size() == 0) {
        qDebug() << "Structuring element is empty" << Qt::endl;
        return "error: empty SE";
    }

    // 先膨胀
    grayEigen temp = grayEigen::Zero(image.rows(),image.cols());
    QString result = morphologicalDilation(image, se, temp);
    if (result != "ok") {
        return result;
    }

    // 再腐蚀
    result = morphologicalErosion(temp, se, rlt);

    return result;
}

/*************************************************
*   距离变换 cv
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::distanceTransform(cv::Mat& image,DISTANCE_TYPE type,cv::Mat& rlt) const{
    if(image.empty()) return "the input image is empty.";

    int distanceType = 0;
    int maskSize = 0;

    switch(type){
    case DISTANCE_TYPE::L1:
        distanceType = cv::DIST_L1;
        maskSize = 3;  // L1 通常使用 3x3 mask
        break;

    case DISTANCE_TYPE::Chess:
        distanceType = cv::DIST_C;
        maskSize = 3;  // Chessboard 距离使用 3x3 mask
        break;

    case DISTANCE_TYPE::L2:
        distanceType = cv::DIST_L2;
        maskSize = 5;  // L2 精度较低但速度快的版本
        break;

    case DISTANCE_TYPE::L2_Precise:
        distanceType = cv::DIST_L2;
        maskSize = cv::DIST_MASK_PRECISE;  // 精确的 L2 距离计算
        break;
    }

    // 执行距离变换和归一化
    cv::Mat normalized;
    dt_dist_map_cv_ = cv::Mat::zeros(image.size(), CV_32F);
    cv::distanceTransform(image, dt_dist_map_cv_, distanceType, maskSize);

    cv::normalize(dt_dist_map_cv_, normalized, 0, 255, cv::NORM_MINMAX);
    normalized.convertTo(rlt, CV_8U);

    return "ok";
}

/*************************************************
*   距离变换 eigen
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::distanceTransform(grayEigen& image,DISTANCE_TYPE type,grayEigen& rlt) const{
    double a=0.0; // 直线代价
    double b=0.0; // 对角代价
    switch(type){
    case DISTANCE_TYPE::L1:
        a = 1.0;
        b = 2.0;
        // mask.resize(maskSize,maskSize); mask.setZero();
        // mask <<
        break;

    case DISTANCE_TYPE::Chess:
        a = 1.0;
        b = 1.0;
        break;
    case DISTANCE_TYPE::L2:
        a = 1.0;
        b = 1.44;
        break;

    case DISTANCE_TYPE::L2_Precise:
        a = 1.0;
        b = 1.44;
        // maskSize = cv::DIST_MASK_PRECISE;  // 精确的 L2 距离计算
        break;
    default:
        break;
    }

    borgefors(image,a,b,rlt);
    return "ok";
}

/*************************************************
*   borgefors倒角算法求解距离变换
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::borgefors(const grayEigen& image,double a,double b,grayEigen& rlt)const{
    // intensityEigen dist(image.rows(),image.cols()); // 距离图
    // dt_dist_map_eigen_.set
    dt_dist_map_eigen_.resize(image.rows(),image.cols());
    // dt_dist_map_eigen_.res
    dt_dist_map_eigen_.setZero();
    intensityEigen& dist = dt_dist_map_eigen_;

    double inf = std::numeric_limits<double>::infinity();

    int rows = image.rows();
    int cols = image.cols();

    for(int i =0;i<rows;++i){
        for(int j =0;j<cols;++j){
            if(image(i,j) != 0) // 前景
                dist(i,j) = inf;
        }
    }

    // 动态规划找最小距离
    // 前向扫描
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (dist(i, j) != 0) { // 前景
                if (i > 0 && j > 0) // 左上
                    dist(i, j) = std::min(dist(i, j), dist(i-1, j-1) + b);
                if (i > 0)  // 左
                    dist(i, j) = std::min(dist(i, j), dist(i-1, j) + a);
                if (i > 0 && j < cols-1)    // 左下
                    dist(i, j) = std::min(dist(i, j), dist(i-1, j+1) + b);
                if (j > 0)      // 上
                    dist(i, j) = std::min(dist(i, j), dist(i, j-1) + a);
            }
        }
    }

    // 后向扫描
    for (int i = rows-1; i >= 0; i--) {
        for (int j = cols-1; j >= 0; j--) {
            if (dist(i, j) != 0) { // 前景
                if (i < rows-1 && j < cols-1)   // 右下
                    dist(i, j) = std::min(dist(i, j), dist(i+1, j+1) + b);
                if (i < rows-1) // 右
                    dist(i, j) = std::min(dist(i, j), dist(i+1, j) + a);
                if (i < rows-1 && j > 0)    // 右上
                    dist(i, j) = std::min(dist(i, j), dist(i+1, j-1) + b);
                if (j < cols-1)             // 下
                    dist(i, j) = std::min(dist(i, j), dist(i, j+1) + a);
            }
        }
    }

    // dist 归一化
    double max_dist = dist.maxCoeff();

    for(int i=0;i<rows;++i){
        for(int j =0;j<cols;++j){
            rlt(i,j) = static_cast<uchar>(255.0 * (dist(i,j)/max_dist));
        }
    }

    return "ok";
}

/*************************************************
*   骨架 cv
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::skeleton(const cv::Mat& image,DISTANCE_TYPE type,cv::Mat& rlt) const{
    if(image.empty()) return "the input image is empty.";
    cv::Mat se =cv::getStructuringElement(cv::MORPH_CROSS,{3,3});

    cv::Mat ori_image;
    cv::Mat ero;
    cv::Mat temp;
    image.copyTo(ori_image);
    rlt.setTo(0);  // 必须清除，因为不一定对所有pixel进行了处理

    while(1){
        // 腐蚀
        cv::erode(ori_image,ero,se);
        // 膨胀
        cv::dilate(ero, temp,se);
        // 相减
        cv::subtract(ori_image,temp,temp);
        // 或操作
        cv::bitwise_or(rlt,temp,rlt);

        ero.copyTo(ori_image);

        if(cv::countNonZero(ori_image) == 0)
            break;

    }

    return "ok";
}

/*************************************************
*   骨架算法 eigen
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::skeleton(const grayEigen& image,DISTANCE_TYPE type,grayEigen& rlt) const{
    grayEigen se;
    se.resize(3,3);
    genSEKernel(se,SE_TYPE::Cross,3);

    grayEigen ori_image = image;
    grayEigen ero(image.rows(), image.cols());
    grayEigen dilated(image.rows(), image.cols());
    grayEigen skeleton_part(image.rows(), image.cols());

    ero.setZero();
    dilated.setZero();
    skeleton_part.setZero();
    rlt.setZero();

    while(1){
        // 腐蚀
        morphologicalErosion(ori_image, se, ero);

        // 膨胀
        morphologicalDilation(ero, se, dilated);

        // 相减（得到当前层的骨架）
        subtract(ori_image, dilated, skeleton_part);

        // 累积骨架
        bitwise_or(rlt, skeleton_part, rlt);

        // 更新原图像为腐蚀结果
        ori_image = ero;


        if(countNonZero(ori_image) == 0)
            break;

        // itr_cnt++;
        // if(itr_cnt > 1000) break;

    }

    return "ok";
}

/*************************************************
*   带限相减
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::subtract(grayEigen& image1,grayEigen& image2,grayEigen& rlt)const{
    rlt = image1 - image2;
    for(int i = 0; i < rlt.rows(); ++i) {
        for(int j = 0; j < rlt.cols(); ++j) {
            if(rlt(i,j) < 0) rlt(i,j) = 0;
            if(rlt(i,j) > 255) rlt(i,j) = 255;  // 也截断上限
        }
    }
    return "ok";
}

/*************************************************
*   图像或操作
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::bitwise_or(
    const grayEigen& image1,const grayEigen& image2,grayEigen& rlt)const{
    // 检查尺寸
    if (image1.rows() != image2.rows() || image1.cols() != image2.cols()) {
        return QString("Image size mismatch");
    }
    for(int i=0;i<image1.rows();++i){
        for(int j=0;j<image1.cols();++j){
            if(image1(i,j)!=0||image2(i,j) !=0) rlt(i,j) = 255;
            else rlt(i,j) = 0;
        }
    }

    return "ok";
}

/*************************************************
*   非0计数
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
int ImageProcesser::countNonZero(grayEigen& image)const{
    int cntn0 = 0;
    for(int i =0;i<image.rows();++i){
        for(int j=0;j<image.cols();++j){
            if(image(i,j) != 0) ++cntn0;
        }
    }
    return cntn0;
}

/*************************************************
*   骨架重建
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::skeletonRestoration(cv::Mat& image,DISTANCE_TYPE type,cv::Mat& rlt) const{
    if(image.empty()) return "the input image is empty.";
    // cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));

    cv::Mat skeletonDist = cv::Mat::zeros(image.size(), CV_32F);
    for(int i = 0; i < image.rows; ++i) {
        for(int j = 0; j < image.cols; ++j) {
            if(image.at<uchar>(i, j) != 0) {
                skeletonDist.at<float>(i, j) = dt_dist_map_cv_.at<float>(i, j);
            }
        }
    }

    // 从距离图转到膨胀次数图
    cv::Mat times;
    skeletonDist.convertTo(times, CV_32S);

    rlt = cv::Mat::zeros(image.size(), CV_8U);

    // 不做也可以
    std::vector<int> uniqueTimes;
    for(int i = 0; i < times.rows; ++i) {
        for(int j = 0; j < times.cols; ++j) {
            int t = times.at<int>(i, j);
            if(t > 0 && std::find(uniqueTimes.begin(), uniqueTimes.end(), t) == uniqueTimes.end()) {
                uniqueTimes.push_back(t);
            }
        }
    }

    // 从大到小处理,自适应核大小
    std::sort(uniqueTimes.begin(), uniqueTimes.end(), std::greater<int>());
    cv::Mat current = image.clone();

    for(int t : uniqueTimes) {
        // 提取需要膨胀t次的点
        int radius = static_cast<int>(t + 0.5);
        cv::Mat mask;
        cv::compare(times, t, mask, cv::CMP_EQ);
        cv::bitwise_and(current, mask, mask);
        if(cv::countNonZero(mask) > 0) {
            cv::Mat kernel = cv::getStructuringElement(
                cv::MORPH_ELLIPSE,
                cv::Size(2*radius + 1, 2*radius + 1)
                );
            cv::dilate(mask, mask, kernel);
            cv::bitwise_or(rlt, mask, rlt);
        }

        // 更新current用于下一轮（距离减1）
        cv::Mat nextMask;

        cv::compare(times, t - 1, nextMask, cv::CMP_GE);
        cv::bitwise_and(image, nextMask, current);
    }

    return "ok";
}

/*************************************************
*   骨架重建
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::skeletonRestoration(grayEigen& image,DISTANCE_TYPE type,grayEigen& rlt) const{
    int rows = image.rows();
    int cols = image.cols();

    std::unordered_set<int> uniqueTimes;
    // Eigen::Matrix<int,-1,-1> timesMap;
    // timesMap.resize(rows,cols);
    // timesMap.setZero();
    for(int i =0;i<rows;++i){
        for(int j=0;j<cols;++j){
            if(image(i,j) != 0){
                uniqueTimes.insert(static_cast<int>(dt_dist_map_eigen_(i,j)));
                // timesMap(i,j) = static_cast<int>(dt_dist_map_eigen_(i,j));
            }
        }
    }

    rlt.resize(rows,cols);
    rlt.setZero();

    // 排序
    std::vector<int> sortedTimes(uniqueTimes.begin(), uniqueTimes.end());
    std::sort(sortedTimes.begin(), sortedTimes.end(),std::greater<int>());

    grayEigen current = image;
    for(auto& t : sortedTimes) {
        grayEigen mask;
        compare(dt_dist_map_eigen_,t,image,mask,CMP_TYPE::COMP_E);
        bitwise_and(current, mask, mask);
        std::cout << "t:" << t << std::endl;
        // show(mask,"mask");
        if(countNonZero(mask) > 0) {
            grayEigen dilated;
            grayEigen se;

            genSEKernel(se,SE_TYPE::Ellipse,2*t+1);
            morphologicalDilation(mask, se,dilated);
            bitwise_or(rlt, dilated, rlt);
            // show(rlt,"the rlt_"+std::to_string(t));
        }

        // 更新current用于下一轮（距离减1）
        grayEigen nextMask;

        compare(dt_dist_map_eigen_, t - 1, image,nextMask, CMP_TYPE::COMP_GE);
        bitwise_and(image, nextMask, current);
    }

    return "ok";
}

/*************************************************
*   compare
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void ImageProcesser::compare(intensityEigen& image,int t,const grayEigen& mask,grayEigen& rlt,CMP_TYPE cmp_type)const{
    // 确保输出图像尺寸与输入一致
    rlt.resize(image.rows(), image.cols());
    rlt.setZero();

    // 根据比较类型定义比较函数
    std::function<bool(int, int)> cmp_func;
    switch (cmp_type) {
    case CMP_TYPE::COMP_E:  // 等于
        cmp_func = [t](int pixel, int) { return pixel == t; };
        break;
    case CMP_TYPE::COMP_GE: // 大于等于
        cmp_func = [t](int pixel, int) { return pixel >= t; };
        break;
    default:
        throw std::invalid_argument("Unsupported comparison type");
    }

    // 遍历图像，仅处理mask非0的位置
    for (int i = 0; i < image.rows(); ++i) {
        for (int j = 0; j < image.cols(); ++j) {
            if (mask(i, j) != 0) {  // 仅处理mask中的有效像素
                if (cmp_func(image(i, j), t)) {
                    rlt(i, j) = BINARY_ONE;
                } else {
                    rlt(i, j) = BINARY_ZERO;
                }
            }
        }
    }
}

/*************************************************
*   图像与
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString ImageProcesser::bitwise_and(
    const grayEigen& image1,const grayEigen& image2,grayEigen& rlt)const{
    // 检查尺寸是否一致
    if (image1.rows() != image2.rows() || image1.cols() != image2.cols()) {
        return QString("Image size mismatch: (%1,%2) vs (%3,%4)")
        .arg(image1.rows()).arg(image1.cols())
            .arg(image2.rows()).arg(image2.cols());
    }

    for(int i=0;i<image1.rows();++i){
        for(int j=0;j<image1.cols();++j){
            rlt(i,j) = (image1(i,j)!=0 && image2(i,j) != 0) ? BINARY_ONE:BINARY_ZERO;
        }
    }

    return "ok";
}

