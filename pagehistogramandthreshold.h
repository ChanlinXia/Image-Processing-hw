#ifndef PAGEHISTOGRAMANDTHRESHOLD_H
#define PAGEHISTOGRAMANDTHRESHOLD_H

#include "pagebase.h"

class PageHistogramAndThreshold : public PageBase
{
public:
    explicit PageHistogramAndThreshold(PageBase *parent = nullptr);

signals:

private:

    void setContainer();
    void updateChart();
    void calcIntensity();
    QChartView* initTheChart();
    void updateThresholdLine();
    void updateMask();
    void updateOutputImage();

    QChart* chart;
    cv::Mat image_mat_;
    Eigen::Matrix<uint8_t,Eigen::Dynamic,Eigen::Dynamic> image_matrix_;
    Eigen::Matrix<uint8_t,Eigen::Dynamic,Eigen::Dynamic> image_matrix_mask_;

    uint8_t intensity_threshold_ = 127;
    std::array<uint8_t, 256> gray_histogram_{};
    QBarSet* above_threshold_set_;
    QBarSet* below_threshold_set_;

    QLineSeries* thresholdLineSeries;

    bool threshold_for_up_ = false;
    // std::unique_ptr<uint8_t[]> gray_histogram_;
    // uint8_t* gray_histogram_;

};

#endif // PAGEHISTOGRAMANDTHRESHOLD_H
