#ifndef LOGGING_H
#define LOGGING_H

#include <QObject>
#include <QVector>
#include <QQueue>
#include <QHash>
#include <QStringList>

class IoDev;

class Logging : public QObject
{
Q_OBJECT
public:
    Logging(QVector<IoDev*> src);
    ~Logging();

    void setTables(QStringList list) { tables=list;}

private slots:
    void dbStore();
private:
    QVector<IoDev*> s;

    QQueue<QString> log;
    QVector<QHash<QString,QVector<qint16> > > tags_list; // довбанута структура, буду надіятися що бібліотека Qt правильно все зрозуміє.
    QStringList tables;

protected:
    virtual void timerEvent (QTimerEvent *e); // таймер
};

#endif // LOGGING_H
