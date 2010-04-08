#include "RxModbus.h"

#include <QString>
#include <QTimer>
#include <QtAlgorithms>

#define GETHR 3
#define PUTHR 16


RxModbus::RxModbus(): QObject(),nPort(502) ,nC(0) // кноструктор, треба уточнити
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
    qDebug() <<  "Connected to host";
    slotSend(); // розпочати обмін
    nC=0;
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
    qDebug() << "Connection error";
}


// виявилося що не получається виконувавти асинхронні запити до контролера I-8000, це не дуже добре.

void RxModbus::slotSend()
{
#ifdef ASYNC

    if(1>local_read[0])
   {
      pS->write(query_list[0]);
      local_read[0]=query_read[0];
   }
   local_read[0]--;
    nC=1;
#else
    // асинхронне виконання
    for(int i=0;i<query_list.size();++i)
    {
        if(1>local_read[i])
        {
            pS->write(query_list[i]);
            local_read[i]=query_read[i];
        }
        local_read[i]--;
    }
#endif
}


void RxModbus::slotRead()
{
    QDataStream in(pS);
    in.setByteOrder(QDataStream::BigEndian); // встановити порядок байт
    qint16 v16; // змінна для різних потреб
    qint8  as,fc,bc;

    for(;;)
    {
        if(nLen==0) // читати заголовок
        {
            if(pS->bytesAvailable()<6) // якщо тут мало байт
            {
                break;
            }
            in >> Index; // id транзакції воно ж зміщення індекса в масиві даних
            in >> v16; // id протоколу
            in >> v16; // довжина пакунка
            nLen=v16;
        }
        if(pS->bytesAvailable()<nLen)
        {
            break;
        }

        // отримано весь пакунок, розібрати на частини
        in >> as; // адреса ведомого
        in >> fc; // код функції
        qDebug() << "Index" << Index;
        switch(fc)
        {
            case GETHR:
                in >> bc; // прочитати кількість байт
                bc >>= 1; // розрахувати кількість слів
                qDebug() << "bc" << bc;

                for(int i=0;i<bc;++i) // в циклі
                {
                        in >> v16; // прочитати слова
                        data_raw[Index+i]=v16; // та записати в масив даних
                }

                break;
            case PUTHR:
            default: // якщо якась неочікувана функція то просто очистити весь буфер
                qDebug() << "Uncnown fc" << fc;
                for(int i=2;i<nLen;++i)
                    in >>bc;
        }
#ifdef ASYNC
        // відправити наступний запит

        // тут треба попрацювати
        for(;1>local_read[nC] && nC<query_list.size();++nC)
            local_read[nC]--;
        pS->write(query_list[nC]);
        local_read[nC]=query_read[nC];

        if(!query_queue.isEmpty()) // перевірити чергу
        {
            pS->write(query_queue.dequeue()); // якщо не пуста, передати
        }
#endif
        qDebug() << "Packet #" <<nC << "Returned bytes:"<< nLen;
        qDebug() << data_raw;
        qDebug() ;

        nLen=0;
    }
    connTimeout->stop();
    connTimeout->start();
}

int RxModbus::loadList(QString fileName)
{

    QFile f(fileName);
    int i;
    QString s;
    QStringList sl;
    int wc=0, wc_last=0; // лічильник слів
    qint16 next_addr=0,current_addr=0; //адреси
    qint16 current_len=0,packet_len=0; // поточна довжина
    qint8 current_rf=0,last_rf=0; // прапори читання

    QByteArray query;
    QDataStream qry(&query,QIODevice::WriteOnly);

    qry.setByteOrder(QDataStream::BigEndian); // встановити порядок байт

    qDebug() << "file " << fileName;

        // очистити все на випадок повторного завантаження
        tags.clear();

        query_list.clear();
        query_read.clear();
        local_read.clear();

    if(f.open(QIODevice::ReadOnly))
    {
        for(i=0;!f.atEnd();++i)
        {
            s=QString::fromUtf8(f.readLine()).trimmed(); //читати
            sl= s.split("\t"); // розбити на поля
            if(sl.size()>4) // якщо є всі поля
            {
                s= sl[0]; // назва тега
                current_addr=sl[1].toInt(); // індекс, тут би для повного щася треба б було перевірити чи воно правильно перетворилося на число
                tags[s] << wc             // index
                        << current_addr ; // address
                current_rf=sl[3].toInt();
                wc_last=wc; // це потрібно для правильного формування поля id транзакції яке містить зміщення індексу в масиві даних
                // метод не зовсім стандартний, на інших контролерах може і не буде працювати
                // розпізнати типи даних
                if(sl[2]=="Integer" || sl[2]=="Bool" )
                {
                    ++wc;
                    current_len=1;
                }
                else if (sl[2]=="Real" || sl[2]=="Timer" || sl[2]=="Long" )
                {
                    wc+=2;
                    current_len=2;
                }
                else // невідомий тип даних
                {
                    qDebug() << tr("Unknown data type");
                    ::exit(1);
                }
                tags[s] << current_len   // довжина
                        << current_rf   //
                        << sl[4].toInt(); // прапори

                packet_len+=current_len;

                if(packet_len>124 || current_addr>next_addr || current_rf!=last_rf) //виявити дірки, межі пакунків, кратність читання.
                {
                    if(query.size()) // якщо щось є,
                    {
                        query_list <<  query; // зберегти
                        //qDebug() << query;
                    }

                    // підготуватися до нового запиту
                    qry.device()->seek(0);
                    query.clear();
                    // сформувати заголовок
                    packet_len=current_len;
                    qry << qint16(wc_last) << qint16(0) << qint16(6) << qint8(1) <<  qint8(GETHR) << qint16(current_addr-1); // ід транзакції << ід протокола << довжина << адреса слейва << код функції << стартова адреса
                                                                                          //^^^^^^^^^^^^^^^^^^^^^^ можливо для інших контролерів цей декримент непотрібен
                    query_read << current_rf; //прапор read на пакунок
                    local_read << 0;
                }
                else // в іншому разі поновити дані про довжину.
                {
                    qry.device()->seek(query.size()-2);
                }
                qry << packet_len; //додати довжину пакунка
                next_addr=current_addr+current_len; // розрахувати новий наступний очікуваний адрес
                last_rf=current_rf;

            }
        }

        if(query.size()) // зберегти останній запит.
        {
            query_list << query;
            query_read << current_rf;
        }
        qDebug() << query_list.size();
        qDebug() << wc;
        data_raw.resize(wc); // ініціалізувати пам’ять під змінні
        qDebug() << data_raw;

        f.close();
        return i;
    }
    else
    {
        return 0;
    }

}

void RxModbus::setHostName(QString hostName)
{
    sHostname=hostName;
}

void RxModbus::setPort(int Port)
{
    nPort=Port;
}

void RxModbus::start()
{
    // тут би треба зробити якісь додаткові перевірки
    pS->connectToHost(sHostname,nPort);
}

void RxModbus::sendValue(QString tag,qint16 v)
{

}

void RxModbus::sendValue(QString tag,double v)
{
}

void RxModbus::sendValue(QString tag,QVector<qint16> v)
{
   QByteArray q;
   QDataStream qry(&q,QIODevice::WriteOnly);

   qry.setByteOrder(QDataStream::BigEndian);

   if(tags.contains(tag) ) // перевірити наявність заданого тега
   {
        qry << qint16(0) << qint16(0) << qint16(qint16((v.size()<<1)+7))  // TCP заголовок
           << qint8(1) << qint8(PUTHR)               // модбас заголовок
           << qint16(tags[tag][1]-1)             // адреса даних
           << qint16(v.size())                     // довжина даних
           << qint8(v.size()<< 1);                 // кількість байт
        int x=tags[tag][0];
        foreach(qint16 t,v)
        {
             qry << t; // завантажити дані
             data_raw[x++]=t; // записати в буфер
        }
#ifdef ASYNC
        query_queue.enqueue(q); // поставити в чергу на відправку в контролер
#endif
    }
}


