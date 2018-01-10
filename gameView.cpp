#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>

#include "gameView.h"
#include "game.h"
#include "boardView.h"

#include <iostream>
using namespace std;

GameView::GameView( Game* game, QWidget* parent ) :
    QStackedWidget( parent ),
    game_( game ),
    playersSelection_( 0 ), players3_( 0 ), players4_( 0 ),
    gameView_( 0 ), boardView_( 0 )
{
    connect( game, SIGNAL( requestNbPlayers() ), this, SLOT( pickNbPlayers() ) );
    connect( this, SIGNAL( nbPlayersPicked( int ) ), game, SLOT( startWithPlayers( int ) ) );

    connect( game, SIGNAL( requestStartPositions() ), this, SLOT( pickStartPositions() ) );
}


void GameView::pickNbPlayers() {
    buildPlayersSelection();
    setCurrentWidget( playersSelection_ );
}


void GameView::pickStartPositions() {
    buildGameView();
    setCurrentWidget( gameView_ );
    connect( game_, SIGNAL( requestNode() ), this, SLOT( pickStartNode() ) );
    connect( game_, SIGNAL( requestRoad( int, int ) ), this, SLOT( pickStartRoad( int, int ) ) );
    connect( boardView_, SIGNAL( nodeSelected( unsigned int, unsigned int ) ), game_, SLOT( startNodePicked( unsigned int, unsigned int ) ) );
    connect( boardView_, SIGNAL( roadSelected( unsigned int, unsigned int, unsigned int, unsigned int ) ), game_, SLOT( startRoadPicked( unsigned int, unsigned int, unsigned int, unsigned int ) ) );
    pickStartNode();
}


void GameView::pickStartNode() {
    boardView_->setSelectionMode( BoardView::Node );
}


void GameView::pickStartRoad( int fromX, int fromY ) {
    boardView_->setSelectionMode( BoardView::Road );
    boardView_->fromX_ = fromX;
    boardView_->fromY_ = fromY;
}


void GameView::buildPlayersSelection() {
    if( playersSelection_ != 0 ) {
        return;
    }
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
    if( sender() == players3_ ) {
        emit nbPlayersPicked( 3 );
    } else {
        emit nbPlayersPicked( 4 );
    }
}


void GameView::buildGameView() {
    if( gameView_ != 0 ) {
        return;
    }
    gameView_ = new QWidget;
    addWidget( gameView_ );
    auto l = new QHBoxLayout( gameView_ );
    boardView_ = new BoardView;
    boardView_->board_ = &game_->board_;
    l->addWidget( boardView_ );
}