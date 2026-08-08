#include <QApplication>
#include <QScrollArea>
#include <QWidget>
#include <QDir>

#include "widgets/MainWindow.h"
#include "widgets/FlowLayout.h"
#include "widgets/VideoCard.h"
#include "widgets/VideoData.h"
#include "widgets/VideoDataLoader.h"

static QWidget *createContentArea()
{
    // Load mock video data from JSON
    QString jsonPath = QApplication::applicationDirPath() + "/resources/mock_videos.json";
    QList<VideoData> videoList = VideoDataLoader::loadFromJsonFile(jsonPath);

    if (videoList.isEmpty()) {
        // Fallback: try relative path from source
        jsonPath = QDir::currentPath() + "/resources/mock_videos.json";
        videoList = VideoDataLoader::loadFromJsonFile(jsonPath);
    }

    auto *container = new QWidget();
    auto *flow = new FlowLayout(container, 16, 12, 12);

    for (const auto &data : videoList) {
        auto *card = new VideoCard(data);
        flow->addWidget(card);
    }

    // Wrap in scroll area
    auto *scrollArea = new QScrollArea();
    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; }");

    return scrollArea;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(R"(
        QWidget {
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
        }
        /* Dark scrollbar */
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
    window.setWindowTitle("Bili Desktop UI");
    window.setContentWidget(createContentArea());
    window.show();

    return app.exec();
}
