#ifndef RXMODBUS_H
#define RXMODBUS_H

// даний клас реалізує інтерфейс доступу до пам’яті контролера.


#include <QObject>
#include <QtNetwork>

#define  ASYNC

class QString;
class QTimer;

class RxModbus: public QObject
{
    Q_OBJECT
public:
        RxModbus(); // кноструктор, треба уточнити
        ~RxModbus(); // поки-що тривіальний деструктор

        int loadList(QString fileName);
        void setHostName(QString hostName);
        void setPort(int Port);
        void start();
public slots:
            void sendValue(QString tag,qint16 v);
            void sendValue(QString tag,double v);
            void sendValue(QString tag,QVector<qint16> v);
private slots:
    void slotConnected (); // приєдналися
    void slotNewConnect();
    void slotTimeout(); // таймаут отримання даних від сервера
    void slotDisconnect(); // відєднання зі сторони сервера

    void slotError(QAbstractSocket::SocketError);

    void slotRead();
    void slotSend();
private:
    QTcpSocket *pS; //сокет для зв’язку
    QString sHostname; // ім’я чи IP-адреса контролера
    int nPort; // номер порта, за замовчанням має бути 502
    qint16 nI; // це буде індекс пакунка
    int nLen; // довжина наступноно бока даних

    QTimer *connSend; // таймер для відправки чергового запиту
    QTimer *connWait; // тайсер очікування перед спробою встановити нове з’єднання
    QTimer *connTimeout; // таймер таймауту з’єднання, можливо в нових версіях QT цей алгоритм буде непотрібен


    // Сховище даних. можливо треба буде переробляти
    QHash<QString,QVector<qint16> > tags; // таблиця тегів
    //QStringList tag_name;            // назва змінної
    //QVector<int> tag_index;       // індекс в масиві
    //QVector<qint16> tag_address;  // адреса змінної в контролері
    //QVector<qint8> tag_len;         // довжина поля в словах
    //QVector<qint8> tag_history;     // прапор "запис в історію"
    //QVector<qint8> tag_read;        // прапор "завжди читати"
    QVector<qint16> data_raw;            // сирі дані із PLC

    // Список запитів
    QVector<QByteArray> query_list;
    QVector<qint16> query_read,local_read;

#ifdef ASYNC
    QQueue<QByteArray> query_queue; // черга на відправку даних в контролер
#endif

    int nC;
    qint16 Index;

};


#endif // RXMODBUS_H
