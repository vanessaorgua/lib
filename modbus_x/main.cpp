#include <QtCore/QCoreApplication>

#include <RxModbus.h>


int main(int argc, char *argv[])
{
    // QCoreApplication a(argc, argv);
    RxModbus r;

    r.loadList(":/text/list.txt");

        exit(0);
    //return a.exec();
}
