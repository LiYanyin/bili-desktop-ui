#include "BiliLogin.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

BiliLogin::BiliLogin(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_pollTimer(new QTimer(this))
{
    connect(m_pollTimer, &QTimer::timeout, this, &BiliLogin::pollStatus);
}

void BiliLogin::startLogin()
{
    m_pollCount = 0;
    m_sessdata.clear();

    QNetworkRequest req(QUrl("https://passport.bilibili.com/x/passport-login/web/qrcode/generate"));
    req.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");

    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit loginFailed("Network error: " + reply->errorString());
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();
        if (root["code"].toInt() != 0) {
            emit loginFailed("API error: " + root["message"].toString());
            return;
        }

        QJsonObject data = root["data"].toObject();
        m_qrcodeKey = data["qrcode_key"].toString();
        QString qrUrl = data["url"].toString();

        // Load QR code as image from online QR generator
        QString qrImgUrl = "https://api.qrserver.com/v1/create-qr-code/?size=220x220&data="
                         + QUrl::toPercentEncoding(qrUrl);
        loadQrImage(qrImgUrl);

        emit statusChanged("请用B站App扫码登录");
        m_pollTimer->start(2000); // poll every 2s
    });
}

void BiliLogin::loadQrImage(const QString &qrImgUrl)
{
    QUrl u(qrImgUrl);
    QNetworkRequest req(u);
    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            QPixmap pm;
            pm.loadFromData(reply->readAll());
            if (!pm.isNull())
                emit qrImageReady(pm);
        }
    });
}

void BiliLogin::pollStatus()
{
    m_pollCount++;
    if (m_pollCount > 90) { // 3 minutes timeout
        m_pollTimer->stop();
        emit loginFailed("登录超时，请重试");
        return;
    }

    QUrl url("https://passport.bilibili.com/x/passport-login/web/qrcode/poll");
    QUrlQuery query;
    query.addQueryItem("qrcode_key", m_qrcodeKey);
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
    req.setRawHeader("Referer", "https://www.bilibili.com/");

    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();
        int code = root["data"].toObject()["code"].toInt();

        switch (code) {
        case 0: { // Success
            m_pollTimer->stop();

            // Extract SESSDATA from response headers
            auto headers = reply->rawHeaderPairs();
            QString cookies;
            for (const auto &h : headers) {
                if (h.first.toLower() == "set-cookie") {
                    QString cookie = h.second;
                    int semi = cookie.indexOf(';');
                    if (semi > 0) cookie = cookie.left(semi);
                    if (cookie.startsWith("SESSDATA=")) {
                        m_sessdata = cookie.mid(9);
                    }
                    if (!cookies.isEmpty()) cookies += "; ";
                    cookies += cookie;
                }
            }

            // Also try parsing from the response body
            if (m_sessdata.isEmpty()) {
                QString refreshToken = root["data"].toObject()["refresh_token"].toString();
                Q_UNUSED(refreshToken);
            }

            emit statusChanged("登录成功！");
            emit loginSuccess();
            break;
        }
        case 86038: // Expired
            m_pollTimer->stop();
            emit loginFailed("二维码已过期，请重新获取");
            break;
        case 86090: // Scanned, waiting confirm
            emit statusChanged("已扫码，请在手机上确认...");
            break;
        case 86101: // Not scanned
            // Keep polling
            break;
        default:
            break;
        }
    });
}
