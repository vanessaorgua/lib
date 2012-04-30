#include "melsec.h"

#include <QString>
#include <QTimer>



RxMelsec::RxMelsec(): nPort(5002) ,nC(0),plcAddr(1) // кноструктор, треба уточнити
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
    connTimeout->setInterval(30000);
    connect(connTimeout,SIGNAL(timeout()),this,SLOT(slotTimeout()));

    // сокет для здійснення обміну даними
    pS=new QTcpSocket(this);
    connect(pS,SIGNAL(connected()),this,SLOT(slotConnected()));
    connect(pS,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotError(QAbstractSocket::SocketError)));
    connect(pS,SIGNAL(readyRead()),this,SLOT(slotRead()));
    connect(pS,SIGNAL(disconnected()),this,SLOT(slotDisconnect()));

    cmdpref["D"]=qint8(0xA8);
    cmdpref["X"]=qint8(0x9C);
    cmdpref["Y"]=qint8(0x9D);
    cmdpref["M"]=qint8(0x90);
    cmdpref["L"]=qint8(0x92);
    // десь тут ще потрібно сформувати пакунок на запити
}

RxMelsec::~RxMelsec() // поки-що тривіальний деструктор
{
    pS->close();
}

void RxMelsec::slotConnected () // приєдналися
{
    //connSend->start();
    connTimeout->start();
    nLen=0;
    qDebug() <<  "Connected to host" << sHostname;
    // slotSend(); // розпочати обмін
    pS->write(query_list[0]);
    // qDebug() << "query_list[0]" << query_list[0].size();
    nC=0;
    emit Alert(QString("Connected to PLC: %1:%2").arg(sHostname).arg(nPort));
}

void RxMelsec::slotNewConnect()
{
    connWait->stop();
    pS->connectToHost(sHostname,nPort);
}

void RxMelsec::slotTimeout() // таймаут отримання даних від сервера
{
//   connSend->stop();
    connTimeout->stop();
    connWait->start();
    pS->close();
    emit Alert(QString("Connection to PLC lost: %1:%2").arg(sHostname).arg(nPort));
    qDebug() << QString("Connection to PLC lost: %1:%2").arg(sHostname).arg(nPort);

}

void RxMelsec::slotDisconnect() // відєднання зі сторони сервера
{
    //connSend->stop(); // зупинити таймер, коли від’єднано немає сенсу слати запити
    pS->close();
}

void RxMelsec::slotError(QAbstractSocket::SocketError)
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
void RxMelsec::slotSend()
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

void RxMelsec::slotRead()
{
    QDataStream in(pS);
    qint16 v16;
    quint8 v8,bc;

    in.setByteOrder(QDataStream::LittleEndian); // встановити порядок байт

    // qDebug() << "slotRead()" <<  pS->bytesAvailable() ;

    for(;;)
    {

        if(nLen==0) // читати заголовок
        {
            if(pS->bytesAvailable()<9) // якщо тут мало байт
            {
                break;
            }
            // вичитати мусор, можливо воно щось означає...
            in >> v8;
            in >> v16;
            in >> v16;
            in >> v16;

            in >> v16; // вичитати довжину
            nLen=v16; // зкорегувати та зберегти довжину

            // qDebug() << "nLen" << nLen;

        }
        if(pS->bytesAvailable()<nLen)
        {
            break;
        }

        in >> v16; // знову мусор
        // отримано весь пакунок, розібрати на частини
        //qDebug() << "Start packet proccess Index" << Index << "nLen" << nLen << "as " << as << "fc" << fc;

        //тепер тут є чотири варіанти
        // 1 - отримано певну кількісит слів, котрі є регістрами D
        // 2 - отримано певну кількість тетрад (спакованих в байти) котрі є бітами
        // 3 - нічого не отримано, якщо перед цим була передача даних
        // 4 - отримано якесь повідомлення про помилку у відповідь на передачу даних.
        if(nC<query_list.size())  // якщо попереду був пакунок із query_list
        { // тоді треба зберегти отримані від контролера дані
            if(query_list[nC][18]==0xA8) // що то за дані якщо 0xA8 згачить D-регістри
            {
                for(int i=0;i<(nLen-2)/2;++i) // читати слова
                {
                    in >> v16;
                    data_raw[Index[nC]+i]=v16;
                }
            }
            else  // інакще там біти
            {
                for(int i=0;i<nLen-2;++i)
                {
                    in >> v8;
                    data_raw[Index[nC]+i*2]=v8&0x10?-1:0;  // якось так.
                    data_raw[Index[nC]+i*2+1]=v8&0x1?-1:0;
                }
            }
        }
        else // інакще щоб там не було споржнити до кінця
        {
            while(!in.atEnd()) // чи це спрацює ? треба ретельно перевірити....
                in >> v8;
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

    // qDebug() << data_raw;

}

int RxMelsec::loadList(QString fileName)
{

    QFile f(fileName);
    int i;
    QString s;
    QStringList sl;
    QString type,type_old;
    int wc=0, wc_last=0; // лічильник слів

    qint16 next_addr=0,current_addr=0; //адреси

    qint16 current_len=0,packet_len=0; // поточна довжина

    qint16 current_rf=0,last_rf=0; // прапори читання

    QByteArray query;
    QDataStream qry(&query,QIODevice::WriteOnly);
    qry.setByteOrder(QDataStream::LittleEndian);

    QHash<QString,QString> tag_scale; // тут будуть теги, які шкалюються по іншому параметру

    QStringList ft;
    ft << "Integer" << "Bool" << "Real" << "Timer" << "Long" << "EBOOL" ;
    qint16 current_ft=0,last_ft=0; // пити полів, для виявлення EBOOL



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
            //qDebug() << i << s;
            sl= s.split("\t"); // розбити на поля
            if(sl.size()>4) // якщо є всі поля
            {
                s= sl[0]; // назва тега
                //qDebug() << s;

                type_old=type;
                type=sl[1].left(1);
                if(type=="X" || type=="Y") // це кодується в 16-й системі
                    current_addr=sl[1].right(sl[1].size()-1).toInt(0,16);
                else
                    current_addr=sl[1].right(sl[1].size()-1).toInt(); // індекс, тут би для повного щася треба б було перевірити чи воно правильно перетворилося на число


                tags[s] << wc             // 0-index
                        << current_addr ; // 1- address


                current_rf=sl[3].toInt();
                wc_last=wc; // це потрібно для правильного формування поля id транзакції яке містить зміщення індексу в масиві даних
                            // метод не зовсім стандартний, на інших контролерах може і не буде працювати

                // розпізнати типи даних
                if(type=="D")
                {
                    if(sl[2]=="Integer"  )
                    {
                        ++wc;
                        current_len=1;
                    }
                    else if (sl[2]=="Real" || sl[2]=="Timer" || sl[2]=="Long" )
                    {
                        wc+=2;
                        current_len=2;
                    }
                    else
                    {
                        qDebug() << tr("Unknown data type") << sl[2] << sl;
                        ::exit(1);
                    }
                }
                else if(type=="M" || type=="L" || type=="X" || type=="Y") // це точно байт
                {
                    ++wc;
                    current_len=1;
                    // тут треба зімітувати дірку
                }
                else // невідомий тип даних
                {
                    qDebug() << tr("Unknown data type") << type << sl[2] << sl;
                    ::exit(1);
                }

                current_ft=ft.indexOf(sl[2]);
                //qDebug() << sl[2] << ft[current_ft];
                tags[s] << current_ft   // 2-довжина !!! це місце треба перевірити
                        << current_rf   // 3-кратність читання
                        << sl[4].toInt() // 4-прапори запису історії
                        << 0  // 5 шкаліровка, може мінятися далі в програмі
                        << cmdpref[type] ; // 6 тип змінної

                packet_len+=current_len;

                if( // packet_len>124 ||
                   current_addr>next_addr ||         // виявити дірки,
                   current_rf!=last_rf ||            // межі пакунків,
                   type != type_old )                 // зміна типу
                   // (current_ft==5 && last_ft!=5) ||  // кратність читання
                   // (current_ft!=5 && last_ft==5))    // чи зміну типу
                {
                    // Зберегти індекс в масиві даних
                    Index << wc_last;

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
                        // сформувати запит
                        qry << qint8(0x50) << qint8(0)  // subheader
                        << qint8(1)                     // netv No
                        << plcAddr                      // Addres PLC
                        << qint8(0xFF) << qint8(0x03) << qint8(0x0) // не знаю що це
                        << qint8(0x0c) << qint8(0x0)     // data length
                        << qint8(0x30) << qint8(0x0)    // timer
                        << qint8(0x01) << qint8(0x4)    // command
                        << (type=="D" ?qint8(0x0):qint8(1)) << qint8(0x0)     // subcommand
                        << current_addr << qint8(0x0)     // start
                                                        // адреса задається в трома байтами, тут старший завжди нуль, відповідно можна отримати тільки 65536 слів
                        << cmdpref[type] ;                  // Data type
                        //<< qint16(0x0);    // len
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
                //qDebug() << "sl[5] " << sl[5];
                if(sl[5]=="+")
                {
                    tags[s][5]=1;
                    QSettings set;
                    //qDebug() << QString("/ioserv/scale/%1").arg(objectName());

                    set.beginGroup(QString("/ioserv/scale/%1").arg(objectName()));

                        data_scale[s] << 0.0
                        << set.value(QString("Zero/%1").arg(s),0.0).toDouble()
                        << set.value(QString("Full/%1").arg(s),100.0).toDouble();

                        //qDebug() << QString("Zero/%1").arg(s) <<  set.value(QString("Zero/%1").arg(s),0.0).toDouble();
                        //qDebug() << QString("Full/%1").arg(s) <<  set.value(QString("Full/%1").arg(s),100.0).toDouble() << "\n\n";

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

        //loadScale(fileName);
/*
        qDebug() << "Query list ------------";
        foreach(QByteArray ba, query_list)
        {
            QString out="";
            foreach(qint8 b,ba)
            {
                out+=QString("0x%1,\n").arg(qint32(b)&0xFF,2,16,QChar('0'));
            }
            qDebug() << ba.size() << "--\n" << out << "--";
        }
        qDebug() << "-----------------------"; */

        return i;
    }
    else
    {
        return 0;
    }

}

void RxMelsec::setHostName(QString hostName)
{
    sHostname=hostName;
}

void RxMelsec::setPort(int Port)
{
    nPort=Port;
}

void RxMelsec::start()
{
    // тут би треба зробити якісь додаткові перевірки
    pS->connectToHost(sHostname,nPort);
}

void RxMelsec::sendValue(QString tag,qint16 v)
{
    /*
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
    } */
}

void RxMelsec::sendValue(QString tag,qint32 v)
{
    /*
    QVector<qint16> t(2);
    *((qint32*)t.data())=v;

    if(data_scale.contains(tag)) // якщо датий  тег присутній в масиві шкальованих значень тоді відшкалювати його
        data_scale[tag][0]=((double)v/4000.0*(data_scale[tag][2]-data_scale[tag][1])+data_scale[tag][1]);

    sendValue(tag,t);
*/
}


void RxMelsec::sendValue(QString tag,double v)
{
/*    QVector<qint16> t(2);
    *((float*)t.data())=(float)v;

    if(data_scale.contains(tag)) // якщо датий  тег присутній в масиві шкальованих значень тоді відшкалювати його
    {
        data_scale[tag][0]=(v/4000.0*(data_scale[tag][2]-data_scale[tag][1])+data_scale[tag][1]);
        qDebug() << "tag "  << tag << " value " <<  v << "scaled " << data_scale[tag][0];
    }

    sendValue(tag,t); */
}

void RxMelsec::sendValue(QString tag,QVector<qint16> &v)
{

   QByteArray q;
   QDataStream qry(&q,QIODevice::WriteOnly);

   qry.setByteOrder(QDataStream::LittleEndian);
   QVector<qint8> vp; // масив, у якому будуть підгтовані для передачі дані
   qint16 pLen,vLen=v.size();


   if(tags.contains(tag) ) // перевірити наявність заданого тега
   {

   if(quint8(tags[tag][6])!=0xA8)  // якщо D-регістер
   {
       if(tags[tag][6]==0x9D) // якщ там Y-решісти
           return; // далі можна не продовжувати
       if(v.size()%2) // кількість змінних має бути парною, інакше буде проблема
           v << 0; // дописати нуль, контролер його проігнорує
     for(int i=0;i<v.size();i+=2)
           vp << qint8( (v[i]?0x10:0 )| (v[i+1]?0x01:0)  );

     pLen=vp.size();
     qDebug() << "vp" <<  vp;
   }
   else
   {
       pLen=v.size()*2;
   }

   //qDebug() << tag << QString("%1").arg(qint32(tags[tag][6])&0xff,2,16,QChar('0')) <<  v;


       qry << qint8(0x50) << qint8(0)                                 // subheader
       << qint8(1)                                                    // netv No
       << plcAddr                                                     // Addres PLC
       << qint8(0xFF) << qint8(0x03) << qint8(0x0)                    // не знаю що це
       << qint16(pLen+12)          // data length
       << qint8(0x30) << qint8(0x0)                                   // timer
       << qint8(0x01) << qint8(0x14)                                  // command
       << ( quint8(tags[tag][6])==0xA8 ?qint8(0x0):qint8(1)) << qint8(0x0)    // subcommand
       // << qint8(0x1) << qint8(0x0)    // subcommand

       << qint16(tags[tag][1]) << qint8(0x0)     // start
                                                 // адреса задається в трома байтами, тут старший завжди нуль, відповідно можна отримати тільки 65536 слів
       << qint8(tags[tag][6])                           // Data type
       << vLen ;                     // Розмір масиву з даними.


        if(quint8(tags[tag][6])==0xA8)
        {
            foreach(qint16 t,v)
            {
             qry << t; // завантажити дані
            }
        }
        else
        {
            foreach(qint8 t,vp)
            {
             qry << t; // завантажити дані
            }
        }

        // наступний фрагмент занесе змінені дані в сховище
        int x=tags[tag][0];
        foreach(qint16 t,v)
            data_raw[x++]=t; // записати в буфер

        {       QString s="";
        qDebug() << "sendValue() ----------------------------";
        foreach (quint8 val, q) {
            s+=QString("%1 ").arg(quint32(val)&0xFF,2,16,QChar('0'));
        }
        qDebug() << "Size" << q.size() << "\n" << s;
        qDebug() << "-----------------------------------------";
 }
#ifdef ASYNC
       query_queue.enqueue(q); // поставити в чергу на відправку в контролер
#endif
        if(data_scale.contains(tag) && v.size()==2) // якщо датий  тег присутній в масиві шкальованих значень тоді відшкалювати його і записувалося одиночне значення
        {
            double tv;
            switch(tags[tag][2]) // тип даних
                {
                    default:
                    case 2: // Real
                        tv=*(float*)v.data();
                        break;
                    case 3: // Timer
                    case 4: // Long
                        tv=*(int*)v.data();
                        break;
                }
                data_scale[tag][0]=(tv/4000.0*(data_scale[tag][2]-data_scale[tag][1])+data_scale[tag][1]);

        }

    }
}


