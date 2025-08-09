#ifndef TOOL_H
#define TOOL_H

#include <QString>

namespace Launcher {

class Tool{
public:
    static QString getVersionJsonUrl(const QByteArray& versionManifest, const QString& id);
    static QString getVersionClientUrl(const QByteArray& versionJson);
    static bool versionGreaterOrEqual(const QString& v1, const QString& v2);
};

}

#endif // TOOL_H
