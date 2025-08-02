#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qt/versioncard.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void enableRoundedCorners(bool enable);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void changeEvent(QEvent *event) override;
private:
    void setPlaceholders();
    void setSideBarButtons();
    void setComboBoxes();
    void setPlayMenu();
    bool isRounded = true;
    bool m_wasMaximizedBeforeMinimize = false;
    constexpr static int CornerRadius = 20;
    Ui::MainWindow *ui;
    VersionCard* versionCard;
};
#endif // MAINWINDOW_H
