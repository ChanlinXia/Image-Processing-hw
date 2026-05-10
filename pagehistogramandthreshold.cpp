#include "pagehistogramandthreshold.h"

PageHistogramAndThreshold::PageHistogramAndThreshold(PageBase *parent)
    : PageBase{parent}
{
    setContainer();
}


void PageHistogramAndThreshold::setContainer(){
    auto container_layout = getContainerLayout();
    container_layout->setDirection(QBoxLayout::Direction::TopToBottom);

    chart = new QChart();
    QChartView *chartView = new QChartView(chart);

    chartView->setRenderHint(QPainter::Antialiasing); // 抗锯齿
    container_layout->addWidget(chartView);

    // 设定值
    QHBoxLayout* val_set_layout = new QHBoxLayout();

    QSlider* slider = new QSlider(Qt::Horizontal);
    slider->setMaximum(100);
    slider->setMinimum(0);
    val_set_layout->addWidget(slider,4);

    QLineEdit* line_edit_threshold = new QLineEdit(0);
    val_set_layout->addWidget(line_edit_threshold,1);
    container_layout->addLayout(val_set_layout);




}

void PageHistogramAndThreshold::update(){

}
