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
    resources_ = new QLabel( "0 resource cards" );
    vl->addWidget( resources_ );
    devCards_ = new QLabel( "0 dev cards" );
    vl->addWidget( devCards_ );
    tokens_ = new QLabel( "tokens" );
    vl->addWidget( tokens_ );
    // action buttons
    auto hl = new QHBoxLayout();
    vl->addLayout( hl );
    roll_ = new QPushButton( "Roll" );
    roll_->setEnabled( false );
    hl->addWidget( roll_ );
    trade_ = new QPushButton( "Trade" );
    hl->addWidget( trade_ );
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
    // card buttons
    hl = new QHBoxLayout();
    vl->addLayout( hl );
    playKnight_ = new QPushButton( "Knight" );
    hl->addWidget( playKnight_ );
    playRoads_ = new QPushButton( "Roads" );
    hl->addWidget( playRoads_ );
    playMonopoly_ = new QPushButton( "Monopoly" );
    hl->addWidget( playMonopoly_ );
    playInvention_ = new QPushButton( "Invention" );
    hl->addWidget( playInvention_ );    
}


void PlayerView::updateView() {
    QString text = QString::number( player_->number_ + 1 );
    text += " (" + QString::number( player_->score() ) + " points)";
    text += ", " + QString::number( player_->armySize_ ) + " knights ";
    if( player_->largestArmy_ ) {
        text += " (largest army)";
    }
    text += ", " + QString::number( player_->roadLength_ ) + " roads";
    if( player_->longestRoad_ ) {
        text += " (longest road)";
    }
    playerName_->setText( QString( player_->number_ == player_->game_->curPlayer_ ? "<b>" : "" ) + "Player " + text + QString( player_->number_ == player_->game_->curPlayer_ ? "</b>" : "" ) );
    // update resources
    text = "";
    for( int i = 0; i < player_->resources_.size(); i++ ) {
        int n = player_->resources_[ i ];
        int prevN = prevPlayer_.resources_[ i ];
        bool diff = n != prevN;
        if( n > 0 ) {
            text += ( diff ? "<b>" : "" ) + QString::number( n ) + " " + Hex::typeName[ i ] + ( diff ? "</b>" : "" ) + " ";
        }
    }
    // dev cards
    int total = player_->nbResourceCards();
    int prevTotal = prevPlayer_.nbResourceCards();
    bool diff = total != prevTotal;
    resources_->setText( ( diff ? "<b>" : "" ) + QString::number( total ) + " resource cards: " + ( diff ? "</b>" : "" ) + text );
    total = 0;
    prevTotal = 0;
    text = "";
    for( int i = 0; i < player_->devCards_.size(); i++ ) {
        int n = player_->devCards_[ i ];
        int prevN = prevPlayer_.devCards_[ i ];
        bool diff = n != prevN;
        if( n > 0 ) {
            text += ( diff ? "<b>" : "" ) + QString::number( n ) + " " + cardName( DevCard( i ) ) + ( diff ? "</b>" : "" ) + " ";
        }
        total += n;
        prevTotal += prevN;
    }
    diff = total != prevTotal;
    devCards_->setText( ( diff ? "<b>" : "" )  + QString::number( total ) + " dev cards: " + ( diff ? "</b>" : "" )  + text );
    tokens_->setText( QString::number( player_->roads_ ) + " roads " + QString::number( player_->towns_ ) + " towns " + QString::number( player_->cities_ ) + " cities" );
    prevPlayer_ = *player_;
}


void PlayerView::enableButtons( bool enabled ) {
    const auto game = player_->game_;
    trade_->setEnabled( enabled && game->canTrade() );
    trade_->setToolTip( enabled && game->canTrade() ? "" : "Cannot trade now" );
    pass_->setEnabled( enabled && player_->state_ == Player::Waiting );
    buildRoad_->setEnabled( enabled && game->canBuildRoad() );
    buildRoad_->setToolTip( enabled && game->canBuildRoad() ? "" : "Cannot be built" );
    buildTown_->setEnabled( enabled && game->canBuildTown() );
    buildTown_->setToolTip( enabled && game->canBuildTown() ? "" : "Cannot be built" );
    buildCity_->setEnabled( enabled && game->canBuildCity() );
    buildCity_->setToolTip( enabled && game->canBuildCity() ? "" : "Cannot be built" );
    buildCard_->setEnabled( enabled && game->canBuildCard() );
    buildCard_->setToolTip( enabled && game->canBuildCard() ? "" : "Cannot be built" );
    playKnight_->setEnabled( enabled && player_->canPlayKnight() );
    playKnight_->setToolTip( enabled && player_->canPlayKnight() ? "" : "Cannot play this card" );
    playRoads_->setEnabled( enabled && player_->canPlayRoads() );
    playRoads_->setToolTip( enabled && player_->canPlayRoads() ? "" : "Cannot play this card" );
    playMonopoly_->setEnabled( enabled && player_->canPlayMonopoly() );
    playMonopoly_->setToolTip( enabled && player_->canPlayMonopoly() ? "" : "Cannot play this card" );
    playInvention_->setEnabled( enabled && player_->canPlayInvention() );
    playInvention_->setToolTip( enabled && player_->canPlayInvention() ? "" : "Cannot play this card" );
}