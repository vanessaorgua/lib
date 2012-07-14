#include "logging.h"
#include "../iodev.h"
#include <QtSql>


Logging::Logging(QVector<IoDev*> src,int collectInterval): s(src)
{
    QSqlDatabase dbs=QSqlDatabase::addDatabase("QMYSQL","logging");

    QSettings set;
    // заточка під drizzle, який створює сокети в /tmp
    if(QFile::exists("/tmp/mysql.socket")) // якщо такий файл існує
    {
        dbs.setConnectOptions("UNIX_SOCKET=/tmp/mysql.socket");
    }

    dbs.setHostName(set.value("/db/host","localhost").toString());
    dbs.setDatabaseName(set.value("/db/db","test").toString());
    dbs.setUserName(set.value("/db/username","scada").toString());
    dbs.setPassword(set.value("/db/passwd","").toString());
    dbs.open(); // спробувати відкрити

    QTimer *tmr=new QTimer(this);
    connect(tmr,SIGNAL(timeout()),this,SLOT(dbStore()));
    tmr->setInterval(collectInterval);
    tmr->start();

    startTimer(60000); // збиради дані раз в п’ять секунд
    foreach(IoDev *v,s)
    {
        tags_list << v->getTags();
    }
    tables << "trend";

}

Logging::~Logging()
{
    {
        QSqlDatabase dbs=QSqlDatabase::database("logging",true); // знайти моє з’єднання
        dbs.close();
    }
    QSqlDatabase::removeDatabase("logging");
}


// збирає дані
void Logging::dbStore()
{
    QStringList field,value;
    int i,j=0;
    QDateTime tm=QDateTime::currentDateTime();
    //QVector<qint16> data;

    // фрагмет із RxModbus.cpp для орієнтування по типах даних
    //QSet<QString> ft;
    //ft << "Integer" << "Bool" << "Real" << "Timer" << "Long";
    QHash<QString,QVector<qint16> > v;

    foreach( v ,tags_list) // перебрати всі джерела даних
    {
        i=0;
        //data=s[j]->getDataRaw();
        field.clear();
        field << "Dt";
        value.clear();
        value << QString("%1").arg(tm.toTime_t());

        foreach(QString tag, v.keys()) // перебрати всі теги
        {
            if(v[tag][4]) // якщо цей тег пишеться в історію
            {
                field << tag;
                switch(v[tag][2]) // тип даних
                {
                    default:
                    case 0: // Integer
                        value << QString("%1").arg(s[j]->getValue16(tag));
                        break;
                    case 1: // Bool
                        value << QString("%1").arg(s[j]->getValue16(tag)?1:0);
                        break;
                    case 2: // Real
                        value << QString("%1").arg(s[j]->getValueFloat(tag),5,'f',0);
                        break;
                    case 3: // Timer
                    case 4: // Long
                        value << QString("%1").arg(s[j]->getValue32(tag));
                        break;
                }
            }
        }
        log << QString("REPLACE INTO %1 (%2) VALUE ('%3')").arg(tables[j]).arg(field.join(",")).arg(value.join("\',\'"));
        //qDebug() <<  log.size() <<"-" <<  QString("INSERT INTO %1 (%2) VALUE ('%3')").arg(tables[j]).arg(field.join(",")).arg(value.join("\',\'"));
           ++j;
    }

}

// скидає зібрані дані в базу
void Logging::timerEvent (QTimerEvent *)
{
    int i=0;
    QSqlDatabase dbs=QSqlDatabase::database("logging",true); // знайти моє з’єднання

if(dbs.isOpen()) // якщо з’єднання відкрите тоді
{
    QSqlQuery query(dbs);
    //int si = log.size();
    //qDebug() << "Queue size " << si;

    if(!log.empty()) // якщо в черзі є дані для запису
    {
#ifndef WIN32
        if(dbs.transaction())
        {
#else
            query.exec("START TRANSACTION");
#endif
            while(!log.empty())
            {
                ++i;
                if(!query.exec(log.dequeue()))
                {
                    qDebug() << query.lastError() << "\n"
                          << query.lastQuery();
                }
            }
#ifndef WIN32
            dbs.commit();
            //qDebug("Скинуто в базу %d записів",i);
        }
        else
        {
            qDebug() << "No start transaction "
             << dbs.lastError().databaseText();
            log.clear();
        }
#else
        query.exec("COMMIT");
#endif

    }
    else
    {
        qDebug() << "No records to store" ;
    }
    dbs.close();
}
else
{
    QDateTime dt= QDateTime::currentDateTime();
    qDebug() << dt.toString("yyyy/MM/dd hh:mm:ss")  << " MYSQL error:"<< dbs.lastError().databaseText() << "\n";
    log.clear(); // очистити чергу
}

}
