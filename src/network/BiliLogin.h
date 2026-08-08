#pragma once

#include <QObject>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QTimer>

class BiliLogin : public QObject
{
    Q_OBJECT

public:
    explicit BiliLogin(QObject *parent = nullptr);

    /// Start QR code login flow. emits qrCodeUrl with the image URL for the QR code.
    void startLogin();

    /// User credentials after successful login
    QString sessdata() const { return m_sessdata; }
    bool isLoggedIn() const { return !m_sessdata.isEmpty(); }

signals:
    void qrCodeUrl(const QString &url);      // QR code data URL to display
    void qrImageReady(const QPixmap &pixmap); // QR code as pixmap
    void statusChanged(const QString &status); // Human-readable status
    void loginSuccess();
    void loginFailed(const QString &error);

private:
    void pollStatus();
    void loadQrImage(const QString &qrDataUrl);

    QNetworkAccessManager *m_nam;
    QTimer *m_pollTimer;
    QString m_qrcodeKey;
    QString m_sessdata;
    int m_pollCount = 0;
};
