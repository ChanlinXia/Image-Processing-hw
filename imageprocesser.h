#ifndef IMAGEPROCESSER_H
#define IMAGEPROCESSER_H

#include "sys.h"

enum class KERNEL_TYPE{Roberts,Prewitt,Sobel,Gaussian,Median};
enum class CONVOL_DIR{Vertical,Horizontal,Both};

using grayEigen = Eigen::Matrix<uchar,Eigen::Dynamic,Eigen::Dynamic>;
/*************************************************
*   用于存储图片
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
struct ImageContainer{
    cv::Mat origin_image;
    Eigen::Matrix<uchar,Eigen::Dynamic,Eigen::Dynamic> intensity_matrix;
    Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic> r_channel_matrix;
    Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic> b_channel_matrix;
    Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic> g_channel_matrix;
    bool is_color;
    int rows_;
    int cols_;

    ImageContainer():
        is_color(false),
        rows_(0),
        cols_(0){};

    bool loadImage(const QString& path){
        origin_image=cv::imread(path.toStdString());

        if(origin_image.rows == 0) return false;

        if(origin_image.channels() == 3) is_color = true;
        else is_color = false;

        rows_ = origin_image.rows;
        cols_ = origin_image.cols;
        intensity_matrix.resize(rows_,cols_);
        if(is_color){
            r_channel_matrix.resize(rows_,cols_);
            b_channel_matrix.resize(rows_,cols_);
            g_channel_matrix.resize(rows_,cols_);
        }

        calcIntensity();
        return true;
    }

    void calcIntensity(){
        // if(intensity_matrix.)
        std::function<uint8_t(int,int)> calcPixelIntensity;  // 声明一个可调用对象
        if(is_color){ // three channel bgr default
            if(origin_image.type() == CV_8UC3){ // bgr8
                calcPixelIntensity = [this](int x,int y)->uint8_t{
                    cv::Vec3b pixel = origin_image.at<cv::Vec3b>(y, x);  // (行, 列)
                    return static_cast<uint8_t>(0.114 * pixel[0] + 0.587 * pixel[1] + 0.299 * pixel[2] + 0.5);
                };
            }
            else if(origin_image.type() == CV_16UC3 ){ // bgr16
                calcPixelIntensity = [this](int x,int y)->uint8_t{
                    cv::Vec3w pixel = origin_image.at<cv::Vec3w>(y, x);  // (行, 列)
                    return static_cast<uint8_t>(255 *
                                                    (0.114 * pixel[0] + 0.587 * pixel[1] + 0.299 * pixel[2] + 0.5)
                                                    / 65535.0+0.5);
                };
            }
            else{
                calcPixelIntensity = [this](int x,int y)->uint8_t{
                    cv::Vec3b pixel = origin_image.at<cv::Vec3b>(y, x);  // (行, 列)
                    return static_cast<uint8_t>((0.114 * pixel[0] + 0.587 * pixel[1] + 0.299 * pixel[2])*255+0.5);
                };
            }
        }
        else if(origin_image.channels() == 1){
            calcPixelIntensity = [this](int x,int y)->uint8_t{
                // cv::Vec3b pixel = origin_image.at<cv::Vec3b>(y, x);  // (行, 列)
                return origin_image.at<uint8_t>(y,x);
            };
        }
        else{
            return;
        }

        for(int i = 0;i<origin_image.rows;++i){
            for(int j=0;j<origin_image.cols;++j){
                auto brightness = calcPixelIntensity(j,i);
                intensity_matrix(i,j) = brightness;
            }
        }

        if(is_color){
            for(int i = 0;i<origin_image.rows;++i){
                for(int j=0;j<origin_image.cols;++j){
                    cv::Vec3b pixel = origin_image.at<cv::Vec3b>(i, j);
                    // auto brightness = calcPixelIntensity(j,i);
                    b_channel_matrix(i,j) = pixel[0];
                    g_channel_matrix(i,j) = pixel[1];
                    r_channel_matrix(i,j) = pixel[2];
                }
            }
        }
    }
};


class ImageProcesser
{
public:
    ImageProcesser();

    bool loadImage(const QString& path){
        bool ok = image_container_.loadImage(path);
        if(ok){
            int rows = image_container_.rows_;
            int cols = image_container_.cols_;
            result_cv_= cv::Mat::zeros(rows,cols,CV_8UC1);
            result_eigen_mat_=cv::Mat::zeros(rows,cols,CV_8UC1);
            result_eigen_.resize(rows,cols);
            // result_eigen_=grayEigen::Zero(rows,cols);
        }
        return ok;
        // return image_container_.loadImage(path);
    }

    const QString convolution(KERNEL_TYPE kernel,int kernel_size,CONVOL_DIR dir);
    // cv::Mat& getResultCV();

    cv::Mat& getResultCV(){
        return result_cv_;
    }

    cv::Mat& getResultEigen(){
        // cv::imshow("get result eigen before set", result_eigen_mat_);
        // cv::waitKey(1);  // 非阻塞，允许刷新
        return result_eigen_mat_;
    }


private:
    ImageContainer image_container_{};
    cv::Mat result_cv_;
    cv::Mat result_eigen_mat_;
    grayEigen result_eigen_;

    void eigenToCV();
    const QString mat_convolution(Eigen::MatrixXd& kernel);
    void cv_convolution(cv::Mat& kernel);


    void medianFiler();
};

#endif // IMAGEPROCESSER_H
