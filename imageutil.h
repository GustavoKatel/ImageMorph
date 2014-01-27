#ifndef IMAGEUTIL_H
#define IMAGEUTIL_H

#include <QObject>

#include "imagewrapper.h"

class ImageUtil : public QObject {

    Q_OBJECT

public:
    ImageUtil();

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

    void interpolacaoBilinear(ImageWrapper *image, int col, int row, unsigned int *r, unsigned int *g, unsigned int *b);

signals:
    void warp_progress(int progress, int total);
    void morph_progress(int progress, int total);

};

#endif // IMAGEUTIL_H
