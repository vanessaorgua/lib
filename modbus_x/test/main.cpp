#include <QApplication>

#include "RxModbus.h"
#include "form.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RxModbus r;

    r.loadList(":/text/list.txt");
    r.setHostName("rabitsa.org.ua");
    r.setPort(502);

    r.start();
    Form w(&r);
    w.show();

    return a.exec();
}
