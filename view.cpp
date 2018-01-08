#include "view.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPolygonF>

#include <cmath>
#include <iostream>

using namespace std;

const float degToRad = 3.1415926 / 180.0;
const QColor playerColor[ 4 ] = { Qt::red, Qt::green, Qt::blue, QColor( 255, 127, 0.0 ) };
const QColor tileColor[ Hex::nbTypes ] = { Qt::red, Qt::darkGreen, Qt::yellow, Qt::green, Qt::gray, Qt::darkYellow, Qt::blue };


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


// returns the center of the given node
QPointF View::nodeCenter( unsigned int nx, unsigned int ny ) const {
    int hx = 0;
    int hy = 0;
    int rad = 0;
    if( nx % 2 == ny % 2 ) {
        // coords of the hex on the left
        hx = nx - 1;
        hy = ( ny - nx ) / 2;
        // then the node is one radius to the right
        rad = 1;
    } else {
        // coords of the hex on the right
        hx = nx;
        hy = ( ny - nx - 1 ) / 2;
        // then the node is one radius to the left
        rad = -1;
    }
    return QPointF( radius_ * ( 1 + hx * 1.5 + rad ) + centerShiftX_, innerRadius_ * ( 1 + hy * 2 + hx ) + centerShiftY_ );
}


// draws valid hexes of the given type
// if type is Hex::Any, then only land hexes are drawn - it is assumed that water hexes have been drawn first
void View::drawHexes( QPainter& p, Hex::Type type ) const {
    QPolygonF hex;
    for( int i = 0; i < 6; i++ ) {
        hex << QPointF( radius_ * cos( i * 60 * degToRad ), radius_ * sin( i * 60 * degToRad ) );
    }

    for( unsigned int hx = 0; hx < board_->hexWidth(); hx++ ) {
        for( unsigned int hy = 0; hy < board_->hexHeight(); hy++ ) {
            const auto& h = board_->hex_[ hy ][ hx ];

            if(
                ( type == Hex::Any && ( h.type_ == Hex::Invalid || h.type_ == Hex::Water ) ) ||
                ( type != Hex::Any && h.type_ != type )
            ) {
                continue;
            }

            // center of the hex
            QPointF hexCenter( radius_ * ( 1 + hx * 1.5 ) + centerShiftX_, innerRadius_ * ( 1 + hy * 2 + hx ) + centerShiftY_ );

            auto curHex = hex.translated( hexCenter );
            p.setBrush( h.type_ >= 0 ? tileColor[ h.type_ ] : QColor() );
            QPen pen( Qt::black );
            pen.setWidth( 2 );
            p.setPen( pen );
            p.drawConvexPolygon( curHex );

            // highlight hex under mouse
            if( dist( hexCenter.x(), hexCenter.y(), mouseX_, mouseY_ ) < innerRadius_ ) {
                p.setBrush( Qt::NoBrush );
                QPen pen( h.type_ == Hex::Brick ? Qt::black : Qt::red );
                pen.setWidth( 2 );
                p.setPen( pen );
                p.drawEllipse( hexCenter, innerRadius_ * 0.85, innerRadius_ * 0.85 );
            }

            // draw number on land tiles
            if( h.number_ < 0 ) {
                continue;
            }
            // white disc centered inside hex
            p.setPen( Qt::NoPen );
            p.setBrush( Qt::white );
            p.drawEllipse( hexCenter, radius_ / 2.5, radius_ / 2.5 );
            // number centered in disc
            QRectF textBox( hexCenter.x() - textSize_ / 2, hexCenter.y() - textSize_ / 2, textSize_, textSize_ );
            p.setPen( Qt::black );
            p.drawText( textBox, Qt::AlignHCenter | Qt::AlignVCenter, QString::number( h.number_ ) );
        }
    }
}


// draws nodes
// if drawHarbors is true, only harbors are drawn, otherwise only towns and cities are drawn
void View::drawNodes( QPainter& p, bool drawHarbors ) const {
    for( unsigned int nx = 0; nx < board_->nodeWidth(); nx++ ) {
        for( unsigned int ny = 0; ny < board_->nodeHeight(); ny++ ) {
            const auto& n = board_->node_[ ny ][ nx ];
            QPointF nc = nodeCenter( nx, ny );
            if( drawHarbors ) {
                if( n.harborType_ != Hex::Invalid ) {
                    p.setBrush( n.harborType_ == Hex::Any ? Qt::white : tileColor[ n.harborType_ ] );
                    QPen pen( Qt::black );
                    pen.setWidth( 2 );
                    p.setPen( pen );
                    p.drawEllipse( nc, radius_ * 0.4, radius_ * 0.4 );
                }
            } else if( n.type_ != Node::None ) {
                QPolygonF curNode;
                if( n.type_ == Node::Town ) {
                    curNode << QPointF( nc.x() - nodeDiag_, nc.y() - nodeDiag_ )
                        << QPointF( nc.x(), nc.y() - 2 * nodeDiag_ )
                        << QPointF( nc.x() + nodeDiag_, nc.y() - nodeDiag_ )
                        << QPointF( nc.x() + nodeDiag_, nc.y() + nodeDiag_ )
                        << QPointF( nc.x() - nodeDiag_, nc.y() + nodeDiag_ );
                } else {
                    curNode << QPointF( nc.x() - 2 * nodeDiag_, nc.y() - nodeDiag_ )
                        << QPointF( nc.x() - nodeDiag_, nc.y() - 2 * nodeDiag_ )
                        << QPointF( nc.x(), nc.y() - nodeDiag_ )
                        << QPointF( nc.x() + 2 * nodeDiag_, nc.y() - nodeDiag_ )
                        << QPointF( nc.x() + 2 * nodeDiag_, nc.y() + nodeDiag_ )
                        << QPointF( nc.x() - 2 * nodeDiag_, nc.y() + nodeDiag_ );
                }
                p.setBrush( playerColor[ n.player_ ] );
                QPen pen( Qt::black );
                pen.setWidth( 2 );
                p.setPen( pen );
                p.drawPolygon( curNode );
            }
     
            // highlight node under mouse
            if( dist( nc.x(), nc.y(), mouseX_, mouseY_ ) < nodeRadius_ ) {
                p.setBrush( Qt::NoBrush );
                QPen pen( Qt::red );
                pen.setWidth( 2 );
                p.setPen( pen );
                p.drawEllipse( nc, nodeRadius_ * 0.5, nodeRadius_ * 0.5 );
            }
        }
    }
}


// draws roads
void View::drawRoads( QPainter& p ) const {
    for( const auto& r : board_->road_ ) {
        QPointF from = nodeCenter( r.fromX_, r.fromY_);
        QPointF to = nodeCenter( r.toX_, r.toY_ );
        QPen pen( Qt::black );
        pen.setWidth( radius_ * 0.23 );
        p.setPen( pen );
        p.drawLine( from, to );
        pen = QPen( playerColor[ r.player_ ] );
        pen.setWidth( radius_ * 0.16 );
        p.setPen( pen );
        p.drawLine( from, to );
    }    
}


// draws the board
// highlights selection under mouse
void View::paintEvent( QPaintEvent* event ) {
    if( board_ == 0 ) {
        QWidget::paintEvent( event );
        return;
    }

    // it is assumed that the hex grid has at least one valid hex on its leftmost and rightmost X columns
    // so the full widget width can always be used
    // for rows, the hexes with the min/max Y coords are not necessarily the highest/lowest ones from a
    // graphical standpoint so we have to compute the exact vertical extent and possibly a vertical shift
    // number of inner radius necessary to draw the full grid height
    float totalHeight = 2 + board_->maxHeight() - board_->minHeight();
    // vertical shift upwards to first valid hex center
    float verticalShift = board_->minHeight() - 1;

    // determine best radius to view full grid
    float radiusFromHeight = height() / totalHeight / sin( 60 * degToRad );
    float radiusFromWidth = width() / ( 1 + board_->hexWidth() * 1.5 );
    radius_ = min( radiusFromWidth, radiusFromHeight );
    innerRadius_ = radius_ * sin( 60 * degToRad );

    // shifts to center the display within the widget
    centerShiftX_ = ( width() - radius_ * ( 1 + board_->hexWidth() * 1.5 ) ) / 2;
    centerShiftY_ = ( height() - innerRadius_ * totalHeight ) / 2 - verticalShift * innerRadius_;

    // node dimensions
    nodeRadius_ = radius_ * 0.2;
    nodeDiag_ = nodeRadius_ * cos( 45 * degToRad );

    QPainter p( this );
    p.setRenderHint( QPainter::Antialiasing );
    p.setRenderHint( QPainter::TextAntialiasing );

    // font scaling
    textSize_ = 2 * radius_ * cos( 45 * degToRad );
    auto f = p.font();
    f.setPixelSize( max( 6.0, textSize_ / 2.5 ) );
    p.setFont( f );

    // superposed layers drawn in order
    drawHexes( p, Hex::Water ); // water only
    drawNodes( p, true ); // harbors
    drawHexes( p ); // land hexes
    drawRoads( p );
    drawNodes( p ); // land nodes

    QWidget::paintEvent( event );
}
