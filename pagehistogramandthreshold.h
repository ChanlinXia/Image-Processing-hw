#ifndef PAGEHISTOGRAMANDTHRESHOLD_H
#define PAGEHISTOGRAMANDTHRESHOLD_H

#include "sys.h"

class PageHistogramAndThreshold : public PageBase
{
public:
    explicit PageHistogramAndThreshold(PageBase *parent = nullptr);

signals:

private:

    void setContainer();
    void update();

    QChart* chart;
};

#endif // PAGEHISTOGRAMANDTHRESHOLD_H
