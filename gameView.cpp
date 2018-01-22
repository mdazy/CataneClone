#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include "gameView.h"
#include "game.h"
#include "boardView.h"
#include "playerView.h"
#include "die.h"

#include <iostream>
using namespace std;

GameView::GameView( Game* game, QWidget* parent ) :
    QStackedWidget( parent ),
    game_( game ),
    playersSelection_( 0 ), players3_( 0 ), players4_( 0 ),
    gameView_( 0 ), boardView_( 0 )
{
    buildPlayersSelection();
    buildGameView();

    connect( game, SIGNAL( requestNbPlayers() ), this, SLOT( pickNbPlayers() ) );
    connect( this, SIGNAL( nbPlayersPicked( int ) ), game, SLOT( startWithPlayers( int ) ) );

    connect( game, SIGNAL( requestStartPositions() ), this, SLOT( pickStartPositions() ) );

    connect( game, SIGNAL( rollDice() ), this, SLOT( rollDice() ) );
    connect( roll_, SIGNAL( clicked() ), game_, SLOT( playTurn() ) );

    connect( game, SIGNAL( diceRolled( int, int ) ), this, SLOT( diceRolled( int, int ) ) );
}


void GameView::pickNbPlayers() {
    setCurrentWidget( playersSelection_ );
}


void GameView::pickStartPositions() {
    setCurrentWidget( gameView_ );
    connect( game_, SIGNAL( requestNode() ), this, SLOT( pickStartNode() ) );
    connect( game_, SIGNAL( requestRoad( Pos ) ), this, SLOT( pickStartRoad( const Pos& ) ) );
    connect( boardView_, SIGNAL( nodeSelected( Pos ) ), game_, SLOT( startNodePicked( const Pos& ) ) );
    connect( boardView_, SIGNAL( roadSelected( Pos, Pos ) ), game_, SLOT( startRoadPicked( const Pos&, const Pos& ) ) );
    pickStartNode();
}


void GameView::pickStartNode() {
    updatePlayer();
    boardView_->setSelectionMode( BoardView::Node );
}


void GameView::pickStartRoad( const Pos& from ) {
    boardView_->setSelectionMode( BoardView::Road );
    boardView_->from_ = from;
}


void GameView::buildPlayersSelection() {
    playersSelection_ = new QWidget;
    addWidget( playersSelection_ );
    auto l = new QHBoxLayout( playersSelection_ );
    players3_ = new QPushButton( "3 players" );
    l->addWidget( players3_ );
    players4_ = new QPushButton( "4 players" );
    l->addWidget( players4_ );

    connect( players3_, SIGNAL( clicked() ), this, SLOT( nbPlayersPicked() ) );
    connect( players4_, SIGNAL( clicked() ), this, SLOT( nbPlayersPicked() ) );
}


void GameView::nbPlayersPicked() {
    int nbPlayers = sender() == players3_ ? 3 : 4;
    for( int i = nbPlayers; i < 4; i++ ) {
        playerView_[ i ]->hide();
    }
    emit nbPlayersPicked( nbPlayers );
}


void GameView::buildGameView() {
    gameView_ = new QWidget;
    addWidget( gameView_ );
    // layout for board + game state
    auto l = new QHBoxLayout( gameView_ );
    boardView_ = new BoardView;
    boardView_->setMinimumSize( 500, 500 );
    boardView_->board_ = &game_->board_;
    l->addWidget( boardView_, 1 );
    // layout for game state
    auto vl = new QVBoxLayout();
    l->addLayout( vl );
    // layout for dice
    l = new QHBoxLayout();
    vl->addLayout( l );
    die1_ = new Die();
    die2_ = new Die();
    l->addWidget( die1_ );
    l->addWidget( die2_ );
    roll_ = new QPushButton( "Roll" );
    roll_->setEnabled( false );
    l->addWidget( roll_ );
    // players
    for( int i = 0; i < 4; i++ ) {
        auto pv = new PlayerView( &game_->player_[ i ] );
        pv->enableButtons( false );
        vl->addWidget( pv );
        playerView_.push_back( pv );
        connect( pv->pass_, SIGNAL( clicked() ), game_, SLOT( nextPlayer() ) );       
    }
    vl->addStretch();
    connect( game_, SIGNAL( updatePlayer( int ) ), this, SLOT( updatePlayer( int ) ) );
}


void GameView::updatePlayer( int player ) {
    if( player >= 0 ) {
        playerView_[ player ]->updateView();
    } else {
        for( int i = 0; i < game_->nbPlayers_; i++ ) {
            playerView_[ i ]->updateView();
        }
    }
}


void GameView::rollDice() {
    roll_->setEnabled( true );
    playerView_[ ( game_->curPlayer_ + game_->nbPlayers_ - 1 ) % game_->nbPlayers_ ]->enableButtons( false );
    updatePlayer();
}


void GameView::diceRolled( int die1, int die2 ) {
    roll_->setEnabled( false );
    playerView_[ game_->curPlayer_ ]->enableButtons( true );
    die1_->setValue( die1 );
    die2_->setValue( die2 );
    updatePlayer();
}