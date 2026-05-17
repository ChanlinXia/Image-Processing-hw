#include "pageimagefilter.h"

pageImageFilter::pageImageFilter(PageBase *parent)
    : PageBase{parent}
{
    setContainer();

}

void pageImageFilter::setContainer(){
    auto container_layout = getContainerLayout();
    container_layout->setDirection(QBoxLayout::Direction::LeftToRight);

    // 创建 QWidget 作为网格布局的容器
    QWidget* grid_widget = new QWidget();
    QGridLayout* grid_layout = new QGridLayout(grid_widget);

    // 创建控件
    QHBoxLayout* kernel_size_layout = new QHBoxLayout();
    QLineEdit* kernel_size_edit = new QLineEdit("3");
    QLabel* kernel_size_lable = new QLabel("kernel_size");
    kernel_size_layout->addWidget(kernel_size_lable);
    kernel_size_layout->addWidget(kernel_size_edit);

    QComboBox* direction_combo = new QComboBox();
    direction_combo->addItem("Vertical",  static_cast<int>(CONVOL_DIR::Vertical));
    direction_combo->addItem("Horizontal",  static_cast<int>(CONVOL_DIR::Horizontal));
    direction_combo->addItem("Both", static_cast<int>(CONVOL_DIR::Both));

    // button group
    QButtonGroup* group = new QButtonGroup(this);

    QPushButton* btn1 = new QPushButton("Roberts");
    QPushButton* btn2 = new QPushButton("Prewitt");
    QPushButton* btn3 = new QPushButton("Sobel");
    QPushButton* btn4 = new QPushButton("Guassian");
    QPushButton* btn5 = new QPushButton("Median");

    group->addButton(btn1, static_cast<int>(KERNEL_TYPE::Roberts));
    group->addButton(btn2, static_cast<int>(KERNEL_TYPE::Prewitt));
    group->addButton(btn3, static_cast<int>(KERNEL_TYPE::Sobel));
    group->addButton(btn4, static_cast<int>(KERNEL_TYPE::Gaussian));
    group->addButton(btn5, static_cast<int>(KERNEL_TYPE::Median));


    // 添加到网格布局，形成 2 行 3 列
    // grid_layout->addLayout(container_layout,0,0);
    grid_layout->addWidget(direction_combo, 0, 0);               // 第2行第3列

    grid_layout->addWidget(btn1, 1, 0);               // 第2行第3列
    grid_layout->addWidget(btn2, 1, 1);               // 第1行第2列
    grid_layout->addWidget(btn3, 1, 2);               // 第1行第3列
    grid_layout->addWidget(btn4, 2, 0);               // 第2行第1列
    grid_layout->addWidget(btn5, 2, 1);               // 第2行第2列
    grid_layout->addLayout(kernel_size_layout, 2, 2);  // 第1行第1列

    // 设置各列拉伸比例（可选，让布局更美观）
    grid_layout->setColumnStretch(0, 1);
    grid_layout->setColumnStretch(1, 1);
    grid_layout->setColumnStretch(2, 1);

    // 将网格容器添加到原布局
    container_layout->addWidget(grid_widget);

    // connect
    // connect(this,&PageBase::loadAImage,[&processer](const QString& path){
    //     // auto image_container = getImageContainer();
    //     // processer.loadImage(path);

    // });

    // connect buttongroup
    connect(group, QOverload<int>::of(&QButtonGroup::buttonClicked),
            [this,kernel_size_edit,direction_combo](int id) {
                bool ok = false;
                int kernelSize = kernel_size_edit->text().toInt(&ok);
                if(!ok) {
                    debug("Invalid kernel size");
                    return;
                }

                CONVOL_DIR dir = static_cast<CONVOL_DIR>(
                    direction_combo->currentData().toInt());

                QString debug_info = execConvoluton(
                    static_cast<KERNEL_TYPE>(id),
                    kernelSize,
                    dir
                    );

                if(debug_info != "ok") debug(debug_info);
                updateOutputImage();
            });

}

/*************************************************
*   执行卷积
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
const QString pageImageFilter::execConvoluton(KERNEL_TYPE kernel_type,int kernel_size,CONVOL_DIR dir){
    if(kernel_type == KERNEL_TYPE::Median){
        getImageProcessor().medianFilter(getImageContainer().origin_image,kernel_size,getResultCV());
        QString debug_info = getImageProcessor().medianFilter(getImageContainer().intensity_matrix,kernel_size,getResultEigen());
        if(debug_info == "ok") setEigenRlt();;
        return debug_info;
    }

    const auto& image_proceser = getImageProcessor();
    auto& image_container = getImageContainer();
    QString debug_info = "ok";
    QString return_debug_info ="ok";

    if(dir != CONVOL_DIR::Both || kernel_type == KERNEL_TYPE::Gaussian){
        // cv::mat
        cv::Mat kernel_cv;
        getKernel(kernel_cv,kernel_type,kernel_size,dir);
        debug_info=image_proceser.convolution(image_container.origin_image,kernel_cv,getResultCV());
        if(debug_info != "ok") return_debug_info = debug_info;

        // Eigen
        intensityEigen kernel_eigen;
        intensityEigen image_eigen;
        getKernel(kernel_eigen,kernel_type,kernel_size,dir);
        debug_info = image_proceser.convolution(image_container.intensity_matrix,kernel_eigen,getResultEigen());
        if(debug_info != "ok") return_debug_info += debug_info;

        setEigenRlt();
    }
    else{
        // horizontal
        cv::Mat kernel;
        intensityEigen kernel_eigen;

        cv::Mat rlt_h;
        grayEigen rlt_h_eigen;
        ImageTraits<grayEigen>::setZero(rlt_h_eigen,image_container.rows_,image_container.cols_);

        getKernel(kernel,kernel_type,kernel_size,CONVOL_DIR::Horizontal);
        getKernel(kernel_eigen,kernel_type,kernel_size,CONVOL_DIR::Horizontal);

        debug_info=image_proceser.convolution(image_container.origin_image,kernel,rlt_h);
        if(debug_info != "ok") return_debug_info= debug_info;

        debug_info = image_proceser.convolution(image_container.intensity_matrix,kernel_eigen,rlt_h_eigen);
        if(debug_info != "ok") return_debug_info+= debug_info;

        // vertical
        cv::Mat rlt_v;
        grayEigen rlt_v_eigen;
        ImageTraits<grayEigen>::setZero(rlt_v_eigen,image_container.rows_,image_container.cols_);

        getKernel(kernel,kernel_type,kernel_size,CONVOL_DIR::Vertical);
        getKernel(kernel_eigen,kernel_type,kernel_size,CONVOL_DIR::Vertical);

        debug_info=image_proceser.convolution(image_container.origin_image,kernel,rlt_v);
        if(debug_info != "ok") return_debug_info+= debug_info;

        debug_info = image_proceser.convolution(image_container.intensity_matrix,kernel_eigen,rlt_v_eigen);
        if(debug_info != "ok") return_debug_info+= debug_info;

        debug_info=image_proceser.norm(rlt_h,rlt_v,getResultCV());
        if(debug_info != "ok") return_debug_info+= debug_info;

        debug_info=image_proceser.norm(rlt_h,rlt_v,getResultEigen());
        if(debug_info != "ok") return_debug_info+= debug_info;

        setEigenRlt();
    }

    return return_debug_info;
}

/*************************************************
*   生成核
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
template<typename T>
void pageImageFilter::getKernel(T& kernel,KERNEL_TYPE kernel_type,int kernel_size,CONVOL_DIR dir){
    int _kernel_size = kernel_size;
    intensityEigen temp_kernel;
    // ImageTraits<intensityEigen>::setZero(temp_kernel,_kernel_size,_kernel_size,CV_32FC1);
    switch(kernel_type){
    case KERNEL_TYPE::Roberts:
        _kernel_size = 2;
        temp_kernel = Eigen::MatrixXd::Zero(_kernel_size,_kernel_size);
        if(dir == CONVOL_DIR::Horizontal) temp_kernel << 1.0, 0.0,0.0, -1.0;
        else temp_kernel << 0.0, 1.0,-1.0,0.0;
        // kernel_cv = (cv::Mat_<double>(_kernel_size, _kernel_size) << 1.0, 0.0, 0.0, -1.0);
        break;
    case KERNEL_TYPE::Prewitt:
        _kernel_size = 3;
        temp_kernel = Eigen::MatrixXd::Zero(_kernel_size,_kernel_size);
        temp_kernel << -1, 0, 1,
                        -1, 0, 1,
                        -1, 0, 1;
        if(dir == CONVOL_DIR::Vertical) temp_kernel.transposeInPlace();
        // kernel_cv = (cv::Mat_<double>(_kernel_size, _kernel_size) <<
        //                  -1, 0, 1,
        //              -1, 0, 1,
        //              -1, 0, 1);
        break;
    case KERNEL_TYPE::Sobel:
        _kernel_size = 3;
        temp_kernel = Eigen::MatrixXd::Zero(_kernel_size,_kernel_size);
        temp_kernel <<-1, 0, 1,
                        -2, 0, 2,
                        -1, 0, 1;
        if(dir == CONVOL_DIR::Vertical) temp_kernel.transposeInPlace();;
        // kernel = Eigen::MatrixXd::Zero(_kernel_size, _kernel_size);
        break;
    case KERNEL_TYPE::Gaussian:
        if(_kernel_size % 2 == 0) _kernel_size++;
        // // _kernel_size = 3;
        // sigma = 0.3 * ((_kernel_size - 1) * 0.5 - 1) + 0.8;

        temp_kernel = Eigen::MatrixXd::Zero(_kernel_size,_kernel_size);
        getImageProcessor().geneGaussionKernel<intensityEigen>(temp_kernel,_kernel_size);
        // int center = kernel_size / 2;
        // double sum = 0.0;
        // double sigma2 = sigma * sigma;
        // double sigma2pi = 2 * M_PI * sigma2;


        // temp_kernel << 0.057118, 0.124760, 0.057118,
        //     0.124760, 0.272483, 0.124760,
        //     0.057118, 0.124760, 0.057118;
        break;
    case KERNEL_TYPE::Median:
        // return getImageProcessor().medianFilter()
        break;
    default: qDebug() << "please input a correct kernel type" <<Qt::endl;
        break;
    }

     ImageTraits<T>::setZero(kernel,_kernel_size,_kernel_size,CV_32FC1);
    // 为kernel赋值
    for(int i=0;i<_kernel_size;++i){
        for(int j=0;j<_kernel_size;++j){
            ImageTraits<T>::setValue(kernel,i,j,temp_kernel(i,j));
        }
    }
}


