#ifndef ROTATEDLABEL_H
#define ROTATEDLABEL_H

#include <QWidget>
#include <QPixmap>
#include <QLabel>

class RotatedLabel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal Angle READ angle WRITE setAngle)
    Q_PROPERTY(QString Text READ text WRITE setText)
public:
    RotatedLabel( QWidget *parent = 0);
    
    void setAngle(qreal angle);
    void setText(const QString &text);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    qreal angle() {return _angle;}
    QString text() {return _label.text();}

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    void updatePixmap();
    qreal _angle;
    QLabel _label;
    QPixmap _pixmap;
    QColor _bg;
};

#endif // ROTATEDLABEL_H
