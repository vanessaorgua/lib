#ifndef NETIODEV_H
#define NETIODEV_H

class IoNetClient;

#include "../iodev.h"

class NetIoDev: public IoDev
{
public:
    NetIoDev(IoNetClient *parent);

    virtual void sendValue(QString tag,qint16 v);
    virtual void sendValue(QString tag,qint32 v);
    virtual void sendValue(QString tag,double v);
    virtual void sendValue(QString tag,QVector<qint16> &v);

    // це незовсім правильно, доступ до цього повинен бути тільки в класу IoClient
    inline void setTags(QHash<QString,QVector<qint16> > t) {tags=t;}
    inline void setData(QVector<qint16> t) { data_raw=t;}

private:
    IoNetClient *p;

};

#endif // NETIODEV_H
