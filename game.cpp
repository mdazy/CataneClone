#include "game.h"

#include <QtCore/QTimer>

#include <fstream>
#include <algorithm>

#include "messenger.h"

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
    armySize_( 0 ), roadLength_( 0 ), longestRoad_( false ), largestArmy_( false ) {
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


int Player::costOf( Hex::Type resourceType ) const {
    int cost = 4;
    for( const auto& row : game_->board_.node_ ) {
        for( const auto& n : row ) {
            if( n.player_ != number_ ) {
                continue;
            }
            if( n.harborType_ == resourceType ) {
                cost = min( cost , 2 );
            } else if( n.harborType_ == Hex::Any ) {
                cost = min( cost, 3 );
            }
        }
    }
    return cost;
}


std::vector<int> Player::cardCosts() const {
    std::vector<int> costs;
    for( int i = 0; i < resources_.size(); i++ ) {
        costs.push_back( costOf( Hex::Type( i ) ) );
    }
    return costs;
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


Game::Game( Messenger* messenger, bool server ) :
    QObject( Q_NULLPTR ),
    nbPlayers_( 0 ), curPlayer_( 0 ),
    pickStartAscending_( true ), nextPlayerState_( Player::Waiting ),
    messenger_( messenger ), server_( server )
{
    for( int i = 0; i < 4; i++ ) {
        player_.emplace_back( this );
        player_[ i ].number_ = i;
    }

    if( !server_ ) {
        connect( messenger_, &Messenger::gameCommand, this, &Game::handleCommand );
    }

    // TODO: get cards from server
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


bool Game::canTrade() const {
    const auto& p =curPlayer();
    return !p.built_ && p.state_ == Player::Waiting;
}


void Game::newGame() {
    if( server_ ) {
        emit requestNbPlayers();
    }
}


void Game::handleCommand( const QString& cmd ) {
    QStringList parts = cmd.split( "/" );
    if( parts[ 0 ] == "board" ) {
        parts.removeFirst();
        board_.initFromStrings( parts );
    } else if( parts[ 0 ] == "nbPlayers" ) {
        QTimer::singleShot( 0, this, [ this, parts ](){ startWithPlayers( parts[ 1 ].toInt() ); } );
    } else if( parts[ 0 ] == "startNode" ) {
        QTimer::singleShot( 0, this, [ this, parts ](){ startNodePicked( Pos(parts[ 1 ].toInt(), parts[ 2 ].toInt() ) ); } );
    } else if( parts[ 0 ] == "startRoad" ) {
        QTimer::singleShot( 0, this,
            [ this, parts ](){
                startRoadPicked(
                    Pos(parts[ 1 ].toInt(), parts[ 2 ].toInt() ),
                    Pos(parts[ 3 ].toInt(), parts[ 4 ].toInt() )
                );
            } 
        );
    }
}


void Game::startWithPlayers( int nbPlayers ) {
    if( server_ ) {
        messenger_->sendGameCommand( "board/" + board_.toString() );
        messenger_->sendGameCommand( QString( "nbPlayers/%1" ).arg( nbPlayers ) );
    }
    nbPlayers_ = nbPlayers;
    curPlayer_ = 0;
    curPlayer().state_ = Player::PickStartTown;
    setupAllowedBuildNodes( true );
    emit updatePlayer();
    if( server_ ) {
        emit requestStartPositions();
    } else {
        emit updateBoard();
    }
}


void Game::startNodePicked( const Pos& np ) {
    if( server_ ) {
        messenger_->sendGameCommand( QString( "startNode/%1/%2" ).arg( np.x() ).arg( np.y() ) );
    }
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
    if( server_ ) {
        setupAllowedRoadEndNodes( np );
        emit requestRoad( np );
    } else {
        emit updateBoard();
    }
}


void Game::startRoadPicked( const Pos& from, const Pos& to ) {
    if( server_ ) {
        messenger_->sendGameCommand(
            QString( "startRoad/%1/%2/%3/%4" )
                .arg( from.x() ).arg( from.y() )
                .arg( to.x() ).arg( to.y() )
        );
    }
    board_.road_.emplace_back( curPlayer_, from, to );
    curPlayer().roads_--;
    updateLongestRoad();
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
    if( server_ ) {
        setupAllowedBuildNodes( true );
        emit requestNode();
    } else {
        emit updateBoard();
    }
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
            // look for neighbors and owned roads leading to this node
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
            const auto& cur = board_.node_[ np.y() ][ np.x() ];
            bool hasConstruction = cur.type_ != Node::None && cur.player_ == curPlayer_;
            // inspect neighbor nodes for owned, connected roads and free target nodes
            bool hasConnectedRoad = false;
            bool hasFreeNeighbor = false;
            for( const auto& nn : Board::nodesAroundNode( np ) ) {
                if( !nn.valid() || nn.x() >= board_.nodeWidth() || nn.y() >= board_.nodeHeight() || !board_.landNode( nn ) ) {
                    continue;
                }
                const auto& neighbor = board_.node_[ nn.y() ][ nn.x() ];
                hasFreeNeighbor |=  neighbor.type_ == Node::None && !board_.roadExists( np, nn );
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
        const auto& neighbor = board_.node_[ nn.y() ][ nn.x() ];
        if( board_.landNode( nn ) && !board_.roadExists( from, nn ) ) {
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
    p.built_ = false;
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
    auto& p = curPlayer();
    p.resources_[ Hex::Brick ] -= roadCost_;
    p.resources_[ Hex::Wood ] -= roadCost_;
    p.roads_--;
    p.built_ = true;
    board_.road_.emplace_back( curPlayer_, from, to );
    curPlayer().state_ = Player::Waiting;
    emit updatePlayer( curPlayer_ );
    nbRoadsToBuild_--;
    if( nbRoadsToBuild_ > 0 ) {
        buildRoad();
    } else {
        nbRoadsToBuild_ = 1;
    }
    updateLongestRoad();
    checkEndGame();
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
    p.built_ = true;
    auto& n = board_.node_[ np.y() ][ np.x() ];
    n.type_ = Node::Town;
    n.player_ = curPlayer_;
    curPlayer().state_ = Player::Waiting;
    emit updatePlayer( curPlayer_ );
    updateLongestRoad();
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
    p.built_ = true;
    board_.node_[ np.y() ][ np.x() ].type_ = Node::City;
    curPlayer().state_ = Player::Waiting;
    emit updatePlayer( curPlayer_ );
    updateLongestRoad();
    checkEndGame();
}


void Game::buildCard() {
    auto& p = curPlayer();
    p.builtCard_ = devCards_.back();
    p.built_ = true;
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
    updateLargestArmy();
    checkEndGame();
}


void Game::updateLargestArmy() {
    // get current leader and largest army size
    int largestArmy = 2;
    int largestArmyPlayer = -1;
    for( int i = 0; i < nbPlayers_; i++ ) {
        if( player_[ i ].largestArmy_ ) {
            largestArmy = player_[ i ].armySize_;
            largestArmyPlayer = i;
            break;
        }
    }
    // check if the current player's army is larger
    if( curPlayer().armySize_ > largestArmy ) {
        if( largestArmyPlayer != -1 ) {
            player_[ largestArmyPlayer ].largestArmy_ = false;
            emit updatePlayer( largestArmyPlayer, false );
        }
        curPlayer().largestArmy_ = true;
        emit updatePlayer( curPlayer_ );
    }
}


void Game::updateLongestRoad() {
    //cerr << "updateLongestRoad" << endl;
    // get current leader and longest road
    int longestRoad = 4;
    int longestRoadPlayer = -1;
    for( int i = 0; i < nbPlayers_; i++ ) {
        if( player_[ i ].longestRoad_ ) {
            longestRoad = player_[ i ].roadLength_;
            longestRoadPlayer = i;
            break;
        }
    }
    //cerr << "current leader is " << longestRoadPlayer << " with " << longestRoad << " roads" << endl;

    for( int i = 0; i < nbPlayers_; i++ ) {
        player_[ i ].roadLength_ = board_.longestRoadForPlayer( i );
        emit( updatePlayer( i, false ) );
    }

    // the longest road can be cut by new towns or cities of opponents, check
    // that the leader's longest road did not decrease
    bool interrupted = false;
    int firstP = longestRoadPlayer < 0 ? 0 : longestRoadPlayer;
    if( longestRoadPlayer != -1 && player_[ longestRoadPlayer ].roadLength_ < longestRoad ) {
        //cerr << "interrupted" << endl;
        interrupted = true;
        longestRoad = 4;
        player_[ longestRoadPlayer ].longestRoad_ = false;
        longestRoadPlayer = -1;
    }

    // update leader - always start with (possibly previous) leader so he retains
    // priority in case of equality after his longest road was cut
    int curP = firstP;
    bool multiple = false;
    do {
        if( player_[ curP ].roadLength_ > longestRoad ) {
            if( longestRoadPlayer != -1 ) {
                player_[ longestRoadPlayer ].longestRoad_ = false;
            }
            longestRoad = player_[ curP ].roadLength_;
            //cerr << "new leader " << curP << " with " << longestRoad << " roads" << endl;
            longestRoadPlayer = curP;
            player_[ longestRoadPlayer ].longestRoad_ = true;
            multiple = false;
        } else if( player_[ curP ].roadLength_ == longestRoad && longestRoadPlayer != -1 && interrupted ) {
            // several players with the same longest road after leader was cut
            //cerr << "multiple leaders with " << longestRoad << " roads after interruption" << endl;
            multiple = true;
        }
        curP = ( curP + 1 ) % nbPlayers_;
    } while( curP != firstP );
    if( multiple ) {
        // several players with the same longest road after leader was cut
        // nobody gets the longest road
        player_[ longestRoadPlayer ].longestRoad_ = false;
    }

    for( int i = 0; i < nbPlayers_; i++ ) {
        emit( updatePlayer( i, false ) );
    }
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


void Game::startTrade() {
    emit requestTrade( &curPlayer() );
}


void Game::trade( const std::vector<int>& sold, const std::vector<int>& bought ) {
    auto& p = player_[ curPlayer_ ];
    for( int i = 0; i < p.resources_.size(); i++ ) {
        p.resources_[ i ] += bought[ i ] - sold[ i ];
    }
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