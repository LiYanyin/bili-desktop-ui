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
    // Don't use Qt::FramelessWindowHint — it breaks child widget mouse events.
    // Instead, we strip WS_CAPTION via Windows API in showEvent().
    setWindowFlags(Qt::Window | Qt::WindowSystemMenuHint
                   | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    // Enable translucent background for Mica effect.
    // Combined with DwmExtendFrameIntoClientArea in showEvent, this lets
    // the Windows Mica material show through our semi-transparent overlay.
    setAttribute(Qt::WA_TranslucentBackground, true);

    setupUi();

    // Fixed window — 3 columns (300*3 + 12*2 + 32 + 200 = 1156)
    // Fullscreen → 5+ columns depending on screen
    setMinimumSize(1160, 720);
    resize(1160, 780);
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

    // Sidebar page switching
    connect(m_sidebar, &Sidebar::itemSelected, this, &MainWindow::switchToPage);
    connect(m_sidebar, &Sidebar::loginRequested, this, &MainWindow::loginRequested);
}

void MainWindow::setContentWidget(QWidget *widget)
{
    m_contentArea->addWidget(widget);
    m_contentArea->setCurrentWidget(widget);
}

void MainWindow::addPage(int index, QWidget *widget)
{
    m_contentArea->insertWidget(index, widget);
}

void MainWindow::switchToPage(int index)
{
    if (index >= 0 && index < m_contentArea->count())
        m_contentArea->setCurrentIndex(index);
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

            // Light overlay so the Mica material is clearly visible
            this->setStyleSheet(R"(
                MainWindow {
                    background: rgba(18, 18, 28, 0.50);
                }
            )");

            m_titleBar->setStyleSheet(R"(
                TitleBar {
                    background: rgba(0, 0, 0, 0.15);
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
            background: #1A1A28;
        }
    )");

    m_titleBar->setStyleSheet(R"(
        TitleBar {
            background: #0F0F1A;
        }
    )");

    m_contentArea->setStyleSheet(R"(
        QStackedWidget {
            background: #1E1E30;
        }
    )");
}

void MainWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(winId());
    if (hwnd) {
        // Strip Windows title bar. No WS_THICKFRAME = fixed size, no resize border.
        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        style &= ~(WS_CAPTION | WS_THICKFRAME);
        style |= WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
        SetWindowLongPtr(hwnd, GWL_STYLE, style);

        // Extend DWM frame into entire client area so Mica shows through
        MARGINS margins = {-1, -1, -1, -1};
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
    }
#endif

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
    if (m_maximized) {
        m_maximized = false;
        setGeometry(m_normalGeometry);
        m_titleBar->setMaximized(false);
    } else {
        m_normalGeometry = geometry();
        m_maximized = true;
        QScreen *screen = QApplication::primaryScreen();
        if (screen)
            setGeometry(screen->availableGeometry());
        m_titleBar->setMaximized(true);
    }
}

void MainWindow::onClose()
{
    close();
}


bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType); Q_UNUSED(message); Q_UNUSED(result);
    return false;
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        m_titleBar->setMaximized(isMaximized());
    }
    QWidget::changeEvent(event);
}
