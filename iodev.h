#ifndef __IODEV__H__
#define __IODEV__H__

#include <QObject>
#include <QHash>
#include <QVector>
#include <QDebug>

class IoDev : public QObject
{
//    Q_OBJECT
public:
    IoDev() {}   // кноструктор, треба уточнити
    virtual ~IoDev() =0 ;// поки-що тривіальний деструктор



    // методи доступу до даних
    inline QHash<QString,QVector<qint16> > &getTags()   {   return tags;    }
    inline qint16 getIndex(QString &tag) { return tags.contains(tag) ? tags[tag][0]:qint16(-1);  }
    inline qint16 getAddress(QString &tag)  { return tags.contains(tag) ?tags[tag][1]:qint16(-1);  }
    inline qint16 fieldType(QString &tag) { return tags.contains(tag) ?tags[tag][2]:qint16(-1); }
    inline bool isLogging(QString &tag) { return tags.contains(tag) ?(bool)tags[tag][4]:false; }
    inline const QVector<qint16> &getDataRaw() {return data_raw;}

    inline QHash<QString,QVector<double> > &getDataScaled() { return data_scale;}

    inline const QHash<QString,QString> &getText() { return text; }
    inline const QString getDescription(QString &tag) { return text.contains(tag)?text[tag]:QString("");}

    inline qint16 getValue16(QString tag) {
        if(tags.contains(tag) )
        {
            return tags[tag][0]<data_raw.size()?data_raw[tags[tag][0]]:0 ;
        } else
        {
            qDebug() << "Tag \"" <<tag << "\" not found";
            return 0;
        }
    }

    inline qint32 getValue32(QString tag)
    {
        return tags.contains(tag)?*(qint32*)(data_raw.data()+tags[tag][0]):0;
    }



    double getValueFloat(QString tag)
    {
        double v=0.0;
        if (tags.contains(tag) && tags[tag][0]<data_raw.size() )
        {
            switch(tags[tag][2]) // перевірити тип змінної
            {   case 2:
                    v= *(float*)(data_raw.data()+tags[tag][0]);
                    break;
                case 0:
                    v=data_raw[tags[tag][0]];
                    break;
                case 3:
                case 4:
                    v=*(qint32*)(data_raw.data()+tags[tag][0]);
                    break;
                default:
                    break;
            }
        }
        return v;
    }





//public slots:
    virtual void sendValue(QString tag,qint16 v) =0 ;
    virtual void sendValue(QString tag,qint32 v) =0 ;
    virtual void sendValue(QString tag,double v) = 0 ;
    virtual void sendValue(QString tag,QVector<qint16> &v) =0 ;

// це стосується шкал, у деяких потомках треба перевизначити деякі методи
    bool isScaleChange(QString tag) { return tags.contains(tag)?tags[tag][5]:false ; }

    double getValueScaled(QString tag) { return data_scale.contains(tag)?data_scale[tag][0]:0.0; }
    inline double scaleZero(QString tag) { return data_scale.contains(tag)?data_scale[tag][1]:0.0;    }
    inline double scaleFull(QString tag) { return data_scale.contains(tag)?data_scale[tag][2]:100.0;    }

    virtual void sendValueScaled(QString tag,double v) ;
    virtual void setScaleZero(QString tag,double v) ;
    virtual void setScaleFull(QString tag,double v) ;


protected:
    // Сховище даних. можливо треба буде переробляти
    QHash<QString,QVector<qint16> > tags; // таблиця тегів
    QVector<qint16> data_raw;            // сирі дані із PLC

    QHash<QString,QVector<double> > data_scale;          // оброблені дані, хеш дублює tags. у векторі значення розташовані у наспупному порядку: value,zero,full

    QHash<QString,QString> text; // це тоже погано.....

};


#endif 
