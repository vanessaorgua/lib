#include <QtCore/QCoreApplication>

#include <RxModbus.h>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    RxModbus r;

    r.loadList(":/text/test_map.txt");
    r.setHostName("192.168.1.51");
    r.setPort(502);

    r.start();




    return a.exec();
}
