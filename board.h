#pragma once

#include <QtGui/QColor>

#include <vector>

#include "utils.h"

class Hex {
public:
	enum Type { Invalid = -1, Brick, Wood, Wheat, Sheep, Rock, Desert, Water, nbTypes, Any };
	static const QString typeName[ Desert ];
   
	Hex( Type type = Invalid );

public:
	Type type_;
	int number_;
};


/**/


class Node {
public:
	enum Type { None = 0, Town, City };
	Node();

public:
	int player_;
	Type type_;
	Hex::Type harborType_;
};


/**/


class Road {
public:
	Road();

public:
	int player_;
	Pos from_;
	Pos to_;
};


/**/


class Board {
public:
	Board();

	int hexHeight() const;
	int hexWidth() const;

	int nodeHeight() const;
	int nodeWidth() const;

	int minHeight() const;
	int maxHeight() const;

	static std::vector<Pos> nodesAroundHex( const Pos& h );
	static std::vector<Pos> nodesAroundNode( const Pos& n );
	static std::vector<Pos> hexesAroundNode( const Pos& n );

	bool landNode( const Pos& n ) const;

public:
	std::vector<std::vector<Hex>> hex_;
	std::vector<std::vector<Node>> node_;
	std::vector<Road> road_;

	std::vector<Pos> allowedNodes_;
};