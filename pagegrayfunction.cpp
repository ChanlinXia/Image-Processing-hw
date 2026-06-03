#include "pagegrayfunction.h"

pageGrayFunction::pageGrayFunction(bool show_origin_image):
    PageBase(nullptr,show_origin_image)
{
    setContainer();
}

void pageGrayFunction::setContainer() {
    auto container_layout = getContainerLayout();

    // button widget
    QGridLayout* setting_layout = new QGridLayout();
    std::vector<QString> button_labels = {
        "Edge Detection","Reconstruction","Gradient",
    };

    QButtonGroup* operation_group = new QButtonGroup(this);

    // 将前3个按钮放入第一行前三列
    for (int i = 0; i < 3; ++i) {
        QPushButton* btn = new QPushButton(button_labels[i]);
        setting_layout->addWidget(btn, 0, i);  // 第0行，第i列
        operation_group->addButton(btn, i);  // 设置ID为i
    }

    // 设置拉伸比例
    setting_layout->setColumnStretch(0, 1);
    setting_layout->setColumnStretch(1, 1);
    setting_layout->setColumnStretch(2, 1);

    container_layout->addLayout(setting_layout);

    connect(operation_group, QOverload<int>::of(&QButtonGroup::buttonClicked),
            [this](int id) {
                // operation_type_= static_cast<ImageProcesser::MORPHOLOGICAL_OPERATION_TYPE>(id);
                doFunction(id);
                // updateOutput();
            });

    connect(this,&PageBase::loadAImage,[this](const QString& path){
        getGrayImage();
    });

}


void pageGrayFunction::getGrayImage() {
    auto& image_container = getImageContainer();

    // 灰度转换
    if(image_container.origin_image.channels() == 1) gray_mat_ = image_container.origin_image;
    else cv::cvtColor(image_container.origin_image,gray_mat_,cv::COLOR_BGR2GRAY);

    gray_matrix_ = std::move(image_container.gray_matrix);

    displayImage(gray_mat_,0,0);
    displayImage(gray_matrix_,1,0);
}

void pageGrayFunction::doFunction(int id){
    auto& image_processer = getImageProcessor();

    if(id == 0){ // edge
        image_processer.morphologicalEdgeDetection(gray_mat_,getResultCV());
        image_processer.morphologicalEdgeDetection(gray_matrix_,getResultEigen());
    }
    else if(id == 1){ // reconstruction
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9,9));
        cv::Mat opened;
        cv::morphologyEx(gray_mat_, opened, cv::MORPH_OPEN, kernel);
        cv::Mat marker = opened;
        displayImage(marker,0,0);

        image_processer.morphologicalReconstruction(marker,gray_mat_,getResultCV());
        grayEigen marker_eigen;
        marker_eigen.resize(marker.rows,marker.cols);
        cv::cv2eigen(marker,marker_eigen);
        image_processer.morphologicalReconstruction(marker_eigen,gray_matrix_,getResultEigen());

    }
    else if(id == 2){ // gradient
        image_processer.morphologicalGradient(gray_mat_,getResultCV());
        image_processer.morphologicalGradient(gray_matrix_,getResultEigen());

    }
    updateOutputImage();
}


