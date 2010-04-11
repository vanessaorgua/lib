#include "logging.h"
#include "../modbus/RxModbus.h"
#include <QtSql>


Logging::Logging(QVector<RxModbus*> src): s(src)
{
    QSqlDatabase dbs=QSqlDatabase::addDatabase("QMYSQL","logging");

    QSettings set;

    dbs.setHostName(set.value("/db/host","localhost").toString());
    dbs.setDatabaseName(set.value("/db/db","vipgr").toString());
    dbs.setUserName(set.value("/db/username","scada").toString());
    dbs.setPassword(set.value("/db/passwd","").toString());
    dbs.open(); // спробувати відкрити

    QTimer *tmr=new QTimer(this);
    connect(tmr,SIGNAL(timeout()),this,SLOT(dbStore()));
    tmr->setInterval(60000);
    tmr->start();

    startTimer(5000); // збиради дані раз в п’ять секунд
    foreach(RxModbus *v,s)
    {
        tags_list << v->getTags();
    }

}

Logging::~Logging()
{
    {
        QSqlDatabase dbs=QSqlDatabase::database("logging",true); // знайти моє з’єднання
        dbs.close();
    }
    QSqlDatabase::removeDatabase("logging");
}

void Logging::dbStore()
{
    QString field,value;
    int i,j=0;
    QDateTime tm=QDateTime::currentDateTime();
    QVector<qint16> data;

    // фрагмет із RxModbus.cpp для орієнтування по типах даних
    //QSet<QString> ft;
    //ft << "Integer" << "Bool" << "Real" << "Timer" << "Long";
    QHash<QString,QVector<qint16> > v;

    foreach( v ,tags_list) // перебрати всі джерела даних
    {
        i=0;
        data=s[j]->getDataRaw();
        field="Dt";
        value=QString("\"%1\"").arg(tm.toTime_t());

        foreach(QString tag, v.keys()) // перебрати всі теги
        {
            if(v[tag][4]) // якщо цей тег пишеться в історію
            {
                field+=(","+tag);
                switch(v[tag][2]) // тип даних
                {
                    default:
                    case 0: // Integer
                        value+=QString(",\"%1\"").arg(data[s[j]->getIndex(tag)]);
                        break;
                    case 1: // Bool
                        break;
                    case 2: // Real
                        break;
                    case 3: // Timer
                        break;
                    case 4: // Long
                        break;
                }
            }
        }

        ++j;
    }

}


void Logging::timerEvent (QTimerEvent *)
{
    int i=0;
    QSqlDatabase dbs=QSqlDatabase::database("logging",true); // знайти моє з’єднання

if(dbs.isOpen()) // якщо з’єднання відкрите тоді
{
    QSqlQuery query(dbs);

    if(!log.empty()) // якщо в черзі є дані для запису
    {
        if(dbs.transaction())
        {
        //query.exec("START TRANSACTION");
            while(!log.empty())
            {
                ++i;
                query.exec(log.dequeue());
            }
            //query.exec("COMMIT");
            dbs.commit();
            //qDebug("Скинуто в базу %d записів",i);
        }


    }
}
else
{
    QDateTime dt= QDateTime::currentDateTime();
    qDebug() << dt.toString("yyyy/MM/dd hh:mm:ss")  << " MYSQL error:"<< dbs.lastError().databaseText() << "\n";
    log.clear(); // очистити чергу
}

}
