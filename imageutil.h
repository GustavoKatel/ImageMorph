#ifndef IMAGEUTIL_H
#define IMAGEUTIL_H

#include <QObject>
#include <QColor>

#include "imagewrapper.h"

class ImageUtil : public QObject {

    Q_OBJECT

public:
    /**
     * @brief ImageUtil Default constructor
     */
    ImageUtil();

    /**
     * @brief ImageUtil Constructor
     * @param outside Image outside color
     */
    ImageUtil(QColor &outside);

    /**
     * @brief ImageUtil Default copy constructor
     * @param cpy ImageUtil to copy
     */
    ImageUtil(const ImageUtil &cpy);

    /**
     * @brief warp
     * @param src
     * @param dst
     * @param out
     * @param a
     * @param b
     * @param p
     * @param t t E [ 0.0 , 1.0 ]
     */
    void warp(ImageWrapper *src, ImageWrapper *dst, ImageWrapper *out, double a, double b, double p, double t);

    /**
     * @brief morph
     * @param image1
     * @param image2
     * @param imager
     * @param t t E [ 0.0 , 1.0 ]
     */
    void morph(ImageWrapper *image1, ImageWrapper *image2, ImageWrapper *imager, double t);

    void bilinearInterpolation(ImageWrapper *image, int col, int row, unsigned int *r, unsigned int *g, unsigned int *b, int dist);

    QColor getOutside_color() const;
    void setOutside_color(const QColor &value);

signals:
    void warp_progress(int progress, int total);
    void morph_progress(int progress, int total);

private:
    QColor outside_color;

};

#endif // IMAGEUTIL_H
