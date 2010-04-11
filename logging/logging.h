#ifndef LOGGING_H
#define LOGGING_H

#include <QObject>
#include <QVector>
#include <QQueue>
#include <QHash>

class RxModbus;

class Logging : public QObject
{
Q_OBJECT
public:
    Logging(QVector<RxModbus*> src);
    ~Logging();

private slots:
    void dbStore();
private:
    QVector<RxModbus*> s;

    QQueue<QString> log;
    QVector<QHash<QString,QVector<qint16> > > tags_list; // довбанута структура, буду надіятися що бібліотека Qt правильно все зрозуміє.

protected:
    virtual void timerEvent (QTimerEvent *e); // таймер
};

#endif // LOGGING_H
