#ifndef CUSTOMMESSAGEHANDLER_H
#define CUSTOMMESSAGEHANDLER_H

#include <QString>
#include <QFile>
#include <QMutex>
#include <QMessageLogContext>

extern QFile logFile;
extern QMutex logMutex;

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void setLogFile(const QString& fullPath);

#endif // CUSTOMMESSAGEHANDLER_H
