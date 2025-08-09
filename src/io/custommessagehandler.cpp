#include "io/custommessagehandler.h"
#include <QDateTime>
#include <QTextStream>
#include <QDebug>

QFile logFile;
QMutex logMutex;

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString typeStr;
    FILE* consoleOut = stdout;

    switch (type) {
    case QtDebugMsg: typeStr = "DEBUG"; break;
    case QtInfoMsg: typeStr = "INFO"; break;
    case QtWarningMsg: typeStr = "WARNING"; consoleOut = stderr; break;
    case QtCriticalMsg: typeStr = "CRITICAL"; consoleOut = stderr; break;
    case QtFatalMsg: typeStr = "FATAL"; consoleOut = stderr; break;
    }

    QString finalMessage = QString("[%1] [%2] %3").arg(timeStr, typeStr, msg);

    QTextStream(consoleOut) << finalMessage << "\n";

    if (consoleOut == stderr) {
        QMutexLocker locker(&logMutex);
        if (logFile.isOpen()) {
            QTextStream ts(&logFile);
            ts << finalMessage << "\n";
            ts.flush();
        }
    }

    if (type == QtFatalMsg)
        abort();
}

void setLogFile(const QString& fullPath){
    QMutexLocker locker(&logMutex);
    if (logFile.isOpen())
        logFile.close();

    logFile.setFileName(fullPath);
    if(!logFile.open(QIODevice::Append | QIODevice::Text)){
        qCritical() << "Не удалось открыть файл лога!";
    }
}
