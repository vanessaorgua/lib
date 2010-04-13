#ifndef FORM_H
#define FORM_H

#include <QtGui/QWidget>

#include "ui_form.h"

class IoDev;

class Form : public QWidget , public Ui::Form
{
    Q_OBJECT
public:
    Form(IoDev *src, QWidget *parent = 0);
    ~Form();
private slots:
    void sendData();
    void slotUpdate();

private:
    IoDev *s;

};

#endif // FORM_H
