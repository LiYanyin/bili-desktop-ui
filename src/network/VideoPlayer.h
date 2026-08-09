#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QProcess>

class VideoPlayer : public QObject
{
    Q_OBJECT

public:
    explicit VideoPlayer(QObject *parent = nullptr);

    /// Start playing a video by BV ID. Launches mpv.exe in a separate window.
    void play(const QString &bvid, const QString &title = {});

private:
    void fetchCid(const QString &bvid);
    void fetchPlayUrl(const QString &bvid, int cid);
    void launchMpv(const QStringList &urls, const QString &title);

    QNetworkAccessManager *m_nam;
};
