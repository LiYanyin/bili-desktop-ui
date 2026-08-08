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
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onMinimize();
    void onMaximize();
    void onClose();

private:
    void setupUi();
    void setupMica();
    void applyFallbackBackground();
    bool isWindows11OrGreater();

    TitleBar *m_titleBar;
    Sidebar   *m_sidebar;
    QStackedWidget *m_contentArea;
    bool m_micaEnabled = false;
    bool m_maximized = false;
    QRect m_normalGeometry;
};
