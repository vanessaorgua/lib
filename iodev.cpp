#include <QSettings>

#include "iodev.h"


IoDev::~IoDev()
{

}


void IoDev::sendValueScaled(QString tag,double v)
{
    if(tags.contains(tag))
    {
        data_scale[tag][0]=v;
        QVector<qint16> t(2);
        *(float*)t.data()=(float)((v-data_scale[tag][1])/(data_scale[tag][2] - data_scale[tag][1]) *4000.0 );
        sendValue(tag,t);

    }
    else
        qDebug() << "Tag not found " << tag;

}

void IoDev::setScaleZero(QString tag,double v)
{
    qDebug() << "setScaleZero("<< tag << "," << v << ")";
    if(data_scale.contains(tag))
    {
        QSettings s;
        s.beginGroup("/ioserv/scale/Zero");
        s.setValue(tag,v);
        data_scale[tag][1]=v;
    }
    else
    {
        qDebug() << "Tag not found " << tag;
    }

}

void IoDev::setScaleFull(QString tag,double v)
{
    qDebug() << "setScaleZero("<< tag << "," << v << ")";
    if(data_scale.contains(tag))
    {
        QSettings s;
        s.beginGroup("/ioserv/scale/Full");
        s.setValue(tag,v);
        data_scale[tag][2]=v;
    }
    else
    {
        qDebug() << "Tag not found " << tag;
    }

}


