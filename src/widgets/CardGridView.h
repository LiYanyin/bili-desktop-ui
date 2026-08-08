#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QVariantAnimation>
#include <QList>
#include "VideoData.h"
#include "VideoCard.h"

class CardGridView : public QWidget
{
    Q_OBJECT

public:
    explicit CardGridView(QWidget *parent = nullptr);
    void setCards(const QList<VideoData> &cards);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    struct CardEntry {
        VideoCard *card;              // real widget (kept for hover etc.)
        QGraphicsProxyWidget *proxy;  // proxy for the real widget
        QGraphicsPixmapItem *pixmapItem; // prerendered snapshot for animation
    };
    void layoutCards(bool animate);
    void showPixmaps();
    void showWidgets();

    QGraphicsView *m_view;
    QGraphicsScene *m_scene;
    QList<CardEntry> m_entries;
    QVariantAnimation *m_activeAnim = nullptr;
    bool m_showingPixmaps = false;
};
