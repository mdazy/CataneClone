#pragma once

#include <QtWidgets/QWidget>

#include "board.h"

class QMouseEvent;
class QPaintEvent;

class View : public QWidget {
public:
	View( QWidget* parent  = Q_NULLPTR );

protected:
	void mouseMoveEvent( QMouseEvent* event ) override;
	void paintEvent( QPaintEvent* event ) override;

public:
	Board* board_;

private:
	int mouseX_;
	int mouseY_;
};