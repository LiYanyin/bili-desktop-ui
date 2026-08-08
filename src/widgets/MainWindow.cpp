#include "MainWindow.h"
#include "TitleBar.h"

#include <QApplication>
#include <QScreen>
#include <QShowEvent>
#include <QResizeEvent>
#include <QWindow>

// Windows-specific
#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

#ifndef DWMSBT_MAINWINDOW
#define DWMSBT_MAINWINDOW 2
#endif

#ifndef DWMSBT_NONE
#define DWMSBT_NONE 1
#endif
#endif // Q_OS_WIN

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

    // Content area fills the rest
    m_contentArea = new QStackedWidget(this);
    // Use semi-transparent background so Mica shows through
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

bool MainWindow::isWindows11OrGreater()
{
#ifdef Q_OS_WIN
    // Use RtlGetVersion to get the real OS version (not affected by manifest)
    using RtlGetVersionFunc = LONG (WINAPI *)(POSVERSIONINFOEXW);
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) return false;

    auto rtlGetVersion = reinterpret_cast<RtlGetVersionFunc>(
        GetProcAddress(ntdll, "RtlGetVersion"));
    if (!rtlGetVersion) return false;

    OSVERSIONINFOEXW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (rtlGetVersion(reinterpret_cast<POSVERSIONINFOEXW>(&osvi)) != 0)
        return false;

    // Windows 11: build number >= 22000
    return osvi.dwMajorVersion >= 10 && osvi.dwBuildNumber >= 22000;
#else
    return false;
#endif
}

void MainWindow::setupMica()
{
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(winId());
    if (!hwnd) return;

    if (isWindows11OrGreater()) {
        // Enable Mica backdrop
        DWM_SYSTEMBACKDROP_TYPE backdropType = static_cast<DWM_SYSTEMBACKDROP_TYPE>(DWMSBT_MAINWINDOW);
        HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE,
                                           &backdropType, sizeof(backdropType));
        if (SUCCEEDED(hr)) {
            m_micaEnabled = true;

            // Also enable dark mode title bar (for window borders/title)
            BOOL darkMode = TRUE;
            DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE,
                                  &darkMode, sizeof(darkMode));

            // Use dark-tinted semi-transparent background to show Mica
            // Mica works as the window background; our content renders on top
            // So we use a dark semi-transparent color for the content area
            this->setStyleSheet(R"(
                MainWindow {
                    background: rgba(20, 20, 35, 0.72);
                }
            )");

            // Title bar blends with Mica
            m_titleBar->setStyleSheet(R"(
                TitleBar {
                    background: rgba(0, 0, 0, 0.35);
                }
            )");

            // Content area background — let Mica show through
            m_contentArea->setStyleSheet(R"(
                QStackedWidget {
                    background: transparent;
                }
            )");

            return;
        }
    }

    // Fallback: Mica not available
    applyFallbackBackground();
#else
    applyFallbackBackground();
#endif
}

void MainWindow::applyFallbackBackground()
{
    m_micaEnabled = false;

    // Solid dark background when Mica is not available
    this->setStyleSheet(R"(
        MainWindow {
            background: #141423;
        }
    )");

    m_titleBar->setStyleSheet(R"(
        TitleBar {
            background: #0D0D1A;
        }
    )");

    m_contentArea->setStyleSheet(R"(
        QStackedWidget {
            background: #1A1A2E;
        }
    )");
}

void MainWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // Set up Mica after the native window is created
    setupMica();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // On Windows 11, Mica can sometimes need a refresh after resize
#ifdef Q_OS_WIN
    if (m_micaEnabled) {
        HWND hwnd = reinterpret_cast<HWND>(winId());
        if (hwnd) {
            // Force a redraw of the non-client area to refresh Mica
            SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                        SWP_FRAMECHANGED);
        }
    }
#endif
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
            // When maximized, the whole window acts as client area
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

            // Title bar area acts as caption for dragging
            if (localPos.y() < m_titleBar->height()) {
                *result = HTCAPTION;
                return true;
            }

            *result = HTCLIENT;
            return true;
        }

        // Handle window maximize state for DPI awareness
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
    if (event->type() == QEvent::WindowStateChange) {
        // The maximize button text could be updated here
        // (We'll refine button icons later)
    }
    QWidget::changeEvent(event);
}
