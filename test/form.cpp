#include "form.h"
#include "ui_form.h"
#include "RxModbus.h"

Form::Form(RxModbus *src,QWidget *parent) :
    QWidget(parent),
    s(src)
{
    setupUi(this);
    connect(bn_Send,SIGNAL(clicked()),this,SLOT(sendData()));
    connect(bn_Update,SIGNAL(clicked()),this,SLOT(slotUpdate()));
    slotUpdate();

}

Form::~Form()
{

}
void Form::sendData()
{
    qDebug() << "Form::sendData()" ;
    s->sendValue("Tper_zd",qint32(le_Tper_zd->text().toDouble()*1000.0));
    s->sendValue("Nc_zd",qint16(sb_Nc_zd->value()));
}


void Form::slotUpdate()
{
    sb_Nc_zd->setValue(s->getDataRaw()[s->getIndex("Nc_zd")]);

    QString str="%1";
    le_Tper_zd->setText(str.arg((double)(*(qint32*)(s->getDataRaw().data()+s->getIndex("Tper_zd")))/1000.0));

}
