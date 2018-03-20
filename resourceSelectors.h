#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>
#include <vector>

#include "board.h"

class QGridLayout;
class QPushButton;
class QSpinBox;

class Player;


/**/


class ResourceSelector : public QWidget {
    Q_OBJECT;
public:
    ResourceSelector( int nbResources, QWidget* parent = Q_NULLPTR );
    virtual ~ResourceSelector();

    void setMaxima( const std::vector<int>& maxima );

    std::vector<int> selection() const;

signals:
    void selectionChanged();

protected slots:    
    void updateLimits();

protected:
    QGridLayout* layout_;
    QSpinBox* spin_[ Hex::Desert ];
    int nbResources_;
    std::vector<int> maxima_;
};


/**/


class DiscardSelector : public QDialog {
    Q_OBJECT;
public:
    DiscardSelector( Player* p, QWidget* parent = Q_NULLPTR );
    virtual ~DiscardSelector();

    void accept() override;

signals:
    void selected( Player* p, std::vector<int> selection );

protected slots:
    void updateOKButton();

protected:
    int nbCards_;
    Player* p_;
    ResourceSelector* selector_;
    QPushButton* OKButton_;
};
