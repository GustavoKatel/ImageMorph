#include "imagehighlight.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QSize>
#include <QDebug>

ImageHighlight::ImageHighlight(QWidget *parent, ImageWrapper * image) :
    QWidget(parent)
{
    mouseDown = false;
    this->image = image;
    connect(image, SIGNAL(update()), this, SLOT(slot_image_update()) );
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ImageHighlight::~ImageHighlight()
{

}

void ImageHighlight::setHighlight(bool state)
{
    enabled = state;
    repaint();
}

bool ImageHighlight::isHighlight()
{
    return enabled;
}

void ImageHighlight::paintEvent(QPaintEvent *event)
{
    QPainter p(this);

    // draw image
    this->resize( ((QWidget*) this->parent())->size()  );
    QImage img = image->getQImage()->scaled(this->size(), Qt::KeepAspectRatio);
    p.drawImage( img.rect() , img );

    // draw segments
    QPen penLines;
    penLines.setWidth(2);
    penLines.setColor(Qt::red);

    QPen penPoints;
    penPoints.setWidth(4);
    penPoints.setColor(Qt::black);

    QPen penNumbers;
    penNumbers.setWidth(4);
    penNumbers.setColor(Qt::blue);

    if(mouseDown)
    {
        penLines.setColor(Qt::blue);
        p.setPen(penLines);
        p.drawLine(temp_begin, temp_end);

        p.setPen(penPoints);
        p.drawPoint(temp_end);

    }

    // scale
    QImage imgScaled = image->getQImage()->scaled(this->size(), Qt::KeepAspectRatio);
    double scaleX = ( (double) imgScaled.width() ) / image->getSize().width();
    double scaleY = ( (double) imgScaled.height() ) / image->getSize().height();

    penLines.setColor(Qt::red);

    for(int i=0;i<image->getSegments().size();i++)
    {
        QPair<QPointF, QPointF> pair = image->getSegments().at(i);

        QPointF begin = pair.first, end = pair.second;

        begin   = QPointF( begin.x() * scaleX, begin.y() * scaleY );
        end     = QPointF( end.x() * scaleX, end.y() * scaleY );

        p.setPen(penLines);
        p.drawLine(begin, end);

        p.setPen(penPoints);
        p.drawPoint(end);

        p.setPen(penNumbers);
        p.setFont(QFont("Arial", 10, 2));
        p.drawText(end, QString::number(i+1));

    }


    QWidget::paintEvent(event);
}

void ImageHighlight::mousePressEvent(QMouseEvent *event)
{
    if(enabled)
    {
        mouseDown = true;
        temp_begin = event->pos();
        temp_end = temp_begin;
    }

    QWidget::mousePressEvent(event);
    this->repaint();
}

void ImageHighlight::mouseReleaseEvent(QMouseEvent *event)
{
    if(enabled)
    {
        mouseDown = false;
        QPointF end = event->pos();
        QPointF begin = temp_begin;

        QImage imgScaled = image->getQImage()->scaled(this->size(), Qt::KeepAspectRatio);

        double scaleX = image->getSize().width()  / ( (double) imgScaled.width() );
        double scaleY = image->getSize().height() / ( (double) imgScaled.height() );

        begin   = QPointF( begin.x() * scaleX, begin.y() * scaleY );
        end     = QPointF( end.x() * scaleX, end.y() * scaleY );

        image->addSegment(begin, end);
    }

    QWidget::mouseReleaseEvent(event);
    this->repaint();
}

void ImageHighlight::mouseMoveEvent(QMouseEvent *event)
{
    if(mouseDown)
    {
        temp_end = event->pos();
    }

    QWidget::mouseMoveEvent(event);
    this->repaint();
}

void ImageHighlight::slot_image_update()
{
    this->repaint();
}
