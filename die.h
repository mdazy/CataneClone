#pragma once

#include <QtWidgets/QWidget>

class QPaintEvent;

class Die : public QWidget {
public:
    Die( QWidget* parent = Q_NULLPTR );

    void paintEvent( QPaintEvent* event ) override;
    int heightForWidth( int w ) const override;

public slots:
    void setValue( int value );

private:
    int value_;
};