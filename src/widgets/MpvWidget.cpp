#include "MpvWidget.h"
#include <QResizeEvent>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

static const QString MPV_PATH = "C:/Program Files/MPV Player/mpv.exe";

MpvWidget::MpvWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setStyleSheet("background: black;");
}

MpvWidget::~MpvWidget()
{
    if (m_proc) {
        m_proc->terminate();
        m_proc->waitForFinished(3000);
        m_proc->kill();
    }
}

void MpvWidget::play(const QString &bvid, const QString &title)
{
    if (m_proc) {
        m_proc->terminate();
        m_proc->waitForFinished(3000);
        delete m_proc;
    }

    m_proc = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", env.value("PATH") + ";C:/Users/19588/anaconda3/Scripts");
    m_proc->setProcessEnvironment(env);

    QStringList args;
    args << "--referrer=https://www.bilibili.com/"
         << "--keep-open=yes"
         << "--osc=yes"
         << "--script-opts=osc-visibility=always"
         << "--ytdl-format=bestvideo+bestaudio/best";

    if (!title.isEmpty())
        args << ("--title=" + title);

    // Embed into our widget
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(winId());
    args << ("--wid=" + QString::number(reinterpret_cast<intptr_t>(hwnd)));
#endif

    args << ("https://www.bilibili.com/video/" + bvid);
    m_proc->start(MPV_PATH, args);
}

void MpvWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
}
