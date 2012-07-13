#include "historythread.h"


#include <QtSql>
#include <QString>
#include <QDebug>


HistoryThread::HistoryThread(TrendWindow *parentObject,QString host,QString base,QString user,QString passwd):
    // QThread(parentObject),
    dbHost(host),
    dbBase(base),
    dbUser(user),
    dbPasswd(passwd),
    p(parentObject)
{

}


HistoryThread::~HistoryThread()
{


    qDebug() << "HistoryThread destroed" ;

}


void HistoryThread::run()
{
    // Створюємо з’єднання із БД
    if(dbHost=="QSQLITE")
    {
        QSqlDatabase dbs=QSqlDatabase::addDatabase("QSQLITE","historyThread");
        dbs.setDatabaseName(dbBase);
    }
    else
    {
        QSqlDatabase dbs=QSqlDatabase::addDatabase("QMYSQL","historyThread");
        // заточка під drizzle, який створює сокети в /tmp
        if(QFile::exists("/tmp/mysql.socket")) // якщо такий файл існує
        {
            dbs.setConnectOptions("UNIX_SOCKET=/tmp/mysql.socket");
        }

        dbs.setHostName(dbHost);
        dbs.setDatabaseName(dbBase);
        dbs.setUserName(dbUser);
        dbs.setPassword(dbPasswd);
    }

    {
        QSqlRunner s;
        s.setObjectName("sqlRunner");
        QSqlDatabase dbs=QSqlDatabase::database("historyThread");



        connect(this,SIGNAL(dbError(QString)),p,SLOT(showErrorText(QString)),Qt::QueuedConnection);


        if(dbs.isOpen())             // важко сказати чи так правильно....
        {
            // з’єднати класи
            connect(&s,SIGNAL(pullRows(QStringList)),p,SLOT(processRow(QStringList)),Qt::QueuedConnection); // ,Qt::QueuedConnection
            connect(&s,SIGNAL(endQuery()),p,SLOT(changeState()),Qt::QueuedConnection);
            connect(&s,SIGNAL(dbError(QString)),p,SLOT(showErrorText(QString)),Qt::QueuedConnection); // під’єднати повідомлення про помилку.

            connect(p,SIGNAL(execQuery(QString)),&s,SLOT(runQuery(QString)),Qt::QueuedConnection);


            qDebug() << "Start HistoryThread";

            exec(); // але це має бути в саому кінці....

            qDebug() << "Stop HistoryThread";

        }
        else
        {
            emit dbError(QString(tr("Open database\n%1\n%2"))
                     .arg(dbs.lastError().driverText())
                     .arg(dbs.lastError().databaseText())); // випустити сигнал про помилку
            qDebug() << this->objectName() <<
                        QString(tr("Open database\n%1\n%2"))
                                             .arg(dbs.lastError().driverText())
                                             .arg(dbs.lastError().databaseText());
        }
        // запустити цикл обробки подій, пісдя отримання сигналу quit()
    }

    // після виконання цього роз’єднатися із бд
    {
	QSqlDatabase dbs=QSqlDatabase::database("historyThread");
        if(dbs.isOpen())
        {
            dbs.close();
            // qDebug() << "databases closed" ;
        }
    }

    {
        QSqlDatabase::removeDatabase("historyThread");
    }
}


void QSqlRunner::runQuery(QString v)
{
     QSqlDatabase dbs=QSqlDatabase::database("historyThread");

     if(dbs.isOpenError()) // якщо сталася помилка при відкритті бази даних тоді
     {
        emit dbError(QString(tr("Exec query\n%1\n%2"))
                     .arg(dbs.lastError().driverText())
                     .arg(dbs.lastError().databaseText())); // випустити сигнал про помилку
        // хоча тут можлива ситуація коли тре
     }
     else // інакше можна виконувати запит
     {
        QSqlQuery query(dbs);
        qDebug() << "Query" << v;

        if(query.exec(v))
        {
            QStringList rows;
            while(query.next())
            {
                QSqlRecord r=query.record();
                int len=r.count();
                rows.clear();
                for(int i=0;i<len;++i)
                    rows << r.value(i).toString();

                emit pullRows(rows);
            }
            query.clear();
        }
        else
        {
            emit dbError(query.lastError().databaseText());
        }
        qDebug() << "emit endQuery()";
        emit endQuery();

     }



}



