#pragma once

class Pos : public std::pair<int, int> {
public:
    Pos( int x = -1, int y = -1 ) : std::pair<int, int>( x, y ) {}

    int& x() { return first; }
    int& y() { return second; }
    const int& x() const { return first; }
    const int& y() const { return second; }

    bool valid() const { return first >= 0 && second >= 0; };
};