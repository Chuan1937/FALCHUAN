#include <QApplication>
//#include <QPushButton>
#include "Sources/Headers/mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow window;
    window.show();
    return QApplication::exec();
}

