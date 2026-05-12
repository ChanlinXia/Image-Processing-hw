#include "pagehistogramandthreshold.h"

PageHistogramAndThreshold::PageHistogramAndThreshold(PageBase *parent)
    : PageBase{parent}
{
    setContainer();

    // gray_histogram_ = std::make_unique<uint8_t []>(new uint8_t(256));
}


void PageHistogramAndThreshold::setContainer(){
    auto container_layout = getContainerLayout();
    container_layout->setDirection(QBoxLayout::Direction::TopToBottom);

    // init the chart
    container_layout->addWidget(initTheChart());

    // set the threshold
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
        // updateChart();
        updateMask();
        updateOutputImage();
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
            updateThresholdLine();
            updateMask();
            updateOutputImage();
    });

    // PageBase *parentWidget = qobject_cast<PageBase*>(this->parent());
    connect(this,&PageBase::loadAImage,[this](const QString& path){
        image_mat_ = cv::imread(path.toStdString());

        int rows = image_mat_.rows;
        int cols = image_mat_.cols;

        image_matrix_.resize(rows, cols);
        image_matrix_mask_.resize(rows,cols);
        calcIntensity();
        updateChart();
        updateOutputImage();
        // cv::cv2eigen(image_mat_, image_matrix_);
        // qDebug() << "cv to eigen"<<  Qt::endl;
        // qDebug() << "image size"<< rows <<"*" <<cols << Qt::endl;
        // qDebug() << "Eigen size"<< image_matrix_.size() << Qt::endl;
    });

}

/*************************************************
*   初始化Chart
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
QChartView* PageHistogramAndThreshold::initTheChart(){
    chart = new QChart();

    // 1. 创建数据集
    barSet = new QBarSet(QStringLiteral("intensity"));
    // QBarSet *set2 = new QBarSet("类别 B");
    for (int i = 0; i < 256; i++) {
        *barSet << 0;
    }

    // 2. 创建柱状图系列并添加数据集
    QBarSeries* series = new QBarSeries();
    series->append(barSet);

    // 3. 创建图表并添加系列
    chart = new QChart();
    chart->addSeries(series);
    // chart->setTitle("柱状图示例");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // 4. 创建x轴
    qDebug() <<"init axisX" << Qt::endl;
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    QStringList categories;
    for (int i = 0; i < 256; i++) {
        categories.append(QString::number(i));
    }
    axisX->append(categories);

    axisX->setTitleText(QStringLiteral("intensity"));
    axisX->setLabelsVisible(false);  // 隐藏x轴坐标值
    axisX->setGridLineVisible(false);  // 隐藏 X 轴主网格线

    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    qDebug() <<"init axisY" << Qt::endl;

    // 5. 创建Y轴
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 100);
    axisY->setLabelFormat("%d");        // 显示整数
    axisY->setTitleText("count");
    axisY->setGridLineVisible(true);
    axisY->setGridLineVisible(false);  // 隐藏 Y 轴主网格线

    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    chart->legend()->hide();

    double yMin = axisY ? axisY->min() : 0;
    double yMax = axisY ? axisY->max() : 100;

    // 6.画一条从 (thresholdValue, yMin) 到 (thresholdValue, yMax) 的垂直线
    thresholdLineSeries = new QLineSeries();
    thresholdLineSeries->append(intensity_threshold_, yMin);
    thresholdLineSeries->append(intensity_threshold_, yMax);

    // 样式
    QPen pen(Qt::red);
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);  // 虚线
    thresholdLineSeries->setPen(pen);
    chart->addSeries(thresholdLineSeries);

    thresholdLineSeries->attachAxis(chart->axes(Qt::Horizontal).first());
    thresholdLineSeries->attachAxis(chart->axes(Qt::Vertical).first());
    // 7. 显示
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing); // 抗锯齿
    return chartView;
}

/*************************************************
*   更新Chart数据和图像显示
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void PageHistogramAndThreshold::updateChart(){
    // refresh the histogram
    // barSet->remove(0,barSet->count());
    int maxCount = 0;
    for(int i =0;i<256;++i){
        if(gray_histogram_[i] > maxCount) maxCount = gray_histogram_[i];
        barSet->replace(i, gray_histogram_[i]);
    }

    QValueAxis* axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    if (axisY) {
        axisY->setRange(0, maxCount * 1.1);
    }

    updateThresholdLine();
}

/*************************************************
*   更新threshold line
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void PageHistogramAndThreshold::updateThresholdLine(){
    QValueAxis* axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    if(!axisY) return;
    double yMin =  axisY->min();
    double yMax =  axisY->max();

    QVector<QPointF> newPoints;
    newPoints.append(QPointF(intensity_threshold_, yMin));
    newPoints.append(QPointF(intensity_threshold_, yMax));

    // redraw the line
    thresholdLineSeries->replace(newPoints);
}

/*************************************************
*   计算EigenMatrix中的亮度并进行记录，如果是三通道，则使用BT.601进行计算
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void PageHistogramAndThreshold::calcIntensity(){
    // if(image_matrix_.)
    std::function<uint8_t(int,int)> calcPixel;  // 声明一个可调用对象
    if(image_mat_.channels() == 3){ // three channel bgr default
        if(image_mat_.type() == CV_8UC3){ // bgr8
            calcPixel = [this](int x,int y)->uint8_t{
                cv::Vec3b pixel = image_mat_.at<cv::Vec3b>(y, x);  // (行, 列)
                return static_cast<uint8_t>(0.114 * pixel[0] + 0.587 * pixel[1] + 0.299 * pixel[2] + 0.5);
            };
        }
        else if(image_mat_.type() == CV_16UC3 ){ // bgr16
            calcPixel = [this](int x,int y)->uint8_t{
                cv::Vec3b pixel = image_mat_.at<cv::Vec3b>(y, x);  // (行, 列)
                return static_cast<uint8_t>(255 *
                                            (0.114 * pixel[0] + 0.587 * pixel[1] + 0.299 * pixel[2] + 0.5)
                                            / 65535.0+0.5);
            };
        }
        else{
            calcPixel = [this](int x,int y)->uint8_t{
                cv::Vec3b pixel = image_mat_.at<cv::Vec3b>(y, x);  // (行, 列)
                return static_cast<uint8_t>((0.114 * pixel[0] + 0.587 * pixel[1] + 0.299 * pixel[2])*255+0.5);
            };
        }
    }
    else if(image_mat_.channels() == 1){
        calcPixel = [this](int x,int y)->uint8_t{
            // cv::Vec3b pixel = image_mat_.at<cv::Vec3b>(y, x);  // (行, 列)
            return image_mat_.at<uint8_t>(y,x);
        };
    }
    else{
        return;
    }

    for(int i = 0;i<image_mat_.rows;++i){
        for(int j=0;j<image_mat_.cols;++j){
            auto brightness = calcPixel(j,i);
            image_matrix_(i,j) = brightness;
            if(threshold_for_up_){
                if(brightness < intensity_threshold_)image_matrix_mask_(i,j) = 1;
                else image_matrix_mask_(i,j) = 0;
            }
            else{
                if(brightness < intensity_threshold_)image_matrix_mask_(i,j) = 0;
                else image_matrix_mask_(i,j) = 1;
            }
            ++gray_histogram_[brightness];
        }
    }

}

/*************************************************
*   更新mask
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void PageHistogramAndThreshold::updateMask(){
    for(int i = 0;i<image_mat_.rows;++i){
        for(int j=0;j<image_mat_.cols;++j){
            auto brightness = image_matrix_(i,j);
            if(threshold_for_up_){
                if(brightness < intensity_threshold_)image_matrix_mask_(i,j) = 1;
                else image_matrix_mask_(i,j) = 0;
            }
            else{
                if(brightness < intensity_threshold_)image_matrix_mask_(i,j) = 0;
                else image_matrix_mask_(i,j) = 1;
            }
            ++gray_histogram_[brightness];
        }
    }
}

/*************************************************
*   更新处理后的图像
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void PageHistogramAndThreshold::updateOutputImage(){
    if(image_mat_.rows == 0 || image_matrix_.rows()==0) return ;
    // 更新右上角的，通过cv pipeline
        // qDebug() << "start to gray" << Qt::endl;
    cv::Mat gray_image;
    cv::cvtColor(image_mat_,gray_image,cv::COLOR_BGR2GRAY);

        // qDebug() << "start to hist" << Qt::endl;
    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange = {range};
    cv::Mat hist;
    cv::calcHist(&gray_image, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

        // qDebug() << "start to mask" << Qt::endl;
    cv::Mat mask;
    if(threshold_for_up_) cv::threshold(gray_image, mask, intensity_threshold_, 255, cv::THRESH_TOZERO_INV);
    else cv::threshold(gray_image, mask, intensity_threshold_, 255, cv::THRESH_TOZERO);

        // qDebug() << "start to result" << Qt::endl;
    cv::Mat result;
    result = cv::Mat::zeros(image_mat_.size(), image_mat_.type());
    image_mat_.copyTo(result, mask);

    // qDebug() << "start to display" << Qt::endl;
    auto&  image_displayer = getImageDisplayer();
    auto& top_right = image_displayer[1];
    top_right.setImage(result);

    // 更新右下角的
    cv::Mat origin;
    image_mat_.copyTo(origin);

    if (origin.type() == CV_8UC3) {
        // 彩色图：3通道
        for (int i = 0; i < image_matrix_.rows(); ++i) {
            for (int j = 0; j < image_matrix_.cols(); ++j) {
                if (!image_matrix_mask_(i, j)) {
                    // 掩码为 false 的位置设为黑色
                    origin.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 0);
                }
                // 掩码为 true 的位置保持原样（不用处理）
            }
        }
    }
    else if (origin.type() == CV_8UC1) {
        // 灰度图：1通道
        for (int i = 0; i < image_matrix_.rows(); ++i) {
            for (int j = 0; j < image_matrix_.cols(); ++j) {
                if (!image_matrix_mask_(i, j)) {
                    origin.at<uchar>(i, j) = 0;
                }
            }
        }
    }

    auto& buttom_right = image_displayer[3];
    buttom_right.setImage(origin);
}


