#include "trendconstruct.h"
#include "ui_trendconstruct.h"
#include <QWidget>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QHash>

#include "../ionetclient/IoNetClient.h"

TrendConstruct::TrendConstruct(IoNetClient &source,QWidget *parent) :
    QDialog(parent),
    src(source),
    ui(new Ui::TrendConstruct)
{
    ui->setupUi(this);


    ui->treeTag->clear();

    //qDebug() << "Size of NetIoDev" << src.size();
    for(int i=0;i<src.size();++i)
    {
        //qDebug() << src[i]->getTags().keys();
        QHash<QString,QTreeWidgetItem*> groups;
        QTreeWidgetItem *_qtwi = new QTreeWidgetItem(ui->treeTag);
        _qtwi->setText(0,QString(tr("ПЛС %1")).arg(i));
        ui->treeTag->addTopLevelItem(_qtwi);

        foreach(QString tag,src[i]->getTags().keys())
        {
            if(src[i]->isLogging(tag)) // якщо тег записується в історію тоді завантажити його в список
            {
                QTreeWidgetItem *p; // батьківська група
                QString g=tag.split("_")[0]; // визначити префікс
                if(groups.contains(g))
                {
                    p=groups[g];
                }
                else
                {
                    p= new QTreeWidgetItem(_qtwi);
                    _qtwi->addChild(p);
                    p->setText(0,g);
                    groups[g]=p;
                }

                QTreeWidgetItem *_twc = new QTreeWidgetItem(p);
                _twc->setText(0,tag);
                _twc->setText(1,src[i]->getDescription(tag));
            }
        }
    }
    ui->treeTag->setCurrentItem(ui->treeTag->itemAt(0,0));

    connect(ui->treeTag,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(itemAdd(QTreeWidgetItem*,int)));
    connect(ui->listTag,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(itemRemove(QListWidgetItem*)));
    connect(ui->addButton,SIGNAL(clicked()),this,SLOT(buttonAddItem()));
    connect(ui->removeButton,SIGNAL(clicked()),this,SLOT(buttonRemoveItem()));

}

TrendConstruct::~TrendConstruct()
{
    while(ui->listTag->count())
    {
        QListWidgetItem *p=ui->listTag->item(0);
        ui->listTag->removeItemWidget(p);
        delete p;
    }
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
    QStringList res;
    for(int i=0;i<ui->listTag->count();++i)
    {
        res <<  ui->listTag->item(i)->data(Qt::ToolTipRole).toString();
    }
    return res;
}

void TrendConstruct::itemAdd(QTreeWidgetItem* item,int col)
{
    if(! item->isDisabled() && item->text(1).size())
    {
        QListWidgetItem *li=new QListWidgetItem;
        li->setText(item->text(1));
        li->setData(Qt::ToolTipRole,item->text(0));
        ui->listTag->addItem(li);
        item->setDisabled(true);
    }
}

void TrendConstruct::itemRemove(QListWidgetItem* item)
{
    if(item)
    {
        foreach(QTreeWidgetItem *p,ui->treeTag->findItems(item->data(Qt::ToolTipRole).toString(),Qt::MatchRecursive,0))
        {
            p->setDisabled(false);
        }

        ui->listTag->removeItemWidget(item);
        delete item;
    }
}

void TrendConstruct::buttonAddItem()
{
    if(ui->treeTag)
    itemAdd(ui->treeTag->currentItem(),0);
}


void TrendConstruct::buttonRemoveItem()
{
    itemRemove(ui->listTag->currentItem());
}

