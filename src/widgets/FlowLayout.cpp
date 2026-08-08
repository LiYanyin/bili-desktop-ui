#include "FlowLayout.h"

#include <QWidget>
#include <QStyle>
#include <QPointer>
#include <QVariantAnimation>
#include <QTimer>

FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    while (auto *item = takeAt(0)) {
        delete item;
    }
}

void FlowLayout::addItem(QLayoutItem *item)
{
    m_itemList.append(item);
}

int FlowLayout::horizontalSpacing() const
{
    if (m_hSpace >= 0) {
        return m_hSpace;
    }
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const
{
    if (m_vSpace >= 0) {
        return m_vSpace;
    }
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int FlowLayout::count() const
{
    return m_itemList.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    if (index >= 0 && index < m_itemList.size()) {
        return m_itemList.at(index);
    }
    return nullptr;
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index >= 0 && index < m_itemList.size()) {
        return m_itemList.takeAt(index);
    }
    return nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return {}; // no expanding directions — fixed size items
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    return doLayoutCalc(QRect(0, 0, width, 0));
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    m_animTasks.clear();
    doLayoutApply(rect);
    if (!m_animTasks.isEmpty())
        QTimer::singleShot(100, this, &FlowLayout::runAnimation);
}

void FlowLayout::runAnimation()
{
    auto tasks = m_animTasks;
    m_animTasks.clear();
    if (tasks.isEmpty()) return;

    // Flash widgets back to pre-layout positions
    for (const auto &t : tasks) {
        if (t.widget) {
            t.widget->move(t.oldPos);
            t.widget->resize(t.newSize);
        }
    }

    auto *anim = new QVariantAnimation;
    anim->setDuration(300);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);

    QObject::connect(anim, &QVariantAnimation::valueChanged, [tasks](const QVariant &v) {
        float t = v.toFloat();
        for (const auto &task : tasks) {
            if (task.widget) {
                task.widget->move(
                    task.oldPos.x() + static_cast<int>((task.newPos.x() - task.oldPos.x()) * t),
                    task.oldPos.y() + static_cast<int>((task.newPos.y() - task.oldPos.y()) * t));
            }
        }
    });

    QObject::connect(anim, &QVariantAnimation::finished, [tasks]() {
        for (const auto &t : tasks) {
            if (t.widget) t.widget->move(t.newPos);
        }
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (const auto *item : m_itemList) {
        size = size.expandedTo(item->minimumSize());
    }
    const QMargins margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

int FlowLayout::doLayoutCalc(const QRect &rect) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(left, top, -right, -bottom);
    int x = effectiveRect.x(), y = effectiveRect.y(), lineHeight = 0;
    int hSpace = horizontalSpacing(), vSpace = verticalSpacing();

    for (auto *item : m_itemList) {
        QSize sz = item->sizeHint();
        int nextX = x + sz.width() + hSpace;
        if (nextX - hSpace > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x(); y = y + lineHeight + vSpace;
            nextX = x + sz.width() + hSpace; lineHeight = 0;
        }
        x = nextX;
        lineHeight = qMax(lineHeight, sz.height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::doLayoutApply(const QRect &rect)
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(left, top, -right, -bottom);
    int x = effectiveRect.x(), y = effectiveRect.y(), lineHeight = 0;
    int hSpace = horizontalSpacing(), vSpace = verticalSpacing();

    for (auto *item : m_itemList) {
        QSize sz = item->sizeHint();
        int nextX = x + sz.width() + hSpace;
        if (nextX - hSpace > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x(); y = y + lineHeight + vSpace;
            nextX = x + sz.width() + hSpace; lineHeight = 0;
        }

        QWidget *w = item->widget();
        QRect target(QPoint(x, y), sz);
        if (w) {
            QPoint oldPos = w->pos();
            w->setGeometry(target);
            if (oldPos != target.topLeft()) {
                AnimTask t;
                t.widget = w;
                t.oldPos = oldPos;
                t.newPos = target.topLeft();
                t.newSize = target.size();
                m_animTasks.append(t);
            }
        } else {
            item->setGeometry(target);
        }

        x = nextX;
        lineHeight = qMax(lineHeight, sz.height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *p = parent();
    if (!p) {
        return -1;
    }
    if (p->isWidgetType()) {
        auto *pw = static_cast<QWidget *>(p);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    }
    return static_cast<QLayout *>(p)->spacing();
}
