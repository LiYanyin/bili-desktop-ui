#pragma once

#include <QWidget>
#include <QLabel>
#include "VideoData.h"

class VideoCard : public QWidget
{
    Q_OBJECT

public:
    explicit VideoCard(const VideoData &data, QWidget *parent = nullptr);

    void setData(const VideoData &data);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();
    QString truncateTitle(const QString &title, int maxLines) const;

    VideoData m_data;

    QLabel *m_coverLabel;
    QLabel *m_durationLabel;
    QLabel *m_titleLabel;
    QLabel *m_avatarLabel;
    QLabel *m_uploaderLabel;
    QLabel *m_statsLabel;

    bool m_hovered = false;
};
