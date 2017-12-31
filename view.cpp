#include "view.h"

#include <QtGui/QPainter>
#include <QtGui/QPolygonF>

#include <cmath>
#include <iostream>

using namespace std;

const float degToRad = 3.1415926 / 180.0;


View::View( QWidget* parent ) : QWidget( parent ) {
}


void View::paintEvent( QPaintEvent* ) {
    QPainter p( this );
    p.setBrush( Qt::red );

    // determine best radius so that 7-wide circle of hexes is fully visible
    float hexHeight = height() / 7;
    float radiusFromHeight = hexHeight / 2 / sin( 60 * degToRad );
    float hexWidth = width() / 5.5;
    float radiusFromWidth = hexWidth / 2;
    float radius = min( radiusFromWidth, radiusFromHeight );

    hexWidth = radius * 2;
    hexHeight = radius * 2 * sin( 60 * degToRad );


    QPolygonF hex;
    for( int i = 0; i < 6; i++ ) {
        hex << QPointF( radius * cos( i * 60 * degToRad ), radius * sin( i * 60 * degToRad ) );
    }
    
    for( int x = -3; x <= 3; x++ ) {
    	for( int y = -3; y <= 3; y++ ) {

    		float ox = x * ( 1 + cos( 60 * degToRad ) );
    		float oy = y + x / 2.0;

		    p.drawPolygon( hex.translated( QPointF( width() / 2 + ox * radius, height() / 2 + oy * hexHeight ) ) );
    	}
    }
    
}
