#include "RxModbus.h"

#include <QString>
#include <QTimer>


RxModbus::RxModbus(): QObject(),nPort(502) // кноструктор, треба уточнити
{

}
RxModbus::~RxModbus() // поки-що тривіальний деструктор
{

}

void RxModbus::slotConnected () // приєдналися
{

}

void RxModbus::slotNewConnect()
{

}

void RxModbus::slotTimeout() // таймаут отримання даних від сервера
{

}

void RxModbus::slotDisconnect() // відєднання зі сторони сервера
{

}

void RxModbus::slotError(QAbstractSocket::SocketError)
{

}

void RxModbus::slotReadServer()
{

}
