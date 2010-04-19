#include <QApplication>
#include "mainw.h"

#include "../ionetclient/IoNetClient.h"

int main(int argc,char **argv)
{
    QApplication app(argc,argv);

    IoNetClient s("localhost");

    mainW w(s);

    QObject::connect(&s,SIGNAL(updateData()),&w,SLOT(slotUpdate()));

    w.show();


    return app.exec();
}

