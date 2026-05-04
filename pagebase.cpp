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
    container_layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
    center_layout->addLayout(container_layout);

    // error display
    debug_window = new ErrorWindow(this);

    connect(button_open_file,&QPushButton::clicked,[&](){
        QString file_path = file_loader_->getOpenFileName(this,"select an image",QDir::homePath(),tr("所有文件 (*.*);;图片文件(*.png,*.jpg)"));
        qDebug() << "the selected path:" << file_path << Qt::endl;
        if(file_path.isEmpty()){ // 报错
            *debug_window << "the path is empty";
            return;
        }
        static QRegularExpression regex(R"(\.(png|jpg|jpeg|bmp|gif|tiff|webp)$)",
                                            QRegularExpression::CaseInsensitiveOption);
        if(!regex.match(file_path).hasMatch()){
            *debug_window << "the path is empty";
            return;
        }

        // [TODO] read the file and display
        lable_file_path_->setText(file_path);

        QPixmap pixmap(file_path);
        if (pixmap.isNull()) {
            *debug_window << "图片加载失败" ;
            return;
        }
        vec_image_displayer_[0].setImage(QPixmap(file_path));
        vec_image_displayer_[2].setImage(QPixmap(file_path));

        // 2. 创建场景
//        QGraphicsScene* scene = new QGraphicsScene(graphicsView);

//        // 3. 添加图片到场景
//        QGraphicsPixmapItem* item = scene->addPixmap(pixmap);

//        // 4. 设置场景到视图
//        graphicsView->setScene(scene);

        // 5. 可选：自动调整视图大小以适应图片
    });
}
