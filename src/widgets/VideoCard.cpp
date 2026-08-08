#include "VideoCard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QFontMetrics>
#include <QEnterEvent>
#include <QGraphicsDropShadowEffect>

static constexpr int CARD_WIDTH = 300;
static constexpr int COVER_HEIGHT = 169; // 16:9

VideoCard::VideoCard(const VideoData &data, QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(CARD_WIDTH);
    setupUi();
    setData(data);
}

void VideoCard::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(6);

    // ── Cover image (16:9 placeholder) ──
    m_coverLabel = new QLabel(this);
    m_coverLabel->setFixedSize(CARD_WIDTH, COVER_HEIGHT);
    m_coverLabel->setScaledContents(true);
    m_coverLabel->setStyleSheet(R"(
        QLabel {
            border-radius: 8px;
            background: #2A2A3E;
        }
    )");

    // Duration overlay on top of cover
    m_durationLabel = new QLabel(m_coverLabel);
    m_durationLabel->setStyleSheet(R"(
        QLabel {
            background: rgba(0, 0, 0, 0.75);
            color: white;
            font-size: 11px;
            padding: 2px 6px;
            border-radius: 3px;
        }
    )");
    m_durationLabel->move(CARD_WIDTH - 52, COVER_HEIGHT - 22);
    m_durationLabel->setFixedHeight(18);

    root->addWidget(m_coverLabel);

    // ── Title (max 2 lines, ellipsis) ──
    m_titleLabel = new QLabel(this);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setMaximumHeight(40); // ~2 lines at 14px
    m_titleLabel->setStyleSheet(R"(
        QLabel {
            color: #E0E0E0;
            font-size: 13px;
            line-height: 1.4;
            background: transparent;
        }
    )");
    root->addWidget(m_titleLabel);

    // ── Bottom row: avatar + uploader name ──
    auto *bottomRow = new QHBoxLayout();
    bottomRow->setContentsMargins(0, 0, 0, 0);
    bottomRow->setSpacing(8);

    // Avatar placeholder (small circle)
    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(22, 22);
    m_avatarLabel->setScaledContents(true);
    QPixmap avatarPix(22, 22);
    avatarPix.fill(QColor("#555566"));
    m_avatarLabel->setPixmap(avatarPix);
    m_avatarLabel->setStyleSheet("QLabel { border-radius: 11px; }");

    // Uploader name
    m_uploaderLabel = new QLabel(this);
    m_uploaderLabel->setStyleSheet(R"(
        QLabel {
            color: #999999;
            font-size: 12px;
            background: transparent;
        }
    )");

    bottomRow->addWidget(m_avatarLabel);
    bottomRow->addWidget(m_uploaderLabel);
    bottomRow->addStretch();

    root->addLayout(bottomRow);

    // ── Stats: play count + publish time ──
    m_statsLabel = new QLabel(this);
    m_statsLabel->setStyleSheet(R"(
        QLabel {
            color: #888888;
            font-size: 11px;
            background: transparent;
        }
    )");
    root->addWidget(m_statsLabel);

    // Card background with subtle border
    setStyleSheet(R"(
        VideoCard {
            background: #1E1E30;
            border-radius: 10px;
            padding: 0px;
        }
    )");

    // Set cursor
    setCursor(Qt::PointingHandCursor);
}

void VideoCard::setData(const VideoData &data)
{
    m_data = data;

    // Cover placeholder (colored rectangle)
    // Use a hash of the title to pick a consistent color
    uint hash = qHash(data.title);
    QColor coverColor = QColor::fromHsl(hash % 360, 180, 100 + (hash % 60));
    QPixmap coverPix(CARD_WIDTH, COVER_HEIGHT);
    {
        QPainter p(&coverPix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(coverColor);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(0, 0, CARD_WIDTH, COVER_HEIGHT, 8, 8);
        // Draw a subtle play icon in the center
        p.setBrush(QColor(255, 255, 255, 40));
        QPolygonF triangle;
        triangle << QPointF(CARD_WIDTH / 2 - 18, COVER_HEIGHT / 2 - 22)
                 << QPointF(CARD_WIDTH / 2 - 18, COVER_HEIGHT / 2 + 22)
                 << QPointF(CARD_WIDTH / 2 + 20, COVER_HEIGHT / 2);
        p.drawPolygon(triangle);
    }
    m_coverLabel->setPixmap(coverPix);

    // Duration
    m_durationLabel->setText(data.duration);
    m_durationLabel->adjustSize();
    // Position duration at bottom-right of cover
    int durW = m_durationLabel->width();
    m_durationLabel->move(CARD_WIDTH - durW - 8, COVER_HEIGHT - 24);

    // Title (truncate to 2 lines)
    m_titleLabel->setText(data.title);
    m_titleLabel->setToolTip(data.title);

    // Uploader
    m_uploaderLabel->setText(data.uploaderName);

    // Avatar placeholder (colored circle based on name hash)
    QPixmap avaPix(22, 22);
    avaPix.fill(Qt::transparent);
    {
        QPainter p(&avaPix);
        p.setRenderHint(QPainter::Antialiasing);
        uint nameHash = qHash(data.uploaderName);
        p.setBrush(QColor::fromHsl(nameHash % 360, 160, 140));
        p.setPen(Qt::NoPen);
        p.drawEllipse(0, 0, 22, 22);
    }
    m_avatarLabel->setPixmap(avaPix);

    // Stats
    m_statsLabel->setText(QString("%1 · %2").arg(data.playCount, data.publishTime));
}

void VideoCard::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    // Animation and shadow will be added in step 8
    QWidget::enterEvent(event);
}

void VideoCard::leaveEvent(QEvent *event)
{
    m_hovered = false;
    QWidget::leaveEvent(event);
}

void VideoCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    // Draw card with subtle shadow/border
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor("#1E1E30"));
    p.setPen(QPen(QColor(255, 255, 255, 12), 1));
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);
}
