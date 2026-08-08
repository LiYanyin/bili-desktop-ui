#include "VideoDataLoader.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

QList<VideoData> VideoDataLoader::loadFromJsonFile(const QString &filePath)
{
    QList<VideoData> result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open mock data file:" << filePath;
        return result;
    }

    QByteArray rawData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return result;
    }

    if (!doc.isArray()) {
        qWarning() << "Expected JSON array at top level";
        return result;
    }

    const QJsonArray arr = doc.array();
    for (const QJsonValue &val : arr) {
        if (!val.isObject()) continue;

        QJsonObject obj = val.toObject();
        VideoData data;
        data.title               = obj["title"].toString();
        data.coverPath           = obj["cover_path"].toString();
        data.uploaderName         = obj["uploader_name"].toString();
        data.uploaderAvatarPath   = obj["uploader_avatar_path"].toString();
        data.playCount            = obj["play_count"].toString();
        data.publishTime          = obj["publish_time"].toString();
        data.duration             = obj["duration"].toString();

        result.append(data);
    }

    qDebug() << "Loaded" << result.size() << "videos from" << filePath;
    return result;
}

QList<VideoData> VideoDataLoader::loadFromResource(const QString &resourcePath)
{
    // Qt resource system: resourcePath is like ":/resources/mock_videos.json"
    // But for now, we'll just use the file path directly
    return loadFromJsonFile(resourcePath);
}
