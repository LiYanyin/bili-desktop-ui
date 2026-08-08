#include <QApplication>
#include "widgets/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application-wide style
    app.setStyleSheet(R"(
        QWidget {
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
        }
    )");

    MainWindow window;
    window.setWindowTitle("Bili Desktop UI");
    window.show();

    return app.exec();
}
