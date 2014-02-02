#include "imageutil.h"

#include <QDebug>

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

    QPointF PX, PQ, pPQ, P, Q, X, P2, Q2, X2, PQ2, pPQ2;
    double sumX = 0.0, sumY = 0.0, sumW = 0.0;
    for(row=0;row<h;row++)
    {
        for(col=0;col<w;col++)
        {
            X.setX(col);
            X.setY(row);

            sumX = sumY = sumW = 0.0;

            // iterate all segments
            for(seg=0;seg<totalSegments;seg++)
            {
                P = image2->getSegments().at(seg).first;
                Q = image2->getSegments().at(seg).second;

                P2 = image1->getSegments().at(seg).first;
                Q2 = image1->getSegments().at(seg).second;

                // interpolate
                double dist_i_x = ( P2.x() - P.x() ) * t;
                double dist_i_y = ( P2.y() - P.y() ) * t;
                P2.setX(P.x() + dist_i_x);
                P2.setY(P.y() + dist_i_y);

                dist_i_x = ( Q2.x() - Q.x() ) * t;
                dist_i_y = ( Q2.y() - Q.y() ) * t;
                Q2.setX(Q.x() + dist_i_x);
                Q2.setY(Q.y() + dist_i_y);

//                if(seg==4)
//                {
//                    qDebug()<<"P2: "<<P2;
//                    qDebug()<<"Q2: "<<Q2;
//                }
                // end of interpolate

                PX = X - P;
                PQ = Q - P;

                // calculating u
                double pqNorm = std::sqrt( PQ.x()*PQ.x() + PQ.y()*PQ.y() );
                u = PX.x() * PQ.x() + PX.y() * PQ.y();
                u = u / (pqNorm * pqNorm) ;

                // calculating v
                // using perpendicular 1, where pPQ = ( Qy - Py , Px - Qx )
                pPQ = QPointF( Q.y() - P.y() , P.x() - Q.x() );
                v = PX.x() * pPQ.x() + PX.y() * pPQ.y();
                v = v / pqNorm;

                // new position
                PQ2 = Q2 - P2;
                double pq2Norm = std::sqrt( PQ2.x()*PQ2.x() + PQ2.y()*PQ2.y() );
                pPQ2 = QPointF( Q2.y() - P2.y()  , P2.x() - Q2.x() );
                X2 = QPointF();
                X2.setX( P2.x() + u*PQ2.x() + v* pPQ2.x()/pq2Norm );
                X2.setY( P2.y() + u*PQ2.y() + v* pPQ2.y()/pq2Norm );

                // dist
                double sA, sB, sC;
                sA = P.y() - Q.y();
                sB = Q.x() - P.x();
                sC = P.x()*Q.y() - Q.x()*P.y();

                double dist = std::abs( sA * X.x() + sB * X.y() + sC ) / std::sqrt( std::pow(sA, 2) + std::pow(sB, 2) );
                double weight = std::pow( std::pow(pqNorm,p) / (a + dist)  , b );

                double deltaX = X2.x() - X.x(),
                        deltaY = X2.y() - X.y();

                sumX += deltaX * weight;
                sumY += deltaY * weight;

                sumW += weight;

                emit warp_progress(current_progress++ , total_progress);

            }

             X2 = QPointF( X.x() + sumX/sumW, X.y() + sumY/sumW );
//            X2 = QPointF( sumX/sumW, sumY/sumW );

            X2.setX( std::floor(X2.x()+0.5) );
            X2.setY( std::floor(X2.y()+0.5) );

            // collect the color
            unsigned int r, g, b;
//            image1->getPixel( X2.x(), X2.y(), &r, &g, &b);
            this->bilinearInterpolationInPoint(image1, X2.x(), X2.y(), &r, &g, &b, 1);
//            getColor(image1, X2.x(), X2.y(), &r, &g, &b);

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
//    if( col+dist>=w || col-dist<0 || row+dist>=h || row-dist<0 || dist==0 )
//    {
//        testAndSet(r, outside_color.red() );
//        testAndSet(g, outside_color.green() );
//        testAndSet(b, outside_color.blue() );
//        return;
//    }
    int bcol = col, brow = row;
    bool exit=0;
    if( col+dist>=w )
    {
        bcol = w-1;
        exit = true;
    }
    if( col-dist<0 )
    {
        bcol = 0;
        exit = true;
    }
    if( row+dist>=h )
    {
        brow = h-1;
        exit = true;
    }
    if( row-dist<0 )
    {
        brow = 0;
        exit = true;
    }
    if(exit)
    {
        unsigned int borderColor[3];
        image->getPixel(bcol, brow, borderColor, borderColor+1, borderColor+2);
        testAndSet(r, borderColor[0]);
        testAndSet(g, borderColor[1]);
        testAndSet(b, borderColor[2]);
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

