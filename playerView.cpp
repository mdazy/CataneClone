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
    playerName_ = new QLabel( QString::number( p->number_ + 1 ) );
    playerName_->setStyleSheet( "background-color:" + playerColor[ p->number_ ].name( QColor::HexRgb ) );
    vl->addWidget( playerName_ );
    resources_ = new QLabel( "0 cards" );
    vl->addWidget( resources_ );
    tokens_ = new QLabel( "tokens" );
    vl->addWidget( tokens_ );
    // action buttons
    auto hl = new QHBoxLayout();
    vl->addLayout( hl );
    trade_ = new QPushButton( "Trade" );
    trade_->setToolTip( "Not implemented" );
    hl->addWidget( trade_ );
    card_ = new QPushButton( "Play card" );
    card_->setToolTip( "Not implemented" );
    hl->addWidget( card_ );
    pass_ = new QPushButton( "Pass" );
    hl->addWidget( pass_ ); 
    // build buttons
    hl = new QHBoxLayout();
    vl->addLayout( hl );
    buildRoad_ = new QPushButton( "Build road" );
    hl->addWidget( buildRoad_ );
    buildTown_ = new QPushButton( "Build town" );
    hl->addWidget( buildTown_ );
    buildCity_ = new QPushButton( "Build city" );
    hl->addWidget( buildCity_ );
    buildCard_ = new QPushButton( "Build dev card" );
    hl->addWidget( buildCard_ );
    
}


void PlayerView::updateView() {
    playerName_->setText( QString( player_->number_ == player_->game_->curPlayer_ ? "<b>" : "" ) + "Player " + QString::number( player_->number_ + 1 ) + QString( player_->number_ == player_->game_->curPlayer_ ? "</b>" : "" ) );    QString text;
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
    buildRoad_->setEnabled( enabled && player_->game_->canBuildRoad() );
    buildRoad_->setToolTip( enabled && ! player_->game_->canBuildRoad() ? "Cannot be built" : "" );
    buildTown_->setEnabled( enabled && player_->game_->canBuildTown() );
    buildTown_->setToolTip( enabled && ! player_->game_->canBuildTown() ? "Cannot be built" : "" );
    buildCity_->setEnabled( false /*enabled && player_->game_->canBuildCity()*/ );
    buildCity_->setToolTip( "Not implemented" /*enabled && ! player_->game_->canBuildCity() ? "Cannot be built" : ""*/ );
    buildCard_->setEnabled( false /*enabled && player_->game_->canBuildCard()*/ );
    buildCard_->setToolTip( "Not implemented" /*enabled && ! player_->game_->canBuildCard() ? "Cannot be built" : ""*/ );
    card_->setEnabled( false );
    pass_->setEnabled( enabled );
}