#ifndef NETLOADER_H
#define NETLOADER_H

#include "QString"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>

namespace IO {

class NetLoader : public QObject {
    Q_OBJECT
public:
    struct Task
    {
        QString url;
        QString fullPath;
        int retries;
        bool isFile;
    };
public:
    explicit NetLoader(QObject* parent = nullptr, bool isExternalQueue = false);
public:
    void downloadToByteArray(const QString& url, int retries = 3);
    void downloadToFile(const QString& url, const QString& fullPath, int retries = 3);
    void setQueue(QSharedPointer<QQueue<IO::NetLoader::Task>> q);
    void downloadNext();
    void cancel();
private:
    void downloadFile(Task task);
    void downloadByteArray(Task task);
signals:
    void progress(int percent);
    void byteArrayDownloaded(const QByteArray& content);
    void fileDownloaded(const QString& path);
    void failed(const QNetworkReply::NetworkError& err, const QString& errstr);
    void cancelled();
    void allTasksFinished();
private:
    QSharedPointer<QQueue<IO::NetLoader::Task>> m_queue;
    QNetworkAccessManager m_manager;
    QNetworkReply* m_curReply;
    bool m_isBusy;
    bool m_isExternalQueue;
};

}

#endif // NETLOADER_H
