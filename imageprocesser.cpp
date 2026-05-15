#include "imageprocesser.h"

ImageProcesser::ImageProcesser() {}


void ImageProcesser::eigenToCV(){
    cv::eigen2cv(result_eigen_,result_eigen_mat_);
}

const QString ImageProcesser::convolution(KERNEL_TYPE kernel_type,int kernel_size,CONVOL_DIR dir){
    // init the kernel
    int _kernel_size = kernel_size;
    Eigen::MatrixXd kernel;
    cv::Mat kernel_cv;

    switch(kernel_type){
    case KERNEL_TYPE::Roberts:
        _kernel_size = 2;
        kernel.resize(_kernel_size, _kernel_size);
        kernel << 1, 0,
                0, -1;
        kernel_cv = (cv::Mat_<double>(2, 2) << 1.0, 0.0, 0.0, -1.0);
        // kernel_mat = cv
        break;
    case KERNEL_TYPE::Prewitt:
        _kernel_size = 3;
        kernel = Eigen::MatrixXd::Zero(_kernel_size, _kernel_size);
        break;
    case KERNEL_TYPE::Sobel:
        _kernel_size = 3;
        kernel = Eigen::MatrixXd::Zero(_kernel_size, _kernel_size);
        break;
    case KERNEL_TYPE::Gaussian:
        break;
    case KERNEL_TYPE::Median:
        break;
    default: return "please input a correct kernel type";
        break;
    }
    cv_convolution(kernel_cv);

    // 打印 result_cv_ 的数据地址
    // std::cout << "In cv_convolution: result_cv_.data = " << (void*)result_cv_.data << std::endl;
    // std::cout << "In cv_convolution: result_cv_ size = " << result_cv_.size() << std::endl;
    return mat_convolution(kernel);
    // return "ok";
}

void ImageProcesser::cv_convolution(cv::Mat& kernel){
    cv::Mat gray_origin;
    if(image_container_.origin_image.empty()) {
        std::cerr << "Error: Original image is empty!" << std::endl;
        return;
    }

    // cv::imshow("Original Gray Image", gray_origin);    cv::waitKey(1);  // 1ms 刷新，不阻塞

    // std::cout << "Kernel:" << std::endl << kernel << std::endl;

    cv::cvtColor(image_container_.origin_image,gray_origin,cv::COLOR_RGB2GRAY);
    // std::cout << "Image size: " << gray_origin.size() << std::endl;
    cv::Mat result_float;
    cv::filter2D(gray_origin, result_float, CV_32F, kernel);
    // cv::normalize(result_float, result_cv_, 0, 255, cv::NORM_MINMAX);
    // result_cv_.convertTo(result_cv_, CV_8U);
    cv::convertScaleAbs(result_float, result_cv_);
}
// cv::Mat& ImageProcesser::getResultCV(){
//     cv::imshow("get result cv before set", result_cv_);
//     cv::waitKey(1);  // 非阻塞，允许刷新
//     return result_cv_;
// }
const QString ImageProcesser::mat_convolution(Eigen::MatrixXd& kernel){
    if(image_container_.rows_ == 0) return "the image container is empty";

    int kRows = kernel.rows();
    int kCols = kernel.cols();
    int rows = image_container_.rows_;
    int cols = image_container_.cols_;
    int padRows = kRows/2;
    int padCols = kCols/2;

    grayEigen padded(rows + 2 * padRows, cols + 2 * padCols);
    padded.setZero();
    padded.block(padRows,padCols,rows,cols) = image_container_.intensity_matrix;

    cv::Mat intensity_origin;
    cv::eigen2cv(image_container_.intensity_matrix,intensity_origin);

    for(int i =0;i<image_container_.rows_;++i){
        for(int j = 0;j<image_container_.cols_;++j){
            grayEigen window = padded.block(i, j, kRows, kCols);
            double sum = (window.cast<double>().array() * kernel.array()).sum();

            result_eigen_(i, j) = static_cast<uchar>(std::max(0.0, std::min(255.0, abs(sum))));
        }
    }
    eigenToCV();
    return "";
}

void ImageProcesser::medianFiler(){

}
