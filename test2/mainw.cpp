#include "mainw.h"
#include "ui_mainw.h"
#include "../ionetclient/IoNetClient.h"

mainW::mainW(IoNetClient& src,QWidget *parent) :
    QWidget(parent), s(src),
    m_ui(new Ui::mainW)
{
    m_ui->setupUi(this);
    //connect(s,SIGNAL(updateData()),this,SLOT(slotUpdate()));

}

mainW::~mainW()
{
    delete m_ui;
}

void mainW::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void mainW::slotUpdate()
{
//    qDebug() << "updateData()";

    if(s[0]->getValue16("Amr_5_"))
    {
        m_ui->le_Amr->setText(tr("Auto"));
    }
    else
    {
        m_ui->le_Amr->setText(tr("Manual"));
    }

    QString str;

    switch(s[0]->getValue16("State_5_"))
    {
        case 0:
            str=tr("Off");
            break;
        case 1:
            str=tr("Start");
            break;
        case 2:
            str=tr("Filtring");
            break;
        case 3:
        case 4:
        case 5:
            str=tr("Uploading");
            break;
        default:
            str=tr("Unknown");
    }
    m_ui->le_State->setText(str);

    str="%1:%2";
    m_ui->le_Tf->setText(str.arg(s[0]->getValue16("Tf_h_5_"),2,10,QLatin1Char('0')).arg(s[0]->getValue16("Tf_m_5_"),2,10,QLatin1Char('0')));

    m_ui->cb_V_5_1->setChecked(s[0]->getValue16("Vl_5_1"));
    m_ui->cb_V_5_2->setChecked(s[0]->getValue16("Vl_5_2"));
    m_ui->cb_V_5_3->setChecked(s[0]->getValue16("Vl_5_3"));

    m_ui->cb_C_5_1->setChecked(s[0]->getValue16("Cl_5_1"));
    m_ui->cb_C_5_2->setChecked(s[0]->getValue16("Cl_5_2"));
    m_ui->cb_C_5_3->setChecked(s[0]->getValue16("Cl_5_3"));
    m_ui->le_Tper->setText(QString("%1").arg(s[0]->getValue16("Tper")));

}

