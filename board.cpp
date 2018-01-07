#include "board.h"

using namespace std;


/**/


Hex::Hex( Type type ) : type_( type ), number_( -1 ) {
}


const QColor& Hex::color() const {
	static const QColor colors[ nbTypes ] = { Qt::red, Qt::darkGreen, Qt::yellow, Qt::green, Qt::gray, Qt::darkYellow, Qt::blue };
  	if( type_ < 0 ) {
  		return colors[ 0 ];
  	}
  	return colors[ type_ ];
}


/**/


Node::Node() : player_( -1 ), type_( None ), harborType_( Hex::Invalid ) {
}


/**/


Board::Board() {
	// standard map is 7 by 7
	hex_.resize( 7, vector<Hex>( 7 ) );
	node_.resize( hexHeight() * 2 + hexWidth() + 2, vector<Node>( hexWidth() + 1 ) );

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
	// TODO: factorize template
	int nbHexes = types.size();
	for( int i = 0; i < 100; i++ ) {
		int a = rand() % nbHexes;
		int b = ( a + rand() % ( nbHexes - 1 ) ) % nbHexes;
		auto t = types[ a ];
		types[ a ] = types[ b ];
		types[ b ] = t;
	}

	// standard numbering of land hexes
	vector<int> numbers = { 5, 2, 6, 3, 8, 10, 9, 12, 11, 4, 8, 10, 9, 4, 5, 6, 3, 11 };

	// map setup
	vector<pair<int, int>> setup = {
		// land hexes, ordered along inward spiral (counter clockwise, starting at top)
		// for placement of numbers according to rules
		{ 3, 1 }, { 2, 2 }, { 1, 3 }, { 1, 4 }, { 1, 5 }, { 2, 5 },
		{ 3, 5 }, { 4, 4 }, { 5, 3 }, { 5, 2 }, { 5, 1 }, { 4, 1 },
		{ 3, 2 }, { 2, 3 }, { 2, 4 }, { 3, 4 }, { 4, 3 }, { 4, 2 }, { 3, 3 },
		// water hexes
		{ 0, 3 }, { 0, 4 }, { 0, 5 }, { 0, 6 }, { 1, 2 }, { 1, 6 },
		{ 2, 1 }, { 2, 6 }, { 3, 0 }, { 3, 6 }, { 4, 0 }, { 4, 5 },
		{ 5, 0 }, { 5, 4 }, { 6, 0 }, { 6, 1 }, { 6, 2 }, { 6, 3 }
	};
	unsigned int iType = 0;
	unsigned int iNumber = 0;
	for( const auto& s : setup ) {
		auto& h = hex_[ s.second ][ s.first ];
		h.type_ = iType < types.size() ? types[ iType++ ] : Hex::Water;
		h.number_ = h.type_ != Hex::Desert && h.type_ != Hex::Water ? numbers[ iNumber++ ] : -1;
	}

	// random nodes for display test purposes
	for( auto& r : node_ ) {
		for( auto& n : r ) {
			n.type_ = Node::Type( rand() % 3 );
			n.player_ = rand() % 4;
		}
	}
}


// height in number of hexes
unsigned int Board::hexHeight() const {
	return hex_.size();
}


// width in number of hexes
unsigned int Board::hexWidth() const {
	if( hexHeight() == 0 ) {
		return 0;
	}
	return hex_[ 0 ].size();
}


// height in number of nodes
unsigned int Board::nodeHeight() const {
	return node_.size();
}


// width in number of nodes
unsigned int Board::nodeWidth() const {
	if( nodeHeight() == 0 ) {
		return 0;
	}
	return node_[ 0 ].size();
}


// maximum vertical height of the lowest valid hex
unsigned int Board::maxHeight() const {
	unsigned int result = 0;
	for( unsigned int hy = 0; hy < hexHeight(); hy++ ) {
		for( unsigned int hx = 0; hx < hexWidth(); hx++ ) {
			if( hex_[ hy ][ hx ].type_ != Hex::Invalid ) {
				result = max( result, 1 + 2 * hy + hx );
			}
		}
	}
	return result;
}

// minimum vertical height of the highest valid hex
unsigned int Board::minHeight() const {
	unsigned int result = -1;
	for( unsigned int hy = 0; hy < hexHeight(); hy++ ) {
		for( unsigned int hx = 0; hx < hexWidth(); hx++ ) {
			if( hex_[ hy ][ hx ].type_ != Hex::Invalid ) {
				result = min( result, 1 + 2 * hy + hx );
			}
		}
	}
	return result;
}