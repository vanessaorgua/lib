#include <QApplication>
#include <QSettings>

#include "../iodev.h"
#include "RxModbus.h"
#include "form.h"
#include "../logging/logging.h"
#include "../ionetserver/IoNetServer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Rabitsa");
    QCoreApplication::setApplicationName("test");

    QSettings set;
    set.setValue("/db/db","lynovycya");


    RxModbus r;

    r.loadList(":/text/list.txt");
    r.setHostName("192.168.1.51");
    r.setPort(502);

    QVector<IoDev*> src; // джерела даних.
    src << &r;
    Logging l(src);

    r.start();

    IoNetServer s(src);

    Form w(&r);
    w.show();

    return a.exec();
}

