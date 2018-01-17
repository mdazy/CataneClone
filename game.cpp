#include "game.h"

#include <QtCore/QTimer>

#include <iostream>
using namespace std;


/**/


Player::Player( Game* game ) : game_( game ), towns_( 5 ), cities_( 4 ), roads_( 15 ) {
    resources_.resize( Hex::Desert, 0 );
}


/**/


Game::Game( QObject* parent ) :
    QObject( parent ),
    nbPlayers_( 0 ), curPlayer_( 0 ),
    pickStartAscending_( true )
{
    for( int i = 0; i < 4; i++ ) {
        player_.emplace_back( this );
        player_[ i ].number_ = i;
    }
}


bool Game::canBuildTown() const {
    const auto& p = player_[ curPlayer_ ];
    bool hasCards = p.resources_[ Hex::Wood ] > 0 && p.resources_[ Hex::Brick ] > 0 && p.resources_[ Hex::Wheat ] > 0 && p.resources_[ Hex::Sheep ] > 0;
    // TODO: check available positions on board
    return hasCards && p.towns_ > 0;
}


bool Game::canBuildCity() const {
    const auto& p = player_[ curPlayer_ ];
    bool hasCards = p.resources_[ Hex::Wheat ] > 1 && p.resources_[ Hex::Rock ] > 2;
    return hasCards && p.towns_ < 5 && p.cities_ > 0;
}


bool Game::canBuildRoad() const {
    const auto& p = player_[ curPlayer_ ];
    bool hasCards = p.resources_[ Hex::Wood ] > 0 && p.resources_[ Hex::Brick ] > 0;
    // TODO: check available positions on board
    return hasCards && p.roads_ > 0;
}


bool Game::canBuildCard() const {
    const auto& p = player_[ curPlayer_ ];
    bool hasCards = p.resources_[ Hex::Rock ] > 0 && p.resources_[ Hex::Wheat ] > 0 && p.resources_[ Hex::Sheep ] > 0;
    // TODO: check remaining dev cards
    return hasCards;
}


bool Game::canBuild() const {
    return canBuildTown() || canBuildCity() || canBuildRoad() || canBuildCard();
}


void Game::newGame() {
    emit requestNbPlayers();
}


void Game::startWithPlayers( int nbPlayers ) {
    nbPlayers_ = nbPlayers;
    curPlayer_ = 0;
    setupAllowedBuildNodes();
    emit updatePlayer();
    emit requestStartPositions();
}


void Game::startNodePicked( const Pos& np ) {
    auto& n = board_.node_[ np.y() ][ np.x() ];
    n.player_ = curPlayer_;
    n.type_ = Node::Town;
    player_[ curPlayer_ ].towns_--;
    if( !pickStartAscending_ ) {
        for( const auto& hp : Board::hexesAroundNode( np )  ) {
            auto hx = hp.first;
            auto hy = hp.second;
            if( !hp.valid() || hx >= board_.hexWidth() || hy >= board_.hexHeight() ) {
                // out of bounds
                continue;
            }
            const auto& h = board_.hex_[ hy ][ hx ];
            if( h.type_ > Hex::Invalid && h.type_ < Hex::Desert ) {
                player_[ curPlayer_ ].resources_[ h.type_ ]++;
            }
        }
        emit updatePlayer( curPlayer_ );
    }
    // setup allowed nodes around selected town
    board_.allowedNodes_.clear();
    for( const auto& nn : Board::nodesAroundNode( np ) ) {
        if( !nn.valid() || nn.x() >= board_.nodeWidth() || nn.y() >= board_.nodeHeight() ) {
            continue;
        }
        if( board_.landNode( nn ) && board_.node_[ nn.y() ][ nn.x() ].type_ == Node::None ) {
            board_.allowedNodes_.push_back( nn );
        }
    }
    emit requestRoad( np );
}


void Game::startRoadPicked( const Pos& from, const Pos& to ) {
    Road r;
    r.player_ = curPlayer_;
    r.from_ = from;
    r.to_ = to;
    board_.road_.push_back( r );
    player_[ curPlayer_ ].roads_--;
    emit updatePlayer( curPlayer_ );

    if( pickStartAscending_ ) {
        if( curPlayer_ == nbPlayers_ - 1 ) {
            pickStartAscending_ = false;
        } else {
            curPlayer_++;
        }
    } else {
        if( curPlayer_ == 0 ) {
            board_.allowedNodes_.clear();
            // start game
            emit rollDice();
            return;
        }
        curPlayer_--;
    }
    setupAllowedBuildNodes();
    emit requestNode();
}


void Game::setupAllowedBuildNodes() {
    // setup allowed nodes on land
    board_.allowedNodes_.clear();
    for( int ny = 0; ny < board_.nodeHeight(); ny++ ) {
        for( int nx = 0; nx < board_.nodeWidth(); nx++ ) {
            Pos np( nx, ny );
            if( !board_.landNode( np ) ) {
                continue;
            }
            const auto& n = board_.node_[ ny ][ nx ];
            if( n.type_ != Node::None ) {
                // already built
                continue;
            }
            bool hasNeighbor = false;
            for( const auto& nn : Board::nodesAroundNode( np ) ) {
                if( !nn.valid() || nn.x() >= board_.nodeWidth() || nn.y() >= board_.nodeHeight() ) {
                    continue;
                }
                if( board_.node_[ nn.y() ][ nn.x() ].type_ != Node::None ) {
                    hasNeighbor = true;
                    break;
                }
            }
            if( !hasNeighbor ) {
                board_.allowedNodes_.push_back( np );
            }
        }
    }
}


void Game::playTurn() {
    int dice1 = 1 + rand() % 6;
    int dice2 = 1 + rand() % 6;
    int number = dice1 + dice2;

    for( int hx = 0; hx < board_.hexWidth(); hx++ ) {
        for( int hy = 0; hy < board_.hexHeight(); hy++ ) {
            const auto& h = board_.hex_[ hy ][ hx ];
            if( h.number_ != number ) {
                continue;
            }
            Pos curP( hx, hy );
            if( curP == board_.robber_ ) {
                continue;
            }
            for( const auto& p : Board::nodesAroundHex( curP ) ) {
                const auto& n = board_.node_[ p.y() ][ p.x() ];
                if( n.type_ != Node::None ) {
                    player_[ n.player_ ].resources_[ h.type_ ]++;
                }
            }
        }
    }
    emit diceRolled( dice1, dice2 );
}


void Game::nextPlayer() {
    curPlayer_ = ( curPlayer_ + 1 ) % nbPlayers_;
    emit rollDice();
}