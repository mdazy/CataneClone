#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedLayout>
#include <QtCore/QCoreApplication>

#include "gameView.h"
#include "game.h"
#include "boardView.h"
#include "playerView.h"
#include "die.h"
#include "resourceSelectors.h"
#include "chatWidget.h"

#include <iostream>
using namespace std;

GameView::GameView( Game* game, Messenger* messenger ) :
    QWidget( Q_NULLPTR ),
    game_( game ), playing_( false ),
    boardView_( 0 ), players3_( 0 ), players4_( 0 ),
    messenger_( messenger )
{
    auto hl = new QHBoxLayout( this );

    stack_ = new QStackedLayout;
    hl->addLayout( stack_, 3 );
    hl->addWidget( new ChatWidget( messenger ), 1 );

    stack_->addWidget( buildPlayersSelection() );
    stack_->addWidget( buildGameView() );

    connect( game, &Game::requestNbPlayers, this, &GameView::pickNbPlayers );
    connect( this, QOverload<int>::of( &GameView::nbPlayersPicked ), game, &Game::startWithPlayers );

    connect( game, &Game::requestStartPositions, this, &GameView::pickStartPositions );

    connect( game, &Game::rollDice, this, &GameView::rollDice );

    connect( game, &Game::diceRolled, this, &GameView::diceRolled );

    connect( game, &Game::pickDiscard, this, &GameView::discard );

    connect( game, &Game::requestTrade, this, &GameView::trade );

    connect( game, &Game::requestHex, this, &GameView::pickHex );
    connect( boardView_, &BoardView::hexSelected, game, &Game::robAround );

    connect( game, &Game::gameOver, this, &GameView::gameOver );
}


void GameView::pickNbPlayers() {
    stack_->setCurrentIndex( 0 );
}


void GameView::pickStartPositions() {
    stack_->setCurrentIndex( 1 );
    connect( game_, &Game::requestNode, this, &GameView::pickNode) ;
    connect( game_, &Game::requestRoad, this, &GameView::pickRoad );
    connect( boardView_, &BoardView::roadSelected, game_, &Game::startRoadPicked );
    pickNode();
}


void GameView::pickHex() {
    playerView_[ game_->curPlayer_ ]->enableButtons( false );
    boardView_->setSelectionMode( BoardView::Hex );
    boardView_->update();
}


void GameView::pickNode() {
    QObject::disconnect( boardView_, &BoardView::nodeSelected, 0, 0 );
    void (Game::*slot)(const Pos&) = 0;
    switch( game_->curPlayer().state_ ) {
        case Player::PickStartTown: {
            slot = &Game::startNodePicked;
            break;
        }
        case Player::PickBuildTown: {
            slot = QOverload<const Pos&>::of( &Game::buildTown );
            break;
        }
        case Player::PickCity: {
            slot = &Game::buildCity;
            break;
        }
        case Player::PickRobTown: {
            slot = &Game::rob;
            break;
        }
    }
    connect( boardView_, &BoardView::nodeSelected, game_, slot );
    playerView_[ game_->curPlayer_ ]->enableButtons( false );
    boardView_->setSelectionMode( BoardView::Node );
    boardView_->update();
}


void GameView::pickRoad( const Pos& from ) {
    QObject::disconnect( boardView_, &BoardView::roadSelected, 0, 0 );
    QObject::disconnect( boardView_, &BoardView::nodeSelected, 0, 0 );
    if( game_->curPlayer().state_ == Player::PickBuildRoad ) {
        connect( boardView_, &BoardView::nodeSelected, game_, &Game::setupAllowedRoadEndNodes );
        connect( boardView_, &BoardView::roadSelected, game_, QOverload<const Pos&, const Pos&>::of( &Game::buildRoad ) );
    } else if( game_->curPlayer().state_ == Player::PickStartRoad ) {
        connect( boardView_, &BoardView::roadSelected, game_, &Game::startRoadPicked );
    }
    playerView_[ game_->curPlayer_ ]->enableButtons( false );
    boardView_->setSelectionMode( BoardView::Road );
    boardView_->from_ = from;
    boardView_->update();
}


QWidget* GameView::buildPlayersSelection() {
    auto playersSelection = new QWidget;

    auto l = new QHBoxLayout( playersSelection );
    players3_ = new QPushButton( "3 players" );
    l->addWidget( players3_ );
    players4_ = new QPushButton( "4 players" );
    l->addWidget( players4_ );

    connect( players3_, &QPushButton::clicked, this, QOverload<>::of( &GameView::nbPlayersPicked ) );
    connect( players4_, &QPushButton::clicked, this, QOverload<>::of( &GameView::nbPlayersPicked ) );

    return playersSelection;
}


void GameView::nbPlayersPicked() {
    int nbPlayers = sender() == players3_ ? 3 : 4;
    for( int i = nbPlayers; i < 4; i++ ) {
        playerView_[ i ]->hide();
    }
    emit nbPlayersPicked( nbPlayers );
}


QWidget* GameView::buildGameView() {
    auto* gameView = new QWidget;
    gameView->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    // layout for board + game state
    auto l = new QHBoxLayout( gameView );
    boardView_ = new BoardView;
    boardView_->setMinimumSize( 500, 500 );
    boardView_->board_ = &game_->board_;
    l->addWidget( boardView_, 1 );
    // layout for game state
    auto vl = new QVBoxLayout();
    l->addLayout( vl );
    // load/save game state
    l = new QHBoxLayout();
    vl->addLayout( l );
    auto p = new QPushButton( "Save state" );
    l->addWidget( p );
    connect( p, &QPushButton::clicked, game_, &Game::save );
    p = new QPushButton( "Load state" );
    l->addWidget( p );
    connect( p, &QPushButton::clicked, this, &GameView::loadState );
    // layout for dice
    l = new QHBoxLayout();
    vl->addLayout( l );
    die1_ = new Die();
    die2_ = new Die();
    l->addWidget( die1_ );
    l->addWidget( die2_ );
    // players
    for( int i = 0; i < 4; i++ ) {
        auto pv = new PlayerView( &game_->player_[ i ] );
        pv->enableButtons( false );
        vl->addWidget( pv );
        playerView_.push_back( pv );
        connect( pv->roll_, &QPushButton::clicked, game_, &Game::playTurn );
        connect( pv->trade_, &QPushButton::clicked, game_, &Game::startTrade );
        connect( pv->buildRoad_, &QPushButton::clicked, game_, QOverload<>::of( &Game::buildRoad ) );
        connect( pv->buildTown_, &QPushButton::clicked, game_, QOverload<>::of( &Game::buildTown ) );
        connect( pv->buildCity_, &QPushButton::clicked, game_, QOverload<>::of( &Game::buildCity ) );
        connect( pv->buildCard_, &QPushButton::clicked, game_, &Game::buildCard );
        connect( pv->playKnight_, &QPushButton::clicked, game_, &Game::knight );
        connect( pv->playRoads_, &QPushButton::clicked, game_, &Game::buildRoads );
        connect( pv->playInvention_, &QPushButton::clicked, this, &GameView::invention );
        connect( pv->playMonopoly_, &QPushButton::clicked, this, &GameView::monopoly );
        connect( pv->pass_, &QPushButton::clicked, game_, &Game::nextPlayer );       
    }
    vl->addStretch();
    connect( game_, &Game::updatePlayer, this, &GameView::updatePlayer );

    return gameView;
}


void GameView::updatePlayer( int player, bool buttons ) {
    if( player >= 0 ) {
        playerView_[ player ]->updateView();
        if( buttons ) {
            playerView_[ player ]->enableButtons( playing_ );
        }
    } else {
        for( int i = 0; i < game_->nbPlayers_; i++ ) {
            playerView_[ i ]->updateView();
        }
    }
}


void GameView::rollDice() {
    playing_ = true;
    playerView_[ ( game_->curPlayer_ + game_->nbPlayers_ - 1 ) % game_->nbPlayers_ ]->enableButtons( false );
    playerView_[ game_->curPlayer_ ]->roll_->setEnabled( true );
    updatePlayer();
}


void GameView::diceRolled( int die1, int die2 ) {
    playerView_[ game_->curPlayer_ ]->roll_->setEnabled( false );
    playerView_[ game_->curPlayer_ ]->enableButtons( true );
    die1_->setValue( die1 );
    die2_->setValue( die2 );
    updatePlayer();
}


void GameView::loadState() {
    game_->load();
    update();
}


void GameView::discard( Player* p ) {
    auto d = new DiscardSelector( p, this );
    connect( d, &DiscardSelector::selected, game_, &Game::discard );
    d->exec();
}


void GameView::trade( Player* p ) {
    auto d = new TradeSelector( p, this );
    connect( d, &TradeSelector::selected, game_,&Game::trade );
    d->exec();
}


void GameView::invention() {
    auto d = new NumberSelector( 2, this );
    connect( d, &NumberSelector::selected, game_, &Game::invention );
    d->exec();
}


void GameView::monopoly() {
    auto d = new NumberSelector( 1, this );
    connect( d, &NumberSelector::selected, game_, &Game::monopoly );
    d->exec();
}


void GameView::gameOver( const vector<Player>& players ) {
    QMessageBox::information(
        dynamic_cast<QWidget*>( parent() ), "Game over",
        QString( "Game over, player " ) + QString::number( players.front().number_ + 1 ) + " wins."
    );
    qApp->exit();
}