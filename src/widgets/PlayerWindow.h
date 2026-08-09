#pragma once
#include <QWidget>
#include <QTextEdit>
class MpvWidget;

class PlayerWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerWindow(QWidget *parent = nullptr);
    void play(const QString &bvid, const QString &title, const QString &comment = {});

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    MpvWidget *m_mpv;
    QTextEdit *m_comments;
};
