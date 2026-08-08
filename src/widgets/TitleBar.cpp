#include "TitleBar.h"

#include <QApplication>
#include <QMouseEvent>
#include <QStyle>

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
    // Use a simple colored square as placeholder icon
    QPixmap iconPix(20, 20);
    iconPix.fill(QColor("#FB7299")); // B站 pink
    m_iconLabel->setPixmap(iconPix);

    m_titleLabel = new QLabel("Bili Desktop", this);
    m_titleLabel->setStyleSheet("color: #FFFFFF; font-size: 13px; font-weight: 500;");

    layout->addWidget(m_iconLabel);
    layout->addSpacing(8);
    layout->addWidget(m_titleLabel);
    layout->addStretch();

    // Right side: window control buttons
    // Minimize button
    m_minimizeBtn = new QPushButton(this);
    m_minimizeBtn->setFixedSize(46, 32);
    m_minimizeBtn->setText("─"); // ─

    // Maximize button
    m_maximizeBtn = new QPushButton(this);
    m_maximizeBtn->setFixedSize(46, 32);
    m_maximizeBtn->setText("□"); // □

    // Close button
    m_closeBtn = new QPushButton(this);
    m_closeBtn->setFixedSize(46, 32);
    m_closeBtn->setText("✕"); // ✕

    layout->addWidget(m_minimizeBtn);
    layout->addWidget(m_maximizeBtn);
    layout->addWidget(m_closeBtn);

    // Connect signals
    connect(m_minimizeBtn, &QPushButton::clicked, this, &TitleBar::minimizeRequested);
    connect(m_maximizeBtn, &QPushButton::clicked, this, &TitleBar::maximizeRequested);
    connect(m_closeBtn, &QPushButton::clicked, this, &TitleBar::closeRequested);
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
        m_dragging = true;
        m_dragStartPos = event->globalPosition().toPoint();
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        QPoint delta = event->globalPosition().toPoint() - m_dragStartPos;
        if (window() && !window()->isMaximized()) {
            window()->move(window()->pos() + delta);
        }
        m_dragStartPos = event->globalPosition().toPoint();
    }
    QWidget::mouseMoveEvent(event);
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    QWidget::mouseReleaseEvent(event);
}

void TitleBar::setMaximized(bool maximized)
{
    m_maximizeBtn->setText(maximized ? "❐" : "□");
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit maximizeRequested();
    }
    QWidget::mouseDoubleClickEvent(event);
}
