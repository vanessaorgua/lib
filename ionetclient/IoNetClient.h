#ifndef __RNETIOCLIENT_H__
#define __RNETIOCLIENT_H__

#include <QObject>
#include <QtNetwork>
#include <QtCore>

// визначити довжину масивів
#define VALUE_LEN 52
#define VALUERAW_LEN 18
#define PARM_LEN 126
#define CTRL_LEN 32

struct _Header
{
    qint8 Cmd;
    qint8 Type;
    short Index;
    short Len;
};

class RIoNetClient : public QObject
{
Q_OBJECT
public:
    RIoNetClient(QString hostname,int nPort=8184);
    ~RIoNetClient();

    // функції доступу - інтефейс як у відповідного класу сервернох частини
    const QVector<short>& getValue();
    const QVector<short>& getCtrl();
    const QVector<short>& getParm();

    const QVector<double>& getValueRaw();
    const QVector<double>& getValueScale();

    const QVector<double>& getScaleMin();
    const QVector<double>& getScaleMax();

public slots:
	void slotNewConnect();
	void slotTimeout(); // таймаут отримання даних від сервера
	void slotDisconnect(); // відєднання зі сторони сервера

	void slotConnected();
	void slotReadServer();
	void slotSendQuery();
	void slotSendData(qint8 Type,short Index,QVector<short>& data);
	void slotSendData(qint8 Type,short Index,double& data);
	void slotError(QAbstractSocket::SocketError);

private:
    // масиви, які містять сирі дані із контролера копія даних із сервера
    QVector<short> Value;
    QVector<short> Parm;
    QVector<short> Ctrl; 
    // дві черги, для передачі даних в контралер значень із масивів Parm та Ctrl
       
    // оброблені дані
    QVector<double> Value_raw;           // перетворені значення 0-4000
    QVector<double> Value_scale;         // шкальовані дані
    // шкали читати треба в конструкторі, запис - у функціях доступу.
    QVector<double> Scale_min,Scale_max; // шкали. потрібно буде зберігати.
    
    struct _Header connState;
    QString host;
    int Port;
    QTcpSocket *pTcpSock;
    QByteArray query;
    QTimer *rtmr;
    QTimer *connWait; // тайсер очікування перед спробою встановити нове з’єднання
    QTimer *connTimeout;
};

#endif

