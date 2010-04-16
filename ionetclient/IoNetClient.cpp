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

    QDataStream qry(&query,QIODevice::WriteOnly);
    qry.setVersion(QDataStream::Qt_4_2);

    // перший запит  - на кількість наявних джерел даних
    qry << qint8('R') << qint8('C') << qint8(0) << qint16(0) << qint16(0);
    //qDebug() << "Size" << query.size();

}

IoNetClient::~IoNetClient()
{
    pTcpSock->close(); // від’єднатися від сервера

    foreach(NetIoDev* v,src) // звільнити пам’ять з-під даних
    {
        delete v;
    }
}

void IoNetClient::slotConnected()
{
    //qDebug() << "Conected..";
    pTcpSock->write(query); // відправити запит на сервер
    rtmr->start();
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
    //qDebug() << "Send Query";
    pTcpSock->write(query); // відправити запит на сервер
}

void IoNetClient::slotReadServer()
{
    QDataStream in(pTcpSock);
    in.setVersion(QDataStream::Qt_4_2);
    int j=0;
    QDataStream qry(&query,QIODevice::WriteOnly);
    qry.setVersion(QDataStream::Qt_4_2);
    QVector<qint16> ts;

    //qDebug() << "ba: " << pTcpSock->bytesAvailable() << " time;" << QDateTime::currentDateTime();

    for(;;++j)
    {
	if(connState.Len==-1) // умова читання заголовку
	{
	    if(pTcpSock->bytesAvailable()<6)
	    {
		break;
	    }
	     // прочитати заголовок
	    in >> connState.Cmd >> connState.Type >> connState.Index >> connState.Len ;
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
                        query.clear();
                        //QDataStream qry(&query,QIODevice::WriteOnly);
                        //qry.setVersion(QDataStream::Qt_4_2);
                        for(int i=0;i<c;++i)
                        {
                            qry << qint8('R') << qint8('T') << qint8(i) << qint16(0) << qint16(0);
                        }
                        pTcpSock->write(query);
                        query.clear();
                        break;
                    case 'T':
                        // отримання тегів
                        if(connState.iD+1>src.size()) // перевірити чи є виділене місце у сховищі
                        {
                            for(int i=src.size()-1;i<connState.iD+1;++i)
                            {
                                NetIoDev *p=new NetIoDev(this);
                                src << p;
                            }

                        }
                        break;
                    case 'D':
                        ts.clear();
                        in >> ts;
                        src[connState.iD]->setData(ts);
                        break;
                    default:
			break;
		}
		//sendBytes();
		break;
	    case 'W':
	    default:
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



