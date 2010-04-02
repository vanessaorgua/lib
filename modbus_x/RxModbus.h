#ifndef RXMODBUS_H
#define RXMODBUS_H

// даний клас реалізує інтерфейс доступу до пам’яті контролера.


#include <QObject>
#include <QtNetwork>

class QString;
class QTimer;

class RxModbus: public QObject
{
    Q_OBJECT
public:
        RxModbus(); // кноструктор, треба уточнити
        ~RxModbus(); // поки-що тривіальний деструктор

        int loadList(QString fileName);

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
    QStringList tag_name;            // назва змінної
    QVector<qint32> tag_index;       // індекс в контролері
    QVector<qint32> tag_len;         // довжина поля в словах
    QVector<qint32> tag_history;     // прапор "запис в історію"
    QVector<qint32> tag_read;        // прапор "завжди читати"
    QVector<qint16> data;            // сирі дані із PLC



};


#endif // RXMODBUS_H
