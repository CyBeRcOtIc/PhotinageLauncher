#include "qt/titlebar.h"
#include "ui_titlebar.h"
#include "qt/mainwindow.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QPushButton>
#ifdef _WIN32
#include <windows.h>
#endif

TitleBar::TitleBar(QWidget *parent, const int cornerRadius)
    : QWidget(parent)
    , ui(new Ui::TitleBar)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setStyleSheet("");
    this->setMouseTracking(true);
    this->m_cornerRadius = cornerRadius + 4;
    this->setSlots();
}

void TitleBar::setSlots(){
    connect(this->ui->closeButton, &QPushButton::clicked, this, [this](){ this->window()->close(); });
    connect(this->ui->minimizeButton, &QPushButton::clicked, this, [this](){ this->window()->showMinimized(); });
    connect(this->ui->toggleMaximizeButton, &QPushButton::clicked, this, &TitleBar::toggleMaximizeButtonSlot);
}

TitleBar::~TitleBar()
{
    delete ui;
}

void TitleBar::toggleMaximizeButtonSlot(){
    auto w = dynamic_cast<MainWindow*>(this->window());
    if (!w) return;

    if(this->window()->isMaximized()) {
        w->enableRoundedCorners(true);
        w->showNormal();
        w->setGeometry(this->m_normalGeometry);
        this->ui->toggleMaximizeButton->setText("⛶");
    } else {
        w->enableRoundedCorners(false);
        this->m_normalGeometry = w->geometry();
        w->showMaximized();
        this->ui->toggleMaximizeButton->setText("❐");
    }
}

void TitleBar::paintEvent(QPaintEvent *event){
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QRect r = rect();
    QPainterPath path;
    path.moveTo(0, r.height());
    path.lineTo(0, this->m_cornerRadius);
    path.quadTo(0, 0, this->m_cornerRadius, 0);
    path.lineTo(r.width() - this->m_cornerRadius, 0);
    path.quadTo(r.width(), 0, r.width(), this->m_cornerRadius);
    path.lineTo(r.width(), r.height());
    path.closeSubpath();

    p.setPen(Qt::NoPen);
    p.fillPath(path, QColor(0x0e1f32));
}

void TitleBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if(!this->window()->isMaximized()){
#ifdef _WIN32
        QWidget* win = window();
        ReleaseCapture();
        SendMessage(HWND(win->winId()), WM_NCLBUTTONDOWN, HTCAPTION, 0);
#else
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - window()->frameGeometry().topLeft();
#endif
        }
        event->accept();
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent* event) {
#ifndef _WIN32
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        window()->move(event->globalPosition().toPoint() - m_dragPosition);
    }
#endif
        event->accept();
}

void TitleBar::mouseReleaseEvent(QMouseEvent* event) {
#ifndef _WIN32
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
    }
#endif
        event->accept();
}
