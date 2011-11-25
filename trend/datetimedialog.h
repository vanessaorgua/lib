#ifndef _DATETIMEDIALOG_H_
#define _DATETIMEDIALOG_H_


#include "ui_datetime.h"
#include <QDateTime>
class myDateTimeDialog: public QDialog, public Ui::DateTimeDialog
{

public:

    myDateTimeDialog(QWidget *p=NULL,QDateTime dtm=QDateTime::currentDateTime() ) :QDialog(p)
    {
	setupUi(this);
	hour->setValue(dtm.time().hour());
	minute->setValue(dtm.time().minute());
	second->setValue(dtm.time().second());
	date->setSelectedDate(dtm.date());
	
    }
    ~myDateTimeDialog() {}
};



#endif


