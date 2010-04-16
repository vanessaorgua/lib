#include <QApplication>
#include "mainw.h"

#include "../ionetclient/IoNetClient.h"

int main(int argc,char **argv)
{
    QApplication app(argc,argv);

    IoNetClient s("localhost");

    mainW w(s);


    w.show();


    return app.exec();
}

