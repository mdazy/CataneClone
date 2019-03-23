#pragma once

#include <QtGui/QColor>

#include <iostream>
#include <vector>

#include "utils.h"


/**/


class Hex {
public:
	enum Type { Invalid = -1, Brick, Wood, Wheat, Sheep, Rock, Desert, Water, nbTypes, Any };
	static const QString typeName[ Desert ];
   
	Hex( Type type = Invalid );

public:
	Type type_;
	int number_;
};

std::ostream& operator <<( std::ostream& out, const Hex& h );
std::istream& operator >>( std::istream& in, Hex& h );


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

std::ostream& operator <<( std::ostream& out, const Node& n );
std::istream& operator >>( std::istream& in, Node& n );


/**/


class Road {
public:
	Road( int player = -1, const Pos& from = Pos(), const Pos& to = Pos() );

	bool operator ==( const Road& rhs ) const;

public:
	int player_;
	Pos from_;
	Pos to_;
	// internal for longest road computation
	mutable int visit_;
};

std::ostream& operator <<( std::ostream& out, const Road& h );
std::istream& operator >>( std::istream& in, Road& h );


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

	bool roadExists( const Pos& from, const Pos& to, int player = -1 ) const;

	int longestRoadForPlayer( int p ) const;

	QString toString() const;
	void initFromStrings( const QStringList& parts );

protected:
	int longestRoad( const Pos& from, int p, int visit ) const;

public:
	std::vector<std::vector<Hex>> hex_;
	std::vector<std::vector<Node>> node_;
	std::vector<Road> road_;

	std::vector<Pos> allowedNodes_;

	Pos robber_;
};

std::ostream& operator <<( std::ostream& out, const Board& h );
std::istream& operator >>( std::istream& in, Board& h );