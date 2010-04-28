#include "alert.h"
#include "../iodev.h"
#include "../ionetserver/IoNetServer.h"

#include <QtCore>
#include <QDebug>

Alert::Alert(IoDev *src): s(src)
{


}


void Alert::checkAlert()
{
    //qDebug() << "signal updateData()";
    QString str;


    foreach(str,diAlert.keys())
    {
        if(s->getTags().contains(str)) // перевірити, чи є що перевіряти
        {
            bool f,o,v;
            f=diAlert[str][0].toInt(); // що сигналізуємо
            o=diAlert[str][3].toInt(); // що там було, попереднє значення
            v=s->getValue16(str); // отримали, поточне значення
            //qDebug() << "Tag " << str << "f" << f << "o" << o << "v" << v;

            //f~ov+~fov
            if(( f & !o & v ) | (!f & o & !v )) // вираз оптимізований на основі карти карно
            {
                emit newAlert(diAlert[str][2]);
                //qDebug() << "emit newAlert(diAlert[str][2])" ;
            }
            diAlert[str][3]=v?"1":"0";
        }
    }
}


int Alert::loadList(QString fileName)
{
    QFile f(fileName);
    QStringList sl;
    int i=0;

    if(f.open(QIODevice::ReadOnly))
    {
        for(i=0;!f.atEnd();++i)
        {
            sl=QString::fromUtf8(f.readLine()).trimmed().split("\t");
            if(sl.size()>3)
            {
                diAlert[sl[0]] << sl[1] << sl[2] << sl[3] << sl[1]; // в останньому полі зберігаю останнє оброблене значення
            }
            else
            {
                qDebug() << "Few options "  << sl;
            }
        }
        f.close();
        //qDebug() << diAlert;
    }
    else
    {
        qDebug() << "Unamble to open file " << fileName;
    }
    return i;
}
