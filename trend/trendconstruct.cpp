#include "trendconstruct.h"
#include "ui_trendconstruct.h"
#include <QWidget>
#include "../ionetclient/IoNetClient.h"

TrendConstruct::TrendConstruct(IoNetClient &source,QWidget *parent) :
        src(source),
    QDialog(parent),
    ui(new Ui::TrendConstruct)
{
    ui->setupUi(this);
    ui->tagView->children();
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


