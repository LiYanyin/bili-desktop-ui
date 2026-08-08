#pragma once

#include <QDialog>
#include <QLabel>
#include "network/BiliLogin.h"

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);

    QString sessdata() const;

signals:
    void loginSuccess(const QString &sessdata);

private:
    BiliLogin *m_login;
    QLabel *m_qrLabel;
    QLabel *m_statusLabel;
    QLabel *m_successLabel;
};
