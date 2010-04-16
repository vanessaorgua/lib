#ifndef __IODEV__H__
#define __IODEV__H__

#include <QObject>
#include <QHash>
#include <QVector>

class IoDev //: public QObject
{
//    Q_OBJECT
public:
    IoDev() {}   // кноструктор, треба уточнити
    virtual ~IoDev() =0 ;// поки-що тривіальний деструктор

    // методи доступу до даних
    inline QHash<QString,QVector<qint16> > &getTags()   {   return tags;    }
    inline qint16 getIndex(QString tag) { return tags.contains(tag) ? tags[tag][0]:qint16(-1);  }
    inline qint16 getAddress(QString tag)  { return tags.contains(tag) ?tags[tag][1]:qint16(-1);  }

    inline const QVector<qint16> &getDataRaw() {return data_raw;}
    inline qint16 getValue16(QString tag) {return data_raw[tags[tag][0]];}
    inline qint32 getValue32(QString tag) {return *(qint32*)(data_raw.data()+tags[tag][0]); }
    inline float getValueFloat(QString tag) { return *(float*)(data_raw.data()+tags[tag][0]); }

//public slots:
    virtual void sendValue(QString tag,qint16 v) =0 ;
    virtual void sendValue(QString tag,qint32 v) =0 ;
    virtual void sendValue(QString tag,double v) = 0 ;
    virtual void sendValue(QString tag,QVector<qint16> &v) =0 ;
protected:
    // Сховище даних. можливо треба буде переробляти
    QHash<QString,QVector<qint16> > tags; // таблиця тегів
    QVector<qint16> data_raw;            // сирі дані із PLC

};


#endif 
