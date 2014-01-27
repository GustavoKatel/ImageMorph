#ifndef IMAGEWRAPPER_H
#define IMAGEWRAPPER_H

#include <QObject>
#include <QImage>
#include <QPointF>
#include <QVector>
#include <QPair>

/**
  * Define the list of segments. QMap<begin, end>
  */
#define SEGMENT_LIST QVector< QPair<QPointF, QPointF> >

class ImageWrapper : public QObject
{
    Q_OBJECT
public:
    ImageWrapper(QSize &size, QColor &fillColor);
    ImageWrapper(int width, int height, QColor &fillColor);
    ImageWrapper(QString &fileName);
    ImageWrapper(const char* fileName);
    ImageWrapper(ImageWrapper &obj);

    virtual ~ImageWrapper();

    void setPixel(int col, int row, unsigned int r, unsigned int g, unsigned b, unsigned int a = 255);
    void getPixel(int col, int row, unsigned int *r, unsigned int *g, unsigned int *b, unsigned int *a=0);

    void fill(QColor &color);

    void resize(int width, int height, bool keepAspectRation=false);

    QSize getSize();

    void load(QString &fileName);
    void load(const char* fileName);
    bool save(QString &fileName);

    void copy(QImage *image);
    void copy(ImageWrapper *image);

    QImage *getQImage() const;

    void addSegment(QPointF &begin , QPointF &end);
    SEGMENT_LIST &getSegments();

signals:
    void update();
    void newSegment(QPair<QPointF, QPointF> &segment);

public slots:

private:
    QImage *image;

    SEGMENT_LIST orientedSegments;

    void loadStructure(QString fileName);
    void fixedSegments();

};

#endif // IMAGEWRAPPER_H
