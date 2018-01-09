#pragma once

#include <QtWidgets/QStackedWidget>

class QPushButton;

class Game;
class BoardView;

class GameView : public QStackedWidget {
    Q_OBJECT;
public:
    GameView( Game* game, QWidget* parent = Q_NULLPTR );

public slots:
    void pickNbPlayers();
    void pickStartPositions();
    void pickStartNode();
    void pickStartRoad( int fromX, int fromY );

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
};