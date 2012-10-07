#ifndef __PANEL_REG__
#define __PANEL_REG__

#include <QtGui>
#include <QDebug>

class TrendChart;
class IoDev;
class QTimer;
class QCloseEvent;
class TrendLoadThead;

//namespace Ui {
//    class panelReg;
//}
// це для того, щоб його можна було наслідувати...
#include "../build/ui_panelReg.h"


namespace Ri
{
    enum Index {Deskritp = 0,
                PV_1  = 1,
                PV_2  = 2,
                PV_3  = 3,
                SPR_1 = 4,
                X     = 5,
                SP_1  = 6,
                SP_2  = 7,
                SP_3  = 8,
                AM    = 9,
                Rej   = 10,
                Rev   = 11,
                Kpr   = 12,
                TI    = 13,
                Td    = 14,
                Xmin  = 15,
                Xmax  = 16,
                K_1   = 17,
                K_2   = 18,
                K_3   = 19,
                K_4   = 20,
                Kkor  = 21,
                Mode  = 22,
                P0    = 23
                };
}


class RpanelReg: public QDialog
{
Q_OBJECT
public:
    RpanelReg(IoDev &source,int n=0,QWidget *p=NULL,QString cfName=":/text/reg.txt",QString tableName="trend");
    ~RpanelReg();

protected slots:
    void changeReg(int Index); // зміна регулятор
    void Control(bool); // відображення-приховувавння частини вікна з настройками регулятора
    void runTrend();
    void updateTrend(int);

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
    void setParamMode(bool v);
    
    void setGraph(); // слот на 5ти-секундний інтервал для виводу гріфіків
    //void startTimer();

protected:
    void closeEvent( QCloseEvent * event);

    Ui::panelReg *ui; // морда лиця
    int RegNum;

    QVector<QStringList> RegDes; // масив опису параметрів регуляторів
    IoDev &src;

    TrendChart *trChart;
    QVector<double> v; // дані для запису на графік
    // масив для пошуку віджетів
    QHash<QString, Ri::Index> ctrlSearch;     // хеш для пошуку тегів
    QString tblName; // назва таблиці
    QTimer *t1;

    double kk_1 ;  // коефіцієнт корекції для K_1
    TrendLoadThead *trLoader;
};


#endif

