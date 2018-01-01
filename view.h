#pragma once

#include <QtWidgets/QWidget>

#include "board.h"

class QPaintEvent;

class View : public QWidget {
public:
	View( QWidget* parent  = Q_NULLPTR );

	void paintEvent( QPaintEvent* event ) override;

public:
	Board* board_;
};