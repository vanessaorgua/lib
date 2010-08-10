#ifndef NETIODEV_H
#define NETIODEV_H

class IoNetClient;

#include "../iodev.h"

class NetIoDev: /*public QObject,*/ public IoDev
{
public:
    NetIoDev(IoNetClient *parent);
    ~NetIoDev();
    virtual void sendValue(QString tag,qint16 v);
    virtual void sendValue(QString tag,qint32 v);
    virtual void sendValue(QString tag,double v);
    virtual void sendValue(QString tag,QVector<qint16> &v);

    virtual void sendValueScaled(QString tag,double v) ;
    virtual void setScaleZero(QString tag,double v) ;
    virtual void setScaleFull(QString tag,double v) ;


private:
    IoNetClient *p;

    // становити правильний індекс
    // питання: а може краже назву ?
    qint8 iD;


    friend class IoNetClient; // це для того, щоб можна було напряму працювати із tags та data_raw

};

#endif // NETIODEV_H
