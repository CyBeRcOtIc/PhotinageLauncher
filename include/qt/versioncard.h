#ifndef VERSIONCARD_H
#define VERSIONCARD_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class VersionCard : public QWidget{
    Q_OBJECT
public:
    explicit VersionCard(QWidget* parent = nullptr);
public:
    void setVersionData(const QString& version, const QString& imgPath, const QString& desc);
public:
    QLabel* imageLabel;
    QLabel* versionLabel;
    QLabel* descriptionLabel;
    QPushButton* playButton;
private:
    QPixmap createRoundedImage(const QString& path, int radius, int width);
};

#endif // VERSIONCARD_H
