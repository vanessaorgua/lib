#include "mainw.h"
#include "ui_mainw.h"
#include "../ionetclient/IoNetClient.h"

mainW::mainW(IoNetClient& src,QWidget *parent) :
    QWidget(parent), s(src),
    m_ui(new Ui::mainW)
{
    m_ui->setupUi(this);
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
