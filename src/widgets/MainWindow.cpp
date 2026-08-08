#include "MainWindow.h"
#include "TitleBar.h"
#include "Sidebar.h"

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

    m_sidebar = new Sidebar(this);
    bodyLayout->addWidget(m_sidebar);

    m_contentArea = new QStackedWidget(this);
    bodyLayout->addWidget(m_contentArea, 1);

    rootLayout->addLayout(bodyLayout, 1);

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
        DWM_SYSTEMBACKDROP_TYPE backdropType = static_cast<DWM_SYSTEMBACKDROP_TYPE>(DWMSBT_MAINWINDOW);
        HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE,
                                           &backdropType, sizeof(backdropType));
        if (SUCCEEDED(hr)) {
            m_micaEnabled = true;

            BOOL darkMode = TRUE;
            DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE,
                                  &darkMode, sizeof(darkMode));

            this->setStyleSheet(R"(
                MainWindow {
                    background: rgba(20, 20, 35, 0.72);
                }
            )");

            m_titleBar->setStyleSheet(R"(
                TitleBar {
                    background: rgba(0, 0, 0, 0.35);
                }
            )");

            m_contentArea->setStyleSheet(R"(
                QStackedWidget {
                    background: transparent;
                }
            )");

            return;
        }
    }

    applyFallbackBackground();
#else
    applyFallbackBackground();
#endif
}

void MainWindow::applyFallbackBackground()
{
    m_micaEnabled = false;

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
    setupMica();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
#ifdef Q_OS_WIN
    if (m_micaEnabled) {
        HWND hwnd = reinterpret_cast<HWND>(winId());
        if (hwnd) {
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
        constexpr int borderWidth = 6;

        if (msg->message == WM_NCHITTEST) {
            if (isMaximized()) {
                *result = HTCLIENT;
                return true;
            }

            int xPos = GET_X_LPARAM(msg->lParam);
            int yPos = GET_Y_LPARAM(msg->lParam);
            QPoint localPos = mapFromGlobal(QPoint(xPos, yPos));

            int w = width();
            int h = height();

            bool left = localPos.x() < borderWidth;
            bool right = localPos.x() > w - borderWidth;
            bool top = localPos.y() < borderWidth;
            bool bottom = localPos.y() > h - borderWidth;

            if (top && left)        { *result = HTTOPLEFT;     return true; }
            if (top && right)       { *result = HTTOPRIGHT;    return true; }
            if (bottom && left)     { *result = HTBOTTOMLEFT;  return true; }
            if (bottom && right)    { *result = HTBOTTOMRIGHT; return true; }
            if (top)                { *result = HTTOP;          return true; }
            if (bottom)             { *result = HTBOTTOM;       return true; }
            if (left)               { *result = HTLEFT;         return true; }
            if (right)              { *result = HTRIGHT;        return true; }

            if (localPos.y() < m_titleBar->height()) {
                *result = HTCLIENT;
                return true;
            }

            *result = HTCLIENT;
            return true;
        }

        if (msg->message == WM_GETMINMAXINFO) {
            auto *mmi = reinterpret_cast<MINMAXINFO *>(msg->lParam);
            QScreen *screen = QApplication::primaryScreen();
            if (screen) {
                QRect avail = screen->availableGeometry();
                mmi->ptMaxPosition.x = avail.x();
                mmi->ptMaxPosition.y = avail.y();
                mmi->ptMaxSize.x = avail.width();
                mmi->ptMaxSize.y = avail.height();
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
        m_titleBar->setMaximized(isMaximized());
    }
    QWidget::changeEvent(event);
}
