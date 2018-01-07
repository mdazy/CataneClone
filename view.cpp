#include "view.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPolygonF>

#include <cmath>
#include <iostream>

using namespace std;

const float degToRad = 3.1415926 / 180.0;


View::View( QWidget* parent ) : QWidget( parent ), board_( 0 ), mouseX_( 0 ), mouseY_( 0 ) {
    setMouseTracking( true );
}


void View::mouseMoveEvent( QMouseEvent* event ) {
    // track mouse and update viewer for picking feedback
    mouseX_ = event->pos().x();
    mouseY_ = event->pos().y();
    update();
    QWidget::mouseMoveEvent( event );
}


// euclidean distance
float dist( float ax, float ay, float bx, float by ) {
    return sqrt( ( ax - bx ) * ( ax - bx ) + ( ay - by ) * ( ay - by ) );
}


void View::paintEvent( QPaintEvent* event ) {
    if( board_ == 0 ) {
        QWidget::paintEvent( event );
        return;
    }

    // number of inner radius necessary to draw the full grid height
    float totalHeight = 2 + board_->maxHeight() - board_->minHeight();
    // vertical shift upwards to first valid hex center
    float verticalShift = board_->minHeight() - 1;

    // determine best radius to view full grid
    float radiusFromHeight = height() / totalHeight / sin( 60 * degToRad );
    float radiusFromWidth = width() / ( 1 + board_->hexWidth() * 1.5 );
    float radius = min( radiusFromWidth, radiusFromHeight );
    float innerRadius = radius * sin( 60 * degToRad );

    // shifts to center display
    float centerShiftX = ( width() - radius * ( 1 + board_->hexWidth() * 1.5 ) ) / 2;
    float centerShiftY = ( height() - innerRadius * totalHeight ) / 2;

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
    for( unsigned int hx = 0; hx < board_->hexWidth(); hx++ ) {
    	for( unsigned int hy = 0; hy < board_->hexHeight(); hy++ ) {
            const auto& h = board_->hex_[ hy ][ hx ];

    		if( h.type_ == Hex::Invalid ) {
    			continue;
    		}

            // center of the hex
            QPointF hexCenter( radius * ( 1 + hx * 1.5 ) + centerShiftX, innerRadius * ( 1 + hy * 2 + hx - verticalShift ) + centerShiftY );

            auto curHex = hex.translated( hexCenter );
            p.setBrush( h.color() );
            p.setPen( Qt::black );
		    p.drawConvexPolygon( curHex );

            // highlight hex under mouse
            if( dist( hexCenter.x(), hexCenter.y(), mouseX_, mouseY_ ) < innerRadius ) {
                p.setBrush( Qt::NoBrush );
                QPen pen( h.color() == Qt::red ? Qt::black : Qt::red );
                pen.setWidth( 2 );
                p.setPen( pen );
                p.drawEllipse( hexCenter, innerRadius * 0.85, innerRadius * 0.85 );
            }

            // draw number on land tiles
            if( h.number_ < 0 ) {
                continue;
            }
            // white disc centered inside hex
            p.setPen( Qt::NoPen );
            p.setBrush( Qt::white );
            p.drawEllipse( hexCenter, radius / 2.5, radius / 2.5 );
            // number centered in disc
            QRectF textBox( hexCenter.x() - textSize / 2, hexCenter.y() - textSize / 2, textSize, textSize );
            p.setPen( Qt::black );
            p.drawText( textBox, Qt::AlignHCenter | Qt::AlignVCenter, QString::number( h.number_ ) );
    	}
    }

    // nodes
    float nodeRadius = radius * 0.2;
    float nodeDiag = nodeRadius * cos( 45 * degToRad );
    for( unsigned int nx = 0; nx < board_->nodeWidth(); nx++ ) {
        for( unsigned int ny = 0; ny < board_->nodeHeight(); ny++ ) {
            const auto& n = board_->node_[ ny ][ nx ];
            if( n.harborType_ != Hex::Invalid ) {
                // TODO: draw harbor
            }

            int hx = 0;
            int hy = 0;
            int rad = 0;
            if( nx % 2 == ny % 2 ) {
                hx = nx - 1;
                hy = ( ny - nx ) / 2;
                rad = 1;
            } else {
                hx = nx;
                hy = ( ny - nx - 1 ) / 2;
                rad = -1;
            }
            QPointF nodeCenter( radius * ( 1 + hx * 1.5 + rad ) + centerShiftX, innerRadius * ( 1 + hy * 2 + hx - verticalShift ) + centerShiftY );

            if( n.type_ != Node::None ) {
                QPolygonF curNode;
                if( n.type_ == Node::Town ) {
                    curNode << QPointF( nodeCenter.x() - nodeDiag, nodeCenter.y() - nodeDiag )
                        << QPointF( nodeCenter.x(), nodeCenter.y() - 2 * nodeDiag )
                        << QPointF( nodeCenter.x() + nodeDiag, nodeCenter.y() - nodeDiag )
                        << QPointF( nodeCenter.x() + nodeDiag, nodeCenter.y() + nodeDiag )
                        << QPointF( nodeCenter.x() - nodeDiag, nodeCenter.y() + nodeDiag );
                } else {
                    curNode << QPointF( nodeCenter.x() - nodeDiag, nodeCenter.y() - nodeDiag )
                        << QPointF( nodeCenter.x(), nodeCenter.y() - 2 * nodeDiag )
                        << QPointF( nodeCenter.x() + nodeDiag, nodeCenter.y() - nodeDiag )
                        << QPointF( nodeCenter.x() + 3 * nodeDiag, nodeCenter.y() - nodeDiag )
                        << QPointF( nodeCenter.x() + 3 * nodeDiag, nodeCenter.y() + nodeDiag )
                        << QPointF( nodeCenter.x() - nodeDiag, nodeCenter.y() + nodeDiag );
                }
                p.setBrush( Qt::white ); // TODO: player color
                p.setPen( Qt::black );
                p.drawPolygon( curNode );
            }

            // highlight node under mouse
            if( dist( nodeCenter.x(), nodeCenter.y(), mouseX_, mouseY_ ) < nodeRadius ) {
                p.setBrush( Qt::NoBrush );
                QPen pen( Qt::red );
                pen.setWidth( 2 );
                p.setPen( pen );
                p.drawEllipse( nodeCenter, nodeRadius * 0.5, nodeRadius * 0.5 );
            }
        }
    }

    QWidget::paintEvent( event );
}
