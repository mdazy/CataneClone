#pragma once

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStackedWidget>

#include "board.h"
#include "utils.h"

class QPushButton;

class Game;
class BoardView;
class PlayerView;
class Die;
class Player;

class GameView : public QStackedWidget {
    Q_OBJECT;
public:
    GameView( Game* game, QWidget* parent = Q_NULLPTR );

public slots:
    void pickNbPlayers();
    void pickStartPositions();
    void pickHex();
    void pickNode();
    void pickRoad( const Pos& from );
    void rollDice();
    void diceRolled( int die1, int die2 );
    void invention();
    void monopoly();

    void discard( Player* p );

    void updatePlayer( int player = -1, bool buttons = true );

signals:
    void nbPlayersPicked( int nbPlayers );

protected slots:
    void nbPlayersPicked();
    void loadState();

protected:
    void buildPlayersSelection();
    void buildGameView();

protected:
    Game* game_;
    bool playing_;

    QWidget* playersSelection_;
    QPushButton* players3_;
    QPushButton* players4_;

    QWidget* gameView_;
    BoardView* boardView_;
    Die* die1_;
    Die* die2_;
    std::vector<PlayerView*> playerView_;
};