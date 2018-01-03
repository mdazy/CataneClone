#pragma once

#include <QtGui/QColor>

#include <vector>

class Hex {
public:
	enum Type { Invalid = -1, Water, Brick, Wood, Wheat, Sheep, Rock, Desert, nbTypes };
   
	Hex( Type type = Invalid );

	const QColor& color() const;

public:
	Type type_;
	int number_;
};


/**/


class Board {
public:
	Board();

	int height() const;
	int width() const;

public:
	std::vector<std::vector<Hex>> hex_;
};