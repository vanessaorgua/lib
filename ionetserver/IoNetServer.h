#ifndef __RNETIOSERVER_H__
#define __RNETIOSERVER_H__

#include <QObject>
#include <QtNetwork>
#include <QtCore>

class IoDev;

struct _Header
{
    qint8   Cmd;        // 1 R|W читати/писати
    qint8   Type;       // 1 Обєкт T-теги | D-сирі діні
    qint8   iD ;        // 1 індекс IoDev
    qint16  Index;      // 2 Індекс елемента
    qint16  Len;        // 2 Довжина тіла даних
    QString host;       // загалом заголовок 7 байт
};

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

private:
    QVector<IoDev*> src;
    QTcpServer *m_pTcpServer;
    QHash<QTcpSocket*,struct _Header>  connState;
};


#endif
