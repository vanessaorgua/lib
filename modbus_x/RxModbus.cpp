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
    for(int i=0;i<query_list.size();++i)
    {
        if(1>local_read[i])
        {
            pS->write(query_list[i]);
            local_read[i]=query_read[i];
        }
        local_read[i]--;
    }
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
    qint16 next_addr=0,current_addr=0; //адреси
    qint16 current_len=0,packet_len=0; // поточна довжина
    qint8 current_rf=0,last_rf=0; // прапори читання

    QByteArray query;
    QDataStream qry(&query,QIODevice::WriteOnly);

    qry.setByteOrder(QDataStream::BigEndian); // встановити порядок байт

    qDebug() << "file " << fileName;

        // очистити все на випадок повторного завантаження
        tag_name.clear();
        tag_index.clear();
        tag_read.clear();
        tag_history.clear();
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
                tag_name << sl[0]; // назва тега
                current_addr=sl[1].toInt(); // індекс, тут би для повного щася треба б було перевірити чи воно правильно перетворилося на число
                tag_index << current_addr ; // зберегти
                tag_history << sl[4].toInt(); // прапори
                current_rf=sl[3].toInt();
                tag_read << current_rf ;
                // розпізнати типи даних
                if(sl[2]=="Integer" || sl[2]=="Bool" )
                {
                    ++wc;
                    current_len=1;
                    tag_len << current_len;
                }
                else if (sl[2]=="Real" || sl[2]=="Timer" || sl[2]=="Long" )
                {
                    wc+=2;
                    current_len=2;
                    tag_len << current_len;
                }
                else // невідомий тип даних
                {
                    qDebug() << tr("Unknown data type");
                    ::exit(1);
                }
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
                    qry << qint16(0) << qint16(0) << qint16(6) << qint8(1) <<  qint8(3) << qint16(current_addr-1); // ід транзакції << ід протокола << довжина << адреса слейва << код функції << стартова адреса
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
        data_raw.fill(wc); // ініціалізувати пам’ять під змінні
        f.close();
        return i;
    }
    else
    {
        return 0;
    }

}
