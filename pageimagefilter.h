#ifndef PAGEIMAGEFILTER_H
#define PAGEIMAGEFILTER_H

#include "pagebase.h"

enum class KERNEL_TYPE{Roberts,Prewitt,Sobel,Gaussian,Median};
enum class CONVOL_DIR{Vertical,Horizontal,Both};

class pageImageFilter : public PageBase
{
public:
    explicit pageImageFilter(PageBase *parent = nullptr);


private:
    void setContainer();
    template<typename T>
    void getKernel(T& kernel_cv,KERNEL_TYPE kernel_type,int kernel_size,CONVOL_DIR dir);
    // void getKernel(Eigen::MatrixXd& kernel_cv,KERNEL_TYPE kernel_type,int kernel_size,CONVOL_DIR dir);

    const QString execConvoluton(KERNEL_TYPE kernel_type,int kernel_size,CONVOL_DIR dir);

    // ImageProcesser image_processer_;
};

#endif // PAGEIMAGEFILTER_H
