#include <QApplication>
#include "qt/mainwindow.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setFont(QFont("Segoe UI"));
    MainWindow w;
    w.setWindowFlag(Qt::FramelessWindowHint, true);
    w.show();
    qDebug() << "Current Qt version:" << QT_VERSION_STR;
    return a.exec();
}
