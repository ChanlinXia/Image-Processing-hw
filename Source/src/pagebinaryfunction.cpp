#include "pagebinaryfunction.h"

pageBinaryFunction::pageBinaryFunction(bool show_original_image)
    :PageBase(nullptr,show_original_image)
{
    setContainer();
}


void pageBinaryFunction::setContainer(){
    auto container_layout = getContainerLayout();

    // button widget
    QGridLayout* setting_layout = new QGridLayout();
    std::vector<QString> button_labels = {
        "L1","Chess","L2","Precise",
        "DT","Skeleton","Skeleton restoration","Closing"
    };

    QButtonGroup* distance_group = new QButtonGroup(this);
    QButtonGroup* function_group = new QButtonGroup(this);

    // 将前3个按钮放入第一行前三列
    for (int i = 0; i < 3; ++i) {
        QPushButton* btn = new QPushButton(button_labels[i]);
        setting_layout->addWidget(btn, 0, i);  // 第0行，第i列
        distance_group->addButton(btn, i);  // 设置ID为i

    }

    // 将后4个按钮放入第二行
    for (int i = 0; i < 3; ++i) {
        QPushButton* btn = new QPushButton(button_labels[4 + i]);
        setting_layout->addWidget(btn, 1, i);  // 第1行，第i列
        function_group->addButton(btn, i);  // 设置ID为i
    }

    // 设置拉伸比例
    setting_layout->setColumnStretch(0, 1);
    setting_layout->setColumnStretch(1, 1);
    setting_layout->setColumnStretch(2, 1);

    container_layout->addLayout(setting_layout);

    connect(distance_group,  &QButtonGroup::idClicked,
            [this](int id) {
                if(id == 3) {
                    is_precise_ = !is_precise_;
                    if(distance_type_ == ImageProcesser::DISTANCE_TYPE::L2 )
                        distance_type_=ImageProcesser::DISTANCE_TYPE::L2_Precise;
                    return;
                }
                distance_type_ = static_cast<ImageProcesser::DISTANCE_TYPE>(id);
            });

    connect(function_group,  &QButtonGroup::idClicked,
            [this](int id) {
                doFunction(id);
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

void pageBinaryFunction::updateOriginalBin(){
    auto& image_container = getImageContainer();

    getImageProcessor().threshold(image_container.origin_image,intensity_threshold_,bin_mat_,threshold_for_up_);
    getImageProcessor().threshold(image_container.gray_matrix,intensity_threshold_,bin_matrix_,threshold_for_up_);
    // setEigenRlt();
    displayImage(bin_mat_,0,0);
    displayImage(bin_matrix_,1,0);
}

void pageBinaryFunction::doFunction(int id){
    QString debug_info;
    auto& image_processer =getImageProcessor();
    if(id == 0){ // distance transform
        debug_info = image_processer.distanceTransform(bin_mat_,distance_type_,getResultCV());
        if(debug_info != "ok") debug(debug_info);
        debug_info=image_processer.distanceTransform(bin_matrix_,distance_type_,getResultEigen());
        if(debug_info != "ok") debug(debug_info);
    }
    else if(id == 1){ // skeleton
        debug_info = image_processer.skeleton(bin_mat_,distance_type_,getResultCV());
        if(debug_info != "ok") debug(debug_info);
        debug_info=image_processer.skeleton(bin_matrix_,distance_type_,getResultEigen());
        if(debug_info != "ok") debug(debug_info);
    }
    else if(id == 2){ // skeleton restoration
        cv::Mat restoration;
        debug_info = image_processer.skeletonRestoration(getResultCV(),distance_type_,restoration);
        getResultCV() = restoration.clone();
        if(debug_info != "ok") debug(debug_info);
        grayEigen restoration_eigen;
        debug_info=image_processer.skeletonRestoration(getResultEigen(),distance_type_,restoration_eigen);
        getResultEigen() = restoration_eigen;
        if(debug_info != "ok") debug(debug_info);

    }
    else{

    }

    updateOutputImage();
}
