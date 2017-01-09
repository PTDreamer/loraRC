#include "lorarc.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoraRC w;
    w.show();

    return a.exec();
}
