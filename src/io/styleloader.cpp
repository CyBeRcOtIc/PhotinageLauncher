#include "io/styleloader.h"
#include <QFile>
#include <QDebug>

QString IO::StyleLoader::loadStyle(const QString& resourcePath){
    QFile file(resourcePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Error open style resource:" << resourcePath;
        return QString();
    }
    QTextStream stream(&file);
    return stream.readAll();
}

void IO::StyleLoader::applyStyle(QWidget* widget, const QString& resourcePath){
    QString style = loadStyle(resourcePath);
    if (!style.isEmpty()) {
        widget->setStyleSheet(style);
    }
}
