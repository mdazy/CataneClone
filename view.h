#pragma once

#include <QtWidgets/QWidget>

class QPaintEvent;

class View : public QWidget {
public:
	View( QWidget* parent  = Q_NULLPTR );

	void paintEvent( QPaintEvent* event ) override;
};