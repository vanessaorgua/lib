#include "form.h"
#include "ui_form.h"
#include "../iodev.h"
#include <QDebug>

Form::Form(IoDev *src,QWidget *parent) :
    QWidget(parent),
    s(src)
{
    setupUi(this);
    connect(bn_Send,SIGNAL(clicked()),this,SLOT(sendData()));
    connect(bn_Update,SIGNAL(clicked()),this,SLOT(slotUpdate()));
    //qDebug() << s->getTags().keys();
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
    s->sendValue("Q_zd",le_Q_zd->text().toDouble());
}


void Form::slotUpdate()
{
    sb_Nc_zd->setValue(s->getValue16("Nc_zd"));

    QString str="%1";
    le_Tper_zd->setText(str.arg((double)s->getValue32("Tper_zd")/1000.0));
    le_Q_zd->setText(str.arg(s->getValueFloat("Q_zd")));

}
