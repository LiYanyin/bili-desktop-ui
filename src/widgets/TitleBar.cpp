#include "TitleBar.h"

#include <QApplication>
#include <QMouseEvent>
#include <QDebug>
#include <QStyle>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(36);
    setupUi();
    setupStyle();
}

void TitleBar::setupUi()
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 0, 0);
    layout->setSpacing(0);

    // Left side: icon + title
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(20, 20);
    m_iconLabel->setScaledContents(true);
    QPixmap iconPix(20, 20);
    iconPix.fill(QColor("#FB7299"));
    m_iconLabel->setPixmap(iconPix);

    m_titleLabel = new QLabel("Bili Desktop", this);
    m_titleLabel->setStyleSheet("color: #FFFFFF; font-size: 13px; font-weight: 500;");

    layout->addWidget(m_iconLabel);
    layout->addSpacing(8);
    layout->addWidget(m_titleLabel);
    layout->addStretch();

    // Right side: window control buttons
    m_minimizeBtn = new QPushButton(this);
    m_minimizeBtn->setFixedSize(46, 32);
    m_minimizeBtn->setText("─");  // ─

    m_maximizeBtn = new QPushButton(this);
    m_maximizeBtn->setFixedSize(46, 32);
    m_maximizeBtn->setText("□");  // □

    m_closeBtn = new QPushButton(this);
    m_closeBtn->setFixedSize(46, 32);
    m_closeBtn->setText("✕");  // ✕

    layout->addWidget(m_minimizeBtn);
    layout->addWidget(m_maximizeBtn);
    layout->addWidget(m_closeBtn);

    // Use lambda connections for reliability
    connect(m_minimizeBtn, &QPushButton::clicked, this, [this]() {
        emit minimizeRequested();
    });
    connect(m_maximizeBtn, &QPushButton::clicked, this, [this]() {
        emit maximizeRequested();
    });
    connect(m_closeBtn, &QPushButton::clicked, this, [this]() {
        emit closeRequested();
    });
}

void TitleBar::setupStyle()
{
    const QString baseBtnStyle = R"(
        QPushButton {
            background: transparent;
            border: none;
            color: #E0E0E0;
            font-size: 12px;
        }
        QPushButton:hover {
            background: rgba(255, 255, 255, 0.1);
        }
        QPushButton:pressed {
            background: rgba(255, 255, 255, 0.05);
        }
    )";

    const QString closeBtnStyle = R"(
        QPushButton {
            background: transparent;
            border: none;
            color: #E0E0E0;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: #E81123;
            color: white;
        }
        QPushButton:pressed {
            background: #BF0F1D;
        }
    )";

    m_minimizeBtn->setStyleSheet(baseBtnStyle);
    m_maximizeBtn->setStyleSheet(baseBtnStyle);
    m_closeBtn->setStyleSheet(closeBtnStyle);
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Check if click is on a button — if so, don't start drag
        QWidget *child = childAt(event->pos());
        if (qobject_cast<QPushButton *>(child)) {
            QWidget::mousePressEvent(event);
            return;
        }
        // Use native Windows dragging
#ifdef Q_OS_WIN
        if (auto *win = window()) {
            ReleaseCapture();
            HWND hwnd = reinterpret_cast<HWND>(win->winId());
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
#endif
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Don't double-click-to-maximize on buttons
        QWidget *child = childAt(event->pos());
        if (qobject_cast<QPushButton *>(child)) {
            QWidget::mouseDoubleClickEvent(event);
            return;
        }
        emit maximizeRequested();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void TitleBar::setMaximized(bool maximized)
{
    m_maximizeBtn->setText(maximized ? "❐" : "□");  // ❐ : □
}
