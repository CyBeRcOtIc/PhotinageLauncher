#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QString>

namespace IO{

class FileManager{
public:
    static bool createAndWriteFile(const QByteArray& content, const QString& fullPath);
};

}

#endif // FILEMANAGER_H
