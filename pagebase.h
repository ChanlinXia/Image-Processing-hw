#ifndef PAGEBASE_H
#define PAGEBASE_H

#include <QWidget>
#include <QFileDialog>
#include <opencv2/core.hpp>

#include "imageprocesser.h"

/*************************************************
*   用于存储图片
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
struct ImageContainer{
    cv::Mat origin_image;
    // cv::Mat gray_image;

    grayEigen gray_matrix;
    intensityEigen intensity_matrix;
    intensityEigen intensity_matrix_r;
    intensityEigen intensity_matrix_b;
    intensityEigen intensity_matrix_g;

    bool is_color;
    int rows_;
    int cols_;

    ImageContainer():
        is_color(false),
        rows_(0),
        cols_(0){};

    bool loadImage(const QString& path){
        origin_image=cv::imread(path.toStdString());

        if(origin_image.rows == 0) return false;

        if(origin_image.channels() == 3) is_color = true;
        else is_color = false;
        // cv::cvtColor(origin_image,gray_image,cv::COLOR_BGR2GRAY);
        rows_ = origin_image.rows;
        cols_ = origin_image.cols;
        intensity_matrix.resize(rows_,cols_);
        gray_matrix.resize(rows_,cols_);
        if(is_color){
            intensity_matrix_r.resize(rows_,cols_);
            intensity_matrix_g.resize(rows_,cols_);
            intensity_matrix_b.resize(rows_,cols_);
        }

        calcIntensity();
        return true;
    }

    void calcIntensity(){
        // if(intensity_matrix.)
        std::function<double(int,int)> calcPixelIntensity;  // 声明一个可调用对象
        if(is_color){ // three channel bgr default
            if(origin_image.type() == CV_8UC3){ // bgr8
                calcPixelIntensity = [this](int x,int y)->double{
                    cv::Vec3b pixel = origin_image.at<cv::Vec3b>(y, x);  // (行, 列)
                    return (0.114 * pixel[0] + 0.587 * pixel[1] + 0.299 * pixel[2] + 0.5);
                };

            }
            else if(origin_image.type() == CV_16UC3 ){ // bgr16
                calcPixelIntensity = [this](int x,int y)->double{
                    cv::Vec3w pixel = origin_image.at<cv::Vec3w>(y, x);  // (行, 列)
                    return (255 *
                            (0.114 * pixel[0] + 0.587 * pixel[1] + 0.299 * pixel[2] + 0.5)
                                                    / 65535.0+0.5);
                };
            }
            else{
                calcPixelIntensity = [this](int x,int y)->double{
                    cv::Vec3b pixel = origin_image.at<cv::Vec3b>(y, x);  // (行, 列)
                    return ((0.114 * pixel[0] + 0.587 * pixel[1] + 0.299 * pixel[2])*255+0.5);
                };
            }
        }
        else if(origin_image.channels() == 1){
            // calcPixelIntensity = [this](int x,int y)->double{
            //     // cv::Vec3b pixel = origin_image.at<cv::Vec3b>(y, x);  // (行, 列)
            //     return origin_image.at<uint8_t>(y,x);
            // };

            for(int i = 0;i<origin_image.rows;++i){
                for(int j=0;j<origin_image.cols;++j){
                    // auto intensity = calcPixelIntensity(j,i);
                    // intensity_matrix(i,j) = intensity;
                    gray_matrix(i,j) = origin_image.at<uint8_t>(i,j);
                }
            }
            return;
        }
        else{
            return;
        }

        for(int i = 0;i<origin_image.rows;++i){
            for(int j=0;j<origin_image.cols;++j){
                auto intensity = calcPixelIntensity(j,i);
                intensity_matrix(i,j) = intensity;
                gray_matrix(i,j) = static_cast<uint8_t> (intensity);

                if(is_color){
                    cv::Vec3b pixel = origin_image.at<cv::Vec3b>(i, j);

                    intensity_matrix_b(i,j) = pixel[0];
                    intensity_matrix_g(i,j) = pixel[1];
                    intensity_matrix_r(i,j) = pixel[2];
                }
            }
        }
    }
};

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
            // std::cout << "display gray image" << std::endl;
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

        cv::Mat rlt;
        cv::eigen2cv(mat,rlt);
        setImage(rlt);
        // generate the QImage
//        cv::eigen2cv(&mat,mat.rows(),mat.cols());
//        QImage img(static_cast<uchar*>(mat.data()),mat.cols(),mat.rows(),QImage::Format_ARGB32);

//        setImage(QPixmap::fromImage(img));
    }

    void setImage(const grayEigen& mat) {
        if (mat.rows() == 0 || mat.cols() == 0) return;

        cv::Mat rlt;
        cv::eigen2cv(mat,rlt);
        setImage(rlt);
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



class PageBase : public QWidget
{
    Q_OBJECT
public:
    explicit PageBase(QWidget *parent = nullptr,bool show_original_image =true);
    ~PageBase(){
        // delete [] &file_path_;
        // delete [] &vec_image_displayer_;
    }

    bool loadImage(const QString& path){
        bool ok = image_container_.loadImage(path);
        if(ok){
            int rows = image_container_.rows_;
            int cols = image_container_.cols_;
            result_cv_= cv::Mat::zeros(rows,cols,CV_8UC1);
            result_eigen_mat_=cv::Mat::zeros(rows,cols,CV_8UC1);
            // result_eigen_.resize(rows,cols);
            result_eigen_=grayEigen::Zero(rows,cols);
        }
        return ok;
        // return image_container_.loadImage(path);
    }

    cv::Mat& getResultCV(){
        return result_cv_;
    }


    void setEigenRlt(){
        cv::eigen2cv(result_eigen_,result_eigen_mat_);
    }

    grayEigen& getResultEigen(){
        // cv::imshow("get result eigen before set", result_eigen_mat_);
        // cv::waitKey(1);  // 非阻塞，允许刷新
        return result_eigen_;
    }

    QBoxLayout* getContainerLayout(){
        return container_layout_;
    }

    ImageContainer& getImageContainer(){
        return image_container_;
    }

    std::vector<ImageDisplayer>& getImageDisplayer(){
        return vec_image_displayer_;
    }

    ImageProcesser& getImageProcessor(){
        return image_processer_;
    }

    void debug(const QString& error_info){
        *debug_window << error_info;
    }

    void updateOutputImage(){
        cv::eigen2cv(result_eigen_,result_eigen_mat_);
        vec_image_displayer_[1].setImage(result_cv_);
        vec_image_displayer_[3].setImage(result_eigen_mat_);
    }

    template<typename T>
    void displayImage(const T& image,int row,int col){
        // std::cout << "setImage in pagebase" << std::endl;
        vec_image_displayer_[2*row+col].setImage(image);
    }

    bool show_original_iamge_;
signals:
    void loadAImage(const QString& file_path);

private:
    ImageContainer image_container_{};

    cv::Mat result_cv_;
    cv::Mat result_eigen_mat_;
    grayEigen result_eigen_;

    std::vector<ImageDisplayer> vec_image_displayer_;
    QFileDialog* file_loader_;
    QLabel* lable_file_path_;
    QGridLayout* image_display_layout_;

    QBoxLayout* container_layout_;

    ErrorWindow* debug_window;
    QString file_path_;

    ImageProcesser image_processer_;

};

#endif // PAGEBASE_H
