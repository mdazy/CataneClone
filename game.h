#pragma once

#include <QtCore/QObject>

#include <iostream>

#include "board.h"

class Game;


/**/


class Player {
public:
    enum State { Waiting, PickStartTown, PickStartRoad, PickBuildRoad, PickBuildTown, PickCity, PickRobTown };
    Player( Game* game );

    int robCard();

public:
    Game* game_;
    int number_;
    std::vector<int> resources_;
    int towns_;
    int cities_;
    int roads_;
    State state_;
};

std::ostream& operator <<( std::ostream& out, const Player& p );
std::istream& operator >>( std::istream& in, Player& p );


/**/


class Game : public QObject {
    Q_OBJECT;
public:
    Game( QObject* parent = Q_NULLPTR );

    bool canBuildTown();
    bool canBuildCity() const;
    bool canBuildRoad();
    bool canBuildCard() const;

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
    void buildTown();
    void buildTown( const Pos& np );
    void buildCity();
    void buildCity( const Pos& np );
    void buildCard();
    void rob( const Pos& np );
    void robAround( const Pos& hp );

    void load();
    void save() const;

signals:
    void requestNbPlayers();
    void requestStartPositions();
    void requestNode();
    void requestRoad( Pos from = Pos() );
    void requestHex();
    void updatePlayer( int player = - 1, bool buttons = true );
    void rollDice();
    void diceRolled( int die1, int die2 );

protected:
    void setupAllowedBuildNodes( bool start = false );
    void setupAllowedRoadStartNodes();
    void setupAllowedCityNodes();
    void rob();

public:
    Board board_;
    std::vector<Player> player_;
    int nbPlayers_;
    int curPlayer_;
    bool pickStartAscending_;
};

std::ostream& operator <<( std::ostream& out, const Game& g );
std::istream& operator >>( std::istream& in, Game& g );