#include "boardView.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPolygonF>

#include <cmath>
#include <iostream>
#include <algorithm>

#include "viewUtils.h"

using namespace std;


BoardView::BoardView( QWidget* parent ) :
    QWidget( parent ),
    board_( 0 ),
    selectionMode_( None ), mouseX_( 0 ), mouseY_( 0 ),
    radius_( 0.0 ), innerRadius_( 0.0 ), nodeRadius_( 0.0 ), nodeDiag_( 0.0 ), centerShiftX_( 0.0 ), centerShiftY_( 0.0 )
{
    setMouseTracking( true );
}


void BoardView::mouseMoveEvent( QMouseEvent* event ) {
    // track mouse and update view for picking feedback
    mouseX_ = event->pos().x();
    mouseY_ = event->pos().y();
    update();
    QWidget::mouseMoveEvent( event );
}


void BoardView::mouseReleaseEvent( QMouseEvent* event ) {
    if( selectionMode_ == Hex && hex_.valid() ) {
        setSelectionMode( None );
        update();
        emit hexSelected( hex_ );
    } else if( selectionMode_ == Node && node_.valid() ) {
        setSelectionMode( None );
        update();
        emit nodeSelected( node_ );
    } else if( selectionMode_ == Road && node_.valid() ) {
        if( from_.valid() ) {
            Pos from = from_;
            Pos to = node_;
            setSelectionMode( None );
            node_ = Pos();
            emit roadSelected( from, to );
        } else {
            from_ = node_;
            node_ = Pos();
            emit nodeSelected( from_ );
        }
        update();
    }
    QWidget::mouseReleaseEvent( event );
}


// euclidean distance
float dist( float ax, float ay, float bx, float by ) {
    return sqrt( ( ax - bx ) * ( ax - bx ) + ( ay - by ) * ( ay - by ) );
}


// returns the center of the given node
QPointF BoardView::nodeCenter( const Pos& n ) const {
    int hx = 0;
    int hy = 0;
    int rad = 0;
    if( n.x() % 2 == n.y() % 2 ) {
        // coords of the hex on the left
        hx = n.x() - 1;
        hy = ( n.y() - n.x() ) / 2;
        // then the node is one radius to the right
        rad = 1;
    } else {
        // coords of the hex on the right
        hx = n.x();
        hy = ( n.y() - n.x() - 1 ) / 2;
        // then the node is one radius to the left
        rad = -1;
    }
    return QPointF( radius_ * ( 1 + hx * 1.5 + rad ) + centerShiftX_, innerRadius_ * ( 1 + hy * 2 + hx ) + centerShiftY_ );
}


// draws valid hexes of the given type
// if type is Hex::Any, then only land hexes are drawn - it is assumed that water hexes have been drawn first
void BoardView::drawHexes( QPainter& p, Hex::Type type ) {
    QPolygonF hex;
    for( int i = 0; i < 6; i++ ) {
        hex << QPointF( radius_ * cos( i * 60 * degToRad ), radius_ * sin( i * 60 * degToRad ) );
    }

    for( int hx = 0; hx < board_->hexWidth(); hx++ ) {
        for( int hy = 0; hy < board_->hexHeight(); hy++ ) {
            const auto& h = board_->hex_[ hy ][ hx ];

            bool allowed = h.type_ != Hex::Invalid && h.type_ != Hex::Water && board_->robber_ != Pos( hx, hy );

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

            // hex selection
            if( selectionMode_ == Hex && allowed ) {
                bool underMouse = dist( hexCenter.x(), hexCenter.y(), mouseX_, mouseY_ ) < innerRadius_;
                p.setBrush( underMouse ? QBrush( Qt::white ) : Qt::NoBrush );
                QPen pen( Qt::white );
                pen.setWidth( 2 );
                p.setPen( pen );
                p.drawEllipse( hexCenter, innerRadius_ * 0.85, innerRadius_ * 0.85 );
                if( underMouse ) {
                    hex_ = Pos( hx, hy );
                }
            }

            // draw number on land tiles
            bool hasRobber = board_->robber_ == Pos( hx, hy );
            if( h.number_ < 0 && !hasRobber ) {
                continue;
            }
            // white disc centered inside hex
            p.setPen( Qt::NoPen );
            p.setBrush( hasRobber ? Qt::black : Qt::white );
            p.drawEllipse( hexCenter, radius_ / 2.5, radius_ / 2.5 );
            // number centered in disc
            if( !hasRobber ) {
                QRectF textBox( hexCenter.x() - textSize_ / 2, hexCenter.y() - textSize_ / 2, textSize_, textSize_ );
                p.setPen( Qt::black );
                p.drawText( textBox, Qt::AlignHCenter | Qt::AlignVCenter, QString::number( h.number_ ) );
            }
        }
    }
}


// draws nodes
// if drawHarbors is true, only harbors are drawn, otherwise only towns and cities are drawn
void BoardView::drawNodes( QPainter& p, bool drawHarbors ) {
    for( int nx = 0; nx < board_->nodeWidth(); nx++ ) {
        for( int ny = 0; ny < board_->nodeHeight(); ny++ ) {
            Pos np( nx, ny );
            bool allowed = std::find( board_->allowedNodes_.begin(), board_->allowedNodes_.end(), np ) != board_->allowedNodes_.end();
            const auto& n = board_->node_[ ny ][ nx ];
            QPointF nc = nodeCenter( np );
            // highlight selectable nodes
            if( ( selectionMode_ == Node || selectionMode_ == Road ) && allowed ) {
                bool underMouse = dist( nc.x(), nc.y(), mouseX_, mouseY_ ) < nodeRadius_;
                p.setBrush( underMouse ? QBrush( Qt::white ) : Qt::NoBrush );
                QPen pen( Qt::white );
                pen.setWidth( 2 );
                p.setPen( pen );
                float radius = nodeRadius_ * ( n.type_ == Node::None ? 1 : 2 );
                p.drawEllipse( nc, radius, radius );
                if( underMouse ) {
                    node_ = np;
                }
            }
            // highlight first road node
            if( np == from_ ) {
                p.setBrush( Qt::black );
                QPen pen( Qt::white );
                pen.setWidth( 2 );
                p.setPen( pen );
                p.drawEllipse( nc, nodeRadius_, nodeRadius_ );
            }
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
        }
    }
}


// draws roads
void BoardView::drawRoads( QPainter& p ) const {
    for( const auto& r : board_->road_ ) {
        QPointF from = nodeCenter( r.from_ );
        QPointF to = nodeCenter( r.to_ );
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
void BoardView::paintEvent( QPaintEvent* event ) {
    if( board_ == 0 ) {
        QWidget::paintEvent( event );
        return;
    }

    // reset selection
    node_ = Pos();
    hex_ = Pos();

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


void BoardView::setSelectionMode( SelectionMode mode ) {
    selectionMode_ = mode;
    // other selections are reset during paint but from for road must be preserved between two clicks
    from_ = Pos();
}
