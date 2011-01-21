#include "trendconstruct.h"
#include "ui_trendconstruct.h"
#include <QWidget>
#include <QTreeWidgetItem>

#include "../ionetclient/IoNetClient.h"

TrendConstruct::TrendConstruct(IoNetClient &source,QWidget *parent) :
        src(source),
    QDialog(parent),
    ui(new Ui::TrendConstruct)
{
    ui->setupUi(this);

    // Очистити все, на всяк випадок. Чи потрібно так складно
    foreach(QVector<QTreeWidgetItem*> t,tags)
    {
        t.clear();
    }
    tags.clear();

    ui->treeTag->clear();

    //qDebug() << "Size of NetIoDev" << src.size();
    for(int i=0;i<src.size();++i)
    {
        //qDebug() << src[i]->getTags().keys();

        QVector<QTreeWidgetItem*> t;
        tags.append(t);
        QTreeWidgetItem *_qtwi = new QTreeWidgetItem(ui->treeTag);
        _qtwi->setText(0,QString(tr("ПЛС %1")).arg(i));
        ui->treeTag->addTopLevelItem(_qtwi);
        foreach(QString tag,src[i]->getTags().keys())
        {
            if(src[i]->isLogging(tag))
            {
                QTreeWidgetItem *_twc = new QTreeWidgetItem(_qtwi);
                _twc->setText(0,tag);
                _twc->setText(1,src[i]->getDescription(tag));
                tags[i] << _twc;
            }
        }
    }
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


