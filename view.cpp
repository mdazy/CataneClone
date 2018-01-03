#include "view.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPolygonF>

#include <cmath>
#include <iostream>

using namespace std;

const float degToRad = 3.1415926 / 180.0;


View::View( QWidget* parent ) : QWidget( parent ), board_( 0 ), mouseX_( 0 ), mouseY_( 0 ) {
    setMinimumSize( 500, 500 );
    setMouseTracking( true );
}


void View::mouseMoveEvent( QMouseEvent* event ) {
    // track mouse and update viewer for picking feedback
    mouseX_ = event->pos().x();
    mouseY_ = event->pos().y();
    update();
    QWidget::mouseMoveEvent( event );
}


void View::paintEvent( QPaintEvent* event ) {
    if( board_ == 0 ) {
        QWidget::paintEvent( event );
        return;
    }

    int gridWidth = board_->width();
    int gridHeight = board_->height();

    // determine best radius so that circle within grid is fully visible
    float hexHeight = height() / gridHeight;
    float radiusFromHeight = hexHeight / 2 / sin( 60 * degToRad );
    float hexWidth = width() / gridWidth / sin( 60 * degToRad );
    float radiusFromWidth = hexWidth / 2;
    float radius = min( radiusFromWidth, radiusFromHeight );
    float innerRadius = radius * sin( 60 * degToRad );

    QPolygonF hex;
    for( int i = 0; i < 6; i++ ) {
        hex << QPointF( radius * cos( i * 60 * degToRad ), radius * sin( i * 60 * degToRad ) );
    }

    QPainter p( this );
    // font scaling
    float textSize = 2 * radius * cos( 45 * degToRad );
    auto f = p.font();
    f.setPixelSize( max( 6.0, textSize / 2.5 ) );
    p.setFont( f );

    // hexes
    for( int x = 0; x < gridWidth; x++ ) {
    	for( int y = 0; y < gridHeight; y++ ) {
            const auto& h = board_->hex_[ y ][ x ];

    		if( h.type_ == Hex::Invalid ) {
    			continue;
    		}

    		p.setBrush( h.color() );

            // shift coordinates so that 0,0 is the center of the map
    		float tx = x - gridWidth / 2;
    		float ty = y - gridHeight / 2;
            // center of the hex
            float ox = tx * 1.5; // 1 + cos(60°)
            float oy = ty + tx / 2;
            QPointF center( width() / 2 + ox * radius, height() / 2 + oy * 2 * innerRadius );

            auto curHex = hex.translated( center );
		    p.drawConvexPolygon( curHex );

            // draw nodes
            //int nx[ 6 ] = { x + 1, x + 1, x, x, x, x + 1 };
            //int ny[ 6 ] = { 2 * y + x + 1, 2 * y + x, 2 * y + x , 2 * y + x + 1, 2 * y + x + 2, 2 * y + x + 2 };
            for( int i = 0; i < 6; i++ ) {
                p.setBrush( Qt::white );
                p.setPen( Qt::black );
                p.drawEllipse( curHex[ i ], radius * 0.2, radius * 0.2 );
                if( ( curHex[ i ].x() - mouseX_ ) * ( curHex[ i ].x() - mouseX_ ) + ( curHex[ i ].y() - mouseY_ ) * ( curHex[ i ].y() - mouseY_ ) < radius * 0.2 * radius * 0.2 ) {
                    p.setBrush( Qt::NoBrush );
                    auto prevPen = p.pen();
                    QPen pen( Qt::red );
                    pen.setWidth( 2 );
                    p.setPen( pen );
                    p.drawEllipse( curHex[ i ], radius * 0.1, radius * 0.1 );
                    p.setPen( prevPen );
                }

            }

            // highlight hex under mouse
            if( ( center.x() - mouseX_ ) * ( center.x() - mouseX_ ) + ( center.y() - mouseY_ ) * ( center.y() - mouseY_ ) < innerRadius * innerRadius ) {
                p.setBrush( Qt::NoBrush );
                auto prevPen = p.pen();
                QPen pen( h.color() == Qt::red ? Qt::black : Qt::red );
                pen.setWidth( 2 );
                p.setPen( pen );
                p.drawEllipse( center, innerRadius * 0.85, innerRadius * 0.85 );
                p.setPen( prevPen );
            }

            // draw number on land tiles
            if( h.number_ < 0 ) {
                continue;
            }
            // white disc centered inside hex
            auto prevPen = p.pen();
            p.setPen( Qt::NoPen );
            p.setBrush( Qt::white );
            p.drawEllipse( center, radius / 2.5, radius / 2.5 );
            p.setPen( prevPen );
            // number centered in disc
            QRectF textBox( center.x() - textSize / 2, center.y() - textSize / 2, textSize, textSize );
            p.drawText( textBox, Qt::AlignHCenter | Qt::AlignVCenter, QString::number( h.number_ ) );
    	}
    }

    QWidget::paintEvent( event );
}
