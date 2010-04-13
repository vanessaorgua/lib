#include "RIoNetClient.h"

#include <QDebug>
#include <QMessageBox>

RIoNetClient::RIoNetClient(QString hostname,int nPort) : host(hostname),Port(nPort)
{
    int i;

    // ініціалізувати масиви
    for(i=0;i<VALUERAW_LEN;++i)
    {
	Scale_min<< 0;
	Scale_max<< 0;
	Value_raw << 0.0;
	Value_scale << 0.0;
    }

    // ініціалізувати пам’ять контролера
    for(i=0;i<VALUE_LEN;++i)
	Value << 0;
    for(i=0;i<CTRL_LEN;++i)
	Ctrl << 0;
    for(i=0;i<PARM_LEN;++i)
	Parm<<0;

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

    qry << qint8('R') << qint8('V') << qint16(0) << qint16(0);
    qry << qint8('R') << qint8('C') << qint16(0) << qint16(0);
    qry << qint8('R') << qint8('P') << qint16(0) << qint16(0);
    qry << qint8('R') << qint8('R') << qint16(0) << qint16(0);
    qry << qint8('R') << qint8('S') << qint16(0) << qint16(0);
    qry << qint8('R') << qint8('N') << qint16(0) << qint16(0);
    qry << qint8('R') << qint8('X') << qint16(0) << qint16(0);
    //qDebug() << "Size" << query.size();

}

void RIoNetClient::slotConnected()
{
    //qDebug() << "Conected..";
    pTcpSock->write(query); // відправити запит на сервер
    rtmr->start();
    connTimeout->start();
    connState.Len=-1; // підготуватися до прийому пакунку
}


void RIoNetClient::slotTimeout() // таймаут отримання даних від сервера
{
    // якщо від сервера не отримуються дані тоді треба спробувати встановити з’єднання знову
    //qDebug() << "Signal slotTimeout()";
    rtmr->stop();

    connTimeout->stop();
    connWait->start();
    pTcpSock->close(); // закрити з’єднання. Приведе до генерації сигналу disconnected() або я помиляюся
}


void RIoNetClient::slotDisconnect() // від’єднання зі сторони сервера
{
    //qDebug() << "Signal slotDisconnect()";
    // якщо сервер закрив з’єднання тоді спрбувати відкрити його знову
    rtmr->stop();
}


void RIoNetClient::slotError(QAbstractSocket::SocketError)
{
    //qDebug() << "Signal slotError()";

    // зупинити таймера
    rtmr->stop();
    connTimeout->stop();
    
    pTcpSock->close(); // закрити з’єднання

    //QMessageBox::critical(0,QString::fromUtf8("Помилка"),pTcpSock->errorString());

    connWait->start() ; // зробити затримку часу перед повторною спробою встановити з’єднання
}

void RIoNetClient::slotNewConnect()
{
    //qDebug() << "Signal slotNewConnect()";

    connWait->stop();
    pTcpSock->connectToHost(host,Port);
}

void RIoNetClient::slotSendQuery()
{
    //qDebug() << "Send Query";
    pTcpSock->write(query); // відправити запит на сервер
}


void RIoNetClient::slotSendData(qint8 Type,short Index,QVector<short>& data)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock,QIODevice::WriteOnly);
    int i;
    out.setVersion(QDataStream::Qt_4_2);
    
    out << qint8('W') << Type << Index << qint16(0) << data;
    out.device()->seek(4);
    out << qint16(arrBlock.size()-4);

    //qDebug() << "QVector" << Index << data;

    pTcpSock->write(arrBlock);

    // внести зміни в локальну копію даних
    switch(Type)
    {
	case 'C':
	    for(i=0;i<data.size();++i)
		Ctrl[Index+i]=data[i];
	    break;
	case 'P':
	    for(i=0;i<data.size();++i)
		Parm[Index+i]=data[i];
	    break;
    }
}


void RIoNetClient::slotSendData(qint8 Type,short Index,double& data)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock,QIODevice::WriteOnly);
    
    out.setVersion(QDataStream::Qt_4_2);
    
    out << qint8('W') << Type << Index << qint16(0) << data;
    out.device()->seek(4);
    out << qint16(arrBlock.size()-4);

    qDebug() << "double" << Index << data;

    pTcpSock->write(arrBlock);

}



RIoNetClient::~RIoNetClient()
{
    pTcpSock->close(); // від’єднатися від сервера
}


void RIoNetClient::slotReadServer()
{
    QDataStream in(pTcpSock);
    in.setVersion(QDataStream::Qt_4_2);
    int j=0;
    
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
		    case 'V':
			in >> Value;
			break;
		    case 'C':
			in >> Ctrl;
			break;
		    case 'P':
			in >> Parm;
			break;
		    case 'R':
			in >> Value_raw;
			//qDebug() << Value_raw;
			break;
		    case 'S':
			in >> Value_scale;
			break;
		    case 'N':
			in >> Scale_min;
			break;
		    case 'X':
			in >> Scale_max;
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

const QVector<short>& RIoNetClient::getValue()
{
    return Value;
}

const QVector<short>& RIoNetClient::getCtrl()
{
    return Ctrl;
}

const QVector<short>& RIoNetClient::getParm()
{
    return Parm;
}

const QVector<double>& RIoNetClient::getValueRaw()
{
    return Value_raw;
}

const QVector<double>& RIoNetClient::getValueScale()
{
    return Value_scale;
}

const QVector<double>& RIoNetClient::getScaleMin()
{
    return Scale_min;
}

const QVector<double>& RIoNetClient::getScaleMax()
{
    return Scale_max;
}


