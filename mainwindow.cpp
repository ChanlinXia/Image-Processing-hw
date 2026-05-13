#include "mainwindow.h"
#include "pagehistogramandthreshold.h"
#include "pageimagefilter.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->resize(1000,1050);
    this->setWindowTitle("Digital Image Processing");
    std::vector<QString> tasks_lists={
        "Histogram and Threshold",          // pdf3
        "Conclution and Image Filters",     // pdf4
        "Binary Morphological Operation",   //pdf7
        "Binary Morphological Function",    // pdf7
        "Gray Morphological Operation",    // pdf11
        "Gray Morphological Function",    // pdf11
    };

    // auto page = ;
    ui->tabWidget_task_tabs->addTab(new PageHistogramAndThreshold(),"Histogram and Threshold");
    ui->tabWidget_task_tabs->addTab(new pageImageFilter(),"Image Filters");

}

MainWindow::~MainWindow()
{
    delete ui;
}

