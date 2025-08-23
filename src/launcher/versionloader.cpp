#include "launcher/versionloader.h"
#include "io/netloader.h"
#include "io/filemanager.h"
#include "launcher/tool.h"
#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QMessageBox>

const Launcher::VersionLoader::VersionStrategys Launcher::VersionLoader::AllTypes = VersionStrategy::release_new
    | VersionStrategy::release_old
    | VersionStrategy::snapshot
    | VersionStrategy::old_beta
    | VersionStrategy::old_alpha;


Launcher::VersionLoader::VersionLoader(QObject *parent) : QObject(parent){
}

void Launcher::VersionLoader::downloadVersion(const QString &id, const QString &mcPath)
{
    if(IO::FileManager::ensureDirectoryStructure(mcPath)){
        connect(this, &VersionLoader::downloadedVersionAndIndexJson, this, [this, id, mcPath](bool suc){
            if(suc){
                downloadClient(mcPath + "/versions/" + id + "/" + id + ".jar", m_data.getVersionJson());
            }
        }, Qt::SingleShotConnection);

        connect(this, &VersionLoader::downloadedClient, this, [this, id, mcPath](bool suc){
            if(suc){
                downloadAssets(mcPath, m_data.getIndexJson());
            }
        }, Qt::SingleShotConnection);

        downloadVersionAndIndexJson(id, mcPath);
    }
    else{
        QMessageBox::critical(nullptr, "Access error", "It was not possible to create or verify the game directory due to lack of rights.\n"
                                                       "Please select another folder for the game, the best option is \"AppData\".");
    }
}

void Launcher::VersionLoader::downloadManifest(const QString &manifestUrl)
{
    IO::NetLoader* loader = new IO::NetLoader(this);
    connect(loader, &IO::NetLoader::byteArrayDownloaded, loader, [loader, this](const QByteArray& content){
        loader->deleteLater();
        m_data.setVersionManifestJson(content);
        emit manifestDownloaded();
    });
    connect(loader, &IO::NetLoader::failed, loader, &QObject::deleteLater);
    loader->downloadToByteArray(manifestUrl);
}

void Launcher::VersionLoader::downloadClient(const QString &fullPath, const QByteArray &versionJson)
{
    QString url = Launcher::Tool::getVersionClientUrl(versionJson);
    if(url.isEmpty()){
        qWarning() << "[downloadClient] error get client url";
        emit downloadedClient(false);
        return;
    }

    IO::NetLoader* loader = new IO::NetLoader(this);
    connect(loader, &IO::NetLoader::fileDownloaded, loader, [loader, this](const QString& path){
        loader->deleteLater();
        qInfo() << "[downloadClient] client downloaded on the way: " << path;
        emit downloadedClient(true);
    });
    connect(loader, &IO::NetLoader::failed, loader, [loader, this](const QNetworkReply::NetworkError& err, const QString& str){
        loader->deleteLater();
        qWarning() << "[downloadClient] Failed download client: " << str;
        emit downloadedClient(false);
    });
    loader->downloadToFile(url, fullPath);
}

void Launcher::VersionLoader::downloadAssets(const QString &mcPath, const QByteArray &indexJson)
{
    auto assets = QSharedPointer<QList<Launcher::Tool::AssetInfo>>::create(
        Launcher::Tool::parseAssets(indexJson)
        );

    if (assets->isEmpty()) {
        qWarning() << "[downloadAssets] No assets to download";
        return;
    }

    auto tasks = QSharedPointer<QQueue<IO::NetLoader::Task>>::create();

    for (int i = 0; i < assets->size(); ++i) {
        const Launcher::Tool::AssetInfo &val = assets->at(i);
        IO::NetLoader::Task task;
        task.url = val.url;
        task.fullPath = mcPath + val.path;
        task.retries = 3;
        task.isFile = true;
        tasks->enqueue(task);
    }

    int loaderCount = 8;
    for (int i = 0; i < loaderCount; ++i) {
        IO::NetLoader *loader = new IO::NetLoader(this, true);
        loader->setQueue(tasks);

        connect(loader, &IO::NetLoader::failed, this, [loader](const QNetworkReply::NetworkError& err, const QString &str){
            loader->deleteLater();
            qWarning() << "[downloadAssets] Failed load asset:" << str;
        });

        connect(loader, &IO::NetLoader::fileDownloaded, this, [loader](const QString &path){
            qDebug() << "[downloadAssets] Loaded asset:" << path;
        });

        connect(loader, &IO::NetLoader::allTasksFinished, loader, [loader](){
            loader->deleteLater();
            qDebug() << "[downloadAssets] Loader finished all tasks";
        });

        loader->downloadNext();
    }
}

void Launcher::VersionLoader::downloadVersionAndIndexJson(const QString &id, const QString &mcPath)
{
    QString verUrl = Launcher::Tool::getVersionJsonUrl(m_data.getVersionManifestJson(), id);
    if(verUrl.isEmpty()){
        qWarning() << "[downloadVersionAndIndexJson] error getVersionJsonUrl";
        emit downloadedVersionAndIndexJson(false);
        return;
    }

    IO::NetLoader* versionJsonLoader = new IO::NetLoader(this);
    connect(versionJsonLoader, &IO::NetLoader::byteArrayDownloaded, versionJsonLoader, [this, mcPath, id, versionJsonLoader](const QByteArray& version){
        versionJsonLoader->deleteLater();
        if(!IO::FileManager::createAndWriteFile(version, mcPath + "/versions/" + id + "/" + id + ".json")){
            qWarning() << "[downloadVersionAndIndexJson] Failed save: " << id + ".json";
            emit downloadedVersionAndIndexJson(false);
            return;
        }

        Launcher::Tool::AssetIndexInfo assetIndexInfo = Launcher::Tool::getAssetIndexInfo(version);
        if(assetIndexInfo.isEmpty()){
            qWarning() << "[downloadVersionAndIndexJson] Failed get assetIndexInfo";
            emit downloadedVersionAndIndexJson(false);
            return;
        }
        m_data.setVersionJson(version);

        IO::NetLoader* indexJsonLoader = new IO::NetLoader(this);
        connect(indexJsonLoader, &IO::NetLoader::byteArrayDownloaded, indexJsonLoader, [this, mcPath, assetIndexInfo, indexJsonLoader](const QByteArray& index){
            indexJsonLoader->deleteLater();
            if(!IO::FileManager::createAndWriteFile(index, mcPath + "/assets/indexes/" + assetIndexInfo.id + ".json")){
                qWarning() << "[downloadVersionAndIndexJson] Failed save: " << assetIndexInfo.id + ".json";
                emit downloadedVersionAndIndexJson(false);
                return;
            }
            m_data.setIndexJson(index);
            emit downloadedVersionAndIndexJson(true);
        });
        connect(indexJsonLoader, &IO::NetLoader::failed, indexJsonLoader, [this, indexJsonLoader](const QNetworkReply::NetworkError& err){
            indexJsonLoader->deleteLater();
            emit downloadedVersionAndIndexJson(false);
        });
        indexJsonLoader->downloadToByteArray(assetIndexInfo.url);
    });
    connect(versionJsonLoader, &IO::NetLoader::failed, versionJsonLoader, [this, versionJsonLoader](const QNetworkReply::NetworkError& err){
        versionJsonLoader->deleteLater();
        emit downloadedVersionAndIndexJson(false);
    });

    versionJsonLoader->downloadToByteArray(verUrl);
}

int Launcher::VersionLoader::calcObjectCount(const QByteArray &versionJson)
{

    return 0;
}

QStringList Launcher::VersionLoader::getVersionsList(VersionStrategys types){
    QJsonDocument doc = QJsonDocument::fromJson(m_data.getVersionManifestJson());

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
