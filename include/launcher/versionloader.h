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
public: signals:
    void manifestDownloaded();
    void versionSucDownloaded();
    void infoMessage(const QString& message);
private: signals:
    void downloadedVersionAndIndexJson(bool suc);
    void downloadedClient(bool suc);
private:
    void downloadClient(const QString& fullPath, const QByteArray& versionJson);
    void downloadAssets(const QString& assetsDir, const QByteArray& indexJson);
    void downloadNatives(const QString& mcPath, const QByteArray& versionJson);
    void downloadLibraries(const QString& mcPath, const QByteArray& versionJson);
private:
    bool downloadManifestJson(const QString& url);
    void downloadVersionAndIndexJson(const QString& id, const QString& mcPath);
private:
    int calcObjectCount(const QByteArray &versionJson);
private:
    struct CurVersionData
    {
    public:
        CurVersionData() {}
    public:
        void setVersionManifestJson(const QByteArray& verManifest) {versionManifestJson = verManifest;}
        void setVersionJson(const QByteArray& verJson) {versionJson = verJson;}
        void setIndexJson(const QByteArray& indJson) {indexJson = indJson;}
        void setElementsCount(qint64 count) {elementsCount = count;}
        void setTotalElementsCount(qint64 count) {totalElementsCount = count;}

        QByteArray getVersionManifestJson() const {return versionManifestJson;}
        QByteArray getVersionJson() const {return versionJson;}
        QByteArray getIndexJson() const {return indexJson;}
        qint64 getElementsCount() const {return elementsCount;}
        qint64 getTotalElementsCount() const {return totalElementsCount;}
    private:
        QByteArray versionManifestJson;
        QByteArray versionJson;
        QByteArray indexJson;
        qint64 elementsCount = 0;
        qint64 totalElementsCount = 0;
    };
    CurVersionData m_data;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Launcher::VersionLoader::VersionStrategys)

}

#endif // VERSIONLOADER_H
