#ifndef __RNETIOCLIENT_H__
#define __RNETIOCLIENT_H__

#include <QObject>
#include <QtNetwork>
#include <QtCore>

#include "../iodev.h"
#include "../header.h"
#include "netiodev.h"


//class NetIoDev;
// клас є нащадком IoDev

class IoNetClient : public QObject
{
Q_OBJECT
public:
    IoNetClient(QString hostname,int nPort=8184);
    ~IoNetClient();

    // методи доступу
    NetIoDev *operator[](int i) { return i<src.size()? src[i] :  zero ; }


public slots: // це все має відношення до з’єднання із сервером
	void slotNewConnect();
	void slotTimeout(); // таймаут отримання даних від сервера
        void slotDisconnect(); // відєднання зі сторони сервера
	void slotConnected();
	void slotReadServer();
	void slotError(QAbstractSocket::SocketError);
        void slotSendQuery();

signals:
        void updateData();
        void updateData(int);
        void updateDataRaw();
        void updateDataScaled();
        void Alert(QString);

private:
    // масиви, які містять сирі дані із контролера копія даних із сервера
    struct _Header connState;
    QString host;
    int Port;
    QTcpSocket *pTcpSock;
    QByteArray query;
    QTimer *rtmr;
    QTimer *connWait; // тайсер очікування перед спробою встановити нове з’єднання
    QTimer *connTimeout;

   // решта даних та функцій доступу наслідуються із батьківського класу
    QVector<NetIoDev*> src;

    NetIoDev* zero;

    friend class NetIoDev; // чи не забагато друзів, щоб не додавати зайві методи ?

};

#endif

