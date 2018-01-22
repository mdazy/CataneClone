#pragma once

#include <QtWidgets/QWidget>

#include "game.h"

class QEvent;
class QLabel;
class QPushButton;

class PlayerView : public QWidget {
public:
    PlayerView( Player* p, QWidget* parent = Q_NULLPTR );

    void enableButtons( bool enable );

    void updateView();

public:  
    Player* player_;
    Player prevPlayer_;
    QLabel* playerName_;
    QLabel* resources_;
    QLabel* tokens_;
    QPushButton* trade_;
    QPushButton* buildRoad_;
    QPushButton* buildTown_;
    QPushButton* buildCity_;
    QPushButton* buildCard_;
    QPushButton* card_;
    QPushButton* pass_;
};