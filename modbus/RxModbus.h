#ifndef RXMODBUS_H
#define RXMODBUS_H

// даний клас реалізує інтерфейс доступу до пам’яті контролера.


#include <QObject>
#include <QtNetwork>
//#include "../iodev.h"

#define  ASYNC

class QString;
class QTimer;


//цей клас реалізує методи шкалювання даних
//class ScaleDev;
#include "../scaledev.h"

// клас базується на шаблонному класі IoDev
// треба написати документацію

class RxModbus:  /*public QObject,*/ public ScaledIoDev // ,  public IoDev
{
    Q_OBJECT
public:
        RxModbus(); // кноструктор, треба уточнити
        ~RxModbus(); // поки-що тривіальний деструктор

        int loadList(QString fileName);
        void setHostName(QString hostName);
        void setPort(int Port);
        void start();

signals:
    void updateData(); // сигнал висилається коли отримано новідані не наслідується
    void Alert(QString); // будуть висилатися сигнали згідно із станом зв’язку

public slots:
            void sendValue(QString tag,qint16 v);
            void sendValue(QString tag,qint32 v);
            void sendValue(QString tag,double v);
            void sendValue(QString tag,QVector<qint16> &v);


private slots:
    void slotConnected (); // приєдналися
    void slotNewConnect();
    void slotTimeout(); // таймаут отримання даних від сервера
    void slotDisconnect(); // відєднання зі сторони сервера

    void slotError(QAbstractSocket::SocketError);

    void slotRead();
    //void slotSend();
private:
    QTcpSocket *pS; //сокет для зв’язку
    QString sHostname; // ім’я чи IP-адреса контролера
    int nPort; // номер порта, за замовчанням має бути 502
    qint16 nI; // це буде індекс пакунка
    int nLen; // довжина наступноно бока даних

    //QTimer *connSend; // таймер для відправки чергового запиту
    QTimer *connWait; // тайсер очікування перед спробою встановити нове з’єднання
    QTimer *connTimeout; // таймер таймауту з’єднання, можливо в нових версіях QT цей алгоритм буде непотрібен


    // Список запитів
    QVector<QByteArray> query_list;
    QVector<int> dataLen; // довжина відповіді в словах

    QVector<qint16> query_read,local_read;

#ifdef ASYNC
    QQueue<QByteArray> query_queue; // черга на відправку даних в контролер
#endif

    int nC;
    qint16 Index;

};


#endif // RXMODBUS_H
