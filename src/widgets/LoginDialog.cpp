#include "LoginDialog.h"

#include <QVBoxLayout>
#include <QTimer>
#include <QPushButton>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , m_login(new BiliLogin(this))
{
    setWindowTitle("B站扫码登录");
    setFixedSize(340, 420);
    setStyleSheet("QDialog { background: #1E1E30; }");

    auto *lay = new QVBoxLayout(this);
    lay->setSpacing(12);

    // QR code image
    m_qrLabel = new QLabel(this);
    m_qrLabel->setFixedSize(240, 240);
    m_qrLabel->setAlignment(Qt::AlignCenter);
    m_qrLabel->setStyleSheet("background: white; border-radius: 8px;");
    lay->addWidget(m_qrLabel, 0, Qt::AlignCenter);

    // Status text
    m_statusLabel = new QLabel("正在获取二维码...", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #AAA; font-size: 14px;");
    lay->addWidget(m_statusLabel);

    // Success indicator
    m_successLabel = new QLabel(this);
    m_successLabel->setAlignment(Qt::AlignCenter);
    m_successLabel->setStyleSheet("color: #6DC781; font-size: 14px; font-weight: bold;");
    m_successLabel->setVisible(false);
    lay->addWidget(m_successLabel);

    lay->addStretch();

    // Close button
    auto *closeBtn = new QPushButton("取消", this);
    closeBtn->setStyleSheet(R"(
        QPushButton { background: transparent; border: 1px solid #555; border-radius: 6px; color: #888; padding: 6px; }
        QPushButton:hover { background: rgba(255,255,255,0.05); color: #CCC; }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    lay->addWidget(closeBtn);

    // Connect login signals
    connect(m_login, &BiliLogin::qrImageReady, this, [this](const QPixmap &pm) {
        m_qrLabel->setPixmap(pm.scaled(240, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    });

    connect(m_login, &BiliLogin::statusChanged, m_statusLabel, &QLabel::setText);

    connect(m_login, &BiliLogin::loginFailed, this, [this](const QString &err) {
        m_statusLabel->setText("失败: " + err);
        m_statusLabel->setStyleSheet("color: #E81123; font-size: 14px;");
    });

    connect(m_login, &BiliLogin::loginSuccess, this, [this]() {
        m_successLabel->setText("登录成功!");
        m_successLabel->setVisible(true);
        m_statusLabel->setVisible(false);
        emit loginSuccess(m_login->sessdata());
        QTimer::singleShot(1000, this, &QDialog::accept);
    });

    m_login->startLogin();
}

QString LoginDialog::sessdata() const
{
    return m_login->sessdata();
}
