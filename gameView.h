#pragma once

#include <QtWidgets/QWidget>

#include "board.h"
#include "utils.h"

class QPushButton;
class QStackedLayout;

class Game;
class BoardView;
class PlayerView;
class Die;
class Player;

class Messenger;

class GameView : public QWidget {
    Q_OBJECT;
public:
    GameView( Game* game, Messenger* messenger );

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
    void gameOver( const std::vector<Player>& players );

    void discard( Player* p );

    void trade( Player* p );

    void updatePlayer( int player = -1, bool buttons = true );

signals:
    void nbPlayersPicked( int nbPlayers );

protected slots:
    void nbPlayersPicked();
    void loadState();

protected:
    QWidget* buildPlayersSelection();
    QWidget* buildGameView();

protected:
    Game* game_;
    bool playing_;

    QStackedLayout* stack_;
 
    QPushButton* players3_;
    QPushButton* players4_;

    BoardView* boardView_;
    Die* die1_;
    Die* die2_;
    std::vector<PlayerView*> playerView_;

private:
    Messenger* messenger_;
};