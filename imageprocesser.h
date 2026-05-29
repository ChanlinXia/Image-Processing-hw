#ifndef IMAGEPROCESSER_H
#define IMAGEPROCESSER_H

#include "sys.h"


#define BINARY_ONE 255
#define BINARY_ZERO 0

using grayEigen = Eigen::Matrix<uchar,Eigen::Dynamic,Eigen::Dynamic>;
using intensityEigen = Eigen::MatrixXd;


// 定义 traits 模板
template<typename T>
struct ImageTraits;

// cv::Mat
template<>
struct ImageTraits<cv::Mat> {
    static int rows(const cv::Mat& img) { return img.rows; }
    static int cols(const cv::Mat& img) { return img.cols; }
    static void setValue(cv::Mat& mat,int row,int col,double value){
        if(mat.type() == CV_64F){
            mat.at<double>(row,col) = value;
        }
        else if(mat.type() == CV_32F){
            mat.at<float>(row,col) = static_cast<float>(value);
        }
        else if(mat.type() == CV_8U){
            mat.at<uchar>(row, col) = static_cast<uchar>(value);
        }
        else{
            return;
        }
    }

    static double getValue(const cv::Mat& mat, int row, int col) {
        if (mat.type() == CV_64F) {
            return mat.at<double>(row, col);
        } else if (mat.type() == CV_32F) {
            return mat.at<float>(row, col);
        } else if (mat.type() == CV_8U) {
            return mat.at<uchar>(row, col);
        }
        else{
            return 0.0;
        }
        return 0.0;
    }

    static void setZero(cv::Mat& mat,int rows,int cols,int type){
        mat = cv::Mat::zeros(rows,cols,type);
    }
};

// Eigen::MatrixXd
template<>
struct ImageTraits<intensityEigen> {
    static int rows(const intensityEigen& img) { return img.rows(); }
    static int cols(const intensityEigen& img) { return img.cols(); }

    static void setValue(intensityEigen& matrix,int row,int col,double value){
        matrix(row,col) = value;
    }

    static double getValue(intensityEigen& matrix,int row,int col){
        return matrix(row,col);
    }

    static void setZero(intensityEigen& matrix,int rows,int cols,int type =0){
        matrix =intensityEigen::Zero(rows,cols);
    }
};

// grayEigen
template<>
struct ImageTraits<grayEigen> {
    static int rows(const grayEigen& img) { return img.rows(); }
    static int cols(const grayEigen& img) { return img.cols(); }

    static void setValue(grayEigen& matrix,int row,int col,uchar value){
        matrix(row,col) = value;
    }

    static uchar getValue(grayEigen& matrix,int row,int col){
        return matrix(row,col);
    }

    static void setZero(grayEigen& matrix,int rows,int cols,int type =0){
        matrix =grayEigen::Zero(rows,cols);
    }
};

class ImageProcesser
{


public:
    enum class SE_TYPE{Rect,Cross,Ellipse,Specific};
    enum class MORPHOLOGICAL_OPERATION_TYPE{Dilation,Erosion,Opening,Closing};
    ImageProcesser();

    // threshold
    const QString threshold(cv::Mat& image,int threshold,cv::Mat& mask,bool is_upper=true) const;
    const QString threshold(grayEigen& image,int threshold,grayEigen& mask,bool is_upper=true) const;

    // convolution
    const QString convolution(cv::Mat& image,cv::Mat& kernel) const;
    const QString convolution(cv::Mat& image,cv::Mat& kernel,cv::Mat& rlt) const;
    const QString convolution(intensityEigen& image,intensityEigen& kernel,grayEigen& rlt) const;

    // filter
    const QString medianFilter(intensityEigen& image,int kernel_size,grayEigen& rlt) const;
    const QString medianFilter(cv::Mat& image,int kernel_size,cv::Mat& rlt) const;

    // morphology
    // template<typename T>
    const QString genSEKernel(cv::Mat& kernel,SE_TYPE se_type,int kernel_size)const;
    const QString genSEKernel(grayEigen& kernel,SE_TYPE se_type,int kernel_size)const;


    const QString morphologicalDilation(cv::Mat& image,const cv::Mat& se,cv::Mat& rlt)const;
    const QString morphologicalDilation(grayEigen& image,const grayEigen& se,grayEigen& rlt)const;

    const QString morphologicalErosion(cv::Mat& image,const cv::Mat& se,cv::Mat& rlt) const;
    const QString morphologicalErosion(grayEigen& image,const grayEigen& se,grayEigen& rlt) const;

    const QString morphologicalOepning(cv::Mat& image,const cv::Mat& se,cv::Mat& rlt) const;
    const QString morphologicalOepning(grayEigen& image,const grayEigen& se,grayEigen& rlt) const;

    const QString morphologicalClosing(cv::Mat& image,const cv::Mat& se,cv::Mat& rlt) const;
    const QString morphologicalClosing(grayEigen& image,const grayEigen& se,grayEigen& rlt) const;

    template<typename T>
    void geneGaussionKernel(T& kernel,int kernel_size){
        double sigma =0.0;
        if(kernel_size % 2 == 0) kernel_size++;

        // 如果没有指定 sigma，使用默认值
        if(sigma <= 0) {
            sigma = 0.3 * ((kernel_size - 1) * 0.5 - 1) + 0.8;
        }

        // cv::Mat kernel(kernel_size, kernel_size, CV_64F);
        int center = kernel_size / 2;
        double sum = 0.0;
        double sigma2 = sigma * sigma;
        double sigma2pi = 2 * M_PI * sigma2;

        for(int i = 0; i < kernel_size; i++) {
            for(int j = 0; j < kernel_size; j++) {
                int x = i - center;
                int y = j - center;

                // 高斯函数公式
                double value = exp(-(x*x + y*y) / (2 * sigma2)) / sigma2pi;
                ImageTraits<T>::setValue(kernel,i,j,value);
                // kernel.at<double>(i, j) = value;
                sum += value;
            }
        }
        // 归一化
        kernel = kernel / sum;
    }

    template<typename T,typename rltT>
    const QString norm(T& image1,T& image2,rltT& rlt) const{
        int rows1 = ImageTraits<T>::rows(image1);
        int cols1 = ImageTraits<T>::cols(image1);

        int rows2 = ImageTraits<T>::rows(image2);
        int cols2 = ImageTraits<T>::cols(image2);

        if(rows1!=rows2 || cols1!=cols2) return "the size of the two image does not match";
        ImageTraits<rltT>::setZero(rlt,rows1,cols2,CV_8UC1);

        for(int i=0;i<rows1;++i){
            for(int j=0;j<cols1;++j){
                auto value1 = ImageTraits<T>::getValue(image1,i,j);
                auto value2 = ImageTraits<T>::getValue(image2,i,j);

                auto norm_val = sqrt(value1*value1+value2*value2) ;

                ImageTraits<rltT>::setValue(rlt,i,j,static_cast<uchar>(norm_val));
            }
        }

        return "ok";
    }

    template<typename T,typename rltT>
    const QString calcHist(T& image,rltT& rlt) const{
        int rows = ImageTraits<T>::rows(image);
        int cols = ImageTraits<T>::cols(image);
        if(rows == 0 || cols ==0) return "[ImageProcesser][calcHist]the image is empty";

        for(int i=0;i<rows;++i){
            for(int j=0;j<cols;++j){
                auto brightNess = ImageTraits<T>::getValue(image,i,j);
                ++rlt[brightNess];
            }
        }

        return "ok";
    }

private:
    void getGray(cv::Mat& image,cv::Mat& gray) const;

    template<typename T>
    void combine(T& image1,T& image2);
};

#endif // IMAGEPROCESSER_H
