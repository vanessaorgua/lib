#ifndef TRENDCONSTRUCT_H
#define TRENDCONSTRUCT_H

#include <QDialog>
#include <QVector>

class IoNetClient;
class QTreeWidgetItem;
class QListWidgetItem;

namespace Ui {
    class TrendConstruct;
}

class TrendConstruct : public QDialog {
    Q_OBJECT
public:
    TrendConstruct(IoNetClient &source,QWidget *parent = 0);
    ~TrendConstruct();

    QStringList tegList();
private slots:
      void itemAdd(QTreeWidgetItem*,int);
      void itemRemove(QListWidgetItem*);
      void buttonAddItem();
      void buttonRemoveItem();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::TrendConstruct *ui;
     IoNetClient &src;


};

#endif // TRENDCONSTRUCT_H
