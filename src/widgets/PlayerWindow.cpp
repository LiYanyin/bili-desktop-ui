#include "PlayerWindow.h"
#include "MpvWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCloseEvent>

PlayerWindow::PlayerWindow(QWidget *parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("Bili Desktop — 播放");
    resize(1400, 820);
    setMinimumSize(1000, 600);
    setStyleSheet("background: #1A1A28;");

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Left: video area
    m_mpv = new MpvWidget(this);
    m_mpv->setMinimumSize(640, 360);
    root->addWidget(m_mpv, 3);

    // Right: comment panel
    auto *right = new QWidget(this);
    auto *rlay = new QVBoxLayout(right);
    rlay->setContentsMargins(0, 0, 0, 0);

    auto *header = new QLabel("  评论区 (弹幕开发中)", right);
    header->setStyleSheet("color: #AAA; font-size: 13px; padding: 10px; background: #0F0F1A;");
    header->setFixedHeight(40);

    m_comments = new QTextEdit(right);
    m_comments->setReadOnly(true);
    m_comments->setStyleSheet(R"(
        QTextEdit {
            background: #141423;
            color: #CCC;
            border: none;
            font-size: 13px;
            padding: 12px;
        }
    )");
    m_comments->setHtml("<p style='color:#888; text-align:center; margin-top:40px;'>"
                        "评论区加载中...<br>"
                        "<span style='font-size:12px;'>弹幕和评论功能即将上线</span></p>");

    rlay->addWidget(header);
    rlay->addWidget(m_comments, 1);
    root->addWidget(right, 1);
}

void PlayerWindow::play(const QString &bvid, const QString &title, const QString &comment)
{
    setWindowTitle(title.isEmpty() ? "Bili Desktop — 播放" : title);
    m_mpv->play(bvid, title);
    Q_UNUSED(comment);
}

void PlayerWindow::closeEvent(QCloseEvent *e)
{
    QWidget::closeEvent(e);
}
