#include "io/filemanager.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

bool IO::FileManager::createAndWriteFile(const QByteArray& content, const QString& fullPath){
    QString path(QFileInfo(fullPath).absolutePath());
    if(!ensureDirectoryExists(path)){
        qWarning() << "[createAndWriteFile]" << "Failed to create path: " << fullPath;
        return false;
    }

    QFile file(fullPath);
    if(!file.open(QIODevice::WriteOnly)){
        qWarning() << "[createAndWriteFile]" << "Failed to open file: " << fullPath;
        return false;
    }

    if(!content.isEmpty()){
        qint64 written = file.write(content);
        if(written != content.size()){
            qWarning() << "[createAndWriteFile]" << "Write error:" << fullPath;
            file.close();
            return false;
        }
    }

    file.close();
    return true;
}

bool IO::FileManager::ensureDirectoryStructure(const QString &mcPath)
{
    QDir dir(mcPath);
    QFileInfo info(mcPath);

    if (!QFileInfo(info.absolutePath()).isWritable()) {
        qWarning() << "[ensureDirectoryStructure] Parent dir not writable:" << info.absolutePath();
        return false;
    }

    if (!dir.exists() && !dir.mkpath(".")) {
        qWarning() << "[ensureDirectoryStructure] Failed to create base dir:" << mcPath;
        return false;
    }

    QStringList subdirs = { "assets", "libraries", "versions", "assets/indexes", "assets/objects" };
    for (const QString &sub : subdirs) {
        if (!ensureDirectoryExists(dir.filePath(sub))) {
            qWarning() << "[ensureDirectoryStructure] Failed to create subdir:" << sub;
            return false;
        }
    }

    return true;
}

bool IO::FileManager::ensureDirectoryExists(const QString &path)
{
    QDir dir;
    return dir.exists(path) || dir.mkpath(path);
}
