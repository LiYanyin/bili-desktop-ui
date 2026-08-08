#include <QApplication>
#include <QScrollArea>
#include <QWidget>

#include "widgets/MainWindow.h"
#include "widgets/FlowLayout.h"
#include "widgets/VideoCard.h"
#include "widgets/VideoData.h"

// Temporary test content with sample VideoCards
static QWidget *createTestContent()
{
    auto *container = new QWidget();
    auto *flow = new FlowLayout(container, 16, 12, 12);

    // Sample video data (will be replaced by JSON loading in step 7)
    const QList<VideoData> samples = {
        {"【4K】绝美星空延时摄影",          "", "摄影师小王", "", "32.5万", "2天前",  "05:23"},
        {"2024年度动画混剪",                "", "MAD大神",    "", "89.1万", "1周前",  "03:45"},
        {"前端开发入门教程第一集",            "", "码农老张",   "", "15.2万", "3天前",  "28:10"},
        {"【音乐推荐】适合写代码听的电子音乐",  "", "音乐盒子",   "", "45.6万", "5天前",  "62:30"},
        {"手机摄影技巧：教你拍出大片感",       "", "摄影达人",   "", "28.3万", "1天前",  "12:08"},
        {"NBA精彩集锦：年度最佳扣篮",         "", "篮球频道",   "", "102.7万","4天前", "08:52"},
        {"美食探店：隐藏在北京巷子里的宝藏餐厅", "", "吃货日记",  "", "56.9万", "2周前",  "15:20"},
        {"【深度解析】量子计算到底能做什么？",   "", "科普基地",   "", "21.8万", "6天前",  "18:45"},
        {"萌宠日常：我家猫主子的沙雕瞬间",      "", "猫奴日记",   "", "67.3万", "3天前",  "06:12"},
        {"游戏实况：挑战史上最难Boss",          "", "老玩家666",  "", "34.1万", "1天前",  "45:30"},
        {"【Vlog】一个人的日本旅行",            "", "旅行博主",   "", "78.4万", "2周前",  "22:15"},
        {"RTX 5090首发评测：性能提升多少？",     "", "硬件评测",   "", "156.2万","5天前", "14:20"},
    };

    for (const auto &data : samples) {
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
    window.setContentWidget(createTestContent());
    window.show();

    return app.exec();
}
