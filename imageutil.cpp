#include "imageutil.h"

#include <QDebug>

#include <cmath>

ImageUtil::ImageUtil() : QObject(0)
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
                // end of interpolate

                PX = X - P;
                PQ = Q - P;

                // calculating u
                double pqNorm = std::sqrt( PQ.x()*PQ.x() + PQ.y()*PQ.y() );
                u = PX.x() * PQ.x() + PX.y() * PQ.y();
                u = u / (pqNorm * pqNorm) ;

                // calculating v
                // using perpendicular 1, where pPQ = ( PQy , -PQx )
                pPQ = QPointF( PQ.y() , -PQ.x() );
                v = PX.x() * pPQ.x() + PX.y() * pPQ.y();
                v = v / pqNorm;

                // new position
                PQ2 = Q2 - P2;
                double pq2Norm = std::sqrt( PQ2.x()*PQ2.x() + PQ2.y()*PQ2.y() );
                pPQ2 = QPointF( PQ2.y(), -PQ2.x() );
                X2 = QPointF();
                X2.setX( P2.x() + u*PQ2.x() + v* pPQ2.x()/pq2Norm );
                X2.setY( P2.y() + u*PQ2.y() + v* pPQ2.y()/pq2Norm );

                double sA, sB, sC;
                sA = P.y() - Q.y();
                sB = Q.x() - P.x();
                sC = P.x()*Q.y() - Q.x()*P.y();

                double dist = std::abs( sA * X.x() + sB * X.y() + sC ) / std::sqrt( std::pow(sA, 2) + std::pow(sB, 2) );
                double weight = std::pow( std::pow(pq2Norm,p) / (a + dist)  , b );

                double deltaX = X2.x() - X.x(),
                        deltaY = X2.y() - X.y();

                sumX += deltaX * weight;
                sumY += deltaY * weight;

                sumW += weight;

                emit warp_progress(current_progress++ , total_progress);

            }

            X2 = QPointF( X.x() + sumX/sumW, X.y() + sumY/sumW );

            X2.setX( std::floor(X2.x()+0.5) );
            X2.setY( std::floor(X2.y()+0.5) );

            // collect the color
            unsigned int r, g, b;
//            qDebug()<<X2;
            image1->getPixel( X2.x(), X2.y(), &r, &g, &b);

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

void ImageUtil::interpolacaoBilinear(ImageWrapper *image, int col, int row, unsigned int *r, unsigned int *g, unsigned int *b)
{
    if(!image) return;



}
