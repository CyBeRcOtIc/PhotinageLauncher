#include "launcher/versionloader.h"
#include "io/netloader.h"
#include "io/filemanager.h"
#include "launcher/tool.h"
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

const Launcher::VersionLoader::VersionStrategys Launcher::VersionLoader::AllTypes = VersionStrategy::release_new
    | VersionStrategy::release_old
    | VersionStrategy::snapshot
    | VersionStrategy::old_beta
    | VersionStrategy::old_alpha;


Launcher::VersionLoader::VersionLoader(QObject *parent) : QObject(parent){
}

void Launcher::VersionLoader::downloadVersion(const QString &id, const QString &mcPath)
{
    IO::NetLoader* loader = new IO::NetLoader();
    connect(loader, &IO::NetLoader::byteArrayDownloaded, loader, [=](const QByteArray& content){
        QString verPath = mcPath + "/versions/" + id + "/";
        if(!IO::FileManager::createAndWriteFile(content, verPath + id + ".json")){
            qWarning() << "[downloadVersion] Failed to save version JSON for" << id;
            return;
        }
        this->downloadClient(verPath + id + ".jar", content);
    });
    connect(loader, &IO::NetLoader::finished, loader, &QObject::deleteLater);

    QString url = Launcher::Tool::getVersionJsonUrl(this->versionManifest, id);
    if(url.isEmpty()){
        qWarning() << "[downloadVersion] URL for version" << id << "is empty";
        loader->deleteLater();
        return;
    }

    loader->downloadToByteArray(url);
}

void Launcher::VersionLoader::downloadManifest(const QString &manifestUrl)
{
    IO::NetLoader* loader = new IO::NetLoader();
    connect(loader, &IO::NetLoader::byteArrayDownloaded, loader, [=](const QByteArray& content){
        this->versionManifest = content;
        emit manifestDownloaded();
    });
    connect(loader, &IO::NetLoader::finished, loader, &QObject::deleteLater);
    loader->downloadToByteArray(manifestUrl);
}

QStringList Launcher::VersionLoader::getVersionsList(VersionStrategys types){
    QJsonDocument doc = QJsonDocument::fromJson(this->versionManifest);

    if(doc.isNull() || !doc.isObject()){
        qWarning() << "[getVersionsList]" << "error parse json";
        return QStringList();
    }

    QJsonObject rootObj = doc.object();
    QJsonArray versions = rootObj.value("versions").toArray();

    QStringList list;
    for(const QJsonValue& val : std::as_const(versions)){
        QJsonObject obj = val.toObject();
        QString id = obj.value("id").toString();
        QString type = obj.value("type").toString();

        bool match = false;

        if((types & VersionStrategy::release_new) && type == "release" && Tool::versionGreaterOrEqual(id, "1.7.10")){
            match = true;
        }
        if((types & VersionStrategy::release_old) && type == "release" && !Tool::versionGreaterOrEqual(id, "1.7.10")){
            match = true;
        }
        if((types & VersionStrategy::snapshot) && type == "snapshot"){
            match = true;
        }
        if((types & VersionStrategy::old_alpha) && type == "old_alpha"){
            match = true;
        }
        if((types & VersionStrategy::old_beta) && type == "old_beta"){
            match = true;
        }

        if(match){
            list.append(id);
        }
    }
    return list;
}

void Launcher::VersionLoader::downloadClient(const QString &fullPath, const QByteArray &versionJson)
{
    IO::NetLoader* loader = new IO::NetLoader();
    connect(loader, &IO::NetLoader::fileDownloaded, loader, [=](const QString& path){
        qInfo() << "[downloadClient] client downloaded on the way: " << path;
    });
    connect(loader, &IO::NetLoader::finished, loader, &QObject::deleteLater);

    QString url = Launcher::Tool::getVersionClientUrl(versionJson);
    if(url.isEmpty()){
        qWarning() << "[downloadClient] error get client url";
        return;
    }
    loader->downloadToFile(url, fullPath);
}
