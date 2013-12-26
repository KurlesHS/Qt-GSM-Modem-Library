#include <QCoreApplication>
#include "worker.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Worker wa("COM3");
    int ax = a.exec();
    qDebug("exit");
    return ax;
}
