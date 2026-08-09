#include "MpvWidget.h"
#include <QResizeEvent>
#include <QMouseEvent>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

static HWND findMpvChild(HWND parent) {
    return FindWindowExW(parent, nullptr, L"mpv", nullptr);
}

static const QString MPV_PATH = "C:/Program Files/MPV Player/mpv.exe";

MpvWidget::MpvWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);
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

static void forwardMouse(HWND hwnd, QMouseEvent *e, UINT msg)
{
    HWND child = findMpvChild(hwnd);
    if (!child) return;
    POINT pt = { (LONG)e->position().x(), (LONG)e->position().y() };
    ScreenToClient(hwnd, &pt);
    WPARAM wParam = 0;
    if (e->buttons() & Qt::LeftButton)  wParam |= MK_LBUTTON;
    if (e->buttons() & Qt::RightButton) wParam |= MK_RBUTTON;
    PostMessageW(child, msg, wParam, MAKELPARAM(pt.x, pt.y));
}

void MpvWidget::mouseMoveEvent(QMouseEvent *e)
{
#ifdef Q_OS_WIN
    forwardMouse(reinterpret_cast<HWND>(winId()), e, WM_MOUSEMOVE);
#endif
    QWidget::mouseMoveEvent(e);
}
void MpvWidget::mousePressEvent(QMouseEvent *e)
{
#ifdef Q_OS_WIN
    UINT msg = (e->button() == Qt::LeftButton) ? WM_LBUTTONDOWN :
               (e->button() == Qt::RightButton) ? WM_RBUTTONDOWN : 0;
    if (msg) forwardMouse(reinterpret_cast<HWND>(winId()), e, msg);
#endif
    QWidget::mousePressEvent(e);
}
void MpvWidget::mouseReleaseEvent(QMouseEvent *e)
{
#ifdef Q_OS_WIN
    UINT msg = (e->button() == Qt::LeftButton) ? WM_LBUTTONUP :
               (e->button() == Qt::RightButton) ? WM_RBUTTONUP : 0;
    if (msg) forwardMouse(reinterpret_cast<HWND>(winId()), e, msg);
#endif
    QWidget::mouseReleaseEvent(e);
}
