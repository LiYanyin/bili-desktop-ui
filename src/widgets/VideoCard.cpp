#include "VideoCard.h"
#include "../network/ImageLoader.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QGraphicsDropShadowEffect>

static constexpr int CARD_WIDTH = 280;
static constexpr int COVER_HEIGHT = 158; // 16:9 (280*9/16)
static constexpr float SCALE_AMOUNT = 1.04f;

VideoCard::VideoCard(const VideoData &data, QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(CARD_WIDTH);
    setupUi();
    setData(data);

    // Set up hover animation
    m_hoverAnimation = new QPropertyAnimation(this, "hoverProgress", this);
    m_hoverAnimation->setDuration(180);
    m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);
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

    // ── Title ──
    m_titleLabel = new QLabel(this);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setMaximumHeight(40);
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

    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(22, 22);
    m_avatarLabel->setScaledContents(true);
    m_avatarLabel->setStyleSheet("QLabel { border-radius: 11px; }");

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

    // ── Stats ──
    m_statsLabel = new QLabel(this);
    m_statsLabel->setStyleSheet(R"(
        QLabel {
            color: #888888;
            font-size: 11px;
            background: transparent;
        }
    )");
    root->addWidget(m_statsLabel);

    setCursor(Qt::PointingHandCursor);
}

void VideoCard::setData(const VideoData &data)
{
    m_data = data;

    // Cover: colored placeholder first, then load real image
    {
        uint hash = qHash(data.title);
        QColor coverColor = QColor::fromHsl(hash % 360, 180, 100 + (hash % 60));
        QPixmap placePix(CARD_WIDTH, COVER_HEIGHT);
        QPainter p(&placePix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(coverColor);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(0, 0, CARD_WIDTH, COVER_HEIGHT, 8, 8);
        m_coverLabel->setPixmap(placePix);
    }

    // Async load real cover from B站 CDN
    QString bvid = data.bvid;
    ImageLoader::instance()->load(data.coverPath, this,
        [this, bvid](const QPixmap &pm) {
            if (m_data.bvid == bvid && !pm.isNull())
                m_coverLabel->setPixmap(pm);
        });

    // Duration
    m_durationLabel->setText(data.duration);
    m_durationLabel->adjustSize();
    int durW = m_durationLabel->width();
    m_durationLabel->move(CARD_WIDTH - durW - 8, COVER_HEIGHT - 24);

    // Title
    m_titleLabel->setText(data.title);
    m_titleLabel->setToolTip(data.title);

    // Uploader
    m_uploaderLabel->setText(data.uploaderName);

    // Avatar: colored placeholder + async load real avatar
    {
        QPixmap avaPix(22, 22);
        avaPix.fill(Qt::transparent);
        QPainter p(&avaPix);
        p.setRenderHint(QPainter::Antialiasing);
        uint nameHash = qHash(data.uploaderName);
        p.setBrush(QColor::fromHsl(nameHash % 360, 160, 140));
        p.setPen(Qt::NoPen);
        p.drawEllipse(0, 0, 22, 22);
        m_avatarLabel->setPixmap(avaPix);
    }
    // Async load real avatar
    if (!data.uploaderAvatarPath.isEmpty()) {
        QString name = data.uploaderName;
        ImageLoader::instance()->load(data.uploaderAvatarPath, this,
            [this, name](const QPixmap &pm) {
                if (m_data.uploaderName == name && !pm.isNull()) {
                    QPixmap rounded(22, 22);
                    rounded.fill(Qt::transparent);
                    QPainter p(&rounded);
                    p.setRenderHint(QPainter::Antialiasing);
                    QPainterPath path;
                    path.addEllipse(0, 0, 22, 22);
                    p.setClipPath(path);
                    p.drawPixmap(0, 0, pm.scaled(22, 22, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    m_avatarLabel->setPixmap(rounded);
                }
            });
    }

    // Stats
    m_statsLabel->setText(QString("%1 · %2").arg(data.playCount, data.publishTime));
}

void VideoCard::setHoverProgress(float progress)
{
    m_hoverProgress = progress;
    update(); // trigger repaint with new scale/shadow
}

void VideoCard::enterEvent(QEnterEvent *event)
{
    m_hoverAnimation->stop();
    m_hoverAnimation->setStartValue(m_hoverProgress);
    m_hoverAnimation->setEndValue(1.0f);
    m_hoverAnimation->start();
    QWidget::enterEvent(event);
}

void VideoCard::leaveEvent(QEvent *event)
{
    m_hoverAnimation->stop();
    m_hoverAnimation->setStartValue(m_hoverProgress);
    m_hoverAnimation->setEndValue(0.0f);
    m_hoverAnimation->start();
    QWidget::leaveEvent(event);
}

void VideoCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    float scale = 1.0f + (SCALE_AMOUNT - 1.0f) * m_hoverProgress;

    // Calculate scaled rect centered in widget
    QRectF cardRect = rect().adjusted(1, 1, -1, -1);
    QPointF center = cardRect.center();
    QSizeF scaledSize(cardRect.width() * scale, cardRect.height() * scale);
    QRectF scaledRect(QPointF(0, 0), scaledSize);
    scaledRect.moveCenter(center);

    // ── Shadow ──
    int shadowAlpha = static_cast<int>(40 + 50 * m_hoverProgress);
    int shadowOffset = static_cast<int>(4 + 6 * m_hoverProgress);
    int shadowBlur = static_cast<int>(12 + 16 * m_hoverProgress);

    for (int i = 0; i < shadowBlur; ++i) {
        float t = static_cast<float>(i) / shadowBlur;
        int alpha = static_cast<int>(shadowAlpha * (1.0f - t) * 0.4f);
        QColor shadowColor(0, 0, 0, alpha);
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(shadowColor, 1));
        QRectF shadowRect = scaledRect.translated(0, shadowOffset + i * 0.5);
        p.drawRoundedRect(shadowRect, 10, 10);
    }

    // ── Card body ──
    p.setBrush(QColor(30, 30, 48, 180));
    p.setPen(QPen(QColor(255, 255, 255, static_cast<int>(12 + 8 * m_hoverProgress)), 1));
    p.drawRoundedRect(scaledRect, 10, 10);
}

void VideoCard::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && rect().contains(event->pos())) {
        emit clicked();
        // Open in browser
        if (!m_data.bvid.isEmpty()) {
            QDesktopServices::openUrl(QUrl("https://www.bilibili.com/video/" + m_data.bvid));
        }
    }
    QWidget::mouseReleaseEvent(event);
}
