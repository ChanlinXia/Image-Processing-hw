#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    std::vector<QString> tasks_lists={
        "Histogram and Threshold",          // pdf3
        "Conclution and Image Filters",     // pdf4
        "Binary Morphological Operation",   //pdf7
        "Binary Morphological Function",    // pdf7
        "Gray Morphological Operation",    // pdf11
        "Gray Morphological Function",    // pdf11
    };

    for(auto& task:tasks_lists){
        ui->tabWidget_task_tabs->addTab(nullptr,task);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

