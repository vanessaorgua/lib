#ifndef __RNETIOSERVER_H__
#define __RNETIOSERVER_H__

#include <QObject>
#include <QtNetwork>
#include <QtCore>

class ;

struct _Header
{
    qint8 Cmd;
    qint8 Type;
    short Index;
    short Len;
    QString host;
};

class RIoNetServer : public QObject
{
Q_OBJECT
public:
    RIoNetServer(RExchange *source,int nPort=8184);
    ~RIoNetServer();

public slots:
    virtual void slotNewConnection();
	    void slotReadClient();
	    void slotDisconnect();

private:
    RExchange *src;
    QTcpServer *m_pTcpServer;
    QHash<QTcpSocket*,struct _Header>  connState;
};


#endif
