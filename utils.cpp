#include "utils.h"

using namespace std;

ostream& operator <<( ostream& out, const Pos& p ) {
    out << p.x() << " " << p.y();
    return out;
}

istream& operator >>( istream& in, Pos& p ) {
    in >> p.x() >> p.y();
    return in;
}