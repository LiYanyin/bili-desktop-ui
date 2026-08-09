#include "VideoPlayer.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

static const QString MPV_PATH = "C:/Program Files/MPV Player/mpv.exe";

VideoPlayer::VideoPlayer(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

void VideoPlayer::play(const QString &bvid, const QString &title)
{
    fetchCid(bvid);
    Q_UNUSED(title); // passed to mpv later
}

void VideoPlayer::fetchCid(const QString &bvid)
{
    QUrl url("https://api.bilibili.com/x/web-interface/view");
    QUrlQuery q;
    q.addQueryItem("bvid", bvid);
    url.setQuery(q);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
    req.setRawHeader("Referer", "https://www.bilibili.com/");

    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, bvid]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();
        if (root["code"].toInt() != 0) return;

        QJsonArray pages = root["data"].toObject()["pages"].toArray();
        if (pages.isEmpty()) return;

        int cid = pages[0].toObject()["cid"].toInt();
        QString title = root["data"].toObject()["title"].toString();
        fetchPlayUrl(bvid, cid);
    });
}

void VideoPlayer::fetchPlayUrl(const QString &bvid, int cid)
{
    QUrl url("https://api.bilibili.com/x/player/playurl");
    QUrlQuery q;
    q.addQueryItem("bvid", bvid);
    q.addQueryItem("cid", QString::number(cid));
    q.addQueryItem("qn", "80");
    q.addQueryItem("fnval", "1");  // single MP4 (no DASH)
    q.addQueryItem("fnver", "0");
    q.addQueryItem("fourk", "1");
    url.setQuery(q);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
    req.setRawHeader("Referer", "https://www.bilibili.com/");

    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();
        if (root["code"].toInt() != 0) return;

        QJsonArray durl = root["data"].toObject()["durl"].toArray();
        if (durl.isEmpty()) return;

        QString videoUrl = durl[0].toObject()["url"].toString();
        if (!videoUrl.isEmpty()) {
            // Ensure HTTPS
            if (videoUrl.startsWith("http://"))
                videoUrl.replace(0, 4, "https");
            launchMpv(videoUrl, {});
        }
    });
}

void VideoPlayer::launchMpv(const QString &url, const QString &title)
{
    auto *proc = new QProcess(this);

    QStringList args;
    args << "--referrer=https://www.bilibili.com/"
         << "--force-window=yes"
         << "--keep-open=yes"
         << url;

    if (!title.isEmpty())
        args << ("--title=" + title);

    proc->start(MPV_PATH, args);
}
