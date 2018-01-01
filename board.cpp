#include "board.h"

using namespace std;


/**/


Hex::Hex( Type type ) : type_( type ), number_( -1 ) {
}


const QColor& Hex::color() const {
  static const QColor colors[ nbTypes ] = { Qt::blue, Qt::red, Qt::darkGreen, Qt::yellow, Qt::green, Qt::gray, Qt::darkYellow };
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

	// standard numbering of land hexes
	vector<int> numbers = { 5, 2, 6, 3, 8, 10, 9, 12, 11, 4, 8, 10, 9, 4, 5, 6, 3, 11 };

	// map setup
	vector<pair<int, int>> setup = {
		// lan hexes, inward spiral (start at top, then counter clockwise) for
		// placement of numbers
		{ 3, 1 }, { 2, 2 }, { 1, 3 }, { 1, 4 }, { 1, 5 }, { 2, 5 },
		{ 3, 5 }, { 4, 4 }, { 5, 3 }, { 5, 2 }, { 5, 1 }, { 4, 1 },
		{ 3, 2 }, { 2, 3 }, { 2, 4 }, { 3, 4 }, { 4, 3 }, { 4, 2 }, { 3, 3 },
		// water hexes
		{ 0, 3 }, { 0, 4 }, { 0, 5 }, { 0, 6 }, { 1, 2 }, { 1, 6 },
		{ 2, 1 }, { 2, 6 }, { 3, 0 }, { 3, 6 }, { 4, 0 }, { 4, 5 },
		{ 5, 0 }, { 5, 4 }, { 6, 0 }, { 6, 1 }, { 6, 2 }, { 6, 3 }
	};
	int iType = 0;
	int iNumber = 0;
	for( const auto& s : setup ) {
		auto& hex = at( s.second )[ s.first ];
		hex.type_ = iType < types.size() ? types[ iType++ ] : Hex::Water;
		hex.number_ = hex.type_ != Hex::Desert && hex.type_ != Hex::Water ? numbers[ iNumber++ ] : -1;
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