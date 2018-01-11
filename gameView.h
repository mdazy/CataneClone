#pragma once

#include <QtWidgets/QStackedWidget>

#include "utils.h"

class QPushButton;

class Game;
class BoardView;
class PlayerView;

class GameView : public QStackedWidget {
    Q_OBJECT;
public:
    GameView( Game* game, QWidget* parent = Q_NULLPTR );

public slots:
    void pickNbPlayers();
    void pickStartPositions();
    void pickStartNode();
    void pickStartRoad( const Pos& from );
    void updatePlayer( int player );

signals:
    void nbPlayersPicked( int nbPlayers );

protected slots:
    void nbPlayersPicked();

protected:
    void buildPlayersSelection();
    void buildGameView();

protected:
    Game* game_;

    QWidget* playersSelection_;
    QPushButton* players3_;
    QPushButton* players4_;

    QWidget* gameView_;
    BoardView* boardView_;
    std::vector<PlayerView*> playerView_;
};