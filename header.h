#ifndef HEADER_H
#define HEADER_H

// Це структура, яка описує заголовок пакунку

struct _Header
{
    qint8   Cmd;        // 1 R|W читати/писати
    qint8   Type;       // 1 Обєкт T-теги | D-сирі діні
    qint8   iD ;        // 1 індекс IoDev
    qint16  Index;      // 2 Індекс елемента
    qint16  Len;        // 2 Довжина тіла даних
    QString host;       // загалом заголовок 7 байт
};

#endif // HEADER_H
