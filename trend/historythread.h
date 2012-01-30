#ifndef HISTORYTHREAD_H
#define HISTORYTHREAD_H


#include <QThread>
#include <QString>
#include <QStringList>
#include <QSqlRecord>
#include <trend.h>


/* В цей клам буде винесена робота із базою даних. Можливо варто зробити його досить універсальним та використати в інших частинах програми.
  наприклад буде цікаво використати його також в класі RPanelReg
*/

// це обгортка для запуску бд в паралельному потоці
class HistoryThread : public QThread
{
    Q_OBJECT
public:
    HistoryThread(TrendWindow *parentObject,QString host="localhost",QString base="test",QString user="",QString passwd="");
    ~HistoryThread();

    void run();

signals:
    void dbError(QString);

private:
    QString dbHost,dbBase,dbUser,dbPasswd;

    TrendWindow *p;

};


class QSqlRunner : public QObject
{
    Q_OBJECT
public:
    QSqlRunner()
    {

    }

public slots:
    void runQuery(QString);

signals:
    void dbError(QString);
    void pullRows(QStringList);
    void endQuery();

};


#endif // HISTORYTHREAD_H
