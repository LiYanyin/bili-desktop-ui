#include "CardGridView.h"
#include "PlayerWindow.h"

#include <QVBoxLayout>
#include <QVariantAnimation>
#include <QScrollBar>
#include <QResizeEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QUrl>

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

    m_view->viewport()->installEventFilter(this);

    // ── Refresh + Scroll buttons as viewport children (proven working) ──
    auto *refreshBtn = new QPushButton("↻ 刷新", m_view->viewport());
    refreshBtn->setFixedSize(72, 30);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(251, 114, 153, 0.25);
            border: 1px solid rgba(251, 114, 153, 0.3);
            border-radius: 6px;
            color: #FB7299; font-size: 12px;
        }
        QPushButton:hover { background: rgba(251, 114, 153, 0.5); }
    )");
    connect(refreshBtn, &QPushButton::clicked, this, &CardGridView::refreshRequested);

    auto *scrollBtn = new QPushButton("▲", m_view->viewport());
    scrollBtn->setFixedSize(44, 44);
    scrollBtn->setCursor(Qt::PointingHandCursor);
    scrollBtn->setToolTip("回到顶部");
    scrollBtn->setStyleSheet(R"(
        QPushButton { background: rgba(30,30,50,0.85); border: 1px solid rgba(255,255,255,0.12); border-radius: 22px; color: #CCC; font-size: 18px; }
        QPushButton:hover { background: rgba(251,114,153,0.7); border-color: rgba(251,114,153,0.5); color: #fff; }
    )");
    scrollBtn->hide();
    connect(scrollBtn, &QPushButton::clicked, this, [this]() { m_view->verticalScrollBar()->setValue(0); });

    // Store as QWidget* for resize positioning
    m_refreshBtn2 = refreshBtn;
    m_scrollBtn2 = scrollBtn;

    connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged, this, [scrollBtn](int v) {
        if (v > 200) { scrollBtn->show(); scrollBtn->raise(); }
        else scrollBtn->hide();
    });
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

    // Position viewport-child buttons
    if (m_refreshBtn2) {
        int vw = m_view->viewport()->width();
        int vh = m_view->viewport()->height();
        m_refreshBtn2->move(vw - 84, 8);
        m_refreshBtn2->raise();
        m_scrollBtn2->move(vw - 58, vh - 58);
        m_scrollBtn2->raise();
    }

    layoutCards(true);
}

void CardGridView::layoutCards(bool animate)
{
    if (m_entries.isEmpty()) return;

    int viewW = m_view->viewport()->width();
    if (viewW <= 0) viewW = width();
    if (viewW <= 0) return;

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
        if (m_activeAnim) { m_activeAnim->stop(); delete m_activeAnim; }
        struct Move { QGraphicsProxyWidget *proxy; QPointF start; QPointF end; };
        QList<Move> moves;
        for (const auto &t : targets)
            moves.append({t.proxy, t.proxy->pos(), t.pos});

        m_activeAnim = new QVariantAnimation(this);
        m_activeAnim->setDuration(250);
        m_activeAnim->setEasingCurve(QEasingCurve::OutCubic);
        m_activeAnim->setStartValue(0.0);
        m_activeAnim->setEndValue(1.0);

        QObject::connect(m_activeAnim, &QVariantAnimation::valueChanged, [moves](const QVariant &v) {
            float t = v.toFloat();
            for (const auto &m : moves)
                m.proxy->setPos(m.start.x() + (m.end.x() - m.start.x()) * t,
                                m.start.y() + (m.end.y() - m.start.y()) * t);
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

bool CardGridView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_view->viewport() && event->type() == QEvent::MouseButtonRelease) {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            QPointF scenePos = m_view->mapToScene(me->pos());
            QGraphicsItem *item = m_scene->itemAt(scenePos, QTransform());
            if (item) {
                while (item && item->type() != QGraphicsProxyWidget::Type)
                    item = item->parentItem();
                if (auto *pw = dynamic_cast<QGraphicsProxyWidget *>(item)) {
                    if (auto *card = qobject_cast<VideoCard *>(pw->widget())) {
                        QString bvid = card->data().bvid;
                        if (!bvid.isEmpty()) {
                            auto *pw2 = new PlayerWindow();
                            pw2->setAttribute(Qt::WA_DeleteOnClose);
                            pw2->play(bvid, card->data().title);
                            pw2->show();
                        }
                    }
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}
