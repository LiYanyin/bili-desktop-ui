#pragma once
#include <QWidget>
#include <QProcess>

class MpvWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MpvWidget(QWidget *parent = nullptr);
    ~MpvWidget() override;

    void play(const QString &bvid, const QString &title = {});

protected:
    void resizeEvent(QResizeEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    QProcess *m_proc = nullptr;
#ifdef Q_OS_WIN
    HWND m_hwnd = nullptr;
#endif
};
