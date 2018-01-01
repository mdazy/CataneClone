#include "board.h"


/**/


Hex::Hex( Type type ) : type_( type ), number_( -1 ) {
}


const QColor& Hex::color() const {
    static const QColor colors[ nbTypes ] = { Qt::blue, Qt::red, Qt::darkRed, Qt::yellow, Qt::green, Qt::gray, Qt::darkYellow };
    if( type_ < 0 ) {
    	return QColor();
    }
    return colors[ type_ ];
}


/**/


Board::Board( int h, int w ) {
	resize( h, vector<Hex>( w ) );
	for( int y = 0; y < h; y++ ) {
		for( int x = 0; x < w; x++ ) {
			at( y )[ x ].type_ = Hex::Type( rand() % ( Hex::nbTypes + 1 ) - 1 );
		}
	}
}


int Board::height() const {
	return size();
}


int Board::width() const {
	if( height() == 0 ) {
		return 0;
	}
	return at( 0 ).size();
}