#include "netiodev.h"
#include "../iodev.h"

#include "IoNetClient.h"
#include <QDebug>

NetIoDev::NetIoDev(IoNetClient *parent) : p(parent)
{


}

NetIoDev::~NetIoDev()
{

}


void NetIoDev::sendValue(QString tag,qint16 v)
{
    QVector<qint16> t;
    t << v;
    sendValue(tag,t);
}

void NetIoDev::sendValue(QString tag,qint32 v)
{
    QVector<qint16> t(2);
    *(qint32*)t.data()  = v;
    sendValue(tag,t);
}

void NetIoDev::sendValue(QString tag,double v)
{
    QVector<qint16> t(2);
    *(float*)t.data()  = (float)v;
    sendValue(tag,t);
}

void NetIoDev::sendValue(QString tag,QVector<qint16> &v)
{
    QByteArray query;
    QDataStream qry(&query,QIODevice::WriteOnly);
    qry.setVersion(QDataStream::Qt_4_2);

    qry << qint8('W') << qint8('D') << iD << qint16(0) << qint16(0) << tag << v;
    qry.device()->seek(5);
    qry << qint16(query.size()-7);

    p->pTcpSock->write(query);
    //qDebug() << tag << v;
}

void NetIoDev::sendValueScaled(QString tag,double v)
{
    if(data_scale.contains(tag))
    {
        QByteArray query;
        QDataStream qry(&query,QIODevice::WriteOnly);
        qry.setVersion(QDataStream::Qt_4_2);
        qry << qint8('W') << qint8('S') << iD << qint16(0) << qint16(0) << tag << v;
        qry.device()->seek(5);
        qry << qint16(query.size()-7);

        p->pTcpSock->write(query);

    }
}

void NetIoDev::setScaleZero(QString tag,double v)
{
    if(data_scale.contains(tag))
    {
        //qDebug() << "NetIoDev::setScaleZero("<< tag << "," << v << ")";
        QByteArray query;
        QDataStream qry(&query,QIODevice::WriteOnly);
        qry.setVersion(QDataStream::Qt_4_2);
        qry << qint8('W') << qint8('Z') << iD << qint16(0) << qint16(0) << tag << v;
        qry.device()->seek(5);
        qry << qint16(query.size()-7);

        p->pTcpSock->write(query);

    }
}


void NetIoDev::setScaleFull(QString tag,double v)
{
    if(data_scale.contains(tag))
    {
        //qDebug() << "NetIoDev::setScaleZero("<< tag << "," << v << ")";
        QByteArray query;
        QDataStream qry(&query,QIODevice::WriteOnly);
        qry.setVersion(QDataStream::Qt_4_2);
        qry << qint8('W') << qint8('F') << iD << qint16(0) << qint16(0) << tag << v;
        qry.device()->seek(5);
        qry << qint16(query.size()-7);

        p->pTcpSock->write(query);

    }
}


