#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include "../widgets/VideoData.h"

class BiliApi : public QObject
{
    Q_OBJECT

public:
    explicit BiliApi(QObject *parent = nullptr);

    /// Fetch popular videos from Bilibili API
    /// @param page  page number (1-based)
    /// @param count videos per page (max ~50)
    void fetchPopular(int page = 1, int count = 30);

signals:
    void videosReady(const QList<VideoData> &videos);
    void fetchError(const QString &error);

private:
    static VideoData parseVideo(const QJsonObject &obj);
    static QString formatCount(int count);
    static QString formatDuration(int seconds);
    static QString formatTime(qint64 unixTime);

    QNetworkAccessManager *m_nam;
};
