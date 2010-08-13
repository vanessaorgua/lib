#include <QtGui>
#include "scale.h"

#include <QDebug>


Scale::Scale(QWidget *parent) :
    QWidget(parent),dMin(0.0),dMax(100.0)
{
  
  // створити мітки
  for(int i=0;i<11;++i)
  {
	QLabel *t=new QLabel(this);
        t->setAlignment(Qt::AlignRight);
        t->setFont(QFont("Times", 7));
        labels << t;
  }
  //setSizePolicy(QSizePolicy::Fixed,QSizePolicy::MinimumExpanding);
}

Scale::~Scale()
{

}

void Scale::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        break;
    default:
        break;
    }
}


void Scale::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    QPen pen;
    pen.setColor(Qt::black);

    int w,h,n,i,l=3,pr=0;

    w=size().width();
    h=size().height();
    //qDebug() << "Object" << objectName() << "Scale width" << w ;
    //p.drawRect(0,0,w-1,h-1);

    switch(h/labels[0]->size().height())
    {
    case 1:
        n=0;
        break;
    case 2:
        n=2;
        break;
    case 3:
    case 4:
        n=3;
        break;
    case 5:
    case 6:
    case 7:
    case 8:
        n=5;
        break;
    case 9:
    case 10:
        n=7;
        break;
    default:
        n=11;
    }

    if(n>0)
    {
        double s=(dMax-dMin)/((double)n-1.0); // крок шкали
        double dS=((double)h-labels[0]->size().height())/((double)n-1.0);
        double offset=labels[0]->size().height() /2.0;

        if(dMax<20.0)
        {
            pr=1;
        }
        else
        {
            pr=0;
        }


        for(i=0; i<n;++i)
        {
            p.drawLine(w-5,dS*i+offset,w-1,dS*i+offset);

            labels[i]->show();
            labels[i]->setText(QString("%1").arg(dMax-s*i,l,'f',pr));
            labels[i]->setGeometry(0,dS*i,25,labels[i]->size().height());
        }
        for(;i<11;++i)
        {
            labels[i]->hide();
        }
    }

    p.end();
}


void Scale::resizeEvent(QResizeEvent *e)
{
  update();
}

void Scale::setScaleMinMax(double min,double max)
{
  dMin=min;
  dMax=max;
  update();
}

