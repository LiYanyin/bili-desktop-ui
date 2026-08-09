#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QVariantAnimation>
#include <QList>
#include <QPushButton>
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
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onScrollChanged(int value);
    void scrollToTop();

private:
    struct CardEntry {
        VideoCard *card;
        QGraphicsProxyWidget *proxy;
    };
    void layoutCards(bool animate);
    void setupScrollToTopButton();

    QGraphicsView *m_view;
    QGraphicsScene *m_scene;
    QList<CardEntry> m_entries;
    QVariantAnimation *m_activeAnim = nullptr;

    QPushButton *m_scrollTopBtn = nullptr;
};
