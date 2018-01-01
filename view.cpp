#include "view.h"

#include <QtGui/QPainter>
#include <QtGui/QPolygonF>

#include <cmath>
#include <iostream>

using namespace std;

const float degToRad = 3.1415926 / 180.0;


View::View( QWidget* parent ) : QWidget( parent ), board_( 0 ) {
}


void View::paintEvent( QPaintEvent* event ) {
    QPainter p( this );
    if( board_ == 0 ) {
        QWidget::paintEvent( event );
        return;
    }

    int gridWidth = board_->width();
    int gridHeight = board_->height();

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
    		if( (*board_)[ y ][ x ].type_ == Hex::Invalid ) {
    			continue;
    		}

    		p.setBrush( (*board_)[ y ][ x ].color() );


    		float tx = x - gridWidth / 2;
    		float ty = y - gridHeight / 2;

    		float ox = tx * ( 1 + cos( 60 * degToRad ) );
    		float oy = ty + tx / 2.0;

		    p.drawPolygon( hex.translated( QPointF( width() / 2 + ox * radius, height() / 2 + oy * hexHeight ) ) );
    	}
    }

    QWidget::paintEvent( event );
}
