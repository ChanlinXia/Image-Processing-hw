#include "pagebinaryoperation.h"

pageBinaryOperation::pageBinaryOperation(bool show_original_image)
    :PageBase(nullptr,show_original_image)
{
    setContainer();
    // auto container_layout = getContainerLayout();

}


void pageBinaryOperation::setContainer(){
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

    connect(operation_group, QOverload<int>::of(&QButtonGroup::buttonClicked),
            [this](int id) {
        operation_type_= static_cast<ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE>(id);
                updateOutput();
    });

    connect(kernel_group, QOverload<int>::of(&QButtonGroup::buttonClicked),
            [this](int id) {
        se_type_ = static_cast<ImageProcesser::SE_TYPE>(id);
    });


    connect(kernel_size_edit,&QLineEdit::textChanged,[this](const QString& text){
        bool ok = false;
        int value = text.toInt(&ok);
        if(ok){
            kernel_size_ = value;
        }
    });

    // threshold widget
    QHBoxLayout* val_set_layout = new QHBoxLayout();

    QSlider* slider_threshold = new QSlider(Qt::Horizontal);
    slider_threshold->setMaximum(255);
    slider_threshold->setMinimum(0);
    slider_threshold->setValue(intensity_threshold_);
    val_set_layout->addWidget(slider_threshold,4);

    QLineEdit* line_edit_threshold = new QLineEdit(QString::number(intensity_threshold_));
    val_set_layout->addWidget(line_edit_threshold,1);

    QPushButton* reverse_button = new QPushButton("reverse");
    val_set_layout->addWidget(reverse_button,1);
    container_layout->addLayout(val_set_layout);

    connect(reverse_button,&QPushButton::clicked,[&](){
        threshold_for_up_=!threshold_for_up_;
        updateOriginalBin();
    });

    connect(line_edit_threshold,&QLineEdit::textChanged,[slider_threshold,this](const QString& text){
        bool ok = false;
        int value = text.toInt(&ok);
        if(ok){
            value = qBound(slider_threshold->minimum(),
                           value,
                           slider_threshold->maximum());
            slider_threshold->setValue(value);
            intensity_threshold_ = value;
        }
    });

    connect(slider_threshold,&QSlider::valueChanged,
            [line_edit_threshold,this](int value){
                line_edit_threshold->setText(QString::number(value));
                intensity_threshold_ = value;
                updateOriginalBin();
            });


 connect(this,&PageBase::loadAImage,[this](const QString& path){
        updateOriginalBin();
    });
}

/*************************************************
*   原始图像二值化展示
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void pageBinaryOperation::updateOriginalBin(){
    auto& image_container = getImageContainer();

    getImageProcessor().threshold(image_container.origin_image,intensity_threshold_,bin_mat_,threshold_for_up_);
    getImageProcessor().threshold(image_container.gray_matrix,intensity_threshold_,bin_matrix_,threshold_for_up_);
    // setEigenRlt();
    displayImage(bin_mat_,0,0);
    displayImage(bin_matrix_,1,0);
}

/*************************************************
*   生成核
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void pageBinaryOperation::updateOutput(){
    int _kernel_size = kernel_size_;

    if(_kernel_size%2 == 0) _kernel_size++;
    // 对CV
    cv::Mat se_kernel_cv;
    getImageProcessor().genSEKernel(se_kernel_cv,se_type_,_kernel_size);


    // 对Eigen
    grayEigen se_kernel_eigen;
    getImageProcessor().genSEKernel(se_kernel_eigen,se_type_,_kernel_size);

    switch(operation_type_){
    case ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE::Dilation:
        getImageProcessor().morphologicalDilation(bin_mat_,se_kernel_cv,getResultCV());
        getImageProcessor().morphologicalDilation(bin_matrix_,se_kernel_eigen,getResultEigen());
        break;
    case ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE::Erosion:
        getImageProcessor().morphologicalErosion(bin_mat_,se_kernel_cv,getResultCV());
        getImageProcessor().morphologicalErosion(bin_matrix_,se_kernel_eigen,getResultEigen());
        break;
    case ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE::Opening:
        getImageProcessor().morphologicalOepning(bin_mat_,se_kernel_cv,getResultCV());
        getImageProcessor().morphologicalOepning(bin_matrix_,se_kernel_eigen,getResultEigen());
        break;
    case ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE::Closing:
        getImageProcessor().morphologicalClosing(bin_mat_,se_kernel_cv,getResultCV());
        getImageProcessor().morphologicalClosing(bin_matrix_,se_kernel_eigen,getResultEigen());
        break;
    default:
        break;
    }

    updateOutputImage();
}




