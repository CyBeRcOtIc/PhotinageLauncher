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
private:
    QPixmap createRoundedImage(const QString& path, int radius, int width);
private:
    QLabel* imageLabel;
    QLabel* versionLabel;
    QLabel* descriptionLabel;
    QPushButton* playButton;
};

#endif // VERSIONCARD_H
