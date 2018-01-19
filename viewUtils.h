#pragma once

#include <QtGui/QColor>

#include "board.h"

const float degToRad = 3.1415926 / 180.0;
const QColor playerColor[ 4 ] = { QColor( 255, 127, 127 ), QColor( 127, 255, 127 ), QColor( 127, 127, 255 ), QColor( 255, 180, 0 ) };
const QColor tileColor[ Hex::nbTypes ] = { Qt::red, Qt::darkGreen, Qt::yellow, Qt::green, Qt::gray, Qt::darkYellow, Qt::blue };