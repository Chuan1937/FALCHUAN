#include "FALCHUAN.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FALCHUAN w;
    w.show();
    return a.exec();
}
