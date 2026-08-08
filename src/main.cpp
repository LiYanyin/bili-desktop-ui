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
    )");

    MainWindow window;
    window.setWindowTitle("Bili Desktop UI");
    window.setContentWidget(createContentArea());
    window.show();

    return app.exec();
}
