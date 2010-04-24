#ifndef SCALEDEV_H
#define SCALEDEV_H

#include <QtCore>
#include "../iodev.h"

class ScaledIoDev: public IoDev
{
public:
    ScaledIoDev() {}
   virtual ~ScaledIoDev()=0;

protected: // наступні функції потрібні тільки на сервері, можливо їх варто винести в окремий клас і класам-нащадкам, які спілкуються із реальними присторями варто робити подвійне наслідування
    virtual void loadScale(QString fileName); // це завантажить шкали, це не повинно бути видно із-зовні, це погане рішення передавати ім’я файла
    void updateScaledValue(); // це поновить шкальовані значення

//private : // Сховище даних. не наслідується, потібно тільки для компіляції, використовуються сховища нащадків
//    QHash<QString,QVector<qint16> > tags; // таблиця тегів
//    QVector<qint16> data_raw;            // сирі дані із PLC
//    QHash<QString,QVector<double> > data_scale;          // оброблені дані, хеш дублює tags. у векторі значення розташовані у наспупному порядку: value,zero,full

};

#endif // SCALEDEV_H

