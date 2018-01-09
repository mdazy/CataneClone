#include "game.h"

#include <iostream>
using namespace std;

Game::Game( QObject* parent ) :
    QObject( parent ),
    nbPlayers_( 0 ), curPlayer_( 0 ),
    pickStartAscending_( true )
{
}


void Game::newGame() {
    emit requestNbPlayers();
}


void Game::startWithPlayers( int nbPlayers ) {
    nbPlayers_ = nbPlayers;
    curPlayer_ = 0;
    emit requestStartPositions();
}


void Game::startNodePicked( unsigned int nx, unsigned int ny ) {
    auto& n = board_.node_[ ny ][ nx ];
    n.player_ = curPlayer_;
    n.type_ = Node::Town;
    emit requestRoad( nx, ny );
}


void Game::startRoadPicked( unsigned int fromX, unsigned int fromY, unsigned int toX, unsigned int toY ) {
    Road r;
    r.player_ = curPlayer_;
    r.fromX_ = fromX;
    r.fromY_ = fromY;
    r.toX_ = toX;
    r.toY_ = toY;
    board_.road_.push_back( r );

    if( pickStartAscending_ ) {
        if( curPlayer_ == nbPlayers_ - 1 ) {
            pickStartAscending_ = false;
        } else {
            curPlayer_++;
        }
    } else {
        if( curPlayer_ == 0 ) {
            // TODO: done
        } else {
            curPlayer_--;
        }
    }
    emit requestNode();
}