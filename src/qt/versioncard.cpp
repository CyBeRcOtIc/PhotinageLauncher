#include "qt/versioncard.h"
#include "io/styleloader.h"
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

VersionCard::VersionCard(QWidget *parent) : QWidget(parent) {
    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->setContentsMargins(10, 10, 10, 10);
    lay->setSpacing(10);
    this->setLayout(lay);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //this->setStyleSheet("background-color: red;");

    this->imageLabel = new QLabel(this);
    this->imageLabel->setAlignment(Qt::AlignCenter);
    lay->addWidget(this->imageLabel, 0, Qt::AlignTop | Qt::AlignLeft);

    this->versionLabel = new QLabel(this);
    versionLabel->setAlignment(Qt::AlignLeft);
    versionLabel->setMaximumHeight(60);
    versionLabel->setStyleSheet("font-size: 18px; color: white;");
    versionLabel->setWordWrap(true);
    versionLabel->setTextFormat(Qt::RichText);
    lay->addWidget(versionLabel, 0, Qt::AlignLeft);

    this->playButton = new QPushButton("Играть", this);
    this->playButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->playButton->setMinimumHeight(50);
    IO::StyleLoader::applyStyle(this->playButton, ":/styles/version_card_play_button_style.qss");
    lay->addWidget(this->playButton);

    this->descriptionLabel = new QLabel(this);
    this->descriptionLabel->setWordWrap(true);
    this->descriptionLabel->setAlignment(Qt::AlignLeft);
    this->descriptionLabel->setStyleSheet("color: white;");
    lay->addWidget(this->descriptionLabel, 0, Qt::AlignLeft);
}

void VersionCard::setVersionData(const QString &version, const QString &imgPath, const QString &desc)
{
    this->imageLabel->setPixmap(createRoundedImage(imgPath, 20, this->width() - 20));
    this->versionLabel->setText(version);
    this->descriptionLabel->setText(desc);
}

QPixmap VersionCard::createRoundedImage(const QString &path, int radius, int width)
{
    QPixmap pix(path);
    if (pix.isNull()) return QPixmap();

    QPixmap scaled = pix.scaled(width, 280, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QPixmap rounded(scaled.size());
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath pathClip;
    pathClip.addRoundedRect(scaled.rect(), radius, radius);
    painter.setClipPath(pathClip);
    painter.drawPixmap(0, 0, scaled);

    return rounded;
}



