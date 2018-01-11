#pragma once

#include <QtWidgets/QWidget>

class QLabel;

class Player;

class PlayerView : public QWidget {
public:
    PlayerView( Player* p, QWidget* parent = Q_NULLPTR );

    void updateView();

public:  
    Player* player_;
    QLabel* resources_;
};