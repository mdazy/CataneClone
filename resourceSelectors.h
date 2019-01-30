#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>
#include <vector>

#include "board.h"

class QGridLayout;
class QLabel;
class QPushButton;
class QSpinBox;

class Player;


/**/


class ResourceSelector : public QWidget {
    Q_OBJECT;
public:
    ResourceSelector( QWidget* parent = Q_NULLPTR );
    virtual ~ResourceSelector();

    void setTotal( int total );
    void setMaxima( const std::vector<int>& maxima );
    void setSteps( const std::vector<int>& steps );

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


class MaxedSelector : public QDialog {
    Q_OBJECT;
public:
    MaxedSelector( QWidget* parent = Q_NULLPTR );
    virtual ~MaxedSelector();

    void accept() override;
    void closeEvent( QCloseEvent* ) override;

protected slots:
    void updateOKButton();
    virtual void doAccept() = 0;

protected:
    int max_;
    QLabel* info_;
    QGridLayout* selectorLayout_;
    ResourceSelector* selector_;
    QPushButton* OKButton_;
};


/**/


class DiscardSelector : public MaxedSelector {
    Q_OBJECT;
public:
    DiscardSelector( Player* p, QWidget* parent = Q_NULLPTR );
    virtual ~DiscardSelector();

signals:
    void selected( Player* p, std::vector<int> selection );

protected:
    void doAccept() override;

protected:
    Player* p_;
};


/**/


class NumberSelector : public MaxedSelector {
    Q_OBJECT;
public:
    NumberSelector( int max, QWidget* parent = Q_NULLPTR );
    virtual ~NumberSelector();

signals:
    void selected( std::vector<int> selection );

protected:
    void doAccept() override;
};


/**/


class TradeSelector : public MaxedSelector {
    Q_OBJECT;
public:
    TradeSelector( Player* p, QWidget* parent = Q_NULLPTR );
    virtual ~TradeSelector();

protected:
    void doAccept() override;

private slots:
    void updateMax();

protected:
    Player* p_;
    ResourceSelector* fromSel_;
};