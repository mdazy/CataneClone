#include "view.h"

#include <QtGui/QPainter>
#include <QtGui/QPolygonF>

#include <cmath>
#include <iostream>

using namespace std;

const float degToRad = 3.1415926 / 180.0;


View::View( QWidget* parent ) : QWidget( parent ), board_( 0 ) {
    setMinimumSize( 500, 500 );
}


void View::paintEvent( QPaintEvent* event ) {
    QPainter p( this );
    if( board_ == 0 ) {
        QWidget::paintEvent( event );
        return;
    }

    int gridWidth = board_->width();
    int gridHeight = board_->height();

    // TODO: use painter transformation instead
    // determine best radius so that circle within grid is fully visible
    float hexHeight = height() / gridHeight;
    float radiusFromHeight = hexHeight / 2 / sin( 60 * degToRad );
    float hexWidth = width() / gridWidth / sin( 60 * degToRad );
    float radiusFromWidth = hexWidth / 2;
    float radius = min( radiusFromWidth, radiusFromHeight );

    hexHeight = radius * 2 * sin( 60 * degToRad );


    QPolygonF hex;
    for( int i = 0; i < 6; i++ ) {
        hex << QPointF( radius * cos( i * 60 * degToRad ), radius * sin( i * 60 * degToRad ) );
    }

    for( int x = 0; x < gridWidth; x++ ) {
    	for( int y = 0; y < gridHeight; y++ ) {
            const auto& h = (*board_)[ y ][ x ];

    		if( h.type_ == Hex::Invalid ) {
    			continue;
    		}

    		p.setBrush( h.color() );

            // shift coordinates so that 0,0 is the center of the map
    		float tx = x - gridWidth / 2;
    		float ty = y - gridHeight / 2;
            // center of the hex
            float ox = tx * 1.5; // 1 + cos(60°)
            float oy = ty + tx / 2.0;
            QPointF center( width() / 2 + ox * radius, height() / 2 + oy * hexHeight );

		    p.drawPolygon( hex.translated( center ) );

            // draw number on land tiles
            if( h.number_ < 0 ) {
                continue;
            }
            // white disc centered inside hex
            auto prevPen = p.pen();
            p.setPen( Qt::NoPen );
            p.setBrush( Qt::white );
            p.drawEllipse( center, radius / 2.5, radius / 2.5 );
            p.setPen( prevPen );
            // number centered in disc
            float half = radius * cos( 45 * degToRad );
            QRectF textBox( center.x() - half, center.y() - half, half * 2, half * 2 );
            p.drawText( textBox, Qt::AlignHCenter | Qt::AlignVCenter, QString::number( h.number_ ) );
    	}
    }

    QWidget::paintEvent( event );
}
