#include "playerView.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QPalette>

#include "game.h"
#include "viewUtils.h"

PlayerView::PlayerView( Player* p, QWidget* parent ) : QWidget( parent ), player_( p ) {
    auto vl = new QVBoxLayout( this );
    auto l = new QLabel( "<b>Player " + QString::number( p->number_ ) + "</b>" );
    l->setStyleSheet( "background-color:" + playerColor[ p->number_ - 1 ].name( QColor::HexRgb ) );
    vl->addWidget( l );
    resources_ = new QLabel( "0 cards" );
    vl->addWidget( resources_ );
}


void PlayerView::updateView() {
    QString text;
    int total = 0;
    for( int i = 0; i < player_->resources_.size(); i++ ) {
        int n = player_->resources_[ i ];
        if( n > 0 ) {
            text += QString::number( n ) + " " + Hex::typeName[ i ] + " ";
        }
        total += n;
    }
    resources_->setText( QString::number( total ) + " cards: " + text );
}

