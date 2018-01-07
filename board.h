#pragma once

#include <QtGui/QColor>

#include <vector>

class Hex {
public:
	enum Type { Invalid = -1, Brick, Wood, Wheat, Sheep, Rock, Desert, Water, nbTypes, Any };
   
	Hex( Type type = Invalid );

	const QColor& color() const;

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


class Board {
public:
	Board();

	unsigned int hexHeight() const;
	unsigned int hexWidth() const;

	unsigned int nodeHeight() const;
	unsigned int nodeWidth() const;

	unsigned int minHeight() const;
	unsigned int maxHeight() const;

public:
	std::vector<std::vector<Hex>> hex_;
	std::vector<std::vector<Node>> node_;
};