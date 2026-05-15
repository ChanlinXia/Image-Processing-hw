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



    QPushButton* btn1 = new QPushButton("Roberts");
    QPushButton* btn2 = new QPushButton("Prewitt");
    QPushButton* btn3 = new QPushButton("Sobel");
    QPushButton* btn4 = new QPushButton("Guassian");
    QPushButton* btn5 = new QPushButton("Median");

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

    ImageProcesser& processer = getImageProcessor();
    // 建立连接
    connect(this,&PageBase::loadAImage,[&processer](const QString& path){
        processer.loadImage(path);

    });

    connect(btn1,&QPushButton::clicked,[this,&processer,kernel_size_edit,direction_combo](){
        bool ok = false;
        int value = kernel_size_edit->text().toInt(&ok);
        if(!ok)  return;
        // qDebug() << value << Qt::endl;
        // CONVOL_DIR dir = direction_combo->currentData().value<CONVOL_DIR>();

        processer.convolution(KERNEL_TYPE::Roberts,value,static_cast<CONVOL_DIR>(direction_combo->currentData().toInt()));
        updateOutputImage();
    });
}
