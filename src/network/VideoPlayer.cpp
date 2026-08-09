#include "VideoPlayer.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QFile>
#include <QDebug>

static const QString MPV_PATH = "C:/Program Files/MPV Player/mpv.exe";

VideoPlayer::VideoPlayer(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

void VideoPlayer::play(const QString &bvid, const QString &title)
{
    // mpv plays B站 URLs directly via its built-in ytdl_hook.lua + yt-dlp
    launchMpv({QString("https://www.bilibili.com/video/") + bvid}, title);
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
        QFile f("F:/project/bili-desktop-ui/playback.log");
        auto log = [&f](const QString &s) {
            if (f.open(QIODevice::Append)) f.write(qPrintable(s + "\n"));
        };
        if (reply->error() != QNetworkReply::NoError) {
            log("fetchCid error: " + reply->errorString());
            return;
        }
        QByteArray data = reply->readAll();
        log("fetchCid got " + QString::number(data.size()) + " bytes");
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();
        log("fetchCid code=" + QString::number(root["code"].toInt()));
        if (root["code"].toInt() != 0) return;
        QJsonArray pages = root["data"].toObject()["pages"].toArray();
        if (pages.isEmpty()) { log("fetchCid: no pages"); return; }
        int cid = pages[0].toObject()["cid"].toInt();
        log("fetchCid cid=" + QString::number(cid));
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
        QFile f("F:/project/bili-desktop-ui/playback.log");
        auto log = [&f](const QString &s) {
            if (f.open(QIODevice::Append)) f.write(qPrintable(s + "\n"));
        };
        if (reply->error() != QNetworkReply::NoError) {
            log("fetchPlayUrl error: " + reply->errorString());
            return;
        }
        QByteArray data = reply->readAll();
        log("fetchPlayUrl got " + QString::number(data.size()) + " bytes: " + QString::fromUtf8(data));
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();
        log("fetchPlayUrl code=" + QString::number(root["code"].toInt()));
        if (root["code"].toInt() != 0) {
            log("fetchPlayUrl msg: " + root["message"].toString());
            return;
        }
        QJsonArray durl = root["data"].toObject()["durl"].toArray();
        if (durl.isEmpty()) { log("fetchPlayUrl: no durl"); return; }
        QString videoUrl = durl[0].toObject()["url"].toString();
        log("fetchPlayUrl url=" + videoUrl.left(80) + "...");
        if (!videoUrl.isEmpty()) {
            if (videoUrl.startsWith("http://"))
                videoUrl.replace(0, 4, "https");
            launchMpv(QStringList{videoUrl}, {});
        }
    });
}

void VideoPlayer::launchMpv(const QStringList &urls, const QString &title)
{
    auto *proc = new QProcess(this);
    // Ensure yt-dlp is on PATH for mpv's ytdl_hook.lua
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", env.value("PATH") + ";C:/Users/19588/anaconda3/Scripts");
    proc->setProcessEnvironment(env);

    QStringList args;
    args << "--referrer=https://www.bilibili.com/"
         << "--ytdl-format=bestvideo+bestaudio/best"
         << "--force-window=yes"
         << "--keep-open=yes";
    if (!title.isEmpty())
        args << ("--title=" + title);
    args << urls;

    proc->start(MPV_PATH, args);
}
