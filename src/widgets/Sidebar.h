#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QVector>

class Sidebar : public QWidget
{
    Q_OBJECT

public:
    explicit Sidebar(QWidget *parent = nullptr);

    void setCurrentIndex(int index);
    int currentIndex() const;

signals:
    void itemSelected(int index);

private:
    void setupUi();
    void setupStyle();
    QPushButton *createNavButton(const QString &text, const QColor &iconColor);

    QVector<QPushButton *> m_buttons;
    QVBoxLayout *m_layout;
    int m_currentIndex = 0;
};
