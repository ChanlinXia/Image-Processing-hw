#ifndef PAGEBINARYOPERATION_H
#define PAGEBINARYOPERATION_H

#include "pagebase.h"

class pageBinaryOperation : public PageBase
{

public:
    explicit pageBinaryOperation(bool show_original_image=true);

private:
    void setContainer();
    void updateOriginalBin();
    // template<typename T>
    // void pageBinaryOperation::getKernel(T& kernel,ImageProcesser::SE_TYPE se_type,int kernel_size,ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE operation_type);
    void updateOutput();


    grayEigen bin_matrix_;
    cv::Mat bin_mat_;

    int intensity_threshold_ = 120;
    bool threshold_for_up_ = true;
    int kernel_size_ = 3;

    ImageProcesser::SE_TYPE se_type_ = ImageProcesser::SE_TYPE::Rect;
    ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE operation_type_ = ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE::Dilation;
};

#endif // PAGEBINARYOPERATION_H
