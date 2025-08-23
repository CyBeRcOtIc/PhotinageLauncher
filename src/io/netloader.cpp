#include <io/netloader.h>
#include <QUrl>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <qdir.h>

IO::NetLoader::NetLoader(QObject *parent, bool isExternalQueue) : QObject(parent), m_isExternalQueue(isExternalQueue),
    m_isBusy(false), m_queue(nullptr), m_curReply(nullptr)
{
    if(!isExternalQueue){
        m_queue = QSharedPointer<QQueue<IO::NetLoader::Task>>::create();
    }
}

void IO::NetLoader::downloadToByteArray(const QString &url, int retries)
{
    Task task;
    task.url = url;
    task.isFile = false;
    task.retries = retries;

    m_queue->enqueue(task);
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

    m_queue->enqueue(task);
    if(!m_isBusy){
        downloadNext();
    }
}

void IO::NetLoader::setQueue(QSharedPointer<QQueue<IO::NetLoader::Task>> q) {
    if (m_isExternalQueue && q) {
        cancel();
        m_queue = q;
        m_isBusy = false;
    }
}

void IO::NetLoader::downloadNext() {
    if (!m_queue) return;

    if (m_queue->isEmpty()) {
        m_isBusy = false;
        emit allTasksFinished();
        return;
    }

    if (m_isBusy) return;

    Task task = m_queue->dequeue();
    m_isBusy = true;

    if (task.isFile)
        downloadFile(task);
    else
        downloadByteArray(task);
}

void IO::NetLoader::cancel() {
    if (m_curReply) {
        disconnect(m_curReply, nullptr, this, nullptr);
        m_curReply->abort();
        m_curReply->deleteLater();
        m_curReply = nullptr;
    }

    if (!m_isExternalQueue && m_queue) {
        m_queue->clear();
    }

    m_isBusy = false;
    emit cancelled();
}

void IO::NetLoader::downloadFile(Task task)
{
    bool res = QDir().mkpath(QFileInfo(task.fullPath).absolutePath());
    if(!res){
        emit failed(QNetworkReply::UnknownNetworkError, "dir: access error");
        return;
    }

    QNetworkRequest request(task.url);
    m_curReply = m_manager.get(request);

    QFile* file = new QFile(task.fullPath, m_curReply);
    if (!file->open(QIODevice::WriteOnly)) {
        emit failed(QNetworkReply::UnknownNetworkError,
                    QString("Не удалось открыть файл: %1").arg(task.fullPath));
        m_curReply->abort();
        m_curReply->deleteLater();
        m_curReply = nullptr;
        return;
    }

    connect(m_curReply, &QNetworkReply::readyRead, this, [file, this]() {
        file->write(m_curReply->readAll());
    });

    connect(m_curReply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                if (total > 0)
                    emit progress(static_cast<int>((received * 100) / total));
            });

    connect(m_curReply, &QNetworkReply::finished, this, [file, task, this]() mutable {
        file->flush();
        file->close();

        QNetworkReply::NetworkError err = m_curReply->error();
        int httpCode = m_curReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        bool localError = (err != QNetworkReply::NoError && err != QNetworkReply::OperationCanceledError);
        bool retryableServerError = (httpCode == 0 || httpCode == 429 || (httpCode >= 500 && httpCode <= 504));
        bool criticalServerError = (httpCode >= 400 && httpCode < 500 && httpCode != 429);

        if (!localError && !retryableServerError && !criticalServerError) {
            emit fileDownloaded(task.fullPath);
        }
        else if (err == QNetworkReply::OperationCanceledError) {
            emit cancelled();
        }
        else if ((localError || retryableServerError) && task.retries > 0) {
            qWarning() << "[downloadToFile] Temporary error, retry..."
                       << m_curReply->errorString() << "HTTP:" << httpCode;

            QFile::remove(task.fullPath);
            m_curReply->deleteLater();
            m_curReply = nullptr;
            m_isBusy = false;

            task.retries -= 1;
            QTimer::singleShot(1000, this, [task, this]() {
                m_queue->enqueue(task);
                downloadNext();
            });
            return;
        }
        else {
            QFile::remove(task.fullPath);
            qWarning() << "[downloadToFile] Failed completely:"
                       << m_curReply->errorString() << "HTTP:" << httpCode;
            emit failed(err, m_curReply->errorString());
        }

        m_curReply->deleteLater();
        m_curReply = nullptr;
        m_isBusy = false;

        downloadNext();
    });
}

void IO::NetLoader::downloadByteArray(Task task)
{
    QNetworkRequest request(task.url);
    m_curReply = m_manager.get(request);

    connect(m_curReply, &QNetworkReply::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                if (total > 0)
                    emit progress(static_cast<int>((received * 100) / total));
            });

    connect(m_curReply, &QNetworkReply::finished, this, [task, this]() mutable {
        QNetworkReply::NetworkError err = m_curReply->error();
        int httpCode = m_curReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        bool localError = (err != QNetworkReply::NoError && err != QNetworkReply::OperationCanceledError);
        bool retryableServerError = (httpCode == 0 || httpCode == 429 || (httpCode >= 500 && httpCode <= 504));
        bool criticalServerError = (httpCode >= 400 && httpCode < 500 && httpCode != 429);

        if (!localError && !retryableServerError && !criticalServerError) {
            const QByteArray data = m_curReply->readAll();
            emit byteArrayDownloaded(data);
        }
        else if (err == QNetworkReply::OperationCanceledError) {
            emit cancelled();
        }
        else if ((localError || retryableServerError) && task.retries > 0) {
            qWarning() << "[downloadToByteArray] Temporary error, retry..."
                       << m_curReply->errorString() << "HTTP:" << httpCode;

            m_curReply->deleteLater();
            m_curReply = nullptr;
            m_isBusy = false;

            task.retries -= 1;
            QTimer::singleShot(1000, this, [task, this]() {
                m_queue->enqueue(task);
                downloadNext();
            });
            return;
        }
        else {
            qWarning() << "[downloadToByteArray] Failed completely:"
                       << m_curReply->errorString() << "HTTP:" << httpCode;
            emit failed(err, m_curReply->errorString());
        }

        m_curReply->deleteLater();
        m_curReply = nullptr;
        m_isBusy = false;

        downloadNext();
    });
}
