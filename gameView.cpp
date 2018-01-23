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
    game_( game ), playing_( false ),
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
    connect( game_, SIGNAL( requestNode( Node::Type ) ), this, SLOT( pickNode( Node::Type ) ) );
    connect( game_, SIGNAL( requestRoad( Pos ) ), this, SLOT( pickRoad( const Pos& ) ) );
    connect( boardView_, SIGNAL( roadSelected( Pos, Pos ) ), game_, SLOT( startRoadPicked( const Pos&, const Pos& ) ) );
    pickNode( Node::None );
}


void GameView::pickNode( Node::Type type ) {
    QObject::disconnect( boardView_, &BoardView::nodeSelected, 0, 0 );
    if( playing_ ) {
        connect(
            boardView_, SIGNAL( nodeSelected( Pos ) ),
            game_, ( type == Node::Town ? SLOT( buildTown( const Pos& ) ) : SLOT( buildCity( const Pos& ) ) )
        );
    } else {
        connect( boardView_, SIGNAL( nodeSelected( Pos ) ), game_, SLOT( startNodePicked( const Pos& ) ) );
    }
    updatePlayer();
    boardView_->setSelectionMode( BoardView::Node );
    boardView_->update();
}


void GameView::pickRoad( const Pos& from ) {
    QObject::disconnect( boardView_, &BoardView::roadSelected, 0, 0 );
    QObject::disconnect( boardView_, &BoardView::nodeSelected, 0, 0 );
    if( playing_ ) {
        connect( boardView_, SIGNAL( nodeSelected( Pos ) ), game_, SLOT( setupAllowedRoadEndNodes( const Pos& ) ) );
        connect( boardView_, SIGNAL( roadSelected( Pos, Pos ) ), game_, SLOT( buildRoad( const Pos&, const Pos& ) ) );
    } else {
        connect( boardView_, SIGNAL( roadSelected( Pos, Pos ) ), game_, SLOT( startRoadPicked( const Pos&, const Pos& ) ) );
    }
    boardView_->setSelectionMode( BoardView::Road );
    boardView_->from_ = from;
    boardView_->update();
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
    // load/save game state
    l = new QHBoxLayout();
    vl->addLayout( l );
    auto p = new QPushButton( "Save state" );
    l->addWidget( p );
    connect( p, SIGNAL( clicked() ), game_, SLOT( save() ) );
    p = new QPushButton( "Load state" );
    l->addWidget( p );
    connect( p, SIGNAL( clicked() ), game_, SLOT( load() ) );
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
        connect( pv->buildRoad_, SIGNAL( clicked() ), game_, SLOT( buildRoad() ) );
        connect( pv->buildTown_, SIGNAL( clicked() ), game_, SLOT( buildTown() ) );
        connect( pv->buildCity_, SIGNAL( clicked() ), game_, SLOT( buildCity() ) );
        connect( pv->buildCard_, SIGNAL( clicked() ), game_, SLOT( buildCard() ) );
        connect( pv->pass_, SIGNAL( clicked() ), game_, SLOT( nextPlayer() ) );       
    }
    vl->addStretch();
    connect( game_, SIGNAL( updatePlayer( int ) ), this, SLOT( updatePlayer( int ) ) );
}


void GameView::updatePlayer( int player ) {
    if( player >= 0 ) {
        playerView_[ player ]->updateView();
        playerView_[ player ]->enableButtons( true );
    } else {
        for( int i = 0; i < game_->nbPlayers_; i++ ) {
            playerView_[ i ]->updateView();
        }
    }
}


void GameView::rollDice() {
    playing_ = true;
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
