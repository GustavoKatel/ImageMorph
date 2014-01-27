#ifndef IMAGEHIGHLIGHT_H
#define IMAGEHIGHLIGHT_H

#include <QWidget>

#include <imagewrapper.h>

class ImageHighlight : public QWidget
{
    Q_OBJECT
public:
    explicit ImageHighlight(QWidget *parent, ImageWrapper *image);
    virtual ~ImageHighlight();

    void setHighlight(bool state);
    bool isHighlight();

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

signals:

public slots:
    void slot_image_update();

private:
    ImageWrapper *image;
    bool mouseDown, enabled;
    QPointF temp_begin, temp_end;

};

#endif // IMAGEHIGHLIGHT_H
