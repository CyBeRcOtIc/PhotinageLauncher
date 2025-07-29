#include "qt/mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QPainterPath>
#include <QRegion>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <qt/titlebar.h>
#include <QWindowStateChangeEvent>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground, true);
    this->setStyleSheet("");
    this->setPlaceholders();
}

void MainWindow::changeEvent(QEvent *event){
    if (event->type() == QEvent::WindowStateChange) {
        if (this->isMinimized()) {
            m_wasMaximizedBeforeMinimize = (event->type() == QEvent::WindowStateChange &&
                                            static_cast<QWindowStateChangeEvent*>(event)->oldState() & Qt::WindowMaximized);
        } else if (event->spontaneous() && !this->isMinimized()) {
            if (m_wasMaximizedBeforeMinimize) {
                QTimer::singleShot(0, this, [this](){
                    this->showMaximized();
                });
            }
        }
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    if(this->isRounded){
        QPainterPath path;
        path.addRoundedRect(rect(), MainWindow::CornerRadius, MainWindow::CornerRadius);
        QRegion mask = QRegion(path.toFillPolygon().toPolygon());
        this->setMask(mask);
    }else{
        this->setMask(QRegion());
    }
}

void MainWindow::paintEvent(QPaintEvent *){
    if(this->isRounded){
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(Qt::NoPen);

        QRect r = rect().adjusted(1, 1, -1, -1);
        QPainterPath path;
        path.addRoundedRect(r, MainWindow::CornerRadius, MainWindow::CornerRadius);
        p.fillPath(path, QColor(0x081625));
    }
}

void MainWindow::setPlaceholders(){
    TitleBar* titleBar = new TitleBar(this, MainWindow::CornerRadius);
    titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleBar->setFixedHeight(titleBar->height());

    QWidget* placeholder = ui->titleBarPlaceholder;
    QBoxLayout* boxLayout = qobject_cast<QBoxLayout*>(placeholder->parentWidget()->layout());
    if(boxLayout){
        int index = boxLayout->indexOf(placeholder);
        boxLayout->removeWidget(placeholder);
        placeholder->deleteLater();
        boxLayout->insertWidget(index, titleBar);

        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect;
        shadow->setBlurRadius(20);
        shadow->setOffset(0, 4);
        shadow->setColor(QColor(0, 0, 0, 100));
        titleBar->setGraphicsEffect(shadow);
    }
}

void MainWindow::enableRoundedCorners(bool enable){
    this->isRounded = enable;
    if(this->isRounded){
        this->setStyleSheet("");
    }else{
        this->setStyleSheet("background-color: #081625");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
