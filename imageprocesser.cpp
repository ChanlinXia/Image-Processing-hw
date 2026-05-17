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
    // std::cout << "Image size: " << gray_origin.size() << std::endl;
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
    mask= cv::Mat::zeros(rows,cols,CV_8UC1);
    cv::Mat gray_image;
    getGray(image,gray_image);

    // qDebug() << "start to hist" << Qt::endl;
    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange = {range};
    cv::Mat hist;
    cv::calcHist(&gray_image, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

    // qDebug() << "start to mask" << Qt::endl;
    if( is_upper) cv::threshold(gray_image, mask, thershold, 255, cv::THRESH_TOZERO_INV);
    else cv::threshold(gray_image, mask, thershold, 255, cv::THRESH_TOZERO);

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

    for(int i=0;i<rows;++i){
        for(int j=0;j<cols;++j){
            auto brightness = image(i,j);
            if(is_upper){
                if(brightness <= threshold)mask(i,j) = 1;
                else mask(i,j) = 0;
            }
            else{
                if(brightness <= threshold)mask(i,j) = 0;
                else mask(i,j) = 1;
            }
        }
    }

    return "ok";
}




template<typename T>
void ImageProcesser::combine(T& image1,T& image2){

    if constexpr (std::is_same_v<T, cv::Mat>) {// cv::Mat
        int rows = image1.rows;
        int cols = image1.cols;

        // 假设 image1 和 image2 是 CV_8U 类型（0-255）
        cv::Mat magnitude(rows, cols, CV_8U);

        for(int i = 0; i < rows; ++i) {
            const uchar* gx_row = image1.ptr<uchar>(i);
            const uchar* gy_row = image2.ptr<uchar>(i);
            uchar* mag_row = magnitude.ptr<uchar>(i);

            for(int j = 0; j < cols; ++j) {
                // 注意：uchar 类型需要提升到 int 避免溢出
                int gx = gx_row[j];
                int gy = gy_row[j];
                double val = std::sqrt(gx*gx + gy*gy);
                mag_row[j] = val > 255 ? 255 : static_cast<uchar>(val);
            }
        }

        // result_cv_ = magnitude.clone();

        // cv::Mat result;
        // cv::magnitude(image1, image2, result);

        // cv::normalize(result, result_cv_, 0, 255, cv::NORM_MINMAX);
        // result.convertTo(result, CV_8UC1);

    }
    else if constexpr(std::is_same_v<T, grayEigen>){  // Eigen::Matrix

        grayEigen result = (image1.array().square() + image2.array().square()).sqrt();

        auto min_val = result.minCoeff();
        auto max_val = result.maxCoeff();

        // 归一化到 [0, 1] 范围
        // result_eigen_ = (result.array() - min_val) / (max_val - min_val);

        // result_eigen_ = (normalized * 255.0).array().cast<uchar>();
    }
}



