#include "game.h"

#include <iostream>
using namespace std;


/**/


Player::Player() {
    resources_.resize( Hex::Desert, 0 );
}


/**/


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
    player_.resize( nbPlayers_ );
    curPlayer_ = 0;
    emit requestStartPositions();
}


void Game::startNodePicked( unsigned int nx, unsigned int ny ) {
    auto& n = board_.node_[ ny ][ nx ];
    n.player_ = curPlayer_;
    n.type_ = Node::Town;
    if( !pickStartAscending_ ) {
        for( const auto& hp : Board::hexesAroundNode( { nx, ny } )  ) {
            auto hx = hp.first;
            auto hy = hp.second;
            if( hx < 0 || hx >= board_.hexWidth() || hy < 0 || hy >= board_.hexHeight() ) {
                // out of bounds
                continue;
            }
            const auto& h = board_.hex_[ hy ][ hx ];
            if( h.type_ > Hex::Invalid && h.type_ < Hex::Desert ) {
                player_[ curPlayer_ ].resources_[ h.type_ ]++;
            }
        }
    }
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