#include "RTBaseline.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RTBaseline w;
    w.show();
    return a.exec();
}
