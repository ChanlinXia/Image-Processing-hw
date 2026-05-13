#ifndef PAGEBASE_H
#define PAGEBASE_H

#include <QWidget>
#include <QFileDialog>
#include <opencv2/core.hpp>

#include "sys.h"

/*************************************************
*   报警窗口
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
struct ErrorWindow : QWidget
{
    ErrorWindow(QWidget* parent = nullptr): QWidget(parent) {
        setWindowTitle("Erro");
        setFixedWidth(450);
        setFixedHeight(270);

        setWindowFlags(Qt::Window);
        QVBoxLayout* center_layout = new QVBoxLayout(this);
        error_info_ = new QLabel("error");
        error_info_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        error_info_->setWordWrap(true);

        QPushButton* closeBtn = new QPushButton("I see", this);

        center_layout->addWidget(error_info_);
        center_layout->addWidget(closeBtn);
        connect(closeBtn,&QPushButton::clicked,[&](){
            close();
        });
    }

    ErrorWindow& operator <<(const QString& str){
        if(error_info_) error_info_->setText(str);
        this->show();
        raise();
        activateWindow();
        qDebug() << str << Qt::endl;
        return *this;
    }
private:
    QLabel* error_info_;
//    std::shared_ptr<QTextEdit> text_edit_;
};

/*************************************************
*   用于展示图片
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
struct ImageDisplayer:public QObject{
    ImageDisplayer() {
        scene_ = std::make_unique<QGraphicsScene>();
        view_ = std::make_unique<QGraphicsView>();
        view_->setScene(scene_.get());
        view_->setInteractive(false);        // 禁止与场景中的项交互
        view_->setCursor(Qt::ArrowCursor);
//        scene_->setCursor(Qt::ArrowCursor);
        view_->setAttribute(Qt::WA_TransparentForMouseEvents);
        view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        view_->installEventFilter(this);

        // 设置一些默认属性
        view_->setDragMode(QGraphicsView::ScrollHandDrag);
        view_->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    }

    void setImage(const QPixmap& pixmap){
        scene_->clear();
        scene_->addPixmap(pixmap);
        scene_->setSceneRect(pixmap.rect());
        view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
    }

    void setImage(const cv::Mat& mat) {
        if (mat.empty()) return;

        cv::Mat displayMat;
        QImage::Format format;

        if (mat.channels() == 1) {
            // 灰度图
            format = QImage::Format_Grayscale8;
            displayMat = mat;
        } else if (mat.channels() == 3) {
            // BGR 转 RGB
            cv::cvtColor(mat, displayMat, cv::COLOR_BGR2RGB);
            format = QImage::Format_RGB888;
        } else {
            return;  // 不支持的格式
        }

        QImage img(displayMat.data, displayMat.cols, displayMat.rows,
                   displayMat.step, format);

        // 必须拷贝，因为 displayMat 会在函数结束时销毁
        QPixmap pixmap = QPixmap::fromImage(img.copy());
        setImage(pixmap);
    }

    void setImage(const Eigen::MatrixXd& mat) {
        if (mat.rows() == 0 || mat.cols() == 0) return;

        // generate the QImage
//        cv::eigen2cv(&mat,mat.rows(),mat.cols());
//        QImage img(static_cast<uchar*>(mat.data()),mat.cols(),mat.rows(),QImage::Format_ARGB32);

//        setImage(QPixmap::fromImage(img));
    }

    bool eventFilter(QObject* obj, QEvent* event) override {
            if (obj == view_.get() && event->type() == QEvent::Resize) {
                QTimer::singleShot(10, this, [this]() {
                    view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
                            });
            }
            return false;
    }

    std::unique_ptr<QGraphicsView> view_;
    std::unique_ptr<QGraphicsScene> scene_;
};

/*************************************************
*   用于存储图片
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
struct ImageContainer{
    cv::Mat origin_image;
    Eigen::Matrix<uchar,Eigen::Dynamic,Eigen::Dynamic> intensity_matrix;
    Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic> r_channel_matrix;
    Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic> b_channel_matrix;
    Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic> g_channel_matrix;
    bool is_color;
    int rows_;
    int cols_;

    ImageContainer():
        is_color(false),
        rows_(0),
        cols_(0){};

    bool loadImage(QString& path){
        origin_image=cv::imread(path.toStdString());

        if(origin_image.rows == 0) return false;

        if(origin_image.depth() == 3) is_color = true;
        else is_color = false;

        rows_ = origin_image.rows;
        cols_ = origin_image.cols;
        if(is_color){
            intensity_matrix.resize(rows_,cols_);
            r_channel_matrix.resize(rows_,cols_);
            b_channel_matrix.resize(rows_,cols_);
            g_channel_matrix.resize(rows_,cols_);
        }
        else{
            intensity_matrix.resize(rows_,cols_);
        }

        calcIntensity();
        return true;
    }


    void calcIntensity(){
        // if(intensity_matrix.)
        std::function<uint8_t(int,int)> calcPixelIntensity;  // 声明一个可调用对象
        if(is_color){ // three channel bgr default
            if(origin_image.type() == CV_8UC3){ // bgr8
                calcPixelIntensity = [this](int x,int y)->uint8_t{
                    cv::Vec3b pixel = origin_image.at<cv::Vec3b>(y, x);  // (行, 列)
                    return static_cast<uint8_t>(0.114 * pixel[0] + 0.587 * pixel[1] + 0.299 * pixel[2] + 0.5);
                };
            }
            else if(origin_image.type() == CV_16UC3 ){ // bgr16
                calcPixelIntensity = [this](int x,int y)->uint8_t{
                    cv::Vec3b pixel = origin_image.at<cv::Vec3b>(y, x);  // (行, 列)
                    return static_cast<uint8_t>(255 *
                                                    (0.114 * pixel[0] + 0.587 * pixel[1] + 0.299 * pixel[2] + 0.5)
                                                    / 65535.0+0.5);
                };
            }
            else{
                calcPixelIntensity = [this](int x,int y)->uint8_t{
                    cv::Vec3b pixel = origin_image.at<cv::Vec3b>(y, x);  // (行, 列)
                    return static_cast<uint8_t>((0.114 * pixel[0] + 0.587 * pixel[1] + 0.299 * pixel[2])*255+0.5);
                };
            }
        }
        else if(origin_image.channels() == 1){
            calcPixelIntensity = [this](int x,int y)->uint8_t{
                // cv::Vec3b pixel = origin_image.at<cv::Vec3b>(y, x);  // (行, 列)
                return origin_image.at<uint8_t>(y,x);
            };
        }
        else{
            return;
        }

        for(int i = 0;i<origin_image.rows;++i){
            for(int j=0;j<origin_image.cols;++j){
                auto brightness = calcPixelIntensity(j,i);
                intensity_matrix(i,j) = brightness;
            }
        }

        if(is_color){
            for(int i = 0;i<origin_image.rows;++i){
                for(int j=0;j<origin_image.cols;++j){
                    cv::Vec3b pixel = origin_image.at<cv::Vec3b>(i, j);
                    // auto brightness = calcPixelIntensity(j,i);
                    b_channel_matrix(i,j) = pixel[0];
                    g_channel_matrix(i,j) = pixel[1];
                    r_channel_matrix(i,j) = pixel[2];
                }
            }
        }

    }

};




class PageBase : public QWidget
{
    Q_OBJECT
public:
    explicit PageBase(QWidget *parent = nullptr);
    ~PageBase(){
        // delete [] &file_path_;
        // delete [] &vec_image_displayer_;
    }

    QBoxLayout* getContainerLayout(){
        return container_layout_;
    }

    std::vector<ImageDisplayer>& getImageDisplayer(){
        return vec_image_displayer_;
    }
signals:
    void loadAImage(const QString& file_path);

private:
    std::vector<ImageDisplayer> vec_image_displayer_;
    QFileDialog* file_loader_;
    QLabel* lable_file_path_;
    QGridLayout* image_display_layout_;

    QBoxLayout* container_layout_;

    ErrorWindow* debug_window;
    QString file_path_;

};

#endif // PAGEBASE_H
