#ifndef NETLOADER_H
#define NETLOADER_H

#include "QString"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace IO {

class NetLoader : public QObject {
    Q_OBJECT
public:
    explicit NetLoader(QObject* parent = nullptr);
public:
    void downloadToByteArray(const QString& url);
    void downloadToFile(const QString& url, const QString& fullPath);
    void cancel();
signals:
    void progress(int percent);
    void byteArrayDownloaded(const QByteArray& content);
    void fileDownloaded(const QString& path);
    void failed(const QString& error);
    void cancelled();
    void finished(QNetworkReply::NetworkError err);
private:
    QNetworkAccessManager manager;
    QNetworkReply* curReply;
};

}

#endif // NETLOADER_H
