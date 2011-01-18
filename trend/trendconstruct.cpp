#include "trendconstruct.h"
#include "ui_trendconstruct.h"
#include <QWidget>

TrendConstruct::TrendConstruct(IoDev &source,QWidget *parent) :
        src(source),
    QDialog(parent),
    ui(new Ui::TrendConstruct)
{
    ui->setupUi(this);
}

TrendConstruct::~TrendConstruct()
{
    delete ui;
}

void TrendConstruct::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


QStringList TrendConstruct::tegList()
{

}


