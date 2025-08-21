#ifndef TOOL_H
#define TOOL_H

#include <QString>

namespace Launcher {

class Tool{
public:
    struct AssetIndexInfo {
    public:
        bool isEmpty() const { return id.isEmpty() || url.isEmpty(); }
    public:
        QString id;
        QString url;
    };
    struct AssetInfo
    {
    public:
        AssetInfo(const QString& url, const QString& path, const QString& name) : url(url), path(path), name(name) {}
        bool isEmpty() const { return url.isEmpty() || path.isEmpty() || name.isEmpty(); }
    public:
        QString url;
        QString path;
        QString name;
    };
public:
    static QString getVersionJsonUrl(const QByteArray& versionManifest, const QString& id);
    static QString getVersionClientUrl(const QByteArray& versionJson);
    static AssetIndexInfo getAssetIndexInfo(const QByteArray& versionJson);
    static bool versionGreaterOrEqual(const QString& v1, const QString& v2);
    static QList<AssetInfo> parseAssets(const QByteArray& indexJson);
};

}

#endif // TOOL_H
