#include <QtCore/QCoreApplication>

#include "RxModbus.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    RxModbus r;

    r.loadList(":/text/list.txt");
    r.setHostName("192.168.1.51");
    r.setPort(502);

    r.start();




    return a.exec();
}
