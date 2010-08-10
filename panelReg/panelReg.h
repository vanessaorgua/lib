#ifndef __PANEL_REG__
#define __PANEL_REG__

#include <QtGui>
#include <QDebug>

class TrendChart;
class IoDev;

namespace Ui {
    class panelReg;
}

namespace Ri
{
    enum Index {PV_1  = 0,
                PV_2  = 1,
                PV_3  = 2,
                SPR_1 = 3,
                X     = 4,
                SP_1  = 5,
                SP_2  = 6,
                SP_3  = 7,
                AM    = 8,
                Rej   = 9,
                Rev   = 10,
                Kpr   = 11,
                TI    = 12,
                Td    = 13,
                Xmin  = 14,
                Xmax  = 15,
                K_1   = 16,
                K_2   = 17,
                K_3   = 18,
                K_4   = 19,
                Kkor  = 20,
                Deskritp = 21};
}


class RpanelReg: public QDialog
{
Q_OBJECT
public:
    RpanelReg(IoDev &source,int n=0,QWidget *p=NULL,QString cfName=":/text/reg.txt",QString tableName="trend");
    ~RpanelReg();

private slots:
    void changeReg(int Index); // зміна регулятор
    void Control(); // відображення-приховувавння частини вікна з настройками регулятора
    void runTrend();
    
    void updateData(); // поновлення даних у віджетах
    // і тут буде ще купа сигналів від різних контролів
    // Ctrl
    void setCtrlValue(double v);
    void setCtrlValue(int v);
    // Parm
    void setParmValue(double v);
    void setParmValue(int v);
    void setParmAM(int v);
    void setParmRej(int v);
    void setParmRev();
    void setParmKprSig(int v);
    
    void setGraph(); // слот на 5ти-секундний інтервал для виводу гріфіків
    
private:
    Ui::panelReg *ui; // морда лиця
    int RegNum;

    QVector<QStringList> RegDes; // масив опису параметрів регуляторів
    IoDev &src;

    TrendChart *trChart;
    QVector<double> v; // дані для запису на графік
    // масив для пошуку віджетів
    QHash<QString, Ri::Index> ctrlSearch;     // хеш для пошуку тегів
    QString tblName; // назва таблиці

    double kk_1 ;  // коефіцієнт корекції для K_1
};


#endif

