#include "RIoNetServer.h"
#include "RExchange.h"

#include <QDebug>


RIoNetServer::RIoNetServer(RExchange *source,int nPort)
    :src(source)
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

RIoNetServer::~RIoNetServer()
{

}

void RIoNetServer::slotNewConnection()
{
    QTcpSocket *pClientSocket = m_pTcpServer->nextPendingConnection();
    connect(pClientSocket,SIGNAL(disconnected()),this,SLOT(slotDisconnect()));  //це треба переробити на видалення значень із хеш-таблиці
    connect(pClientSocket,SIGNAL(readyRead()),this,SLOT(slotReadClient()));
    connState[pClientSocket].Len=-1; // ставорити новий запис в хеш-таблиці, признак читання заголовку
    connState[pClientSocket].host=pClientSocket->peerAddress().toString(); // зберегти ім’я хочта для подальшого використання
    //qDebug() << "Accept new connection";
    
    QFile f(QDir::homePath()+"/vipgr.log");
    if(f.open(QIODevice::Append))
    {
	QTextStream t(&f);
	QDateTime dt= QDateTime::currentDateTime();
        t << dt.toString("yyyy/MM/dd hh:mm:ss") << " CONNECT host:" << pClientSocket->peerAddress().toString() << "\n";
        f.close();
    }
    //else
	//qDebug() << "Cannot open file1";

    
}


void RIoNetServer::slotDisconnect()
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

void RIoNetServer::slotReadClient()
{
    QTcpSocket *pCs=(QTcpSocket*)sender();
//    int i;

    short v;
    QVector<short> ts;
    double std;
    
    QByteArray arrBlock;

    QDataStream in(pCs);
    QDataStream out(&arrBlock,QIODevice::WriteOnly);

    in.setVersion(QDataStream::Qt_4_2);
    out.setVersion(QDataStream::Qt_4_2);
    
    for(;;)
    {
	if(connState[pCs].Len==-1) // умова читання заголовку
	{
	    //qDebug() << pCs->bytesAvailable();
	    if(pCs->bytesAvailable()<6)
	    {
		break;
	    }
	     // прочитати заголовок
	    in >> connState[pCs].Cmd >> connState[pCs].Type >> connState[pCs].Index >> connState[pCs].Len ;
	}

	if(pCs->bytesAvailable()<connState[pCs].Len) // чи доступні інші байти із потоку ?
	{
	    break;
	}
	//qDebug() << "Packet recived " << connState[pCs].Cmd << connState[pCs].Type << connState[pCs].Index << connState[pCs].Len;
	// підготувати заголовок відповіді
	v=0;
	out << connState[pCs].Cmd << connState[pCs].Type << connState[pCs].Index << v; 
	
	switch(connState[pCs].Cmd)
	{
	    case 'R': // запит на передачу даних
		switch(connState[pCs].Type)
		{
		    case 'V':
			out << src->getValue();
			break;
		    case 'C':
			out << src->getCtrl();
			break;
		    case 'P':
			out << src->getParm();
			break;
		    case 'R':
			out << src->getValueRaw();
			break;
		    case 'S':
			out << src->getValueScale();
			break;
		    case 'N':
			out << src->getScaleMin();
			break;
		    case 'X':
			out << src->getScaleMax();
			break;
		    default:
			break;
		}
		    out.device()->seek(4); // переміститись до поля із довжиною
		    out << qint16(arrBlock.size()-6); // записати довжину блку даних
		//sendBytes();
		break;
	    case 'W':
		// прочитати вхідні дані
		switch(connState[pCs].Type)
		{
		    case 'P':
			ts.clear();
			in >> ts;
			src->setParm(connState[pCs].Index,ts);
		{
			QFile fl(QDir::homePath()+"/vipgr.log");
			if(fl.open(QIODevice::Append))
			{
			    QTextStream t(&fl);
			    QDateTime dt= QDateTime::currentDateTime();
			    t << dt.toString("yyyy/MM/dd hh:mm:ss")  << " CHANGE host:" << connState[pCs].host <<" index:" << connState[pCs].Index << " value:";
			    if(ts.size()>1)
				t << *(float*)ts.data();
			    else
				t << ts[0];
			    t << "\n";
			    fl.close();
			}
		}
			break;
		    case 'C':
			ts.clear();
			in >> ts;
			src->setCtrl(connState[pCs].Index,ts);
			break;
		    case 'N':
			in >> std;
			src->setScaleMin(connState[pCs].Index,std);
			break;
		    case 'X':
			in >> std;
			src->setScaleMax(connState[pCs].Index,std);
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


