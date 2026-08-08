#include "BiliApi.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QDateTime>

BiliApi::BiliApi(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

void BiliApi::fetchPopular(int page, int count)
{
    QUrl url("https://api.bilibili.com/x/web-interface/popular");
    QUrlQuery query;
    query.addQueryItem("ps", QString::number(count));
    query.addQueryItem("pn", QString::number(page));
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
    req.setRawHeader("Referer", "https://www.bilibili.com/");

    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit fetchError(reply->errorString());
            return;
        }

        QByteArray data = reply->readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError) {
            emit fetchError("JSON parse: " + err.errorString());
            return;
        }

        QJsonObject root = doc.object();
        if (root["code"].toInt() != 0) {
            emit fetchError("API error: " + root["message"].toString());
            return;
        }

        QJsonArray list = root["data"].toObject()["list"].toArray();
        QList<VideoData> videos;
        for (const QJsonValue &v : list) {
            if (v.isObject())
                videos.append(parseVideo(v.toObject()));
        }

        emit videosReady(videos);
    });
}

VideoData BiliApi::parseVideo(const QJsonObject &obj)
{
    VideoData data;

    data.title       = obj["title"].toString();
    data.coverPath   = obj["pic"].toString(); // URL to cover image
    data.bvid        = obj["bvid"].toString();

    QJsonObject owner = obj["owner"].toObject();
    data.uploaderName       = owner["name"].toString();
    data.uploaderAvatarPath = owner["face"].toString();

    QJsonObject stat = obj["stat"].toObject();
    int views = stat["view"].toInt();
    data.playCount = formatCount(views);

    qint64 pubdate = obj["pubdate"].toInteger();
    data.publishTime = formatTime(pubdate);

    int durationSec = obj["duration"].toInt();
    data.duration = formatDuration(durationSec);

    return data;
}

QString BiliApi::formatCount(int count)
{
    if (count >= 10000) {
        double w = count / 10000.0;
        if (w >= 100)
            return QString::number((int)w) + "万";
        return QString::number(w, 'f', 1) + "万";
    }
    return QString::number(count);
}

QString BiliApi::formatDuration(int seconds)
{
    int m = seconds / 60;
    int s = seconds % 60;
    if (m >= 60) {
        int h = m / 60;
        m = m % 60;
        return QString("%1:%2:%3")
            .arg(h)
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'));
    }
    return QString("%1:%2")
        .arg(m)
        .arg(s, 2, 10, QChar('0'));
}

QString BiliApi::formatTime(qint64 unixTime)
{
    QDateTime dt = QDateTime::fromSecsSinceEpoch(unixTime);
    qint64 secsAgo = QDateTime::currentSecsSinceEpoch() - unixTime;

    if (secsAgo < 3600)
        return QString::number(qMax(1LL, secsAgo / 60)) + "分钟前";
    if (secsAgo < 86400)
        return QString::number(secsAgo / 3600) + "小时前";
    if (secsAgo < 604800)
        return QString::number(secsAgo / 86400) + "天前";
    return dt.toString("MM-dd");
}
