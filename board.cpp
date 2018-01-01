#include "board.h"

#include <tuple>

using namespace std;


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


Board::Board() {
	// standard map is 7 by 7
	resize( 7, vector<Hex>( 7 ) );

	// standard repartition of hexes
	vector<Hex::Type> types = {
		Hex::Wheat, Hex::Wheat, Hex::Wheat, Hex::Wheat,
		Hex::Sheep, Hex::Sheep, Hex::Sheep, Hex::Sheep,
		Hex::Wood, Hex::Wood, Hex::Wood, Hex::Wood,
		Hex::Rock, Hex::Rock, Hex::Rock,
		Hex::Brick, Hex::Brick, Hex::Brick,
		Hex::Desert
	};

	// randomize
	int nbHexes = types.size();
	for( int i = 0; i < 100; i++ ) {
		int a = rand() % nbHexes;
		int b = ( a + rand() % ( nbHexes - 1 ) ) % nbHexes;
		auto t = types[ a ];
		types[ a ] = types[ b ];
		types[ b ] = t;
	}
	types.push_back( Hex::Water );

	// map setup
	vector<tuple<int, int, int>> setup = {
		make_tuple( 0, 3, 19 ),make_tuple( 0, 4, 19 ),make_tuple( 0, 5, 19 ),make_tuple( 0, 6, 19 ),
		make_tuple( 1, 2, 19 ), make_tuple( 1, 3, 0 ), make_tuple( 1, 4, 1 ), make_tuple( 1, 5, 2 ),make_tuple( 1, 6, 19 ),
		make_tuple( 2, 1, 19 ), make_tuple( 2, 2, 3 ), make_tuple( 2, 3, 4 ), make_tuple( 2, 4, 5 ), make_tuple( 2, 5, 6 ),make_tuple( 2, 6, 19 ),
		make_tuple( 3, 0, 19 ), make_tuple( 3, 1, 7 ), make_tuple( 3, 2, 8 ), make_tuple( 3, 3, 9 ), make_tuple( 3, 4, 10 ), make_tuple( 3, 5, 11 ),make_tuple( 3, 6, 19 ),
		make_tuple( 4, 0, 19 ), make_tuple( 4, 1, 12 ), make_tuple( 4, 2, 13 ), make_tuple( 4, 3, 14 ), make_tuple( 4, 4, 15 ),make_tuple( 4, 5, 19 ),
		make_tuple( 5, 0, 19 ), make_tuple( 5, 1, 16 ), make_tuple( 5, 2, 17 ), make_tuple( 5, 3, 18 ),make_tuple( 5, 4, 19 ),
		make_tuple( 6, 0, 19 ),make_tuple( 6, 1, 19 ),make_tuple( 6, 2, 19 ),make_tuple( 6, 3, 19 )
	};
	for( const auto& s : setup ) {
		at( get<1>( s ) )[ get<0>( s ) ].type_ = types[ get<2>( s ) ];
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