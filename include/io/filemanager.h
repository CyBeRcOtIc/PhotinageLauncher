#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QString>

namespace IO{

class FileManager{
public:
    static bool createAndWriteFile(const QByteArray& content, const QString& fullPath);
    static bool ensureDirectoryStructure(const QString& mcPath);
private:
    static bool canCreateDirectory(const QString &path);
    static bool ensureDirectoryExists(const QString& path);
};

}

#endif // FILEMANAGER_H
