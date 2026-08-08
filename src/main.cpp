#include <QApplication>
#include <QScrollArea>
#include <QWidget>
#include <QDir>

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
#include "network/BiliApi.h"

static QWidget *createContentArea()
{
    auto *grid = new CardGridView();

    // Load local mock data first as placeholder
    QString jsonPath = QApplication::applicationDirPath() + "/resources/mock_videos.json";
    QList<VideoData> videoList = VideoDataLoader::loadFromJsonFile(jsonPath);
    if (videoList.isEmpty()) {
        jsonPath = QDir::currentPath() + "/resources/mock_videos.json";
        videoList = VideoDataLoader::loadFromJsonFile(jsonPath);
    }
    grid->setCards(videoList);

    // Then fetch real data from Bilibili API
    auto *api = new BiliApi(grid);
    QObject::connect(api, &BiliApi::videosReady, grid, [grid](const QList<VideoData> &videos) {
        if (!videos.isEmpty())
            grid->setCards(videos);
    });

    api->fetchPopular(1, 30);

    return grid;
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    timeBeginPeriod(1);
#endif
    QApplication app(argc, argv);

    app.setStyleSheet(R"(
        QWidget {
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
        }
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: rgba(255, 255, 255, 0.15);
            border-radius: 4px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: rgba(255, 255, 255, 0.25);
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: transparent;
        }
        QScrollBar:horizontal {
            background: transparent;
            height: 8px;
            margin: 0;
        }
        QScrollBar::handle:horizontal {
            background: rgba(255, 255, 255, 0.15);
            border-radius: 4px;
            min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover {
            background: rgba(255, 255, 255, 0.25);
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0;
        }
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
            background: transparent;
        }
    )");

    MainWindow window;
    window.setWindowTitle("Bili Desktop");
    window.setContentWidget(createContentArea());
    window.show();

    return app.exec();
}
