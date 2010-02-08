#include "rotatedlabel.h"
#include <QPainter>
#include <cmath>

RotatedLabel::RotatedLabel(QString text,QWidget *parent)
        : QWidget(parent),_label(text)
{
    _angle = 0;
}

void RotatedLabel::setAngle(qreal angle)
{
    if (_angle != angle) {
        _angle = angle;
        updatePixmap();
    }
}

void RotatedLabel::setText(const QString &text)
{
    if (text != _label.text()) {
        _label.setText(text);
        updatePixmap();
    }
}

void RotatedLabel::updatePixmap()
{
    _label.setFont(this->font());
    _label.adjustSize();
    _pixmap = QPixmap(_label.size());
    _label.render(&_pixmap);
    QTransform t;
    t.rotate(_angle);
    _pixmap = _pixmap.transformed(t, Qt::SmoothTransformation);
    setMinimumSize(_pixmap.width(), _pixmap.height());
    update();
}

QSize RotatedLabel::sizeHint() const
{
    return minimumSize();
}

QSize RotatedLabel::minimumSizeHint() const
{
    return minimumSize();
}

void RotatedLabel::resizeEvent(QResizeEvent */*event*/)
{
    updatePixmap();
}

void RotatedLabel::paintEvent(QPaintEvent */*event*/)
{ 
    QPainter painter(this);
    int x = 0;
    int y = (height() - _pixmap.height())/2;
    painter.drawPixmap(x, y, _pixmap.width(), _pixmap.height(), _pixmap);
}
