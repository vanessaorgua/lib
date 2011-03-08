#ifndef LOGGING_H
#define LOGGING_H

#include <QObject>
#include <QVector>
#include <QQueue>
#include <QHash>
#include <QStringList>

class IoDev;

class LogSQLite : public QObject
{
Q_OBJECT
public:
    LogSQLite(QVector<IoDev*> src,int collectInterval=5000);
    ~LogSQLite();

    void setTables(QStringList list) { tables=list;}

    void setFileName(QString fileName) { fName=fileName;}

private slots:
    void dbStore();
private:
    QVector<IoDev*> s;

    QQueue<QString> log;
    QVector<QHash<QString,QVector<qint16> > > tags_list; // довбанута структура, буду надіятися що бібліотека Qt правильно все зрозуміє.
    QStringList tables;

    QString fName;
protected:
    virtual void timerEvent (QTimerEvent *e); // таймер
};

#endif // LOGGING_H
