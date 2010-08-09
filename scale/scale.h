#ifndef SCALE_H
#define SCALE_H

#include <QWidget>

namespace Ui {
    class Scale;
}

class Scale : public QWidget {
    Q_OBJECT
public:
    Scale(QWidget *parent = 0);
    ~Scale();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Scale *ui;
};

#endif // SCALE_H
