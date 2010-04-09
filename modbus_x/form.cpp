#include "form.h"
#include "ui_form.h"
#include "RxModbus.h"

Form::Form(RxModbus *src,QWidget *parent) :
    QWidget(parent),
    s(src)
{
    setupUi(this);
    connect(bn_Send,SIGNAL(clicked()),this,SLOT(sendData()));

}

Form::~Form()
{

}
void Form::sendData()
{
    QVector<qint16> v;
    v << le_Tper_zd->text().toInt();
    s->sendValue("Tper_zd",v);

}


