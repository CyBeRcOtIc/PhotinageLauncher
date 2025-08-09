#ifndef VERSIONLOADER_H
#define VERSIONLOADER_H

#include <QString>
#include <QObject>

namespace Launcher{

class VersionLoader : public QObject{
    Q_OBJECT
public:
    enum VersionStrategy {
        release_new = 0x01,
        release_old = 0x02,
        snapshot    = 0x04,
        old_beta    = 0x08,
        old_alpha   = 0x10
    };
    Q_DECLARE_FLAGS(VersionStrategys, VersionStrategy)
    static const VersionStrategys AllTypes;
public:
    explicit VersionLoader(QObject* parent = nullptr);
public:
    void downloadManifest(const QString& manifestUrl = "https://launchermeta.mojang.com/mc/game/version_manifest.json");
    void downloadVersion(const QString& id, const QString& mcPath);
    QStringList getVersionsList(VersionStrategys types = AllTypes);
signals:
    void manifestDownloaded();
    void infoMessage(const QString& message);
private:
    void downloadClient(const QString& fullPath, const QByteArray& versionJson);
    void downloadAssets(const QString& mcPath, const QByteArray& versionJson);
    void downloadNatives(const QString& mcPath, const QByteArray& versionJson);
    void downloadLibraries(const QString& mcPath, const QByteArray& versionJson);
private:
    QByteArray versionManifest;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Launcher::VersionLoader::VersionStrategys)

}

#endif // VERSIONLOADER_H
