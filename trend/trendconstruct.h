#ifndef TRENDCONSTRUCT_H
#define TRENDCONSTRUCT_H

#include <QDialog>

class IoNetClient;
namespace Ui {
    class TrendConstruct;
}

class TrendConstruct : public QDialog {
    Q_OBJECT
public:
    TrendConstruct(IoNetClient &source,QWidget *parent = 0);
    ~TrendConstruct();

    QStringList tegList();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TrendConstruct *ui;
     IoNetClient &src;

};

#endif // TRENDCONSTRUCT_H
