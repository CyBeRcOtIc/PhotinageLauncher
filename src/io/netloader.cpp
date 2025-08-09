#include <io/netloader.h>
#include <QUrl>
#include <QDebug>
#include <QFile>

IO::NetLoader::NetLoader(QObject *parent) : QObject(parent)
{
    curReply = nullptr;
}

void IO::NetLoader::downloadToByteArray(const QString &url)
{
    if (curReply) {
        curReply->abort();
        curReply->deleteLater();
        curReply = nullptr;
    }

    QNetworkRequest request(url);
    curReply = manager.get(request);

    connect(curReply, &QNetworkReply::downloadProgress, this, [=](qint64 received, qint64 total) {
        if (total > 0)
            emit progress(static_cast<int>((received * 100) / total));
    });

    connect(curReply, &QNetworkReply::finished, this, [=]() {
        curReply->deleteLater();
        if(curReply->error() == QNetworkReply::OperationCanceledError){
            emit cancelled();
        }
        else if (curReply->error() == QNetworkReply::NoError) {
            const QByteArray data = curReply->readAll();
            emit byteArrayDownloaded(data);
        }
        else {
            emit failed(curReply->errorString());
        }
        emit finished(curReply->error());
        curReply = nullptr;
    });
}

void IO::NetLoader::downloadToFile(const QString &url, const QString &fullPath)
{
    if (curReply) {
        curReply->abort();
        curReply->deleteLater();
        curReply = nullptr;
    }

    QNetworkRequest request(url);
    curReply = manager.get(request);

    QFile* file = new QFile(fullPath, curReply);
    if (!file->open(QIODevice::WriteOnly)) {
        emit failed(QString("Не удалось открыть файл: %1").arg(fullPath));
        file->deleteLater();
        curReply->abort();
        curReply->deleteLater();
        curReply = nullptr;
        return;
    }

    connect(curReply, &QNetworkReply::readyRead, this, [=]() {
        file->write(curReply->readAll());
    });

    connect(curReply, &QNetworkReply::downloadProgress, this, [=](qint64 received, qint64 total) {
        if (total > 0)
            emit progress(static_cast<int>((received * 100) / total));
    });

    connect(curReply, &QNetworkReply::finished, this, [=]() {
        file->flush();
        file->close();

        if(curReply->error() == QNetworkReply::OperationCanceledError){
            emit cancelled();
        }
        else if (curReply->error() == QNetworkReply::NoError) {
            emit fileDownloaded(fullPath);
        }
        else {
            emit failed(curReply->errorString());
            QFile::remove(fullPath);
        }

        file->deleteLater();
        curReply->deleteLater();
        emit finished(curReply->error());
        curReply = nullptr;
    });
}

void IO::NetLoader::cancel()
{
    if(curReply){
        curReply->abort();
    }
}
