#include "netiodev.h"
#include "../iodev.h"

#include "IoNetClient.h"


NetIoDev::NetIoDev(IoNetClient *parent) : p(parent)
{


}

NetIoDev::~NetIoDev()
{

}


void NetIoDev::sendValue(QString tag,qint16 v)
{

}

void NetIoDev::sendValue(QString tag,qint32 v)
{

}

void NetIoDev::sendValue(QString tag,double v)
{

}
void NetIoDev::sendValue(QString tag,QVector<qint16> &v)
{

}
