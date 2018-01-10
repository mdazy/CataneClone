#pragma once

#include <QtCore/QObject>

#include "board.h"

class Player {
public:
    Player();

public:
    std::vector<int> resources_;
};


class Game : public QObject {
    Q_OBJECT;
public:
    Game( QObject* parent = Q_NULLPTR );

public slots:
    void newGame();
    void startWithPlayers( int nbPlayers );
    void startNodePicked( const Pos& np );
    void startRoadPicked( const Pos& from, const Pos& to );

signals:
    void requestNbPlayers();
    void requestStartPositions();
    void requestNode();
    void requestRoad( Pos from = Pos() );


public:
    Board board_;
    std::vector<Player> player_;
    unsigned int nbPlayers_;
    unsigned int curPlayer_;
    bool pickStartAscending_;
};