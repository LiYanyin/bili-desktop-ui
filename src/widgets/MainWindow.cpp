#include "MainWindow.h"
#include "TitleBar.h"
#include "Sidebar.h"

#include <QApplication>
#include <QScreen>

// Windows-specific for native event handling
#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    // Frameless window with resize capability
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowSystemMenuHint
                   | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    setupUi();

    setMinimumSize(1000, 650);
    resize(1280, 800);
}

void MainWindow::setupUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Title bar at the top
    m_titleBar = new TitleBar(this);
    rootLayout->addWidget(m_titleBar);

    // Body: Sidebar + Content
    auto *bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    // Left sidebar (fixed width 200px, defined in Sidebar)
    m_sidebar = new Sidebar(this);
    bodyLayout->addWidget(m_sidebar);

    // Content area fills the rest
    m_contentArea = new QStackedWidget(this);
    m_contentArea->setStyleSheet("background: #1A1A2E;");
    bodyLayout->addWidget(m_contentArea, 1);

    rootLayout->addLayout(bodyLayout, 1);

    // Connect title bar signals
    connect(m_titleBar, &TitleBar::minimizeRequested, this, &MainWindow::onMinimize);
    connect(m_titleBar, &TitleBar::maximizeRequested, this, &MainWindow::onMaximize);
    connect(m_titleBar, &TitleBar::closeRequested, this, &MainWindow::onClose);
}

void MainWindow::setContentWidget(QWidget *widget)
{
    m_contentArea->addWidget(widget);
    m_contentArea->setCurrentWidget(widget);
}

void MainWindow::onMinimize()
{
    showMinimized();
}

void MainWindow::onMaximize()
{
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
}

void MainWindow::onClose()
{
    close();
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG") {
        auto *msg = static_cast<MSG *>(message);
        constexpr int borderWidth = 6;

        if (msg->message == WM_NCHITTEST) {
            if (isMaximized()) {
                *result = HTCLIENT;
                return true;
            }

            int xPos = GET_X_LPARAM(msg->lParam);
            int yPos = GET_Y_LPARAM(msg->lParam);

            QPoint localPos = mapFromGlobal(QPoint(xPos, yPos));

            int windowWidth = width();
            int windowHeight = height();

            bool left = localPos.x() < borderWidth;
            bool right = localPos.x() > windowWidth - borderWidth;
            bool top = localPos.y() < borderWidth;
            bool bottom = localPos.y() > windowHeight - borderWidth;

            if (top && left)        { *result = HTTOPLEFT;     return true; }
            if (top && right)       { *result = HTTOPRIGHT;    return true; }
            if (bottom && left)     { *result = HTBOTTOMLEFT;  return true; }
            if (bottom && right)    { *result = HTBOTTOMRIGHT; return true; }
            if (top)                { *result = HTTOP;          return true; }
            if (bottom)             { *result = HTBOTTOM;       return true; }
            if (left)               { *result = HTLEFT;         return true; }
            if (right)              { *result = HTRIGHT;        return true; }

            if (localPos.y() < m_titleBar->height()) {
                *result = HTCAPTION;
                return true;
            }

            *result = HTCLIENT;
            return true;
        }

        if (msg->message == WM_GETMINMAXINFO) {
            auto *mmi = reinterpret_cast<MINMAXINFO *>(msg->lParam);
            QScreen *screen = QApplication::primaryScreen();
            if (screen) {
                QRect available = screen->availableGeometry();
                mmi->ptMaxPosition.x = available.x();
                mmi->ptMaxPosition.y = available.y();
                mmi->ptMaxSize.x = available.width();
                mmi->ptMaxSize.y = available.height();
            }
            *result = 0;
            return true;
        }
    }
#endif
    return QWidget::nativeEvent(eventType, message, result);
}

void MainWindow::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
}
