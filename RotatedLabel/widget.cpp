#include "widget.h"
#include "rotatedlabel.h"
#include <QtGui>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    label = new RotatedLabel(this);

    spinBox = new QSpinBox;
    spinBox->setRange(-360, 360);
    spinBox->setValue(0);

    lineEdit = new QLineEdit("<h1>Text<sup>123</sup>&Psi;</h1>");

    QLabel *angleLabel = new QLabel("angle");

    QHBoxLayout *angleLayout = new QHBoxLayout;
    angleLayout->addStretch();
    angleLayout->addWidget(angleLabel);
    angleLayout->addWidget(spinBox);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(label);
    mainLayout->addWidget(lineEdit);
    mainLayout->addLayout(angleLayout);

    setLayout(mainLayout);

    connect(lineEdit, SIGNAL(textChanged(QString)), this, SLOT(textChange(QString)));
    connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(angleChange(int)));
}

Widget::~Widget()
{
}

void Widget::textChange(const QString &text)
{
    label->setText(text);
}

void Widget::angleChange(int angle)
{
    label->setAngle(qreal(angle));
}
