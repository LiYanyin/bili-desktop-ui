#include <QApplication>
#include <QScrollArea>
#include <QWidget>
#include <QLabel>

#include "widgets/MainWindow.h"
#include "widgets/FlowLayout.h"

// Temporary test content to verify FlowLayout works
static QWidget *createTestContent()
{
    auto *container = new QWidget();
    auto *flow = new FlowLayout(container, 16, 12, 12);

    // Create colored test blocks
    const QList<QColor> colors = {
        QColor("#FB7299"), QColor("#00A1D6"), QColor("#FCA700"),
        QColor("#6DC781"), QColor("#9B59B6"), QColor("#E74C3C"),
        QColor("#3498DB"), QColor("#1ABC9C"), QColor("#E67E22"),
        QColor("#2ECC71"), QColor("#E91E63"), QColor("#FF5722"),
    };

    for (int i = 0; i < 12; ++i) {
        auto *block = new QWidget();
        block->setFixedSize(280, 200);
        block->setStyleSheet(QString("background: %1; border-radius: 8px;")
                            .arg(colors[i % colors.size()].name()));

        auto *label = new QLabel(QString("Card %1").arg(i + 1), block);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: white; font-size: 18px; font-weight: bold; background: transparent;");
        label->setGeometry(0, 0, 280, 200);

        flow->addWidget(block);
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

    // Set test content with FlowLayout
    window.setContentWidget(createTestContent());

    window.show();
    return app.exec();
}
