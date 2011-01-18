#ifndef TRENDCONSTRUCT_H
#define TRENDCONSTRUCT_H

#include <QDialog>
#include <iodev.h>

namespace Ui {
    class TrendConstruct;
}

class TrendConstruct : public QDialog {
    Q_OBJECT
public:
    TrendConstruct(IoDev &source,QWidget *parent = 0);
    ~TrendConstruct();

    QStringList tegList();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TrendConstruct *ui;
    IoDev &src;

};

#endif // TRENDCONSTRUCT_H
