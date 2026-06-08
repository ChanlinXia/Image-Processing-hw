#include "pagegrayoperation.h"

pageGrayOperation::pageGrayOperation(bool show_origin_image) :
    PageBase(nullptr,show_origin_image)
{
    setContainer();
}


void pageGrayOperation::setContainer() {
    auto container_layout = getContainerLayout();

    // button widget
    QGridLayout* setting_layout = new QGridLayout();
    std::vector<QString> button_labels = {
        "Rect","Cross","Ellipse",
        "Dilation","Erosion","Opening","Closing"
    };

    QHBoxLayout* kernel_size_layout = new QHBoxLayout();
    QLineEdit* kernel_size_edit = new QLineEdit("3");
    QLabel* kernel_size_label = new QLabel("kernel_size");
    kernel_size_layout->addWidget(kernel_size_label);
    kernel_size_layout->addWidget(kernel_size_edit);

    QButtonGroup* kernel_group = new QButtonGroup(this);
    QButtonGroup* operation_group = new QButtonGroup(this);

    // 将前3个按钮放入第一行前三列
    for (int i = 0; i < 3; ++i) {
        QPushButton* btn = new QPushButton(button_labels[i]);
        setting_layout->addWidget(btn, 0, i);  // 第0行，第i列
        kernel_group->addButton(btn, i);  // 设置ID为i

    }

    // 第一行第四列放入 kernel_size_layout
    setting_layout->addLayout(kernel_size_layout, 0, 3);

    // 将后4个按钮放入第二行
    for (int i = 0; i < 4; ++i) {
        QPushButton* btn = new QPushButton(button_labels[3 + i]);
        setting_layout->addWidget(btn, 1, i);  // 第1行，第i列
        operation_group->addButton(btn, i);  // 设置ID为i
    }

    // 设置拉伸比例
    setting_layout->setColumnStretch(0, 1);
    setting_layout->setColumnStretch(1, 1);
    setting_layout->setColumnStretch(2, 1);

    container_layout->addLayout(setting_layout);

    connect(operation_group, &QButtonGroup::idClicked,
            [this](int id) {
                operation_type_= static_cast<ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE>(id);
                doOperation();
                // updateOutput();
            });

    connect(kernel_group, &QButtonGroup::idClicked,
            [this](int id){
                se_type_ = static_cast<ImageProcesser::SE_TYPE>(id);
            });


    connect(kernel_size_edit,&QLineEdit::textChanged,[this](const QString& text){
        bool ok = false;
        int value = text.toInt(&ok);
        if(ok){
            kernel_size_ = value;
        }
    });

    connect(this,&PageBase::loadAImage,[this](const QString& path){
        getGrayImage();
    });

}

void pageGrayOperation::getGrayImage() {
    auto& image_container = getImageContainer();

    // 灰度转换
    if(image_container.origin_image.channels() == 1) gray_mat_ = image_container.origin_image;
    else cv::cvtColor(image_container.origin_image,gray_mat_,cv::COLOR_BGR2GRAY);

    gray_matrix_ = std::move(image_container.gray_matrix);

    displayImage(gray_mat_,0,0);
    displayImage(gray_matrix_,1,0);
}

void pageGrayOperation::doOperation() {
    auto& image_processer = getImageProcessor();
    int _kernel_size = kernel_size_;

    if(_kernel_size%2 == 0) _kernel_size++;

    cv::Mat se_kernel_cv;
    image_processer.genSEKernel(se_kernel_cv,se_type_,_kernel_size);

    // 对Eigen
    grayEigen se_kernel_eigen;
    image_processer.genSEKernel(se_kernel_eigen,se_type_,_kernel_size);

    switch(operation_type_){
    case ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE::Dilation:
        image_processer.morphologicalDilation(gray_mat_,se_kernel_cv,getResultCV());
        image_processer.morphologicalDilation(gray_matrix_,se_kernel_eigen,getResultEigen());
        break;
    case ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE::Erosion:
        image_processer.morphologicalErosion(gray_mat_,se_kernel_cv,getResultCV());
        image_processer.morphologicalErosion(gray_matrix_,se_kernel_eigen,getResultEigen(),false);
        break;
    case ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE::Opening:
        image_processer.morphologicalOepning(gray_mat_,se_kernel_cv,getResultCV());
        image_processer.morphologicalOepning(gray_matrix_,se_kernel_eigen,getResultEigen(),false);
        break;
    case ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE::Closing:
        image_processer.morphologicalClosing(gray_mat_,se_kernel_cv,getResultCV());
        image_processer.morphologicalClosing(gray_matrix_,se_kernel_eigen,getResultEigen(),false);
        break;
    default:
        break;
    }

    updateOutputImage();
}


