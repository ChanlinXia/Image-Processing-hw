#ifndef PAGEBINARYFUNCTION_H
#define PAGEBINARYFUNCTION_H

#include "pagebase.h"

class pageBinaryFunction : public PageBase
{
public:
    explicit pageBinaryFunction(bool show_original_image = true);


private:
    void setContainer();
    void updateOriginalBin();
    void doFunction(int id);

    grayEigen bin_matrix_;
    cv::Mat bin_mat_;

    uchar intensity_threshold_ = 127;
    bool threshold_for_up_ = true;
    bool is_precise_ = false;
    ImageProcesser::DISTANCE_TYPE distance_type_=ImageProcesser::DISTANCE_TYPE::L1;

};

#endif // PAGEBINARYFUNCTION_H
