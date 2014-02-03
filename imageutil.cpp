#include "imageutil.h"

#include <QDebug>
#include <QPainter>

#include <cmath>

ImageUtil::ImageUtil() : QObject(0)
{

}

ImageUtil::ImageUtil(QColor &outside) : QObject(0), outside_color(outside)
{

}

ImageUtil::ImageUtil(const ImageUtil &cpy) : QObject(0), outside_color(cpy.getOutside_color())
{

}

void ImageUtil::warp(ImageWrapper *image1, ImageWrapper *image2, ImageWrapper *imager, double a, double b, double p, double t)
{
    int w = imager->getSize().width(),
            h = imager->getSize().height(),
            row, col, seg,
            total_progress = w*h, current_progress=0;

    double u, v;

    int totalSegments = image1->getSegments().size() < image2->getSegments().size() ? image1->getSegments().size() : image2->getSegments().size();
    if(!totalSegments) return;

    total_progress *= totalSegments;

    QPainter paint(imager->getQImage());
    QPen penLines;
    penLines.setWidth(2);
    penLines.setColor(Qt::red);

    QPen penPoints;
    penPoints.setWidth(4);
    penPoints.setColor(Qt::black);

    QPen penNumbers;
    penNumbers.setWidth(4);
    penNumbers.setColor(Qt::blue);

    QPointF Pi, Qi, Pil, Qil, PiQi, pPiQi, PilQil, pPilQil, PiX, X, Xl;
    double sumX = 0.0, sumY = 0.0, sumW = 0.0;
    for(row=0;row<h;row++)
    {
        for(col=0;col<w;col++)
        {
            X.setX(col);
            X.setY(row);

            sumX = 0.0;
            sumY = 0.0;
            sumW = 0.0;

            // iterate all segments
            for(seg=0;seg<totalSegments;seg++)
            {
                Pi = image2->getSegments().at(seg).first;
                Qi = image2->getSegments().at(seg).second;

                Pil = image1->getSegments().at(seg).first;
                Qil = image1->getSegments().at(seg).second;

                // ATENTION!
                // Pi and Qi are destination image
                // Pil and Qil are source image

                // interpolate
                double dist_i_x = ( Pil.x() - Pi.x() ) * t;
                double dist_i_y = ( Pil.y() - Pi.y() ) * t;
                Pil.setX(Pi.x() + dist_i_x);
                Pil.setY(Pi.y() + dist_i_y);

                dist_i_x = ( Qil.x() - Qi.x() ) * t;
                dist_i_y = ( Qil.y() - Qi.y() ) * t;
                Qil.setX(Qi.x() + dist_i_x);
                Qil.setY(Qi.y() + dist_i_y);

                // draw lines
//                paint.setPen(penLines);
//                paint.drawLine(Pil, Qil);

//                paint.setPen(penPoints);
//                paint.drawPoint(Qil);

//                paint.setPen(penNumbers);
//                paint.drawText(Qil, QString::number(seg));

                // end of interpolate

                PiX = X - Pi;
                PiQi = Qi - Pi;
                double piqiNorm = std::sqrt( PiQi.x()*PiQi.x() + PiQi.y()*PiQi.y() );

                PilQil = Qil - Pil;
                double pilqilNorm = std::sqrt( PilQil.x()*PilQil.x() + PilQil.y()*PilQil.y() );

                // using perpendicular 1, where pPQ = ( Qy - Py , Px - Qx )
                pPiQi = QPointF( Qi.y() - Pi.y() , Pi.x() - Qi.x() );
                pPilQil = QPointF( Qil.y() - Pil.y() , Pil.x() - Qil.x() );

                // calculating u
                u = PiX.x() * PiQi.x() + PiX.y() * PiQi.y();
                u = u / (piqiNorm * piqiNorm) ;

                // calculating v
                v = PiX.x() * pPiQi.x() + PiX.y() * pPiQi.y();
                v = v / piqiNorm;

                // new position
                Xl = QPointF();
                Xl.setX( Pil.x() + u*PilQil.x() + v* pPilQil.x()/pilqilNorm );
                Xl.setY( Pil.y() + u*PilQil.y() + v* pPilQil.y()/pilqilNorm );

                // dist
                double dist = 0.0;

                if(u >= 1)
                {
                    QPointF QiX = X - Qi;
                    dist = std::sqrt( QiX.x()*QiX.x() + QiX.y()*QiX.y() );
                } else if(u<=0) {
                    dist = std::sqrt( PiX.x()*PiX.x() + PiX.y()*PiX.y() );
                } else {
                    dist = std::abs(v);
                }
//                dist = distance(Pi.x(), Pi.y(), Qi.x(), Qi.y(), X.x(), X.y());

                double tmp = (a + dist);
                double weight = std::pow(piqiNorm,p);
                if(tmp>0) weight = weight / tmp;
                weight = std::pow( weight  , b );

                double deltaX = Xl.x() - X.x(),
                        deltaY = Xl.y() - X.y();

                sumX += deltaX * weight;
                sumY += deltaY * weight;

                sumW += weight;

                emit warp_progress(current_progress++ , total_progress);

            }

             Xl = QPointF( X.x() + sumX/sumW, X.y() + sumY/sumW );
//            X2 = QPointF( sumX/sumW, sumY/sumW );

//            Xl.setX( std::floor(Xl.x()+0.5) );
//            Xl.setY( std::floor(Xl.y()+0.5) );

            // collect the color
            unsigned int r, g, b;
//            image1->getPixel( X2.x(), X2.y(), &r, &g, &b);
//            this->bilinearInterpolationInPoint(image1, Xl.x(), Xl.y(), &r, &g, &b, 1);
            getColor(image1, Xl.x(), Xl.y(), &r, &g, &b);

            imager->setPixel(col, row, r, g, b);

        }
    }
    emit imager->update();
}

void ImageUtil::morph(ImageWrapper *image1, ImageWrapper *image2, ImageWrapper *imager, double t)
{
    int i, j, w, h;
    w = imager->getSize().width();
    h = imager->getSize().height();

    int totalProgress = w*h, currentProgress = 0;

    unsigned int r1, g1, b1,
            r2, g2, b2;

    for(i=0;i<h;i++)
    {
        for(j=0;j<w;j++)
        {
            image1->getPixel(j, i, &r1, &g1, &b1);
            image2->getPixel(j, i, &r2, &g2, &b2);

            r1 *= t;
            g1 *= t;
            b1 *= t;

            r2 *= (1-t);
            g2 *= (1-t);
            b2 *= (1-t);

            r2 += r1;
            g2 += g1;
            b2 += b1;

            imager->setPixel(j, i, r2, g2, b2);

            currentProgress++;
            emit morph_progress(currentProgress, totalProgress);

        }
    }
    emit imager->update();
}

void ImageUtil::bilinearInterpolation(ImageWrapper *image, ImageWrapper *res, int dist)
{
    if(!res || !image) return;

    res->copy(image);

    int w = image->getSize().width();
    int h = image->getSize().height();

    unsigned int q[3];

    for(int col=0;col<w;col++)
    {
        for(int row=0;row<h;row++)
        {
            bilinearInterpolationInPoint(image, col, row, q, q+1, q+2, dist);
            res->setPixel(col, row, q[0], q[1], q[2]);
        }
    }
}

void ImageUtil::bilinearInterpolationInPoint(ImageWrapper *image, int col, int row, unsigned int *r, unsigned int *g, unsigned int *b, int dist)
{
    if(!image) return;

    int w = image->getSize().width();
    int h = image->getSize().height();

    /*
     *      dist       dist
     *  | -------- | -------|
     *  Q12---------------Q22 -
     *  |-------------------| dist
     *  |---- (col,row) ----| -
     *  |-------------------| dist
     *  Q11---------------Q21 -
     *
     * Formula:
     * F(col,row) = ( 1 / pow( ( 2*dist ), 2 ) ) *
     * ( fQ11 * dist * dist +
     *   fQ21 * dist * dist +
     *   fQ12 * dist * dist +
     *   fQ22 * dist * dist )
     *
     */

    // check if the limits were exceeded
    if( col+dist>=w || col-dist<0 || row+dist>=h || row-dist<0 || dist==0 )
    {
        testAndSet(r, outside_color.red() );
        testAndSet(g, outside_color.green() );
        testAndSet(b, outside_color.blue() );
        return;
    }

    unsigned int q12[3],
            q22[3],
            q11[3],
            q21[3];


    // Q12( col-dist, row-dist )
    image->getPixel(col-dist, row-dist, q12, q12 + 1, q12 + 2);

    // Q22( col+dist, row-dist )
    image->getPixel(col+dist, row-dist, q22, q22 + 1, q22 + 2);

    // Q21( col+dist, row+dist )
    image->getPixel(col+dist, row+dist, q21, q21 + 1, q21 + 2);

    // Q11( col-dist, row+dist )
    image->getPixel(col-dist, row+dist, q11, q11 + 1, q11 + 2);

    unsigned int fQ11, fQ21, fQ12, fQ22, c[3];
    double cD;
    for(int i=0;i<3;i++)
    {
        fQ11 = q11[i];
        fQ21 = q21[i];
        fQ12 = q12[i];
        fQ22 = q22[i];

        cD = ( 1 / std::pow( (2*dist), 2.0 ) );
        cD = cD * std::pow(dist, 2.0) * ( fQ11 + fQ21 + fQ12 + fQ22 );

        c[i] = (unsigned int)cD;

    }

    testAndSet( r, c[0] );
    testAndSet( g, c[1] );
    testAndSet( b, c[2] );

}
QColor ImageUtil::getOutside_color() const
{
    return outside_color;
}

void ImageUtil::setOutside_color(const QColor &value)
{
    outside_color = value;
}

void ImageUtil::getColor(ImageWrapper *image, int col, int row, unsigned int *r, unsigned int *g, unsigned int *b)
{
    if(!image) return;

    int w = image->getSize().width();
    int h = image->getSize().height();

    // check if the limits were exceeded
    if( col>=w || col<0 || row>=h || row<0)
    {
        testAndSet(r, outside_color.red() );
        testAndSet(g, outside_color.green() );
        testAndSet(b, outside_color.blue() );
        return;
    }
    unsigned int q[3];
    image->getPixel(col, row, q, q + 1, q + 2);


    testAndSet( r, q[0] );
    testAndSet( g, q[1] );
    testAndSet( b, q[2] );

}

double ImageUtil::distance(double x1, double y1, double x2, double y2, double x3, double y3)
{
    double sA, sB, sC;
    sA = y1 - y2;
    sB = x2 - x1;
    sC = x1*y2 - x2*y1;

    return std::abs( sA * x3 + sB * y3 + sC ) / std::sqrt( std::pow(sA, 2) + std::pow(sB, 2) );

//    double px = x2-x1;
//    double py = y2-y1;

//    double dot = px*px + py*py;

//    double u =  ((x3 - x1) * px + (y3 - y1) * py) / float(dot);

//    if(u > 1)
//        u = 1;
//    else if( u < 0 )
//        u = 0;

//    double x = x1 + u * px;
//    double y = y1 + u * py;

//    double dx = x - x3;
//    double dy = y - y3;

//    double dist = std::sqrt(dx*dx + dy*dy);

//    return dist;

}
