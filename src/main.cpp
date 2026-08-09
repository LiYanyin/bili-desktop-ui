#include <QApplication>
#include <QWidget>
#include <QDir>
#include <QLabel>
#include <QDebug>
#include <QFile>
#include <QDateTime>

// Crash diagnostic: write log at each startup step
static void crashLog(const QString &msg) {
    QFile f("F:/project/bili-desktop-ui/crash.log");
    if (f.open(QIODevice::Append))
        f.write(qPrintable(QDateTime::currentDateTime().toString("hh:mm:ss.zzz ") + msg + "\n"));
}
#include <QVBoxLayout>

#ifdef Q_OS_WIN
extern "C" {
    __declspec(dllimport) unsigned int __stdcall timeBeginPeriod(unsigned int);
    __declspec(dllimport) unsigned int __stdcall timeEndPeriod(unsigned int);
}
#endif

#include "widgets/MainWindow.h"
#include "widgets/VideoCard.h"
#include "widgets/VideoData.h"
#include "widgets/VideoDataLoader.h"
#include "widgets/CardGridView.h"
#include "widgets/LoginDialog.h"
#include "network/BiliApi.h"

// Placeholder page with centered message
static QWidget *createPlaceholderPage(const QString &text)
{
    auto *w = new QWidget();
    auto *lay = new QVBoxLayout(w);
    auto *label = new QLabel(text, w);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("color: #888; font-size: 18px; background: transparent;");
    lay->addWidget(label);
    w->setStyleSheet("background: transparent;");
    return w;
}

// Home page: Bilibili popular videos
static QWidget *createHomePage()
{
    auto *grid = new CardGridView();

    QString jsonPath = QApplication::applicationDirPath() + "/resources/mock_videos.json";
    QList<VideoData> videoList = VideoDataLoader::loadFromJsonFile(jsonPath);
    if (videoList.isEmpty()) {
        jsonPath = QDir::currentPath() + "/resources/mock_videos.json";
        videoList = VideoDataLoader::loadFromJsonFile(jsonPath);
    }
    grid->setCards(videoList);

    auto *api = new BiliApi(grid);
    QObject::connect(api, &BiliApi::videosReady, grid, [grid](const QList<VideoData> &videos) {
        if (!videos.isEmpty())
            grid->setCards(videos);
    });
    api->fetchPopular(1, 30);

    // Refresh button
    QObject::connect(grid, &CardGridView::refreshRequested, grid, [api]() {
        api->fetchPopular(1, 30);
    });

    return grid;
}

int main(int argc, char *argv[])
{
    crashLog("=== START ===");
#ifdef Q_OS_WIN
    timeBeginPeriod(1);
#endif
    crashLog("QApplication...");
    QApplication app(argc, argv);

    crashLog("setStyleSheet...");
    app.setStyleSheet(R"(
        QWidget { font-family: "Microsoft YaHei", "Segoe UI", sans-serif; }
        QScrollBar:vertical { background: transparent; width: 8px; margin: 0; }
        QScrollBar::handle:vertical { background: rgba(255,255,255,0.15); border-radius: 4px; min-height: 30px; }
        QScrollBar::handle:vertical:hover { background: rgba(255,255,255,0.25); }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
        QScrollBar:horizontal { background: transparent; height: 8px; margin: 0; }
        QScrollBar::handle:horizontal { background: rgba(255,255,255,0.15); border-radius: 4px; min-width: 30px; }
        QScrollBar::handle:horizontal:hover { background: rgba(255,255,255,0.25); }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: transparent; }
    )");

    crashLog("MainWindow...");
    // App icon — B站 pink square
    QPixmap iconPix(32, 32);
    iconPix.fill(QColor("#FB7299"));
    app.setWindowIcon(QIcon(iconPix));

    MainWindow window;
    window.setWindowTitle("Bili Desktop");

    crashLog("createHomePage...");
    auto *homePage = createHomePage();
    crashLog("addPage 0...");
    window.addPage(0, homePage);                              // 首页
    window.addPage(1, createPlaceholderPage("动态 — 开发中"));         // 动态
    window.addPage(2, createPlaceholderPage("收藏 — 开发中"));         // 收藏
    window.addPage(3, createPlaceholderPage("历史 — 开发中"));         // 历史
    window.addPage(4, createPlaceholderPage("设置 — 开发中"));         // 设置
        crashLog("switchToPage...");
    window.switchToPage(0);

    crashLog("login connect...");
    QObject::connect(&window, &MainWindow::loginRequested, &window, [&window]() {
        LoginDialog dlg(&window);
        if (dlg.exec() == QDialog::Accepted) {
            qDebug() << "Logged in with SESSDATA:" << dlg.sessdata().left(10) << "...";
        }
    });

    crashLog("window.show()...");
    window.show();
    crashLog("app.exec()...");
    return app.exec();
}
