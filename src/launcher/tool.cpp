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
