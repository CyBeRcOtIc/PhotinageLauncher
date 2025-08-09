#include "io/filemanager.h"
#include <QFile>
#include <QDebug>

bool IO::FileManager::createAndWriteFile(const QByteArray& content, const QString& fullPath){
    QFile file = QFile(fullPath);
    if(!file.open(QIODevice::WriteOnly)){
        qWarning() << "[createAndWriteFile]" << "error open file: " << fullPath;
        return false;
    }

    if(!content.isEmpty()){
        qint64 written = file.write(content);
        if(written != content.size()){
            qWarning() << "[createAndWriteFile]" << "write error:" << fullPath;
            file.close();
            return false;
        }
    }

    file.close();
    return true;
}
