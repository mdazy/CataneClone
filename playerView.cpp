#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

#include "game.h"
#include "playerView.h"

PlayerView::PlayerView( Player* p, QWidget* parent ) : QWidget( parent ), player_( p ) {
    auto l = new QVBoxLayout( this );
    l->addWidget( new QLabel( "<b>Player " + QString::number( p->number_ ) + "</b>" ) );
    resources_ = new QLabel( "0 cards" );
    l->addWidget( resources_ );
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

