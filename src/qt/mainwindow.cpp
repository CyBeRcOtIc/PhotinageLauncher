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
#include <QButtonGroup>
#include "io/styleloader.h"
#include "qt/ccombobox.h"
#include <QVector>
#include "qt/versioncard.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TranslucentBackground, true);
    this->setStyleSheet("");
    this->setPlaceholders();
    this->setSideBarButtons();
    this->setComboBoxes();
    this->setPlayMenu();
}

void MainWindow::setComboBoxes(){
    QVector<QComboBox*> boxes;
    boxes.append(this->ui->versionComboBox);
    boxes.append(this->ui->forgeComboBox);
    boxes.append(this->ui->optifineComboBox);
    QVector<QBoxLayout*> lays;
    lays.append(qobject_cast<QBoxLayout*>(this->ui->versionSelectorWidget->layout()));
    lays.append(qobject_cast<QBoxLayout*>(this->ui->forgeSelectorWidget->layout()));
    lays.append(qobject_cast<QBoxLayout*>(this->ui->optifineSelectorWidget->layout()));

    for(int a = 0; a < 3; a++){
        CComboBox* cm = new CComboBox(this);
        QBoxLayout* lay = lays[a];
        int index = lay->indexOf(boxes[a]);
        lay->removeWidget(boxes[a]);
        lay->insertWidget(index, cm);
        cm->setStyleSheet(boxes[a]->styleSheet());
        cm->setFont(boxes[a]->font());
        boxes[a]->deleteLater();
    }
}

void MainWindow::setPlayMenu()
{
    QVBoxLayout* adLayout = qobject_cast<QVBoxLayout*>(this->ui->advertisementWidget->layout());
    if(!adLayout) return;

    QLabel* noAdLabel = new QLabel(this->ui->advertisementWidget);
    noAdLabel->setTextFormat(Qt::RichText);
    noAdLabel->setText("<div>"
                       "<span style=\"font-size: 20pt; color: #507aa5;\">Этот блок ждёт ваш сервер!</span><br><br>"
                       "<span style=\"font-size: 28pt; font-weight: bold; color: #6c9cc9;\">:)</span>"
                       "</div>");
    noAdLabel->setStyleSheet("font-size: 24px; color: white;");
    noAdLabel->setAlignment(Qt::AlignCenter);
    adLayout->addWidget(noAdLabel, Qt::AlignCenter);

    QWidget* navWidget = new QWidget(this->ui->advertisementWidget);
    QHBoxLayout* navLayout = new QHBoxLayout(navWidget);
    navLayout->setContentsMargins(0, 0, 20, 0);
    navLayout->setSpacing(14);
    navWidget->setLayout(navLayout);
    navWidget->setMinimumHeight(50);
    navWidget->setMaximumHeight(50);
    adLayout->addWidget(navWidget, 0, Qt::AlignBottom);

    QString style = "font-size: 18px; color: white;";
    QLabel* patchNotesLabel = new QLabel(navWidget);
    patchNotesLabel->setText("Patch Notes");
    patchNotesLabel->setStyleSheet(style);
    patchNotesLabel->setAlignment(Qt::AlignCenter);

    QLabel* supportLabel = new QLabel(navWidget);
    supportLabel->setText("Support");
    supportLabel->setStyleSheet(style);
    supportLabel->setAlignment(Qt::AlignCenter);

    QLabel* faqLabel = new QLabel(navWidget);
    faqLabel->setText("FAQ");
    faqLabel->setStyleSheet(style);
    faqLabel->setAlignment(Qt::AlignCenter);

    navLayout->addStretch();
    navLayout->addWidget(patchNotesLabel);
    navLayout->addWidget(supportLabel);
    navLayout->addWidget(faqLabel);

    QVBoxLayout* scrollAreaLayout = qobject_cast<QVBoxLayout*>(this->ui->scrollContentWidget->layout());
    if(!scrollAreaLayout) return;

    this->versionCard = new VersionCard(this->ui->scrollContentWidget);
    scrollAreaLayout->addWidget(this->versionCard);
    QTimer::singleShot(0, this, [this](){
        this->versionCard->setVersionData(
            "<div>"
            "<span style=\"font-size: 14pt;\">LATEST RELEASE</span><br>"
            "<span style=\"font-size: 20pt; font-weight: bold;\">1.19.4</span>"
            "</div>",
            ":/assets/version_1.12.2.png",
            "New release!\n-New Mob\n-New Item"
            );
    });
}

void MainWindow::setSideBarButtons(){
    this->ui->playSideBarButton->setCheckable(true);
    this->ui->installationsSideBarButton->setCheckable(true);

    QButtonGroup* menuGroup = new QButtonGroup(this);
    menuGroup->addButton(this->ui->playSideBarButton, 0);
    menuGroup->addButton(this->ui->installationsSideBarButton, 1);
    menuGroup->setExclusive(true);
    this->ui->playSideBarButton->setChecked(true);

    IO::StyleLoader::applyStyle(this->ui->playSideBarButton,":/styles/side_bar_button_style.qss");
    IO::StyleLoader::applyStyle(this->ui->installationsSideBarButton,":/styles/side_bar_button_style.qss");

    connect(menuGroup, &QButtonGroup::buttonToggled,
            this, [=](QAbstractButton* btn, bool checked) {
                if(checked){
                    this->ui->stackedWidget->setCurrentIndex(menuGroup->id(btn));
                }
            });
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
