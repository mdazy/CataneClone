#include "game.h"

#include <QtCore/QTimer>

#include <fstream>
#include <algorithm>

using namespace std;


/**/


QString cardName( DevCard card ) {
    switch( card ) {
        case Point : {
            return "point";
        }
        case Knight : {
            return "knight";
        }
        case Invention : {
            return "invention";
        }
        case Roads : { 
            return "roads";
        }
        case Monopoly : {
            return "monopoly";
        }
    }
    return "unknown";
}


/**/


Player::Player( Game* game ) : game_( game ), towns_( 5 ), cities_( 4 ), roads_( 15 ), state_( Waiting ),
    armySize_( 0 ), longestRoad_( false ), largestArmy_( false ) {
    resources_.resize( Hex::Desert, 0 );
    devCards_.resize( Invention + 1, 0 );
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


int Player::score() const {
    return ( 5 - towns_ ) + ( 4 - cities_) * 2 + ( longestRoad_ ? 2 : 0 ) + ( largestArmy_ ? 2 : 0 ) + devCards_[ Point ];
}


int Player::nbResourceCards() const {
    return accumulate( resources_.begin(), resources_.end(), 0 );
}


bool Player::canPlayKnight() const {
    return !devCardPlayed_ && devCards_[ Knight ] > ( builtCard_ == Knight ? 1 : 0 );
}


bool Player::canPlayRoads() const {
    return !devCardPlayed_ && state_ == Waiting && devCards_[ Roads ] > ( builtCard_ == Roads ? 1 : 0 );
}


bool Player::canPlayInvention() const {
    return !devCardPlayed_ && state_ == Waiting && devCards_[ Invention ] > ( builtCard_ == Invention ? 1 : 0 );
}


bool Player::canPlayMonopoly() const {
    return !devCardPlayed_ && state_ == Waiting && devCards_[ Monopoly ] > ( builtCard_ == Monopoly ? 1 : 0 );
}


ostream& operator <<( ostream& out, const Player& p ) {
    out << "# Player" << endl;
    out << p.number_ << " ";
    for( int i = 0; i < Hex::Desert; i++ ) {
        out << p.resources_[ i ] << " ";
    }
    out << p.towns_ << " " << p.cities_ << " " << p.roads_;
    // TODO: restore state
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
    // TODO : save state
    p.state_ = Player::Waiting;
    return in;
}


/**/


Game::Game( QObject* parent ) :
    QObject( parent ),
    nbPlayers_( 0 ), curPlayer_( 0 ),
    pickStartAscending_( true ), nextPlayerState_( Player::Waiting )
{
    for( int i = 0; i < 4; i++ ) {
        player_.emplace_back( this );
        player_[ i ].number_ = i;
    }

    devCards_.insert( devCards_.begin(), 14, Knight );
    devCards_.insert( devCards_.begin(), 5, Point );
    devCards_.insert( devCards_.begin(), 2, Roads );
    devCards_.insert( devCards_.begin(), 2, Monopoly );
    devCards_.insert( devCards_.begin(), 2, Invention );
    randomize( devCards_ );
}


bool Game::canBuildTown() {
    const auto& p = curPlayer();
    bool hasCards = p.resources_[ Hex::Wood ] > 0 && p.resources_[ Hex::Brick ] > 0 && p.resources_[ Hex::Wheat ] > 0 && p.resources_[ Hex::Sheep ] > 0;
    setupAllowedBuildNodes();
    int nbNodes = board_.allowedNodes_.size();
    board_.allowedNodes_.clear();
    return hasCards && p.state_ == Player::Waiting && p.towns_ > 0 && nbNodes > 0;
}


bool Game::canBuildCity() const {
    const auto& p = curPlayer();
    bool hasCards = p.resources_[ Hex::Wheat ] > 1 && p.resources_[ Hex::Rock ] > 2;
    return hasCards && p.state_ == Player::Waiting && p.towns_ < 5 && p.cities_ > 0;
}


bool Game::canBuildRoad() {
    const auto& p = curPlayer();
    bool hasCards = p.resources_[ Hex::Wood ] > 0 && p.resources_[ Hex::Brick ] > 0;
    setupAllowedRoadStartNodes();
    int nbNodes = board_.allowedNodes_.size();
    board_.allowedNodes_.clear();
    return hasCards && p.state_ == Player::Waiting && p.roads_ > 0 && nbNodes > 0;
}


bool Game::canBuildCard() const {
    const auto& p = curPlayer();
    bool hasCards = p.resources_[ Hex::Rock ] > 0 && p.resources_[ Hex::Wheat ] > 0 && p.resources_[ Hex::Sheep ] > 0;
    return hasCards && p.state_ == Player::Waiting && devCards_.size() > 0;
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
            curPlayer_--;
            nextPlayer();
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
    } else {
        curPlayer().state_ = Player::Waiting;
        updatePlayer( curPlayer_ );
    }
}


void Game::nextPlayer() {
    curPlayer_ = ( curPlayer_ + 1 ) % nbPlayers_;
    auto& p = curPlayer();
    p.state_ = Player::AboutToRoll;
    p.devCardPlayed_ = false;
    p.builtCard_ = -1;
    roadCost_ = 1;
    nbRoadsToBuild_ = 1;
    updatePlayer( curPlayer_ );
    emit rollDice();
}


void Game::buildRoad() {
    setupAllowedRoadStartNodes();
    curPlayer().state_ = Player::PickBuildRoad;
    emit requestRoad();
}


void Game::buildRoad( const Pos& from, const Pos& to ) {
    // TODO: longest road
    auto& p = curPlayer();
    p.resources_[ Hex::Brick ] -= roadCost_;
    p.resources_[ Hex::Wood ] -= roadCost_;
    p.roads_--;
    board_.road_.emplace_back( curPlayer_, from, to );
    curPlayer().state_ = Player::Waiting;
    emit updatePlayer( curPlayer_ );
    nbRoadsToBuild_--;
    if( nbRoadsToBuild_ > 0 ) {
        buildRoad();
    } else {
        nbRoadsToBuild_ = 1;
    }
}


void Game::buildRoads() {
    curPlayer().devCardPlayed_ = true;
    curPlayer().devCards_[ Roads ]--;
    roadCost_ = 0;
    nbRoadsToBuild_ = min( curPlayer().roads_, 2 );
    buildRoad();
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
    curPlayer().state_ = Player::Waiting;
    emit updatePlayer( curPlayer_ );
    checkEndGame();
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
    curPlayer().state_ = Player::Waiting;
    emit updatePlayer( curPlayer_ );
    checkEndGame();
}


void Game::buildCard() {
    auto& p = curPlayer();
    p.builtCard_ = devCards_.back();
    devCards_.pop_back();
    p.devCards_[ p.builtCard_ ]++;
    p.resources_[ Hex::Rock ]--;
    p.resources_[ Hex::Wheat ]--;
    p.resources_[ Hex::Sheep ]--;
    emit updatePlayer( curPlayer_ );
    checkEndGame();
}


void Game::knight() {
    curPlayer().devCardPlayed_ = true;
    curPlayer().devCards_[ Knight ]--;
    curPlayer().armySize_++;
    nextPlayerState_ = curPlayer().state_;
    moveRobber();
    int largestArmy = 2;
    int largestArmyPlayer = -1;
    for( int i = 0; i < nbPlayers_; i++ ) {
        if( player_[ i ].largestArmy_ ) {
            largestArmy = player_[ i ].armySize_;
            largestArmyPlayer = i;
            break;
        }
    }
    if( curPlayer().armySize_ > largestArmy ) {
        if( largestArmyPlayer != -1 ) {
            player_[ largestArmyPlayer ].largestArmy_ = false;
            emit updatePlayer( largestArmyPlayer, false );
        }
        curPlayer().largestArmy_ = true;
        emit updatePlayer( curPlayer_ );
    }
    checkEndGame();
}


void Game::moveRobber() {
    curPlayer().state_ = Player::PickRobTown;
    emit requestHex();
}

void Game::rob() {
    for( int i = 0; i < nbPlayers_; i++ ) {
        if( player_[ i ].nbResourceCards() > 7 ) {
            emit( pickDiscard( &player_ [ i ] ) );
        }
    }
    nextPlayerState_ = Player::Waiting;
    moveRobber();
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
        curPlayer().state_ = nextPlayerState_;
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
    curPlayer().state_ = nextPlayerState_;
    emit updatePlayer( from, false );
    emit updatePlayer( curPlayer_ );
}


void Game::discard( Player* p, vector<int> selection ) {
    for( int i = 0; i < Hex::Desert; i++ ) {
        p->resources_[ i ] -= selection[ i ];
    }
    emit updatePlayer( p->number_, p->number_ == curPlayer_ );
}


void Game::invention( vector<int> selection ) {
    curPlayer().devCardPlayed_ = true;
    curPlayer().devCards_[ Invention ]--;
    for( int i = 0; i < Hex::Desert; i++ ) {
        curPlayer().resources_[ i ] += selection[ i ];
    }
    emit updatePlayer( curPlayer_ );    
}


void Game::monopoly( vector<int> selection ) {
    curPlayer().devCardPlayed_ = true;
    curPlayer().devCards_[ Monopoly ]--;
    for( int i = 0; i < Hex::Desert; i++ ) {
        if( selection[ i ] > 0 ) {
            for( auto& p : player_ ) {
                if( p.number_ != curPlayer_ ) {
                    curPlayer().resources_[ i ] += p.resources_[ i ];
                    p.resources_[ i ] = 0;
                }
            }
            break;
        }
    }
    emit updatePlayer( -1 , false );
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


void Game::checkEndGame() {
    if( curPlayer().score() < 10 ) {
        return;
    }
    vector<Player> players = player_;
    sort(
        players.begin(), players.end(),
        []( const Player& p1, const Player& p2 ) { return p1.score() > p2.score(); }
    );
    emit gameOver( players );
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