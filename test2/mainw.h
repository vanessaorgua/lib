#ifndef MAINW_H
#define MAINW_H

#include <QtGui/QWidget>

namespace Ui {
    class mainW;
}

class IoNetClient;

class mainW : public QWidget {
    Q_OBJECT
public:
    mainW(IoNetClient& src, QWidget *parent = 0);
    ~mainW();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::mainW *m_ui;
    IoNetClient &s;
};

#endif // MAINW_H
