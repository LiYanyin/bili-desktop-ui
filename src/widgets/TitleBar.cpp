#include "TitleBar.h"

#include <QApplication>
#include <QMouseEvent>

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

    // Use Segoe MDL2 Assets for native Windows 11 titlebar icons
    // ChromeMinimize:   ChromeMaximize:   ChromeRestore:   ChromeClose: 
    m_minimizeBtn = new QPushButton(QChar(0xE921), this);
    m_minimizeBtn->setFixedSize(46, 32);

    m_maximizeBtn = new QPushButton(QChar(0xE922), this);
    m_maximizeBtn->setFixedSize(46, 32);

    m_closeBtn = new QPushButton(QChar(0xE8BB), this);
    m_closeBtn->setFixedSize(46, 32);

    layout->addWidget(m_minimizeBtn);
    layout->addWidget(m_maximizeBtn);
    layout->addWidget(m_closeBtn);

    // Window control button actions via Windows API for reliability
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
    const QString btnStyle = R"(
        QPushButton {
            background: transparent;
            border: none;
            color: #E0E0E0;
            font-family: "Segoe MDL2 Assets";
            font-size: 10px;
        }
        QPushButton:hover {
            background: rgba(255, 255, 255, 0.1);
        }
        QPushButton:pressed {
            background: rgba(255, 255, 255, 0.05);
        }
    )";

    const QString closeStyle = R"(
        QPushButton {
            background: transparent;
            border: none;
            color: #E0E0E0;
            font-family: "Segoe MDL2 Assets";
            font-size: 10px;
        }
        QPushButton:hover {
            background: #E81123;
            color: white;
        }
        QPushButton:pressed {
            background: #BF0F1D;
        }
    )";

    m_minimizeBtn->setStyleSheet(btnStyle);
    m_maximizeBtn->setStyleSheet(btnStyle);
    m_closeBtn->setStyleSheet(closeStyle);
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QWidget *child = childAt(event->pos());
        if (qobject_cast<QPushButton *>(child)) {
            // Don't start drag on buttons
            return;
        }
        // Use native Windows dragging
#ifdef Q_OS_WIN
        if (auto *w = window()) {
            ReleaseCapture();
            HWND hwnd = reinterpret_cast<HWND>(w->winId());
            SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
#endif
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        QPoint delta = event->globalPosition().toPoint() - m_dragStartPos;
        if (auto *w = window()) {
            if (!w->isMaximized()) {
                w->move(w->pos() + delta);
            }
            m_dragStartPos = event->globalPosition().toPoint();
        }
    }
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QWidget *child = childAt(event->pos());
        if (!qobject_cast<QPushButton *>(child)) {
            emit maximizeRequested();
        }
    }
}

void TitleBar::setMaximized(bool maximized)
{
    // ChromeRestore: U+E923  ChromeMaximize: U+E922
    m_maximizeBtn->setText(maximized ? QChar(0xE923) : QChar(0xE922));
}
