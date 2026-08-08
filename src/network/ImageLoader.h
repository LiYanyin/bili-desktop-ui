#pragma once

#include <QObject>
#include <QPixmap>
#include <QHash>
#include <QNetworkAccessManager>

class ImageLoader : public QObject
{
    Q_OBJECT

public:
    static ImageLoader *instance();

    /// Load image from URL. Returns cached pixmap if available, otherwise
    /// starts async download and emits imageReady when done.
    void load(const QString &url, QObject *requester,
              std::function<void(const QPixmap &)> callback);

signals:
    void imageReady(const QString &url, const QPixmap &pixmap);

private:
    explicit ImageLoader(QObject *parent = nullptr);
    QNetworkAccessManager *m_nam;
    QHash<QString, QPixmap> m_cache;
    QHash<QString, QList<std::function<void(const QPixmap &)>>> m_pending;
};
