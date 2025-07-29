#include <QApplication>
#include "qt/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowFlag(Qt::FramelessWindowHint, true);
    w.show();
    return a.exec();
}
