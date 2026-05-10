#include "pagebase.h"

PageBase::PageBase(QWidget *parent)
    : QWidget{parent}
    , vec_image_displayer_(4)
{
    // center layout
    QVBoxLayout* center_layout = new QVBoxLayout(this);

    // file loader
    file_loader_ = new QFileDialog(this);
    QHBoxLayout* file_loader_layout = new QHBoxLayout();
    lable_file_path_ = new QLabel("C:/",this);
    QPushButton* button_open_file = new QPushButton("open");
    file_loader_layout->addWidget(lable_file_path_,2,Qt::AlignJustify);
    file_loader_layout->addWidget(button_open_file,1,Qt::AlignJustify);
    center_layout->addLayout(file_loader_layout);

    // image display
    image_display_layout_ = new QGridLayout();
    center_layout->addLayout(image_display_layout_);
    for(int i=0;i<4;++i)     image_display_layout_->addWidget(vec_image_displayer_[i].view_.get(),i/2,i%2);

    // container layout
    container_layout_ = new QBoxLayout(QBoxLayout::Direction::TopToBottom);
    center_layout->addLayout(container_layout_);

    // error display
    debug_window = new ErrorWindow(this);

    connect(button_open_file,&QPushButton::clicked,[&](){
        file_path_ = file_loader_->getOpenFileName(this,"select an image",QDir::homePath(),tr("所有文件 (*.*);;图片文件(*.png,*.jpg)"));
        qDebug() << "the selected path:" << file_path_ << Qt::endl;
        if(file_path_.isEmpty()){ // 报错
            *debug_window << "the path is empty";
            return;
        }
        static QRegularExpression regex(R"(\.(png|jpg|jpeg|bmp|gif|tiff|webp)$)",
                                            QRegularExpression::CaseInsensitiveOption);
        if(!regex.match(file_path_).hasMatch()){
            *debug_window << "the path is empty";
            return;
        }

        // [TODO] read the file and display
        lable_file_path_->setText(file_path_);

        QPixmap pixmap(file_path_);
        if (pixmap.isNull()) {
            *debug_window << "图片加载失败" ;
            return;
        }

        // 加载图片
        vec_image_displayer_[0].setImage(QPixmap(file_path_));
        vec_image_displayer_[2].setImage(QPixmap(file_path_));

        // 发射信号
        emit loadAImage(file_path_);
    });
}
