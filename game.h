#pragma once

#include <QtCore/QObject>

#include <iostream>

#include "board.h"

class Game;
class Messenger;


/**/


enum DevCard { Knight = 0, Point, Roads, Monopoly, Invention };

QString cardName( DevCard card );


/**/


class Player {
public:
    enum State { Waiting, PickStartTown, PickStartRoad, PickBuildRoad, PickBuildTown, PickCity, PickRobTown, AboutToRoll };
    Player( Game* game );

    int robCard();
    int nbResourceCards() const;
    int score() const;

    bool canPlayKnight() const;
    bool canPlayRoads() const;
    bool canPlayInvention() const;
    bool canPlayMonopoly() const;

    int costOf( Hex::Type resourceType ) const;
    std::vector<int> cardCosts() const;

public:
    Game* game_;
    int number_;
    std::vector<int> resources_;
    std::vector<int> devCards_;
    int towns_;
    int cities_;
    int roads_;
    State state_;
    bool devCardPlayed_;
    bool built_;
    int builtCard_;
    int armySize_;
    int roadLength_;
    bool longestRoad_;
    bool largestArmy_;
};

std::ostream& operator <<( std::ostream& out, const Player& p );
std::istream& operator >>( std::istream& in, Player& p );


/**/


class Game : public QObject {
    Q_OBJECT;
public:
    Game( Messenger* messenger );

    bool canBuildTown();
    bool canBuildCity() const;
    bool canBuildRoad();
    bool canBuildCard() const;
    bool canTrade() const;

    Player& curPlayer() { return player_[ curPlayer_ ]; }
    const Player& curPlayer() const { return player_[ curPlayer_ ]; }

public slots:
    void newGame();
    void startWithPlayers( int nbPlayers );
    void startNodePicked( const Pos& np );
    void startRoadPicked( const Pos& from, const Pos& to );
    void setupAllowedRoadEndNodes( const Pos& from );
    void playTurn();
    void nextPlayer();
    void buildRoad();
    void buildRoad( const Pos& from, const Pos& to );
    void buildRoads();
    void buildTown();
    void buildTown( const Pos& np );
    void buildCity();
    void buildCity( const Pos& np );
    void buildCard();
    void knight();
    void moveRobber();
    void rob( const Pos& np );
    void robAround( const Pos& hp );
    void discard( Player* p, std::vector<int> selection );
    void invention( std::vector<int> selection );
    void monopoly( std::vector<int> selection );
    void startTrade();
    void trade( const std::vector<int>& sold, const std::vector<int>& bought );

    void load();
    void save() const;

signals:
    void requestNbPlayers();
    void requestStartPositions();
    void requestNode();
    void requestRoad( Pos from = Pos() );
    void requestHex();
    void requestTrade( Player* p );
    void updatePlayer( int player = - 1, bool buttons = true );
    void rollDice();
    void diceRolled( int die1, int die2 );
    void pickDiscard( Player* p );
    void gameOver( std::vector<Player> players );

protected:
    void setupAllowedBuildNodes( bool start = false );
    void setupAllowedRoadStartNodes();
    void setupAllowedCityNodes();
    void rob();
    void updateLargestArmy();
    void updateLongestRoadForPlayer( int p );
    void updateLongestRoad();
    void checkEndGame();

public:
    Board board_;
    std::vector<Player> player_;
    std::vector<DevCard> devCards_;
    int nbPlayers_;
    int curPlayer_;
    int nbRoadsToBuild_;
    int roadCost_;
    bool pickStartAscending_;
    Player::State nextPlayerState_;

private:
    Messenger* messenger_;
};

std::ostream& operator <<( std::ostream& out, const Game& g );
std::istream& operator >>( std::istream& in, Game& g );