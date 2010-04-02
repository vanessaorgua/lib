#include <QtCore/QCoreApplication>

#include <RxModbus.h>


int main(int argc, char *argv[])
{
    // QCoreApplication a(argc, argv);
    RxModbus r;

    r.loadList(":/text/test_map.txt");

    //return a.exec();
}
