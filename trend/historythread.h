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

class HistoryThread : public QThread
{
    Q_OBJECT
public:
    HistoryThread(QString host="localhost",QString base="test",QString user="",QString passwd="");
    ~HistoryThread();


    void run();

public slots:
    void runQuery(QString);

signals:
    void dbError(QString);
    void pullRows(QStringList);
    void endQuery();

private:
    QString dbHost,dbBase,dbUser,dbPasswd;

    TrendWindow *p;

};


#endif // HISTORYTHREAD_H
