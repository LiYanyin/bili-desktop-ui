#include "Sidebar.h"

#include <QPainter>
#include <QSpacerItem>

Sidebar::Sidebar(QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(200);
    setupUi();
    setupStyle();
}

void Sidebar::setupUi()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 8, 0, 8);
    m_layout->setSpacing(2);

    // Create navigation buttons
    // Each with a distinct B站-inspired accent color for the icon placeholder
    m_buttons.append(createNavButton("首页",     QColor("#FB7299"))); // pink
    m_buttons.append(createNavButton("动态",     QColor("#00A1D6"))); // blue
    m_buttons.append(createNavButton("收藏",     QColor("#FCA700"))); // orange
    m_buttons.append(createNavButton("历史",     QColor("#6DC781"))); // green
    m_buttons.append(createNavButton("设置",     QColor("#9B9B9B"))); // gray

    // Push buttons to the top; leave the rest of space empty
    m_layout->addStretch();

    // Select first item by default
    setCurrentIndex(0);
}

QPushButton *Sidebar::createNavButton(const QString &text, const QColor &iconColor)
{
    auto *btn = new QPushButton(this);
    btn->setText(text);
    btn->setCheckable(true);

    // Create icon placeholder (small colored circle)
    QPixmap icon(18, 18);
    icon.fill(Qt::transparent);
    QPainter painter(&icon);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(iconColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(2, 2, 14, 14);
    painter.end();
    btn->setIcon(QIcon(icon));
    btn->setIconSize(QSize(18, 18));

    btn->setFixedHeight(44);
    btn->setCursor(Qt::PointingHandCursor);

    m_layout->addWidget(btn);

    return btn;
}

void Sidebar::setupStyle()
{
    // Sidebar background
    setStyleSheet(R"(
        Sidebar {
            background: #0D0D1A;
        }
    )");

    // Individual button style
    const QString btnStyle = R"(
        QPushButton {
            background: transparent;
            border: none;
            color: #AAAAAA;
            font-size: 14px;
            text-align: left;
            padding-left: 20px;
            spacing: 12px;
        }
        QPushButton:hover {
            background: rgba(255, 255, 255, 0.06);
            color: #E0E0E0;
        }
        QPushButton:checked {
            background: rgba(251, 114, 153, 0.15);
            color: #FB7299;
            font-weight: bold;
            border-left: 3px solid #FB7299;
            padding-left: 17px;
        }
    )";

    for (auto *btn : m_buttons) {
        btn->setStyleSheet(btnStyle);
    }
}

void Sidebar::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_buttons.size()) return;

    m_currentIndex = index;
    for (int i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i]->setChecked(i == index);
    }
}

int Sidebar::currentIndex() const
{
    return m_currentIndex;
}
