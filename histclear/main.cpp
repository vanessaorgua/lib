#include <QtCore/QCoreApplication>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QStringList>
#include <QtSql>
#include <QDateTime>


int main(int argc, char *argv[])
{

    {
        QSettings set(QDir::home().filePath("histclear.conf"),QSettings::IniFormat);

        QSqlDatabase dbs=QSqlDatabase::addDatabase("QMYSQL","logging");

        dbs.setHostName(set.value("db/host","localhost").toString());
        dbs.setDatabaseName(set.value("db/db","test1").toString());
        dbs.setUserName(set.value("db/username","scada").toString());
        dbs.setPassword(set.value("db/passwd","").toString());

        //qDebug() << dbs.hostName() << dbs.databaseName() << dbs.userName() << dbs.password();

        if(dbs.open()) // спробувати відкрити
        {
            int tm=QDateTime::currentDateTime().toTime_t()-90*24*3600;
            QSqlQuery qry(dbs);

            set.beginGroup("table");
            QStringList k=set.allKeys();
            foreach(QString s,k)
            {
                if(!qry.exec(QString("DELETE FROM %1 WHERE Dt<%2").arg(set.value(s).toString()).arg(tm)))
                {
                    qDebug() << qry.lastError().databaseText();
                }
            }
            set.endGroup();
            dbs.close();
        }
        else
        {
            qDebug() << "Can\'t open database" << dbs.lastError().driverText();
        }

    }
        QSqlDatabase::removeDatabase("logging");
}
