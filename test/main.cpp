#include <QApplication>

#include "RxModbus.h"
#include "form.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RxModbus r;

    r.loadList(":/text/list.txt");
    r.setHostName("192.168.1.51");
    r.setPort(502);

    r.start();
    Form w(&r);
    w.show();

    return a.exec();
}
