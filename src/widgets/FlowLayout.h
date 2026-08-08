#pragma once

#include <QLayout>
#include <QList>
#include <QStyle>
#include <QWidget>
#include <QPointer>

class FlowLayout : public QLayout
{
    Q_OBJECT

public:
    explicit FlowLayout(QWidget *parent = nullptr, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~FlowLayout() override;

    void addItem(QLayoutItem *item) override;
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

private slots:
    void runAnimation();

private:
    int doLayoutCalc(const QRect &rect) const; // const — for heightForWidth
    int doLayoutApply(const QRect &rect);       // non-const — for setGeometry
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem *> m_itemList;
    int m_hSpace;
    int m_vSpace;
    struct AnimTask { QPointer<QWidget> widget; QPoint oldPos; QPoint newPos; QSize newSize; };
    QList<AnimTask> m_animTasks;
};
