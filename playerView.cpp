#include "playerView.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QPalette>

#include <QtCore/QEvent>

#include "game.h"
#include "viewUtils.h"

PlayerView::PlayerView( Player* p, QWidget* parent ) : QWidget( parent ), player_( p ), prevPlayer_( *p ) {
    auto vl = new QVBoxLayout( this );
    auto l = new QLabel( "<b>Player " + QString::number( p->number_ + 1 ) + "</b>" );
    l->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    l->setStyleSheet( "background-color:" + playerColor[ p->number_ ].name( QColor::HexRgb ) );
    vl->addWidget( l );
    resources_ = new QLabel( "0 cards" );
    vl->addWidget( resources_ );
    tokens_ = new QLabel( "tokens" );
    vl->addWidget( tokens_ );
    auto hl = new QHBoxLayout();
    vl->addLayout( hl );
    trade_ = new QPushButton( "Trade" );
    trade_->setToolTip( "Not implemented" );
    hl->addWidget( trade_ );
    build_ = new QPushButton( "Build" );
    hl->addWidget( build_ );
    card_ = new QPushButton( "Card" );
    card_->setToolTip( "Not implemented" );
    hl->addWidget( card_ );
    pass_ = new QPushButton( "Pass" );
    hl->addWidget( pass_ ); 
}


void PlayerView::updateView() {
    QString text;
    int total = 0;
    int prevTotal = 0;
    for( int i = 0; i < player_->resources_.size(); i++ ) {
        int n = player_->resources_[ i ];
        int prevN = prevPlayer_.resources_[ i ];
        if( n > 0 ) {
            text += ( prevN < n ? "<b>" : "" ) + QString::number( n ) + " " + Hex::typeName[ i ] + ( prevN < n ? "</b>" : "" ) + " ";
        }
        total += n;
        prevTotal += prevN;
    }
    resources_->setText( ( prevTotal < total ? "<b>" : "" ) + QString::number( total ) + " cards: " + ( prevTotal < total ? "</b>" : "" ) + text );
    tokens_->setText( QString::number( player_->roads_ ) + " roads " + QString::number( player_->towns_ ) + " towns " + QString::number( player_->cities_ ) + " cities" );
    prevPlayer_ = *player_;
}


void PlayerView::enableButtons( bool enabled ) {
    trade_->setEnabled( false );
    build_->setEnabled( enabled && player_->game_->canBuild() );
    build_->setToolTip( enabled && ! player_->game_->canBuild() ? "Nothing can be built" : "" );
    card_->setEnabled( false );
    pass_->setEnabled( enabled );
}