#include "IoNetClient.h"


#include <QDebug>
#include <QMessageBox>

IoNetClient::IoNetClient(QString hostname,int nPort) : host(hostname),Port(nPort)
{

    // з’єднатися із сервером
    connState.Len=-1;
    pTcpSock = new QTcpSocket(this);
    pTcpSock->connectToHost(host,Port);
    connect(pTcpSock,SIGNAL(connected()),this,SLOT(slotConnected()));
    connect(pTcpSock,SIGNAL(readyRead()),this,SLOT(slotReadServer()));
    connect(pTcpSock,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotError(QAbstractSocket::SocketError)));
    connect(pTcpSock,SIGNAL(disconnected()),this,SLOT(slotDisconnect()));

    
    // таймер віправки запиту на сервер за  даними
    rtmr= new QTimer(this);
    rtmr->setInterval(1000);
    connect(rtmr,SIGNAL(timeout()),this,SLOT(slotSendQuery())); // під’єднатися до іншого
    
    connWait = new QTimer(this);
    connWait->setInterval(5000);
    connect(connWait,SIGNAL(timeout()),this,SLOT(slotNewConnect()));

    connTimeout=new QTimer(this);
    connTimeout->setInterval(10000); // інтервал очікування даних від сервера
    connect(connTimeout,SIGNAL(timeout()),this,SLOT(slotTimeout()));
    zero = new NetIoDev(this);

}

IoNetClient::~IoNetClient()
{
    pTcpSock->close(); // від’єднатися від сервера

    foreach(NetIoDev* v,src) // звільнити пам’ять з-під даних
    {
        delete v;
    }
    delete zero;
}

void IoNetClient::slotConnected()
{
    qDebug() << "Conected..";

    query.clear();
    QDataStream qry(&query,QIODevice::WriteOnly);
    qry.setVersion(QDataStream::Qt_4_2);

    // перший запит  - на кількість наявних джерел даних
    qry << qint8('R') << qint8('C') << qint8(0) << qint16(0) << qint16(0);

    pTcpSock->write(query); // відправити запит на сервер
    connTimeout->start();
    connState.Len=-1; // підготуватися до прийому пакунку
}


void IoNetClient::slotTimeout() // таймаут отримання даних від сервера
{
    // якщо від сервера не отримуються дані тоді треба спробувати встановити з’єднання знову
    //qDebug() << "Signal slotTimeout()";
    rtmr->stop();

    connTimeout->stop();
    connWait->start();
    pTcpSock->close(); // закрити з’єднання. Приведе до генерації сигналу disconnected() або я помиляюся
}


void IoNetClient::slotDisconnect() // від’єднання зі сторони сервера
{
    //qDebug() << "Signal slotDisconnect()";
    // якщо сервер закрив з’єднання тоді спрбувати відкрити його знову
    rtmr->stop();
}


void IoNetClient::slotError(QAbstractSocket::SocketError)
{
    //qDebug() << "Signal slotError()";

    // зупинити таймера
    rtmr->stop();
    connTimeout->stop();
    
    pTcpSock->close(); // закрити з’єднання

    //QMessageBox::critical(0,QString::fromUtf8("Помилка"),pTcpSock->errorString());

    connWait->start() ; // зробити затримку часу перед повторною спробою встановити з’єднання
}

void IoNetClient::slotNewConnect()
{
    //qDebug() << "Signal slotNewConnect()";

    connWait->stop();
    pTcpSock->connectToHost(host,Port);
}

void IoNetClient::slotSendQuery()
{

    query.clear();
    QDataStream qry(&query,QIODevice::WriteOnly);
    qry.setVersion(QDataStream::Qt_4_2);
    for(int i=0;i<src.size();++i)  // перебрати по кількості джерел даних
    {
          qry << qint8('R') << qint8('S') << qint8(i) << qint16(0) << qint16(0); // сформувати запит на отримання списку тегів
          qry << qint8('R') << qint8('D') << qint8(i) << qint16(0) << qint16(0); // сформувати запит на отримання списку тегів
    }
    pTcpSock->write(query); // відправити запит на сервер
}

void IoNetClient::slotReadServer()
{
    QDataStream in(pTcpSock);
    in.setVersion(QDataStream::Qt_4_6);
    int j=0;
    QDataStream qry(&query,QIODevice::WriteOnly);
    qry.setVersion(QDataStream::Qt_4_6);
    QVector<qint16> ts;
    QHash<QString,QVector<double> >  ss;

    //QHash<QString,QVector<qint16> > tg;

    //qDebug() << "ba: " << pTcpSock->bytesAvailable() << " time;" << QDateTime::currentDateTime();

    for(;;++j)
    {
	if(connState.Len==-1) // умова читання заголовку
	{
            if(pTcpSock->bytesAvailable()<7)
	    {
		break;
	    }
	     // прочитати заголовок
            in >> connState.Cmd >> connState.Type >>connState.iD >> connState.Index >> connState.Len ;
            //qDebug() << "Packet recived " << j << QChar(connState.Cmd) << QChar(connState.Type) << connState.Index << connState.Len;
	}

	if(pTcpSock->bytesAvailable()<connState.Len) // чи доступні інші байти із потоку ?
	{
	    break;
	}
	// підготувати заголовок відповіді
	
	switch(connState.Cmd)
	{
	    case 'R': // запит на передачу даних
		switch(connState.Type)
		{
                    case 'C': // визначення кількості джерел даних на сервері
                        qint16 c;
                        in >> c;  // формування запитів на отримання тегів
                        //qDebug()<< "Source count " << c;
                        query.clear();
                        //QDataStream qry(&query,QIODevice::WriteOnly);
                        //qry.setVersion(QDataStream::Qt_4_2);
                        for(int i=0;i<c;++i)
                        {
                            qry << qint8('R') << qint8('X') << qint8(i) << qint16(0) << qint16(0); // сформувати запит на отримання списку тегів
                            qry << qint8('R') << qint8('T') << qint8(i) << qint16(0) << qint16(0); // сформувати запит на отримання списку назв тегів
                            src << new NetIoDev(this); // створити класи-сховища
                            src[i]->iD=i;
                        }
                        pTcpSock->write(query);
                        query.clear();
                        break;
                    case 'T':
                        // отримання тегів
                        //qDebug()<< "Tags. connState.iD " << connState.iD << " packet len" << connState.Len;
                        if(connState.iD<src.size()) // перевірити чи є виділене місце у сховищі
                            in >> src[connState.iD]->tags; // зберегти отримані наді, перевірки не портібно бо нацей момент пам’ять вже повинна бути виділена
                                                       // хоча тут може бути і проблема
                         //qDebug() << "Tags read is good";
                        if(connState.iD==src.size()-1) // із останнім списком тегів запустити періодичне опитування даних
                           rtmr->start();

                        break;
                    case 'X':
                        in >> src[connState.iD]->text;
                        break;

                    case 'D': // отримати масив з даними
                        //qDebug() << "Data resived connState.iD " << connState.iD << " packet len" << connState.Len;
                        if(connState.iD<src.size()) // якщо є куди писати
                            in >> src[connState.iD]->data_raw; // зберегти отримані дані
                        else // інакше
                            in >>  ts; // просто спорожнити буфер


                        emit updateData();
                        emit updateDataRaw();
                        emit updateData(connState.iD);

                        //qDebug() << "emit updateDataRaw()";
                        break;

                    case 'S': // отримати масив з даними
                        //qDebug() << "Data resived Type S connState.iD " << connState.iD << " packet len" << connState.Len;
                        if(connState.iD<src.size()) // якщо є куди писати
                            in >> src[connState.iD]->data_scale; // зберегти отримані дані
                        else // інакше
                            in >>  ss; // просто спорожнити буфер
                        emit updateData();
                        emit updateDataScaled();
                        //qDebug() << "emit updateData()";
                        break;
                    case 'A':
                        {
                            QString as;
                            in >> as;
                            qDebug() << "Alert "<< as;
                            emit Alert(as);
                        }
                        break;

                    default:
                        qDebug() << "Unknown field Type" << connState.Type;
                        ::exit(1);
			break;
		}
		//sendBytes();
		break;
	    case 'W':
                break;
	    default:
                        qDebug() << "Unknown field Cmd" << connState.Cmd;
                        ::exit(1);
                break;
	}
	connState.Len=-1; // знову читати заголовок
        //qDebug() << "Pack ACK";
    }

    // перезапустити таймер. Чи цього буде достатньо ?
    connTimeout->stop();
    connTimeout->start();
    //qDebug()  << "exit " << pTcpSock->bytesAvailable() <<"\n";
}



