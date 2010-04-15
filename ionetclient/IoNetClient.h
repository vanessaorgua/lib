#ifndef __RNETIOCLIENT_H__
#define __RNETIOCLIENT_H__

#include <QObject>
#include <QtNetwork>
#include <QtCore>

#include "../iodev.h"
#include "../header.h"

class IoDev;
// клас є нащадком IoDev

class IoNetClient : public IoDev
{
Q_OBJECT
public:
    IoNetClient(QString hostname,int nPort=8184);
    ~IoNetClient();

public slots: // це все має відношення до з’єднання із сервером
	void slotNewConnect();
	void slotTimeout(); // таймаут отримання даних від сервера
        void slotDisconnect(); // відєднання зі сторони сервера
	void slotConnected();
	void slotReadServer();
	void slotError(QAbstractSocket::SocketError);
        void slotSendQuery();

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

};

#endif

