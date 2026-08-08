#include "Sidebar.h"

#include <QPainter>
#include <QSpacerItem>
#include <QButtonGroup>

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

    // Use a QButtonGroup for exclusive selection (radio-button behavior)
    auto *group = new QButtonGroup(this);
    group->setExclusive(true);

    m_buttons.append(createNavButton("首页",     QColor("#FB7299")));
    m_buttons.append(createNavButton("动态",     QColor("#00A1D6")));
    m_buttons.append(createNavButton("收藏",     QColor("#FCA700")));
    m_buttons.append(createNavButton("历史",     QColor("#6DC781")));
    m_buttons.append(createNavButton("设置",     QColor("#9B9B9B")));

    for (int i = 0; i < m_buttons.size(); ++i) {
        group->addButton(m_buttons[i], i);
    }

    // Login button at bottom
    auto *loginBtn = new QPushButton("登录", this);
    loginBtn->setFixedHeight(44);
    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(251, 114, 153, 0.2);
            border: 1px solid rgba(251, 114, 153, 0.3);
            border-radius: 8px;
            color: #FB7299;
            font-size: 14px;
            margin: 0 12px;
        }
        QPushButton:hover {
            background: rgba(251, 114, 153, 0.35);
        }
    )");
    m_layout->addWidget(loginBtn);
    connect(loginBtn, &QPushButton::clicked, this, &Sidebar::loginRequested);

    m_layout->addStretch();

    // Connect button group to handle selection
    connect(group, &QButtonGroup::idClicked, this, [this](int id) {
        setCurrentIndex(id);
        emit itemSelected(id);
    });

    setCurrentIndex(0);
}

QPushButton *Sidebar::createNavButton(const QString &text, const QColor &iconColor)
{
    auto *btn = new QPushButton(this);
    btn->setText(text);
    btn->setCheckable(true);

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
    setStyleSheet(R"(
        Sidebar {
            background: rgba(13, 13, 26, 0.75);
        }
    )");

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
