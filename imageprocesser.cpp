#include "imageprocesser.h"

ImageProcesser::ImageProcesser() {}

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

    // 1. 创建 padded 矩阵并填充边界(镜像)
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
    // rlt.resize(rows, cols);
    // rlt.setZero();

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

    // 计算各个方向的边界填充大小
    int top = se_rows / 2;
    int bottom = se_rows - top - 1;
    int left = se_cols / 2;
    int right = se_cols - left - 1;


    int padded_rows = rows + top + bottom;
    int padded_cols = cols + left + right;
    grayEigen padded(padded_rows, padded_cols);
    padded.setZero();  // 边界填充0


    // 复制原图像到中心区域
    padded.block(top, left, rows, cols) = image;

    // 初始化结果矩阵
    rlt.resize(rows, cols);
    // rlt.setZero();

    // 遍历每个像素
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            bool all_foreground = true;

            // 检查结构元素覆盖的所有位置是否都是前景
            for (const auto& offset : offsets) {
                int img_i = i + top + offset.first;
                int img_j = j + left + offset.second;

                // // // 边界检查（超出边界的视为背景）
                if (img_i < 0 || img_i >= rows || img_j < 0 || img_j >= cols) {
                    // all_foreground = false;
                    continue; // 对于边界外选择跳过，这是和opencv结果一致的关键
                }

                if (padded(img_i, img_j) == 0) {  // 遇到背景
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

    // 方法2：手动实现（注释掉，使用内置函数更快）
    // cv::Mat temp;
    // cv::erode(image, temp, se);
    // cv::dilate(temp, rlt, se);

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
