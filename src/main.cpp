#include <QApplication>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Bili Desktop UI");
    window.resize(1280, 800);
    window.setMinimumSize(1000, 650);
    window.show();

    return app.exec();
}
