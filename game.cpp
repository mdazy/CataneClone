#include "game.h"

#include <QtCore/QTimer>

#include <fstream>

using namespace std;


/**/


Player::Player( Game* game ) : game_( game ), towns_( 5 ), cities_( 4 ), roads_( 15 ), state_( Waiting ) {
    resources_.resize( Hex::Desert, 0 );
}


int Player::robCard() {
    int nbCards = 0;
    for( int r : resources_ ) {
        nbCards += r;
    }
    if( nbCards == 0 ) {
        return -1;
    }
    int n = 1 + rand() % nbCards;
    for( int i = 0; i < resources_.size(); i++ ) {
        nbCards -= resources_[ i ];
        if( n > nbCards ) {
            resources_[ i ]--;
            return i;
        }
    }
    // should not reach here
    return -1;
}


ostream& operator <<( ostream& out, const Player& p ) {
    out << "# Player" << endl;
    out << p.number_ << " ";
    for( int i = 0; i < Hex::Desert; i++ ) {
        out << p.resources_[ i ] << " ";
    }
    out << p.towns_ << " " << p.cities_ << " " << p.roads_;
    return out;
}


istream& operator >>( istream& in, Player& p ) {
    string dummy;
    getline( in , dummy );
    in >> p.number_;
    for( int i = 0; i < Hex::Desert; i++ ) {
        in >> p.resources_[ i ];
    }
    in >> p.towns_ >> p.cities_ >> p.roads_;
    // TODO
    p.state_ = Player::Waiting;
    return in;
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


bool Game::canBuildTown() {
    const auto& p = curPlayer();
    bool hasCards = p.resources_[ Hex::Wood ] > 0 && p.resources_[ Hex::Brick ] > 0 && p.resources_[ Hex::Wheat ] > 0 && p.resources_[ Hex::Sheep ] > 0;
    setupAllowedBuildNodes();
    int nbNodes = board_.allowedNodes_.size();
    board_.allowedNodes_.clear();
    return hasCards && p.towns_ > 0 && nbNodes > 0;
}


bool Game::canBuildCity() const {
    const auto& p = curPlayer();
    bool hasCards = p.resources_[ Hex::Wheat ] > 1 && p.resources_[ Hex::Rock ] > 2;
    return hasCards && p.towns_ < 5 && p.cities_ > 0;
}


bool Game::canBuildRoad() {
    const auto& p = curPlayer();
    bool hasCards = p.resources_[ Hex::Wood ] > 0 && p.resources_[ Hex::Brick ] > 0;
    setupAllowedRoadStartNodes();
    int nbNodes = board_.allowedNodes_.size();
    board_.allowedNodes_.clear();
    return hasCards && p.roads_ > 0 && nbNodes > 0;
}


bool Game::canBuildCard() const {
    const auto& p = curPlayer();
    bool hasCards = p.resources_[ Hex::Rock ] > 0 && p.resources_[ Hex::Wheat ] > 0 && p.resources_[ Hex::Sheep ] > 0;
    // TODO: check remaining dev cards
    return hasCards;
}


void Game::newGame() {
    emit requestNbPlayers();
}


void Game::startWithPlayers( int nbPlayers ) {
    nbPlayers_ = nbPlayers;
    curPlayer_ = 0;
    curPlayer().state_ = Player::PickStartTown;
    setupAllowedBuildNodes( true );
    emit updatePlayer();
    emit requestStartPositions();
}


void Game::startNodePicked( const Pos& np ) {
    auto& n = board_.node_[ np.y() ][ np.x() ];
    n.player_ = curPlayer_;
    n.type_ = Node::Town;
    curPlayer().towns_--;
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
                curPlayer().resources_[ h.type_ ]++;
            }
        }
        emit updatePlayer( curPlayer_ );
    }
    // setup allowed nodes around selected town
    player_[ curPlayer_].state_ = Player::PickStartRoad;
    setupAllowedRoadEndNodes( np );
    emit requestRoad( np );
}


void Game::startRoadPicked( const Pos& from, const Pos& to ) {
    board_.road_.emplace_back( curPlayer_, from, to );
    curPlayer().roads_--;
    curPlayer().state_ = Player::Waiting;
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
    curPlayer().state_ = Player::PickStartTown;
    setupAllowedBuildNodes( true );
    emit requestNode();
}


void Game::setupAllowedBuildNodes( bool start ) {
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
            // look for neighbord and owned roads leading to this node
            bool hasNeighbor = false;
            bool hasRoad = false;
            for( const auto& nn : Board::nodesAroundNode( np ) ) {
                if( !nn.valid() || nn.x() >= board_.nodeWidth() || nn.y() >= board_.nodeHeight() ) {
                    continue;
                }
                hasNeighbor |= board_.node_[ nn.y() ][ nn.x() ].type_ != Node::None;
                hasRoad |= board_.roadExists( np, nn, curPlayer_ );
            }
            // cannot build if node has an immediate neighbor
            if( hasNeighbor ) {
                continue;
            }
            // after initial placement, cannot build if there is no owned road leading to this node
            if( !start && !hasRoad ) {
                continue;
            }
            board_.allowedNodes_.push_back( np );
        }
    }
}


void Game::setupAllowedRoadStartNodes() {
    board_.allowedNodes_.clear();
    for( int ny = 0; ny < board_.nodeHeight(); ny++ ) {
        for( int nx = 0; nx < board_.nodeWidth(); nx++ ) {
            Pos np( nx, ny );
            if( !board_.landNode( np ) ) {
                continue;
            }
            // is there a construction from player on this node?
            bool hasConstruction = board_.node_[ np.y() ][ np.x() ].type_ != Node::None && board_.node_[ np.y() ][ np.x() ].player_ == curPlayer_;
            // inspect neighbor nodes for owned, connected roads and free target nodes
            bool hasConnectedRoad = false;
            bool hasFreeNeighbor = false;
            for( const auto& nn : Board::nodesAroundNode( np ) ) {
                if( !nn.valid() || nn.x() >= board_.nodeWidth() || nn.y() >= board_.nodeHeight() || !board_.landNode( nn ) ) {
                    continue;
                }
                hasFreeNeighbor |= board_.node_[ nn.y() ][ nn.x() ].type_ == Node::None && !board_.roadExists( np, nn );
                hasConnectedRoad |= board_.roadExists( np, nn, curPlayer_ );
            }
            if( hasFreeNeighbor && ( hasConstruction || hasConnectedRoad ) ) {
                board_.allowedNodes_.push_back( np );
            }
        }
    }
}


void Game::setupAllowedRoadEndNodes( const Pos& from ) {
    board_.allowedNodes_.clear();
    for( const auto& nn : Board::nodesAroundNode( from ) ) {
        if( !nn.valid() || nn.x() >= board_.nodeWidth() || nn.y() >= board_.nodeHeight() ) {
            continue;
        }
        if( board_.landNode( nn ) && board_.node_[ nn.y() ][ nn.x() ].type_ == Node::None && !board_.roadExists( from, nn ) ) {
            board_.allowedNodes_.push_back( nn );
        }
    }
}


void Game::setupAllowedCityNodes() {
    board_.allowedNodes_.clear();
    for( int ny = 0; ny < board_.nodeHeight(); ny++ ) {
        for( int nx = 0; nx < board_.nodeWidth(); nx++ ) {
            Pos np( nx, ny );
            if( !board_.landNode( np ) ) {
                continue;
            }
            auto& n = board_.node_[ ny ][ nx ];
            if( n.type_ == Node::Town && n.player_ == curPlayer_ ) {
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
                if( n.type_ == Node::Town ) {
                    player_[ n.player_ ].resources_[ h.type_ ]++;
                } else if( n.type_ == Node::City ) {
                    player_[ n.player_ ].resources_[ h.type_ ] += 2;
                }

            }
        }
    }
    emit diceRolled( dice1, dice2 );
    if( number == 7 ) {
        rob();
    }
}


void Game::nextPlayer() {
    curPlayer_ = ( curPlayer_ + 1 ) % nbPlayers_;
    emit rollDice();
}


void Game::buildRoad() {
    setupAllowedRoadStartNodes();
    curPlayer().state_ = Player::PickBuildRoad;
    emit requestRoad();
}


void Game::buildRoad( const Pos& from, const Pos& to ) {
    auto& p = curPlayer();
    p.resources_[ Hex::Brick ]--;
    p.resources_[ Hex::Wood ]--;
    p.roads_--;
    board_.road_.emplace_back( curPlayer_, from, to );
    curPlayer().state_ = Player::Waiting;
    emit updatePlayer( curPlayer_ );
}


void Game::buildTown() {
    setupAllowedBuildNodes();
    curPlayer().state_ = Player::PickBuildTown;
    emit requestNode();
}


void Game::buildTown( const Pos& np ) {
    auto& p = curPlayer();
    p.resources_[ Hex::Brick ]--;
    p.resources_[ Hex::Wheat ]--;
    p.resources_[ Hex::Wood ]--;
    p.resources_[ Hex::Sheep ]--;
    p.towns_--;
    auto& n = board_.node_[ np.y() ][ np.x() ];
    n.type_ = Node::Town;
    n.player_ = curPlayer_;
    curPlayer().state_ = Player::PickBuildRoad;
    emit updatePlayer( curPlayer_ );
}


void Game::buildCity() {
    setupAllowedCityNodes();
    curPlayer().state_ = Player::PickCity;
    emit requestNode();
}


void Game::buildCity( const Pos& np ) {
    auto& p = curPlayer();
    p.resources_[ Hex::Rock ] -= 3;
    p.resources_[ Hex::Wheat ] -= 2;
    p.towns_++;
    p.cities_--;
    board_.node_[ np.y() ][ np.x() ].type_ = Node::City;
    curPlayer().state_ = Player::PickBuildRoad;
    emit updatePlayer( curPlayer_ );
}


void Game::buildCard() {
}


void Game::rob() {
    curPlayer().state_ = Player::PickRobTown;
    emit requestHex();
}


void Game::robAround( const Pos& hp ) {
    board_.robber_ = hp;
    board_.allowedNodes_.clear();
    for( const auto& np : board_.nodesAroundHex( hp ) ) {
        const auto& n = board_.node_[ np.y() ][ np.x() ];
        if( n.type_ != Node::None && n.player_ != curPlayer_ ) {
            board_.allowedNodes_.push_back( np );
        }
    }
    if( board_.allowedNodes_.size() > 0 ) {
        emit requestNode();
    } else {
        curPlayer().state_ = Player::Waiting;
        emit updatePlayer( curPlayer_ );
    }
}

void Game::rob( const Pos& np ) {
    int from = board_.node_[ np.y() ][ np.x() ].player_;
    if( from != -1 ) {
        int r = player_[ from ].robCard();
        if( r != -1 ) {
            curPlayer().resources_[ r ]++;
        }
    }
    board_.allowedNodes_.clear();
    curPlayer().state_ = Player::Waiting;
    emit updatePlayer( from, false );
    emit updatePlayer( curPlayer_ );
}


void Game::load() {
    ifstream file( "game.txt" );
    if( file.good() ) {
        file >> *this;
    }
}


void Game::save() const {
    ofstream file( "game.txt" );
    if( file.good() ) {
        file << *this;
    }
}


ostream& operator <<( ostream& out, const Game& g ) {
    out << "# Game" << endl;
    out << g.nbPlayers_ << " " << g.curPlayer_ << endl;
    for( int i = 0; i < g.nbPlayers_; i++ ) {
        out << g.player_[ i ] << endl;
    }
    out << g.board_ << endl;
    return out;
}


istream& operator >>( istream& in, Game& g ) {
    string dummy;
    getline( in , dummy );
    in >> g.nbPlayers_ >> g.curPlayer_; in.ignore();
    for( int i = 0; i < g.nbPlayers_; i++ ) {
        in >> g.player_[ i ]; in.ignore();
    }
    in >> g.board_; in.ignore();
    return in;
}