#include "MainWindow.h"
#include "TitleBar.h"

#include <QApplication>
#include <QScreen>
#include <QWindow>

// Windows-specific for native event handling
#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    // Frameless window with resize capability
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowSystemMenuHint
                   | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    // Enable translucent background for Mica effect (will be configured later)
    setAttribute(Qt::WA_TranslucentBackground, false); // Will be true when Mica is set up

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

    // Content area fills the rest
    m_contentArea = new QStackedWidget(this);
    m_contentArea->setStyleSheet("background: #1A1A2E;"); // Dark background color
    rootLayout->addWidget(m_contentArea, 1);

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
        constexpr int borderWidth = 6; // resize border thickness

        if (msg->message == WM_NCHITTEST) {
            // Get cursor position in screen coordinates
            int xPos = GET_X_LPARAM(msg->lParam);
            int yPos = GET_Y_LPARAM(msg->lParam);

            // Convert to window-local coordinates
            QPoint localPos = mapFromGlobal(QPoint(xPos, yPos));

            int windowWidth = width();
            int windowHeight = height();

            bool left = localPos.x() < borderWidth;
            bool right = localPos.x() > windowWidth - borderWidth;
            bool top = localPos.y() < borderWidth;
            bool bottom = localPos.y() > windowHeight - borderWidth;

            if (isMaximized()) {
                // No resize when maximized
                *result = HTCLIENT;
                return true;
            }

            if (top && left)        { *result = HTTOPLEFT;     return true; }
            if (top && right)       { *result = HTTOPRIGHT;    return true; }
            if (bottom && left)     { *result = HTBOTTOMLEFT;  return true; }
            if (bottom && right)    { *result = HTBOTTOMRIGHT; return true; }
            if (top)                { *result = HTTOP;          return true; }
            if (bottom)             { *result = HTBOTTOM;       return true; }
            if (left)               { *result = HTLEFT;         return true; }
            if (right)              { *result = HTRIGHT;        return true; }

            // Title bar area (non-client for dragging)
            if (localPos.y() < m_titleBar->height()) {
                // Check if point is on a button (let Qt handle clicks)
                *result = HTCAPTION;
                return true;
            }

            *result = HTCLIENT;
            return true;
        }

        // Handle window maximize state changes for DPI awareness
        if (msg->message == WM_GETMINMAXINFO) {
            auto *mmi = reinterpret_cast<MINMAXINFO *>(msg->lParam);
            // Ensure window doesn't cover taskbar when maximized
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
    if (event->type() == QEvent::WindowStateChange) {
        // Update maximize button icon based on state
        // (We'll handle this more elegantly later)
    }
    QWidget::changeEvent(event);
}
