#ifndef PAGEBASE_H
#define PAGEBASE_H

#include <QWidget>
#include <QFileDialog>
#include <opencv2/core.hpp>

#include "sys.h"

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
        view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
    }

    void setImage(const cv::Mat& mat) {
        if (mat.empty()) return;

        // generate the QImage
        QImage img(mat.cols,mat.rows,QImage::Format_ARGB32);

        setImage(QPixmap::fromImage(img));
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
