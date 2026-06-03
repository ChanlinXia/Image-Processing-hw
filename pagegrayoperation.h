#ifndef PAGEGRAYOPERATION_H
#define PAGEGRAYOPERATION_H

#include "pagebase.h"

class pageGrayOperation : public PageBase
{
public:
    explicit pageGrayOperation(bool show_origin_image=true);


private:
    void setContainer() ;
    void doOperation();
    void getGrayImage();



    grayEigen bin_matrix_;
    cv::Mat bin_mat_;

    grayEigen gray_matrix_;
    cv::Mat gray_mat_;

    int intensity_threshold_ = 120;
    bool threshold_for_up_ = true;
    int kernel_size_ = 3;

    ImageProcesser::SE_TYPE se_type_ = ImageProcesser::SE_TYPE::Rect;
    ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE operation_type_ = ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE::Dilation;
};

#endif // PAGEGRAYOPERATION_H
