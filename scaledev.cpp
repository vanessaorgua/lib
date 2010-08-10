#include "scaledev.h"

#include <QDebug>

ScaledIoDev::~ScaledIoDev()
{
}

void ScaledIoDev::loadScale(QString fn) // це завантажить шкали, це не повинно бути видно із-зовні
{


   QSettings s;
   s.beginGroup(QString("/ioserv/scale/%1").arg(objectName()));
   foreach(QString tag,tags.keys())
   {
        data_scale[tag] << 0.0
                << s.value(QString("%1/Zero").arg(tag),0.0).toDouble()
                << s.value(QString("%1/Full").arg(tag),100.0).toDouble();
   }
}

void ScaledIoDev::updateScaledValue() // це поновить шкальовані значення
{
    QString tag;
    double v;

    foreach(tag,data_scale.keys())
    {
        if(tags.contains(tag))
        {

            switch(tags[tag][2]) // тип даних
                {
                    default:
                    case 0: // Integer
                        v=getValue16(tag);
                        break;
                    case 1: // Bool
                        v=getValue16(tag)?1:0;
                        break;
                    case 2: // Real
                        v=getValueFloat(tag);
                        break;
                    case 3: // Timer
                    case 4: // Long
                        v=getValue32(tag);
                        break;
                }
            data_scale[tag][0]=(v/4000.0*(data_scale[tag][2]-data_scale[tag][1])+data_scale[tag][1]);
        }
        else
            qDebug() << "Tag not found " << tag;
    }
}


