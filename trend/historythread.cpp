#include "historythread.h"


#include <QtSql>
#include <QString>
#include <QDebug>


HistoryThread::HistoryThread(QString host,QString base,QString user,QString passwd):
    dbHost(host),
    dbBase(base),
    dbUser(user),
    dbPasswd(passwd)
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
        QSqlDatabase dbs=QSqlDatabase::database("historyThread");

        dbs.open();

        // запустити цикл обробки подій, пісдя отримання сигналу quit()
        exec(); // але це має бути в саому кінці....
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


void HistoryThread::runQuery(QString v)
{
    qDebug() << v;
}



