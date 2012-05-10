#ifndef _TREND_H_
#define _TREND_H_

//#include "ui_trend.h"

#include <QDateTime>
#include <QString>
#include <QPainter>
#include <QColor>
#include <QCheckBox>
#include <QRadioButton>

#include <QtSql>

class TrendView;
class HistoryThread;
class QMutex;

struct trendinfo
{
    QString host,user,passwd,db; // інформація для зв'язку із БД
    // тут буде додана підтримка SQLITE. Якщо значення параметра host=="QSQLITE" тоді ввачається що потрібно вживати рушій QSQLITE, варіант не самий гарний але кращого
    // поки-що не придумав

    int numPlot;
    QString trend; // ім'я тренада, так назву об'єкт і це ім'я буде використовуватись для збереження параметрів
    QString table; // назва таблиці
    QString fields[8]; //  імена полів
    double  fScale[8][2]; // шкали для перерахунку 
    
    QString trendHead;    // назва тренда
    QStringList fieldHead; // назви полів
    
} ;

namespace Ui
{
    class Trend;
}

// клас головного вікна
class TrendWindow: public QWidget //, public Ui::Trend
{
    Q_OBJECT
public:
    TrendWindow(QWidget *p=0,struct trendinfo *tri=NULL,int nHeight=4000);
    ~TrendWindow();

    void setColors(QVector<QColor> &colors);

private slots:
    void dataChange(); // реакція на навігаційні кнопки зміни дати
    void colorChange();
    void plotChange();     
    void setCursor(int v=-1);
    void slotExit();
    void setGrid(bool);


    // слоти для взаємодії із класом виймання історії
    void startHtr();
    void slotStart();
    void processRow(QStringList); // це отримує дані
    void changeState();     // це викликається в кінці обробки запиту;
    void showErrorText(QString);



signals:
    void repaintRequest();
    void finished();
    void execQuery(QString);

private:
    TrendView *m_tw;
    
    //QSqlDatabase dbs;

    QDateTime m_start,m_stop,m_cursor;
    QVector<QColor> m_Color;
    

    QString m_Field[8];   // назви полів 
    double m_fA[8],m_fB[8]; //коефіцієнти масштабування

    QString m_sTmpl,sQuery; // це буде шаблон запиту  до бази

    
    struct trendinfo *m_trinfo; 
    
    QVector<QCheckBox*> pv;
    QRadioButton *ps[8];
    
    int m_nHeight;

    QVector<unsigned int> m_pnDt; //масив, для пошуку поточного положення курсору
    int m_nLen; // довжина масива
    Ui::Trend *m_ui;

    HistoryThread *htr;
    int mState; // змінна стану для вибоку дії з обробки рядків

    QMutex *mutex;


};


//---------------------------------------------------------------------------------------------------------------------

// Клас малювання самого графіка
class TrendView: public QWidget
{
    Q_OBJECT
public:
    TrendView(QWidget *p=NULL);

    ~TrendView();

    inline QSize sizeHint() const {return QSize(400,300);}
    inline QSizePolicy sizePolicy() const {return QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);}

    void setColors(QVector<QColor> &pColor);
    bool grid() {return m_grid;}

public slots:
    void start(int nLen,int numPlot,int nHeight=4000); // тут створ.ється новий oб'єкт m_px
    void setData(double nX,QVector<int>& Data); // цей слот приймає дані для малювання.
    void draw(); // власне по цій команді об'єкт m_px відображається на екрані
    void setGrid(bool v) {m_grid=v;}

signals:     
    void _redraw();
    void cursorMoved(int pos);
    
protected:
    QPen m_pen; // карандаш для ма
    QPainter m_paint; // об'єкт за допомогю якого буду малювати
    
    QPixmap *m_px; // картинка, на якій малюються графіки
    int m_nPlot;
    bool m_bStart;
    bool m_grid;

    double m_pfData[8]; // масив для зберігання координат попердньої точки
    double m_nX,m_nH; // координата х попередньої точки
    
    QVector<QColor> m_pColor;


    void paintEvent(QPaintEvent *e); // малюю вищенаведену картину на екрані.
    void resizeEvent(QResizeEvent *e); // подія зміни розміру
    // обробники курсоку
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    
    int m_nCursorPos;
    
};

#endif //_TREND_H_

