#ifndef PAGEIMAGEFILTER_H
#define PAGEIMAGEFILTER_H

#include "pagebase.h"

class pageImageFilter : public PageBase
{
public:
    explicit pageImageFilter(PageBase *parent = nullptr);


private:
    void setContainer();

    // ImageProcesser image_processer_;
};

#endif // PAGEIMAGEFILTER_H
