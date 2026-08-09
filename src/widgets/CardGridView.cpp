#include "CardGridView.h"

#include <QVBoxLayout>
#include <QVariantAnimation>
#include <QScrollBar>
#include <QResizeEvent>
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
    m_scrollTopBtn = new QPushButton("▲", this);
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
    m_scrollTopBtn->hide();

    connect(m_scrollTopBtn, &QPushButton::clicked, this, &CardGridView::scrollToTop);
    connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &CardGridView::onScrollChanged);
}

void CardGridView::onScrollChanged(int value)
{
    m_scrollTopBtn->setVisible(value > 200);
}

void CardGridView::scrollToTop()
{
    auto *sb = m_view->verticalScrollBar();
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
    if (m_activeAnim) {
        m_activeAnim->stop();
        delete m_activeAnim;
        m_activeAnim = nullptr;
    }

    for (auto &entry : m_entries) {
        m_scene->removeItem(entry.proxy);
        delete entry.proxy;
    }
    m_entries.clear();

    for (const auto &data : cards) {
        auto *card = new VideoCard(data);
        card->setFixedSize(CARD_W, card->sizeHint().height());
        auto *proxy = m_scene->addWidget(card);
        proxy->setVisible(true);
        m_entries.append({card, proxy});
    }

    layoutCards(false);
}

void CardGridView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_scrollTopBtn) {
        m_scrollTopBtn->move(width() - 64, height() - 64);
        m_scrollTopBtn->raise();
    }
    layoutCards(true);
}

void CardGridView::layoutCards(bool animate)
{
    if (m_entries.isEmpty()) return;

    int viewW = m_view->viewport()->width();
    if (viewW <= 0) viewW = width();
    // Fallback: walk up to find any parent with a known width
    if (viewW <= 0) {
        QWidget *p = parentWidget();
        while (p && viewW <= 0) {
            viewW = p->width();
            p = p->parentWidget();
        }
    }
    if (viewW <= 0) viewW = 960; // default: 1160 - 200 sidebar
    int usableW = viewW - MARGIN * 2;

    int usableW = viewW - MARGIN * 2;
    int cols = qMax(1, (usableW + CARD_GAP) / (CARD_W + CARD_GAP));
    int cardH = m_entries.first().card->sizeHint().height();
    int rowH = cardH + CARD_GAP;

    struct Target { QGraphicsProxyWidget *proxy; QPointF pos; };
    QList<Target> targets;

    for (int i = 0; i < m_entries.size(); ++i) {
        int col = i % cols;
        int row = i / cols;
        int x = MARGIN + col * (CARD_W + CARD_GAP);
        int y = MARGIN + row * rowH;

        targets.append({m_entries[i].proxy, QPointF(x, y)});
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
        struct Move { QGraphicsProxyWidget *proxy; QPointF start; QPointF end; };
        QList<Move> moves;
        for (const auto &t : targets) {
            moves.append({t.proxy, t.proxy->pos(), t.pos});
        }
        m_activeAnim = new QVariantAnimation(this);
        m_activeAnim->setDuration(250);
        m_activeAnim->setEasingCurve(QEasingCurve::OutCubic);
        m_activeAnim->setStartValue(0.0);
        m_activeAnim->setEndValue(1.0);
        QObject::connect(m_activeAnim, &QVariantAnimation::valueChanged, [moves](const QVariant &v) {
            float t = v.toFloat();
            for (const auto &m : moves) {
                m.proxy->setPos(m.start.x() + (m.end.x() - m.start.x()) * t,
                                m.start.y() + (m.end.y() - m.start.y()) * t);
            }
        });
        QObject::connect(m_activeAnim, &QVariantAnimation::finished, this, [this]() {
            m_activeAnim = nullptr;
        });
        m_activeAnim->start();
    } else {
        for (const auto &t : targets)
            t.proxy->setPos(t.pos);
    }
}
