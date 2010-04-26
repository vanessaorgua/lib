#ifndef __RNETIOSERVER_H__
#define __RNETIOSERVER_H__

#include <QObject>
#include <QtNetwork>
#include <QtCore>

#include "../header.h"

class IoDev;

class IoNetServer : public QObject
{
Q_OBJECT
public:
    IoNetServer(QVector<IoDev*> s,int nPort=8184);
    ~IoNetServer();

public slots:
    virtual void slotNewConnection();
	    void slotReadClient();
	    void slotDisconnect();
            void sendAlert(QString);

private:
    QVector<IoDev*> src;
    QTcpServer *m_pTcpServer;
    QHash<QTcpSocket*,struct _Header>  connState;
};


#endif
