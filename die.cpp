#include <Qtgui/QPainter>

#include <vector>

#include "die.h"

using namespace std;


Die::Die( QWidget* parent ) : QWidget( parent ), value_( 1 ) {
    setFixedSize( 50, 50 );
}


void Die::setValue( int value ) {
    value_ = ( value + 5 ) % 6; 
    update();
}


void Die::paintEvent( QPaintEvent* ) {
    QPainter p( this );

    p.setBrush( Qt::white );
    QPen pen( Qt::black );
    pen.setWidth( 2 );
    p.setPen( pen );
    p.drawRoundedRect( rect(), 10.0, 10.0 );

    vector<vector<int>> dots = { { 4 }, { 2, 6 }, { 2, 4, 6 }, { 0, 2, 6, 8 }, { 0, 2, 4, 6, 8 }, { 0, 2, 3, 5, 6, 8 } };
    p.setBrush( Qt::black );

    float cellWidth = width() / 3.0;
    for( auto d : dots[ value_ ] ) {
        int ny = d / 3;
        int nx = d - 3 * ny;
        float x = ( nx + 0.5 ) * cellWidth;
        float y = ( ny + 0.5 ) * cellWidth;
        p.drawEllipse( x, y, cellWidth * 0.2, cellWidth * 0.2 );
    }

}


int Die::heightForWidth( int w ) const {
    return w;
}