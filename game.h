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
    void startNodePicked( unsigned int nx, unsigned int ny );
    void startRoadPicked( unsigned int fromX, unsigned int fromY, unsigned int toX, unsigned int toY );

signals:
    void requestNbPlayers();
    void requestStartPositions();
    void requestNode();
    void requestRoad( int fromX = -1, int fromY = -1 );


public:
    Board board_;
    std::vector<Player> player_;
    unsigned int nbPlayers_;
    unsigned int curPlayer_;
    bool pickStartAscending_;
};