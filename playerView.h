#pragma once

#include <QtWidgets/QWidget>

class QEvent;
class QLabel;
class QPushButton;

class Player;

class PlayerView : public QWidget {
public:
    PlayerView( Player* p, QWidget* parent = Q_NULLPTR );

    void enableButtons( bool enable );

    void updateView();

public:  
    Player* player_;
    QLabel* resources_;
    QPushButton* trade_;
    QPushButton* build_;
    QPushButton* card_;
    QPushButton* pass_;
};