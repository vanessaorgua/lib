#include "RxModbus.h"

#include <QString>
#include <QTimer>


RxModbus::RxModbus(): QObject(),nPort(502) // кноструктор, треба уточнити
{

    // теймер для періодичної відправки запитів
    connSend=new QTimer(this);
    connSend->setInterval(1000);
    connect(connSend,SIGNAL(timeout()),this,SLOT(slotSend()));
    // теймер паузи між спробами встановити нове з’єднання
    connWait=new QTimer(this);
    connWait->setInterval(5000);
    connect(connWait,SIGNAL(timeout()),this,SLOT(slotNewConnect()));
    // таймер для відліку таймайту з’єднання
    connTimeout=new QTimer(this);
    connTimeout->setInterval(10000);
    connect(connTimeout,SIGNAL(timeout()),this,SLOT(slotTimeout()));

    // сокет для здійснення обміну даними
    pS=new QTcpSocket(this);
    connect(pS,SIGNAL(connected()),this,SLOT(slotConnected()));
    connect(pS,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotError(QAbstractSocket::SocketError)));
    connect(pS,SIGNAL(readyRead()),this,SLOT(slotRead()));
    connect(pS,SIGNAL(disconnected()),this,SLOT(slotDisconnect()));

    pS->connectToHost(sHostname,nPort);
    // десь тут ще потрібно сформувати пакунок на запити
}

RxModbus::~RxModbus() // поки-що тривіальний деструктор
{
    pS->close();
}

void RxModbus::slotConnected () // приєдналися
{
    connSend->start();
    connTimeout->start();
    nLen=0;
    pS->write("");
}

void RxModbus::slotNewConnect()
{
    connWait->stop();
    pS->connectToHost(sHostname,nPort);
}

void RxModbus::slotTimeout() // таймаут отримання даних від сервера
{
    connSend->stop();
    connTimeout->stop();
    connWait->start();
    pS->close();
}

void RxModbus::slotDisconnect() // відєднання зі сторони сервера
{
    connSend->stop(); // зупинити таймер, коли від’єднано немає сенсу слати запити
    pS->close();
}

void RxModbus::slotError(QAbstractSocket::SocketError)
{
    connSend->stop();
    connTimeout->stop();
    connWait->start();
    pS->close();
}


void RxModbus::slotSend()
{

}


void RxModbus::slotRead()
{

    connTimeout->stop();
    connTimeout->start();
}

int RxModbus::loadList(QString fileName)
{

    QFile f(fileName);
    int i;
    QString s;
    QStringList sl;
    int wc=0 ; // лічильник слів
    qint16 next_addr=0,start_addr=0,current_addr; //адреси
    qint16 current_len; // поточна довжина

    QByteArray query;
    QDataStream qry(&query,QIODevice::WriteOnly);

    qry.setByteOrder(QDataStream::BigEndian); // встановити порядок байт

    qDebug() << "file " << fileName;

    qry << qint16(0) << qint16(0) << qint16(0) << qint8(1) <<  qint8(4) ;


    if(f.open(QIODevice::ReadOnly))
    {
        for(i=0;!f.atEnd();++i)
        {
            s=QString::fromUtf8(f.readLine()).trimmed();
            sl= s.split("\t");
            if(sl.size()>4) // якщо є всі поля
            {
                tag_name << sl[0];
                tag_index << current_addr=sl[1].toInt(); // тут би для повного щася треба б було перевірити чи воно правильно перетворилося на число
                tag_history << sl[3].toInt();
                tag_read << sl[4].toInt();
                // розпізнати типи даних
                if(sl[2]=="Integer" || sl[2]=="Bool" )
                {
                    ++wc;
                    tag_len << current_len=1;
                }
                else if (sl[2]=="Real" || sl[2]=="Timer" || sl[2]=="Long" )
                {
                    wc+=2;
                    tag_len << current_len=2;
                }
                else // невідомий тип даних
                {
                    qDebug() << tr("Unknown data type");
                    ::exit(1);
                }

            }

        }
        qDebug() << tag_name;
        qDebug() << tag_index;
        qDebug() << tag_len;
        qDebug() << wc;

        f.close();
        return i;
    }
    else
    {
        return 0;
    }

}
