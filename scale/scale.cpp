#include "scale.h"
#include "ui_scale.h"

Scale::Scale(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Scale)
{
    ui->setupUi(this);
}

Scale::~Scale()
{
    delete ui;
}

void Scale::changeEvent(QEvent *e)
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
