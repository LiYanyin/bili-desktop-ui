#include "CardGridView.h"

#include <QVBoxLayout>
#include <QVariantAnimation>
#include <QScrollBar>
#include <QGraphicsOpacityEffect>
#include <QResizeEvent>
#include <QShowEvent>
#include <QTimer>
#include <QPixmap>

static constexpr int CARD_W = 280;
static constexpr int CARD_GAP = 12;
static constexpr int MARGIN = 16;

CardGridView::CardGridView(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene, this);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    m_view->setBackgroundBrush(Qt::transparent);
    m_view->setStyleSheet("QGraphicsView { background: transparent; border: none; }");
    m_view->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    layout->addWidget(m_view);

    setupScrollToTopButton();
}

void CardGridView::setupScrollToTopButton()
{
    m_scrollTopBtn = new QPushButton(this);
    m_scrollTopBtn->setFixedSize(44, 44);
    m_scrollTopBtn->setCursor(Qt::PointingHandCursor);
    m_scrollTopBtn->setToolTip("回到顶部");
    m_scrollTopBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(30, 30, 50, 0.85);
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 22px;
            color: #CCCCCC;
            font-size: 18px;
        }
        QPushButton:hover {
            background: rgba(251, 114, 153, 0.7);
            border-color: rgba(251, 114, 153, 0.5);
            color: white;
        }
    )");
    m_scrollTopBtn->setText("▲");

    // Start hidden
    auto *opacity = new QGraphicsOpacityEffect(m_scrollTopBtn);
    opacity->setOpacity(0.0);
    m_scrollTopBtn->setGraphicsEffect(opacity);

    m_fadeAnim = new QPropertyAnimation(opacity, "opacity", this);
    m_fadeAnim->setDuration(200);

    connect(m_scrollTopBtn, &QPushButton::clicked, this, &CardGridView::scrollToTop);

    // Track scroll position
    connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &CardGridView::onScrollChanged);
}

void CardGridView::onScrollChanged(int value)
{
    auto *opacity = static_cast<QGraphicsOpacityEffect *>(m_scrollTopBtn->graphicsEffect());
    if (!opacity) return;

    bool show = value > 200; // appear after scrolling 200px
    float target = show ? 1.0f : 0.0f;

    if (qFuzzyCompare(double(opacity->opacity()), double(target))) return;

    m_fadeAnim->stop();
    m_fadeAnim->setStartValue(opacity->opacity());
    m_fadeAnim->setEndValue(target);
    m_fadeAnim->start();
}

void CardGridView::scrollToTop()
{
    auto *sb = m_view->verticalScrollBar();
    // Smooth scroll via animation
    auto *anim = new QVariantAnimation(this);
    anim->setDuration(350);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setStartValue(sb->value());
    anim->setEndValue(0);
    connect(anim, &QVariantAnimation::valueChanged, sb, [sb](const QVariant &v) {
        sb->setValue(v.toInt());
    });
    connect(anim, &QVariantAnimation::finished, anim, &QObject::deleteLater);
    anim->start();
}

void CardGridView::setCards(const QList<VideoData> &cards)
{
    for (auto &entry : m_entries) {
        m_scene->removeItem(entry.proxy);
        delete entry.proxy;
        if (entry.pixmapItem) {
            m_scene->removeItem(entry.pixmapItem);
            delete entry.pixmapItem;
        }
    }
    m_entries.clear();

    for (const auto &data : cards) {
        auto *card = new VideoCard(data);
        card->setFixedSize(CARD_W, card->sizeHint().height());

        // Pre-render card to pixmap for lightweight animation
        QPixmap snapshot = card->grab();

        auto *proxy = m_scene->addWidget(card);
        proxy->setVisible(false); // hidden by default, shown after animation

        auto *pixItem = m_scene->addPixmap(snapshot);
        pixItem->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        pixItem->setVisible(true);

        m_entries.append({card, proxy, pixItem});
    }
}

void CardGridView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    QTimer::singleShot(0, this, [this]() { layoutCards(false); });
}

void CardGridView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // Position scroll-to-top button at bottom-right
    int margin = 20;
    m_scrollTopBtn->move(width() - m_scrollTopBtn->width() - margin,
                         height() - m_scrollTopBtn->height() - margin);
    m_scrollTopBtn->raise();
    layoutCards(true);
}

void CardGridView::layoutCards(bool animate)
{
    if (m_entries.isEmpty()) return;

    int viewW = m_view->viewport()->width();
    int usableW = viewW - MARGIN * 2;
    int cols = qMax(1, (usableW + CARD_GAP) / (CARD_W + CARD_GAP));
    int cardH = m_entries.first().card->sizeHint().height();
    int rowH = cardH + CARD_GAP;

    struct Target { QGraphicsItem *item; QPointF pos; };
    QList<Target> targets;

    for (int i = 0; i < m_entries.size(); ++i) {
        int col = i % cols;
        int row = i / cols;
        int x = MARGIN + col * (CARD_W + CARD_GAP);
        int y = MARGIN + row * rowH;

        // Position the pixmap item (always shown during animation)
        m_entries[i].pixmapItem->setPixmap(m_entries[i].card->grab());
        m_entries[i].proxy->setVisible(false);
        m_entries[i].pixmapItem->setVisible(true);
        targets.append({m_entries[i].pixmapItem, QPointF(x, y)});

        // Also remember position for the real widget
        m_entries[i].proxy->setPos(x, y);
        m_entries[i].proxy->resize(CARD_W, cardH);
    }

    int totalRows = (m_entries.size() + cols - 1) / cols;
    int sceneH = MARGIN * 2 + totalRows * rowH - CARD_GAP;
    m_scene->setSceneRect(0, 0, viewW, qMax(sceneH, 100));

    if (animate) {
        if (m_activeAnim) {
            m_activeAnim->stop();
            delete m_activeAnim;
        }
        // Capture current positions as start, targets as end
        struct Move { QGraphicsPixmapItem *item; QPointF start; QPointF end; };
        QList<Move> moves;
        for (const auto &t : targets) {
            auto *pi = static_cast<QGraphicsPixmapItem *>(t.item);
            moves.append({pi, pi->pos(), t.pos});
        }
        m_activeAnim = new QVariantAnimation(this);
        m_activeAnim->setDuration(250);
        m_activeAnim->setEasingCurve(QEasingCurve::OutCubic);
        m_activeAnim->setStartValue(0.0);
        m_activeAnim->setEndValue(1.0);
        QObject::connect(m_activeAnim, &QVariantAnimation::valueChanged, [moves](const QVariant &v) {
            float t = v.toFloat();
            for (const auto &m : moves) {
                m.item->setPos(m.start.x() + (m.end.x() - m.start.x()) * t,
                               m.start.y() + (m.end.y() - m.start.y()) * t);
            }
        });
        QObject::connect(m_activeAnim, &QVariantAnimation::finished, this, [this]() {
            for (auto &e : m_entries) {
                e.pixmapItem->setVisible(false);
                e.proxy->setVisible(true);
            }
            m_activeAnim = nullptr;
        });
        m_activeAnim->start();
    } else {
        for (const auto &t : targets)
            t.item->setPos(t.pos);
        // Show real widgets directly for non-animated layout
        for (auto &e : m_entries) {
            e.pixmapItem->setVisible(false);
            e.proxy->setVisible(true);
        }
    }
}
