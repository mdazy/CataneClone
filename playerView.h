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
    QLabel* resources_;
    QLabel* tokens_;
    QPushButton* trade_;
    QPushButton* build_;
    QPushButton* card_;
    QPushButton* pass_;
};