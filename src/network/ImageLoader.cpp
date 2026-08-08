#include "ImageLoader.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmapCache>

ImageLoader *ImageLoader::instance()
{
    static ImageLoader s;
    return &s;
}

ImageLoader::ImageLoader(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    QPixmapCache::setCacheLimit(64 * 1024); // 64 MB pixmap cache
}

void ImageLoader::load(const QString &url, QObject *requester,
                        std::function<void(const QPixmap &)> callback)
{
    if (url.isEmpty()) {
        QPixmap empty(280, 158);
        empty.fill(QColor("#2A2A3E"));
        callback(empty);
        return;
    }

    // Check Qt pixmap cache first
    QPixmap cached;
    if (QPixmapCache::find(url, &cached)) {
        callback(cached);
        return;
    }

    // Check in-memory cache
    if (m_cache.contains(url)) {
        callback(m_cache[url]);
        return;
    }

    // Check if already downloading
    if (m_pending.contains(url)) {
        m_pending[url].append(callback);
        return;
    }

    // Start download — rewrite http→https for B站 CDN
    m_pending[url] = {callback};

    QString httpsUrl = url;
    if (httpsUrl.startsWith("http://"))
        httpsUrl.replace(0, 4, "https");
    QUrl qurl(httpsUrl);
    QNetworkRequest req(qurl);
    req.setRawHeader("Referer", "https://www.bilibili.com/");
    req.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");

    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, url, reply]() {
        reply->deleteLater();
        QPixmap pix;
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            pix.loadFromData(data);
            if (!pix.isNull()) {
                // Scale to card cover size (280x158)
                pix = pix.scaled(280, 158, Qt::KeepAspectRatioByExpanding,
                                 Qt::SmoothTransformation);
                m_cache[url] = pix;
                QPixmapCache::insert(url, pix);
            }
        }
        if (pix.isNull()) {
            // Fallback: colored placeholder
            pix = QPixmap(280, 158);
            pix.fill(QColor("#2A2A3E"));
        }

        auto callbacks = m_pending.take(url);
        for (auto &cb : callbacks)
            cb(pix);
    });
}
