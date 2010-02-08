#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui/QWidget>

class RotatedLabel;
class QLineEdit;
class QSpinBox;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void textChange(const QString &text);
    void angleChange(int angle);

private:
    RotatedLabel *label;
    QLineEdit *lineEdit;
    QSpinBox *spinBox;
};

#endif // WIDGET_H
