#include "IoNetServer.h"
#include "../iodev.h"

#include <QDebug>


IoNetServer::IoNetServer(QVector<IoDev*> s,int nPort)
    :src(s)
{
    m_pTcpServer = new QTcpServer(this);
    if(!m_pTcpServer->listen(QHostAddress::Any,nPort))
    {
	qDebug() << "Unable to start the server";
	m_pTcpServer->close();
    }
    connect(m_pTcpServer,SIGNAL(newConnection()),this,SLOT(slotNewConnection()));
    //QFile f("~/vipgr.log");

}

IoNetServer::~IoNetServer()
{

}

void IoNetServer::slotNewConnection()
{
    QTcpSocket *pClientSocket = m_pTcpServer->nextPendingConnection();
    connect(pClientSocket,SIGNAL(disconnected()),this,SLOT(slotDisconnect()));  //це треба переробити на видалення значень із хеш-таблиці
    connect(pClientSocket,SIGNAL(readyRead()),this,SLOT(slotReadClient()));
    connState[pClientSocket].Len=-1; // ставорити новий запис в хеш-таблиці, признак читання заголовку
    connState[pClientSocket].host=pClientSocket->peerAddress().toString(); // зберегти ім’я хочта для подальшого використання
    qDebug() << "Accept new connection";
    
    //QFile f(QDir::homePath()+"/vipgr.log");
    //if(f.open(QIODevice::Append))
    //{
        //QTextStream t(&f);
        QDateTime dt= QDateTime::currentDateTime();
        qDebug() << dt.toString("yyyy/MM/dd hh:mm:ss") << " CONNECT host:" << pClientSocket->peerAddress().toString() << "\n";
        //f.close();
    //}
    //else
	//qDebug() << "Cannot open file1";

    
}


void IoNetServer::slotDisconnect()
{
    QTcpSocket *p=(QTcpSocket*)sender();

    QFile f(QDir::homePath()+"/vipgr.log");
    if(f.open(QIODevice::Append))
    {
	QTextStream t(&f);
	QDateTime dt= QDateTime::currentDateTime();
	t << dt.toString("yyyy/MM/dd hh:mm:ss")  << " DISCONNECT host:" << connState[p].host <<"\n";
	f.close();
    }
    //else
	//qDebug() << "Cannot open file2";



    QHash<QTcpSocket*,struct _Header>::iterator i = connState.find(p);
    connState.erase(i);

    p->deleteLater();
}

void IoNetServer::slotReadClient()
{
    QTcpSocket *pCs=(QTcpSocket*)sender();
//    int i;

    qint16 v;
    QVector<qint16> ts;
    QString tag;
    double ds;
    
    QByteArray arrBlock;

    QDataStream in(pCs);
    QDataStream out(&arrBlock,QIODevice::WriteOnly);

    in.setVersion(QDataStream::Qt_4_6);
    out.setVersion(QDataStream::Qt_4_6);
    
    for(;;)
    {
	if(connState[pCs].Len==-1) // умова читання заголовку
	{
	    //qDebug() << pCs->bytesAvailable();
            if(pCs->bytesAvailable() <7) // якщо отримано всі байти заголовку
	    {
		break;
	    }
	     // прочитати заголовок
            in >> connState[pCs].Cmd >> connState[pCs].Type >> connState[pCs].iD >> connState[pCs].Index >> connState[pCs].Len ;
	}

	if(pCs->bytesAvailable()<connState[pCs].Len) // чи доступні інші байти із потоку ?
	{
	    break;
	}
        //qDebug() << "Packet recived " << QChar(connState[pCs].Cmd) << QChar(connState[pCs].Type) << connState[pCs].iD << connState[pCs].Index << connState[pCs].Len;
	// підготувати заголовок відповіді
	v=0;
        out << connState[pCs].Cmd << connState[pCs].Type <<connState[pCs].iD << connState[pCs].Index << v;
	
	switch(connState[pCs].Cmd)
	{
	    case 'R': // запит на передачу даних
		switch(connState[pCs].Type)
		{
                    case 'C':
                        out << qint16(src.size());// відправити наявну кількість джерел даних
                        break;
                    case 'T':
                        out << src[connState[pCs].iD]->getTags();
			break;
                    case 'D':
                        out << src[connState[pCs].iD]->getDataRaw();
			break;
                     case 'S':
                        out << src[connState[pCs].iD]->getDataScaled();
                        break;
                     case 'X':
                        out << src[connState[pCs].iD]->getText();
                        break;
		    default:
			break;
		}
                    out.device()->seek(5); // переміститись до поля із довжиною
                    out << qint16(arrBlock.size()-7); // записати довжину блоку даних
                    //qDebug() << "Resived bytes " << qint16(arrBlock.size()-7);
		//sendBytes();
		break;
	    case 'W':
		// прочитати вхідні дані
		switch(connState[pCs].Type)
		{
                    case 'D':
                        in >> tag >> ts;
                        src[connState[pCs].iD]->sendValue(tag,ts);
			break;
                    case 'S': // відправка шкальованого значення
                        in >> tag >> ds;
                        src[connState[pCs].iD]->sendValueScaled(tag,ds);
                        break;
                    case 'Z':
                        in >> tag >> ds;
                        src[connState[pCs].iD]->setScaleZero(tag,ds);
                        break;
                    case 'F':
                        in >> tag >> ds;
                        src[connState[pCs].iD]->setScaleFull(tag,ds);
                        break;
                    default:
			break;
		}
		break;
	    default:
		break;
	}
	
	//qDebug() << "Send " <<arrBlock.size();
	pCs->write(arrBlock);
	
	connState[pCs].Len=-1; // знову читати заголовок
	// тут треба очистити буфер передавача.
	out.device()->seek(0); // переміститись до поля із довжиною
	arrBlock.clear();
	
    }
    //qDebug() << "Finished " << pCs->bytesAvailable();
    

}


void IoNetServer::sendAlert(QString v)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);

    out << qint8('R') << qint8('A') << qint8(-1) << qint16(0) << qint16(0) << v;
    out.device()->seek(5); // переміститись до поля із довжиною
    out << qint16(arrBlock.size()-7); // записати довжину блоку даних

    foreach(QTcpSocket* p,connState.keys())
    {
        p->write(arrBlock);
    }


}
