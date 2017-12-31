#include "view.h"

#include <QtGui/QPainter>
#include <QtGui/QPolygonF>

#include <cmath>

using namespace std;

const float degToRad = 3.1415926 / 180.0;


View::View( QWidget* parent ) : QWidget( parent ) {
}


void View::paintEvent( QPaintEvent* ) {
    QPainter p( this );
    p.setBrush( Qt::red );
    
    QPolygonF hex;
    int radius = min( width(), height() ) / 2;
    for( int i = 0; i < 6; i++ ) {
        hex << QPointF( radius * cos( i * 60 * degToRad ), radius * sin( i * 60 * degToRad ) );
    }
    
    p.drawPolygon( hex.translated( QPointF( width() / 2, height() / 2 ) ) );	
}
