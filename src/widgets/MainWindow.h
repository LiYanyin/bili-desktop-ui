#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>

class TitleBar;
class Sidebar;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void setContentWidget(QWidget *widget);

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    void changeEvent(QEvent *event) override;

private slots:
    void onMinimize();
    void onMaximize();
    void onClose();

private:
    void setupUi();

    TitleBar *m_titleBar;
    Sidebar   *m_sidebar;
    QStackedWidget *m_contentArea;
};
