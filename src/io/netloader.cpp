#include <io/netloader.h>
#include <QUrl>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <qdir.h>

IO::NetLoader::NetLoader(QObject *parent) : QObject(parent)
{
}

void IO::NetLoader::downloadToByteArray(const QString &url, int retries)
{
    QNetworkRequest request(url);
    QNetworkReply* curReply = manager.get(request);

    connect(curReply, &QNetworkReply::downloadProgress, this, [=](qint64 received, qint64 total) {
        if (total > 0)
            emit progress(static_cast<int>((received * 100) / total));
    });

    connect(curReply, &QNetworkReply::finished, this, [=]() mutable {
        QNetworkReply::NetworkError err = curReply->error();
        int httpCode = curReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if(err == QNetworkReply::NoError){
            const QByteArray data = curReply->readAll();
            emit byteArrayDownloaded(data);
        }
        else if(err == QNetworkReply::OperationCanceledError){
            emit cancelled();
        }
        else {
            bool retryable = false;
            if (err == QNetworkReply::TimeoutError ||
                err == QNetworkReply::TemporaryNetworkFailureError)
            {
                retryable = true;
            }
            else if(httpCode == 429 || httpCode == 500 || httpCode == 502 || httpCode == 503 || httpCode == 504){
                retryable = true;
            }

            if(retryable && retries > 0){
                qWarning() << "[downloadToByteArray] Temporary error, retry..."
                           << curReply->errorString() << "HTTP:" << httpCode;

                curReply->deleteLater();
                curReply = nullptr;

                QTimer::singleShot(1000, this, [=](){
                    downloadToByteArray(url, retries - 1);
                });
                return;
            } else {
                qWarning() << "[downloadToByteArray] Failed completely:"
                           << curReply->errorString() << "HTTP:" << httpCode;
                emit failed(curReply->errorString());
            }
        }

        curReply->deleteLater();
        emit finished(err, curReply->errorString());
        curReply = nullptr;
    });
}

void IO::NetLoader::downloadToFile(const QString &url, const QString &fullPath, int retries)
{
    bool res = QDir().mkpath(QFileInfo(fullPath).absolutePath());
    if(!res){
        emit failed("access error");
        return;
    }

    QNetworkRequest request(url);
    QNetworkReply* curReply = manager.get(request);

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

    connect(curReply, &QNetworkReply::finished, this, [=]() mutable {
        file->flush();
        file->close();

        QNetworkReply::NetworkError err = curReply->error();
        int httpCode = curReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if(err == QNetworkReply::NoError){
            emit fileDownloaded(fullPath);
        }
        else if(err == QNetworkReply::OperationCanceledError){
            emit cancelled();
        }
        else {
            QFile::remove(fullPath);

            bool retryable = false;
            if (err == QNetworkReply::TimeoutError ||
                err == QNetworkReply::TemporaryNetworkFailureError)
            {
                retryable = true;
            }
            else if(httpCode == 429 || httpCode == 500 || httpCode == 502 || httpCode == 503 || httpCode == 504){
                retryable = true;
            }

            if(retryable && retries > 0){
                qWarning() << "[downloadToFile] Temporary error, retry..." << curReply->errorString() << "HTTP:" << httpCode;
                curReply->deleteLater();
                file->deleteLater();
                curReply = nullptr;

                QTimer::singleShot(1000, this, [=](){
                    downloadToFile(url, fullPath, retries - 1);
                });
                return;
            } else {
                qWarning() << "[downloadToFile] Failed completely:" << curReply->errorString() << "HTTP:" << httpCode;
                emit failed(curReply->errorString());
            }
        }

        file->deleteLater();
        curReply->deleteLater();
        emit finished(err, curReply->errorString());
        curReply = nullptr;
    });
}

void IO::NetLoader::cancel()
{
}
