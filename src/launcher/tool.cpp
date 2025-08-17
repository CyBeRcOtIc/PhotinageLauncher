#include "launcher/tool.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

QString Launcher::Tool::getVersionJsonUrl(const QByteArray &versionManifest, const QString &id)
{
    QJsonDocument doc = QJsonDocument::fromJson(versionManifest);

    if(doc.isNull() || !doc.isObject()){
        qWarning() << "[getVersionJsonUrl]" << "error parse json";
        return QString();
    }

    QJsonObject rootObj = doc.object();
    QJsonArray verArr = rootObj.value("versions").toArray();
    for(const QJsonValue& val : std::as_const(verArr)){
        if(!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        if(obj.value("id").toString() == id){
            return obj.value("url").toString();
        }
    }
    return QString();
}

QString Launcher::Tool::getVersionClientUrl(const QByteArray &versionJson)
{
    QJsonDocument doc = QJsonDocument::fromJson(versionJson);
    if(doc.isNull() || !doc.isObject()){
        qWarning() << "[getVersionClientUrl] error parse json";
        return QString();
    }
    QJsonObject rootObj = doc.object();
    QJsonObject downloadObj = rootObj.value("downloads").toObject();
    QJsonObject clientObj = downloadObj.value("client").toObject();
    return clientObj.value("url").toString();
}

Launcher::Tool::AssetIndexInfo Launcher::Tool::getAssetIndexInfo(const QByteArray &versionJson)
{
    QJsonDocument doc = QJsonDocument::fromJson(versionJson);
    if(doc.isNull() || !doc.isObject()){
        qWarning() << "[getAssetIndexInfo] error parse json";
        return AssetIndexInfo();
    }
    QJsonObject rootObj = doc.object();
    QJsonObject assetIndexObj = rootObj.value("assetIndex").toObject();
    AssetIndexInfo assetIndexInfo;
    assetIndexInfo.id = assetIndexObj.value("id").toString();
    assetIndexInfo.url = assetIndexObj.value("url").toString();
    return assetIndexInfo;
}

bool Launcher::Tool::versionGreaterOrEqual(const QString &v1, const QString &v2)
{
    QStringList a = v1.split(".");
    QStringList b = v2.split(".");

    int len = qMax(a.size(), b.size());
    a.resize(len);
    b.resize(len);

    for (int i = 0; i < len; ++i) {
        int numA = a[i].isEmpty() ? 0 : a[i].toInt();
        int numB = b[i].isEmpty() ? 0 : b[i].toInt();
        if (numA > numB) return true;
        if (numA < numB) return false;
    }
    return true;
}

QList<Launcher::Tool::AssetInfo> Launcher::Tool::parseAssets(const QByteArray &indexJson)
{
    QJsonDocument doc = QJsonDocument::fromJson(indexJson);
    QList<AssetInfo> assets;

    if(doc.isNull() || !doc.isObject()){
        qWarning() << "[parseAssets] error parse json";
        return assets;
    }
    QJsonObject rootObj = doc.object();

    if (!rootObj.contains("objects"))
        return assets;

    QJsonObject objects = rootObj["objects"].toObject();

    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        QJsonObject obj = it.value().toObject();

        QString hash = obj["hash"].toString();
        QString subdir = hash.left(2);
        QString url = QString("https://resources.download.minecraft.net/%1/%2").arg(subdir, hash);
        QString path = QString("/assets/objects/%1/%2").arg(subdir, hash);

        AssetInfo info(url, path);
        assets.append(info);
    }

    return assets;
}
