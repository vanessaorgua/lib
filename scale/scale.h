#ifndef SCALE_H
#define SCALE_H

#include <QWidget>
#include <QVector>
class QLabel;

class Scale : public QWidget {
    Q_OBJECT
public:
    Scale(QWidget *parent = 0);
    ~Scale();

    QSize sizeHint() const {return QSize(25,300);}
    //QSizePolicy sizePolicy() const {return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Minimum);}

    inline double scaleMin() {return dMin ;}
    inline double scaleMax() {return dMax ;}
    void setScaleMinMax(double min,double max);

protected:
    void changeEvent(QEvent *e);
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

private:
    double dMin,dMax; // поточні значення шкали
    QVector<QLabel*> labels;
};

#endif // SCALE_H
