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
    Task task;
    task.url = url;
    task.isFile = false;
    task.retries = retries;

    m_queue.enqueue(task);
    if(!m_isBusy){
        downloadNext();
    }
}

void IO::NetLoader::downloadToFile(const QString &url, const QString &fullPath, int retries)
{
    Task task;
    task.url = url;
    task.fullPath = fullPath;
    task.isFile = true;
    task.retries = retries;

    m_queue.enqueue(task);
    if(!m_isBusy){
        downloadNext();
    }
}

void IO::NetLoader::downloadNext()
{
    if(m_queue.isEmpty()){
        m_isBusy = false;
        return;
    }

    m_isBusy = true;
    auto task = m_queue.dequeue();

    if(m_curReply){
        m_curReply->abort();
        m_curReply->deleteLater();
        m_curReply = nullptr;
    }

    if(task.isFile){
        downloadFile(task);
    }
    else{
        downloadByteArray(task);
    }
}

void IO::NetLoader::cancel()
{
    if(m_curReply){
        disconnect(m_curReply, nullptr, this, nullptr);
        m_curReply->abort();
        m_curReply->deleteLater();
        m_curReply = nullptr;
    }

    m_queue.clear();
    m_isBusy = false;
    emit cancelled();
}

void IO::NetLoader::downloadFile(Task task)
{
    bool res = QDir().mkpath(QFileInfo(task.fullPath).absolutePath());
    if(!res){
        emit failed("access error");
        return;
    }

    QNetworkRequest request(task.url);
    m_curReply = m_manager.get(request);

    QFile* file = new QFile(task.fullPath, m_curReply);
    if (!file->open(QIODevice::WriteOnly)) {
        emit failed(QString("Не удалось открыть файл: %1").arg(task.fullPath));
        m_curReply->abort();
        m_curReply->deleteLater();
        m_curReply = nullptr;
        return;
    }

    connect(m_curReply, &QNetworkReply::readyRead, this, [=]() {
        file->write(m_curReply->readAll());
    });

    connect(m_curReply, &QNetworkReply::downloadProgress, this, [=](qint64 received, qint64 total) {
        if (total > 0)
            emit progress(static_cast<int>((received * 100) / total));
    });

    connect(m_curReply, &QNetworkReply::finished, this, [=]() mutable {
        file->flush();
        file->close();

        QNetworkReply::NetworkError err = m_curReply->error();
        int httpCode = m_curReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if(err == QNetworkReply::NoError){
            emit fileDownloaded(task.fullPath);
        }
        else if(err == QNetworkReply::OperationCanceledError){
            emit cancelled();
        }
        else {
            QFile::remove(task.fullPath);

            bool retryable = false;
            if (err != QNetworkReply::NoError || httpCode == 429 || httpCode == 500 ||
                httpCode == 502 || httpCode == 503 || httpCode == 504) {
                retryable = true;
            }

            if(retryable && task.retries > 0){
                qWarning() << "[downloadToFile] Temporary error, retry..." << m_curReply->errorString() << "HTTP:" << httpCode;
                m_curReply->deleteLater();
                m_curReply = nullptr;
                m_isBusy = false;

                QTimer::singleShot(1000, this, [=]() mutable {
                    task.retries -= 1;
                    m_queue.enqueue(task);

                    downloadNext();
                });
                return;
            }
            else {
                qWarning() << "[downloadToFile] Failed completely:" << m_curReply->errorString() << "HTTP:" << httpCode;
                emit failed(m_curReply->errorString());
            }
        }

        QString errstr = m_curReply->errorString();
        m_curReply->deleteLater();
        m_curReply = nullptr;
        m_isBusy = false;
        emit finished(err, errstr);
        downloadNext();
    });
}

void IO::NetLoader::downloadByteArray(Task task)
{
    QNetworkRequest request(task.url);
    m_curReply = m_manager.get(request);

    connect(m_curReply, &QNetworkReply::downloadProgress, this, [=](qint64 received, qint64 total) {
        if (total > 0)
            emit progress(static_cast<int>((received * 100) / total));
    });

    connect(m_curReply, &QNetworkReply::finished, this, [=]() mutable {
        QNetworkReply::NetworkError err = m_curReply->error();
        int httpCode = m_curReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if(err == QNetworkReply::NoError){
            const QByteArray data = m_curReply->readAll();
            emit byteArrayDownloaded(data);
        }
        else if(err == QNetworkReply::OperationCanceledError){
            emit cancelled();
        }
        else {
            bool retryable = false;
            if (err != QNetworkReply::NoError || httpCode == 429 || httpCode == 500 ||
                httpCode == 502 || httpCode == 503 || httpCode == 504) {
                retryable = true;
            }

            if(retryable && task.retries > 0){
                qWarning() << "[downloadToByteArray] Temporary error, retry..."
                           << m_curReply->errorString() << "HTTP:" << httpCode;

                m_curReply->deleteLater();
                m_curReply = nullptr;
                m_isBusy = false;

                QTimer::singleShot(1000, this, [=]() mutable{
                    task.retries -= 1;
                    m_queue.enqueue(task);
                    downloadNext();
                });
                return;
            } else {
                qWarning() << "[downloadToByteArray] Failed completely:"
                           << m_curReply->errorString() << "HTTP:" << httpCode;
                emit failed(m_curReply->errorString());
            }
        }

        QString errstr = m_curReply->errorString();
        m_curReply->deleteLater();
        m_curReply = nullptr;
        m_isBusy = false;
        emit finished(err, errstr);
        downloadNext();
    });
}
