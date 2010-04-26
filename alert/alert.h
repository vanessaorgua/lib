#ifndef ALERT_H
#define ALERT_H

#include <QObject>
#include <QHash>
#include <QStringList>

class IoDev; // звідси брати дані
class IoNetServer; // сюди відправляти
class QString;

class Alert: public QObject
{
    Q_OBJECT
public:
    Alert(IoDev *src);
    int loadList(QString fileName);

signals:
    void newAlert(QString v);

public slots:
    void checkAlert();

private:
   IoDev *s;

   QHash<QString,QStringList> diAlert;
};

#endif // ALERT_H
