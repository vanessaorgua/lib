#include "RxModbus.h"

#include <QString>
#include <QTimer>

#define GETMCR 1
#define PUTSCR 5
#define PUTMCR 15

#define GETMHR 3
#define PUTSHR 6
#define PUTMHR 16


RxModbus::RxModbus(): nPort(502) ,nC(0) // кноструктор, треба уточнити
{

    // теймер для періодичної відправки запитів
    //connSend=new QTimer(this);
    //connSend->setInterval(1000);
    //connect(connSend,SIGNAL(timeout()),this,SLOT(slotSend()));

    // теймер паузи між спробами встановити нове з’єднання
    connWait=new QTimer(this);
    connWait->setInterval(10000);
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
    //connSend->start();
    connTimeout->start();
    nLen=0;
    qDebug() <<  "Connected to host" << sHostname;
    // slotSend(); // розпочати обмін
    pS->write(query_list[0]);
    nC=0;
    emit Alert(QString("Connected to PLC: %1:%2").arg(sHostname).arg(nPort));
}

void RxModbus::slotNewConnect()
{
    connWait->stop();
    pS->connectToHost(sHostname,nPort);
}

void RxModbus::slotTimeout() // таймаут отримання даних від сервера
{
//   connSend->stop();
    connTimeout->stop();
    connWait->start();
    pS->close();
    emit Alert(QString("Connection to PLC lost: %1:%2").arg(sHostname).arg(nPort));
    qDebug() << QString("Connection to PLC lost: %1:%2").arg(sHostname).arg(nPort);

}

void RxModbus::slotDisconnect() // відєднання зі сторони сервера
{
    //connSend->stop(); // зупинити таймер, коли від’єднано немає сенсу слати запити
    pS->close();
}

void RxModbus::slotError(QAbstractSocket::SocketError)
{
    //connSend->stop();
    connTimeout->stop();
    connWait->start();
    //qDebug() << "Connection error";
    emit Alert(QString("Connection to PLC error: %1:%2. %3").arg(sHostname).arg(nPort).arg(pS->errorString()));
    pS->close();
}


// виявилося що не получається виконувавти асинхронні запити до контролера I-8000, це не дуже добре.
/*
void RxModbus::slotSend()
{
#ifdef ASYNC
    //qDebug() << "Start -------------------------------------------------------------------------------";
    if(1>local_read[0])
   {
      pS->write(query_list[0]);
      local_read[0]=query_read[0];
   }
   local_read[0]--;
   nC=0;
#else
    // асинхронне виконання
    //qDebug() << "slotSend";
    for(int i=1;i<query_list.size();++i)
    {
        if(1>local_read[i])
        {
            //qDebug() << i;
            pS->write(query_list[i]);
            local_read[i]=query_read[i];
        }
        local_read[i]--;
    }
#endif
}
*/

void RxModbus::slotRead()
{
    QDataStream in(pS);
    qint16 v16;
    qint8  as,fc;
    quint8 bc;

    in.setByteOrder(QDataStream::BigEndian); // встановити порядок байт

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
        qDebug() << "Start packet proccess Index" << Index << "nLen" << nLen << "as " << as << "fc" << fc;
        switch(fc)
        {
            case GETMCR:
                in >> bc; // прочитати кількість байт
                qDebug() <<  "dataLen " << dataLen[nC];
                for(int i=0;i<bc;++i)
                {
                    qint8 v;
                    in >> v; // a тепер це треба правильно розпакувати..........
                    for(int j=0;j<8;++j)
                    {
                        int ix=i*8+j;
                        if(ix<dataLen[nC])
                        {
                            data_raw[Index+ix]=qint16(v&0x01?-1:0); // все так от тільки чи влізе воно?
                        }
                        else
                            break;
                            v>>=1;
                    }
                }
                break;
            case GETMHR:
                in >> bc; // прочитати кількість байт
                bc >>= 1; // розрахувати кількість слів
                //qDebug() << "bc" << bc;

                for(int i=0;i<bc;++i) // в циклі
                {
                        in >> v16; // прочитати слова
                        data_raw[Index+i]=v16; // та записати в масив даних
                }

                break;

            case PUTSCR:
            case PUTMCR:
            case PUTSHR:
            case PUTMHR:
                //qDebug() << "Ok fc "<< fc << "nLen " << nLen;
                for(int i=2;i<nLen;++i)
                    in >>bc;
                break;
            default: // якщо якась неочікувана функція то просто очистити весь буфер
                qDebug() << "Uncnown fc" << fc;
                for(int i=2;i<nLen;++i)
                    in >>bc;
        }

#ifdef ASYNC
      //qDebug() << "nC " << nC  << "query_list.size()" << query_list.size() ;
        // відправити наступний запит
        ++nC;
        while(nC<query_list.size())
            {
                local_read[nC]--;
                if(1>local_read[nC])
                {
                    pS->write(query_list[nC]);
                    local_read[nC]=query_read[nC];
                    break;
                }
                ++nC;
            }
        // перерахувати шкальовані значення
        if(nC==query_list.size())
        {
            //qDebug() << "Calculate data_scale";
            updateScaledValue();
            emit updateData();
        }

        if(! (nC < query_list.size()))
        {
            //qDebug() << "Process query queue" ;
            if(query_queue.isEmpty()) // перевірити чергу при умові що інших запитів немає.
            {
                    pS->write(query_list[0]);
                    nC=0;

            }
            else
                pS->write(query_queue.dequeue()); // якщо не пуста, передати
        }
#endif
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

    qint16 current_rf=0,last_rf=0; // прапори читання

    QByteArray query;
    QDataStream qry(&query,QIODevice::WriteOnly);

    QHash<QString,QString> tag_scale; // тут будуть теги, які шкалюються по іншому параметру

    QStringList ft;
    ft << "Integer" << "Bool" << "Real" << "Timer" << "Long" << "EBOOL" ;
    qint16 current_ft=0,last_ft=0; // пити полів, для виявлення EBOOL

    qry.setByteOrder(QDataStream::BigEndian); // встановити порядок байт

    //qDebug() << "file " << fileName;

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
                //qDebug() << s;
                current_addr=sl[1].toInt(); // індекс, тут би для повного щася треба б було перевірити чи воно правильно перетворилося на число
                tags[s] << wc             // 0-index
                        << current_addr ; // 1- address
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
                else if(sl[2]=="EBOOL") // це спеціально для шнайдера
                {
                    ++wc;
                    current_len=1;
                    // тут треба зімітувати дірку
                }
                else // невідомий тип даних
                {
                    qDebug() << tr("Unknown data type");
                    ::exit(1);
                }

                current_ft=ft.indexOf(sl[2]);
                //qDebug() << sl[2] << ft[current_ft];
                tags[s] << current_ft   // 2-довжина !!! це місце треба перевірити
                        << current_rf   // 3-кратність читання
                        << sl[4].toInt(); // 4-прапори запису історії

                packet_len+=current_len;

                if(packet_len>124 || current_addr>next_addr || current_rf!=last_rf || (current_ft==5 && last_ft!=5) || (current_ft!=5 && last_ft==5)) //виявити дірки, межі пакунків, кратність читання чи зміну типу
                {
                    if(query.size()) // якщо щось є,
                    {
                        query_list <<  query; // зберегти
                        dataLen << packet_len;
                        //qDebug() << packet_len-current_len; //
                        //qDebug() << query;
                    }

                    // підготуватися до нового запиту
                    qry.device()->seek(0);
                    query.clear();
                    // сформувати заголовок
                    packet_len=current_len;
                    qry << qint16(wc_last) << qint16(0) << qint16(6) << qint8(1) <<  qint8(sl[2]=="EBOOL"?GETMCR:GETMHR) << qint16(current_addr-1); // ід транзакції << ід протокола << довжина << адреса слейва << код функції << стартова адреса
                                                                                          //^^^^^^^^^^^^^^^^^^^^^^ можливо для інших контролерів цей декримент непотрібен
                    //qDebug() << qint16(wc_last) << qint16(0) << qint16(6) << qint8(1) <<  qint8(sl[2]=="EBOOL"?GETMCR:GETMHR) << qint16(current_addr-1);
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
                last_ft=current_ft;

                if(sl.size()>6)
                    text[s]=sl[6] ; // назва тега
                else
                    text[s]="-";


                // цей код би винести в окремий клас
            if(sl[5]!="-")
            {
                if(sl[5]=="+")
                {
                    QSettings set;
                    set.beginGroup("/ioserv/scale/");
                        data_scale[s] << 0.0
                        << set.value(QString("Zero/%1").arg(s),0.0).toDouble()
                        << set.value(QString("Full/%1").arg(s),100.0).toDouble();
                }
                else
                {
                    QStringList f=sl[5].split(" "); // розбити поле на дві частини
                    if(f.size()==2) // якщо там його насправді двое
                    {
                        bool o1,o2;
                        double zs,fs;
                        zs=f[0].toDouble(&o1);
                        fs=f[1].toDouble(&o2);
                        if(o1 && o2)
                        {
                            data_scale[s] << 0.0 << zs << fs;
                        }
                        else
                        {
                            data_scale[s] << 0.0 << 0.0 << 100.0;
                        }
                    }
                    else // якщо дойшли сюди то там мабуть тег
                    {
                        tag_scale[s]=sl[5]; // запам’ятати
                    }
                }
            }


            }
        }

        if(query.size()) // зберегти останній запит.
        {
            query_list << query;
            dataLen << packet_len;
            //qDebug() << packet_len;
            query_read << current_rf;
        }
        data_raw.resize(wc); // ініціалізувати пам’ять під змінні

        f.close();

        // завантаженя відложених тегів, якщо є
        foreach(QString f,tag_scale.keys())
        {
            if(data_scale.contains(tag_scale[f]))
            {
                data_scale[f] << 0.0 << data_scale[tag_scale[f]][1] << data_scale[tag_scale[f]][2] ;
            }
            else
            {
                //qDebug() << "Scale not found tag"<< f << "scaled on " << tag_scale[f];
            }
        }

        //qDebug() << "Scaled tags " << data_scale.size() << "\n" << data_scale.keys();

        //qDebug() << tags.keys();

//        loadScale(fileName);
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
   QByteArray q;
   QDataStream qry(&q,QIODevice::WriteOnly);

   qry.setByteOrder(QDataStream::BigEndian);

   if(tags.contains(tag) ) // перевірити наявність заданого тега
   {
        qry << qint16(0) << qint16(0) << qint16(6)  // TCP заголовок
           << qint8(1) ;
        if(tags[tag][2]!=5)
        {
            qry << qint8(PUTSHR)             // модбас заголовок
           << qint16(tags[tag][1]-1)                // адреса даних
           << v;                            // самі дані
        }
        else
        {
            qry << qint8(PUTSCR)             // модбас заголовок
           << qint16(tags[tag][1]-1)                // адреса даних
           << qint16(v?0xFF00:0);                            // самі дані
        }

        data_raw[tags[tag][0]]=v; // записати в буфер
#ifdef ASYNC
        query_queue.enqueue(q); // поставити в чергу на відправку в контролер
#endif
    }
}

void RxModbus::sendValue(QString tag,qint32 v)
{
    QVector<qint16> t(2);
    *((qint32*)t.data())=v;
    sendValue(tag,t);
}


void RxModbus::sendValue(QString tag,double v)
{
    QVector<qint16> t(2);
    *((float*)t.data())=(float)v;
    sendValue(tag,t);
}

void RxModbus::sendValue(QString tag,QVector<qint16> &v)
{
   QByteArray q;
   QDataStream qry(&q,QIODevice::WriteOnly);

   qry.setByteOrder(QDataStream::BigEndian);

   if(tags.contains(tag) ) // перевірити наявність заданого тега
   {
        qry << qint16(0) << qint16(0) << qint16((v.size()<<1)+7)  // TCP заголовок
           << qint8(1) << qint8(PUTMHR)               // модбас заголовок
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


