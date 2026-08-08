#pragma once

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include "VideoData.h"

class VideoCard : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(float hoverProgress READ hoverProgress WRITE setHoverProgress)

public:
    explicit VideoCard(const VideoData &data, QWidget *parent = nullptr);

    void setData(const VideoData &data);

    float hoverProgress() const { return m_hoverProgress; }
    void setHoverProgress(float progress);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();

    VideoData m_data;

    QLabel *m_coverLabel;
    QLabel *m_durationLabel;
    QLabel *m_titleLabel;
    QLabel *m_avatarLabel;
    QLabel *m_uploaderLabel;
    QLabel *m_statsLabel;

    float m_hoverProgress = 0.0f;
    QPropertyAnimation *m_hoverAnimation;
};
