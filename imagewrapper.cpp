#include "imagewrapper.h"

#include <QRgb>
#include <QPixmap>
#include <QPainter>
#include <QFile>
#include <QBuffer>
#include <QDebug>

ImageWrapper::ImageWrapper(QSize &size, QColor &fillColor) :
    QObject(0)
{
    image = new QImage(size, QImage::Format_ARGB32);
    image->fill(fillColor);
}

ImageWrapper::ImageWrapper(int width, int height, QColor &fillColor) : QObject(0)
{
    image = new QImage(width, height, QImage::Format_ARGB32);
    image->fill(fillColor);
}

ImageWrapper::ImageWrapper(QString &fileName) : QObject(0)
{
//    image = new QImage(fileName);
    image = new QImage();
    this->load(fileName);
}

ImageWrapper::ImageWrapper(const char *fileName) : QObject(0)
{
//    image = new QImage(fileName);
    image = new QImage();
    this->load(fileName);
}

ImageWrapper::ImageWrapper(ImageWrapper &obj) : QObject(0)
{
    this->image = new QImage(*obj.getQImage());

    QPair<QPointF,QPointF> pair;
    for(int i=0;i<obj.getSegments().size();i++)
    {
        pair = obj.getSegments().at(i);
        this->orientedSegments.append(pair);
    }

}

ImageWrapper::~ImageWrapper()
{
    if(image)
        delete image;
}

void ImageWrapper::setPixel(int col, int row, unsigned int r, unsigned int g, unsigned b, unsigned int a)
{
    if(!image || col>=image->width() || row>=image->height() || col<0 || row<0 )
        return;
    image->setPixel(col, row, qRgba(r, g, b, a));

//    emit update();
}

void ImageWrapper::getPixel(int col, int row, unsigned int *r, unsigned int *g, unsigned int *b, unsigned int *a)
{
    QRgb rgb;
    if(row>=0 && col>=0 && row<image->height() && col<image->width())
        rgb = image->pixel(col, row);
    else
        rgb = qRgba(0,0,0,0);

    if(r)
        *r = qRed(rgb);
    if(g)
        *g = qGreen(rgb);
    if(b)
        *b = qBlue(rgb);
    if(a)
        *a = qAlpha(rgb);
}

void ImageWrapper::fill(QColor &color)
{
    image->fill(color);
    emit update();
}

void ImageWrapper::resize(int width, int height, bool keepAspectRation)
{
    QPixmap pix = QPixmap::fromImage(*image);
    QImage *nimg = 0;

    double scaleW = width / (double)image->width(),
            scaleH = height / (double)image->height();

    if(keepAspectRation)
        nimg = new QImage(pix.scaled(width, height, Qt::KeepAspectRatio).toImage());
    else
        nimg = new QImage(pix.scaled(width, height).toImage());

    delete image;
    image = nimg;

    for(int i=0;i<orientedSegments.size();i++)
    {
        QPair<QPointF, QPointF> pair = orientedSegments.at(i);
        pair.first.setX( pair.first.x()*scaleW );
        pair.first.setY( pair.first.y()*scaleH );

        pair.second.setX( pair.second.x()*scaleW );
        pair.second.setY( pair.second.y()*scaleH );

        orientedSegments.replace(i, pair);
    }

    emit update();
}

QSize ImageWrapper::getSize()
{
    QSize s(image->width(), image->height());
    return s;
}

void ImageWrapper::load(const char *fileName)
{
    orientedSegments.clear();

    if(QString(fileName).endsWith(".ims"))
    {
        loadStructure(fileName);
    } else {
        image->load(fileName);
        fixedSegments();
    }

    emit update();
}

void ImageWrapper::load(QString &fileName)
{
    orientedSegments.clear();

    if(fileName.endsWith(".ims"))
    {
        loadStructure(fileName);
    } else {
        image->load(fileName);
        fixedSegments();
    }

    emit update();
}

bool ImageWrapper::save(QString &fileName)
{
//    bool res = image->save(fileName);
    if(fileName.endsWith(".ims"))
    {
        QFile file(fileName);
        file.open(QFile::ReadWrite);

        QDataStream stream(&file);
        stream << orientedSegments;
        stream << *image;

        file.close();
    } else {
        image->save(fileName);
    }

    return true;
}

void ImageWrapper::copy(QImage *image)
{
    *this->image = *image;

    emit update();
}

void ImageWrapper::copy(ImageWrapper *image)
{
    *this->image = *image->getQImage();

    emit update();
}

QImage *ImageWrapper::getQImage() const
{
    return image;
}

void ImageWrapper::addSegment(QPointF &begin, QPointF &end)
{
    QPair<QPointF, QPointF> p;
    p.first = begin;
    p.second = end;
    orientedSegments.append(p);

    emit newSegment(p);
}

SEGMENT_LIST &ImageWrapper::getSegments()
{
    return orientedSegments;
}

void ImageWrapper::loadStructure(QString fileName)
{
    QFile file(fileName);
    file.open(QFile::ReadWrite);

    QDataStream stream(&file);

    stream >> orientedSegments;
    stream >> *image;

    file.close();

}

void ImageWrapper::fixedSegments()
{
    QPair<QPointF, QPointF> pair;

    pair.first.setX(0);
    pair.first.setY(0);
    pair.second.setX(0);
    pair.second.setY(image->height());

    orientedSegments.append(pair);

    pair.first.setX(0);
    pair.first.setY(image->height());
    pair.second.setX(image->width());
    pair.second.setY(image->height());

    orientedSegments.append(pair);

    pair.first.setX(image->width());
    pair.first.setY(image->height());
    pair.second.setX(image->width());
    pair.second.setY(0);

    orientedSegments.append(pair);

    pair.first.setX(image->width());
    pair.first.setY(0);
    pair.second.setX(0);
    pair.second.setY(0);

    orientedSegments.append(pair);
}
