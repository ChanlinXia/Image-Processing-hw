#ifndef PAGEGRAYFUNCTION_H
#define PAGEGRAYFUNCTION_H

#include "pagebase.h"

class pageGrayFunction : public PageBase
{
public:
    explicit pageGrayFunction(bool show_origin_image=true);

    void setContainer();
    void doFunction(int id);
    void getGrayImage();

    grayEigen gray_matrix_;
    cv::Mat gray_mat_;
};

#endif // PAGEGRAYFUNCTION_H
