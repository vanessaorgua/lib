#include "trendloadthead.h"
#include "../trendchar/trendchart.h"

#include <QtSql>

TrendLoadThead::TrendLoadThead(TrendChart *v):
        trChart(v)
{
}


void TrendLoadThead::run()
{

    // завантаження даних на графік із історії
    // QApplication::setOverrideCursor(Qt::WaitCursor);
        QString conName;
    {
        QSettings s;

        if(s.value("/db/hostname","localhost").toString()=="QSQLITE")
        {
            QSqlDatabase dbs=QSqlDatabase::addDatabase("QSQLITE","panelreg");
            dbs.setDatabaseName(s.value("/db/dbname","test").toString());
        }
        else
        {
            QSqlDatabase dbs=QSqlDatabase::addDatabase("QMYSQL","panelreg");
            // заточка під drizzle, який створює сокети в /tmp
            if(QFile::exists("/tmp/mysql.socket")) // якщо такий файл існує
            {
                dbs.setConnectOptions("UNIX_SOCKET=/tmp/mysql.socket");
            }

            dbs.setHostName(s.value("/db/hostname","localhost").toString());
            dbs.setDatabaseName(s.value("/db/dbname","test").toString());
            dbs.setUserName(s.value("/db/username","scada").toString());
            dbs.setPassword(s.value("/db/passwd","").toString());
        }

        QSqlDatabase dbs=QSqlDatabase::database("panelreg");

        conName=dbs.connectionName();
        //qDebug() << conName;

        if(  dbs.isOpen())
        {
            const int tm[3]={3600,1800,900},nLen[3]={1,2,4};
            int i,sLen=nLen[len];
            // очистити поточний графік
            trChart->fill(0);

            QDateTime dt = QDateTime::currentDateTime();
            QSqlQuery qry(dbs);
            if(qry.exec(sQuery))
            {
                QVector<double> v;
                while(qry.next())
                {
                    v.clear(); // cюди будуть завантажуватися дані
                    for(i=1;i<9;++i)
                    {
                        v << qry.value(i).toDouble();
                    }
                    for(int n=0;n<sLen;++n) // заватажити точнку декілька раз, в залежністі від вибраної шкали
                        trChart->loadPoint(v);
                }
                qry.clear();
            }
            else
            {
//            QApplication::setOverrideCursor(Qt::ArrowCursor);
//                QMessageBox::critical(this,tr("!!!Помилка виконання запиту"),qry.lastError().databaseText()+"\n!"+qry.lastQuery());
//                qDebug() << qry.lastQuery();
            }


        }
//        else
//            QMessageBox::critical(this,tr("Не вдалося з\'єднатися із базою даних історії"),dbs.lastError().databaseText());
    }
        QSqlDatabase::removeDatabase(conName);

        //QApplication::setOverrideCursor(Qt::ArrowCursor);

}
