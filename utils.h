#pragma once

#include <iostream>
#include <vector>

class Pos : public std::pair<int, int> {
public:
    Pos( int x = -1, int y = -1 ) : std::pair<int, int>( x, y ) {}

    int& x() { return first; }
    int& y() { return second; }
    const int& x() const { return first; }
    const int& y() const { return second; }

    bool valid() const { return first >= 0 && second >= 0; };
};

std::ostream& operator <<( std::ostream& out, const Pos& p );
std::istream& operator >>( std::istream& in, Pos& p );


// randomize contents a vector
template<typename T>
void randomize( std::vector<T>& v ) {
	int size = v.size();
	for( int i = 0; i < size * 2; i++ ) {
		int a = rand() % size;
		int b = ( a + rand() % ( size - 1 ) ) % size;
		auto t = v[ a ];
		v[ a ] = v[ b ];
		v[ b ] = t;
	}
}