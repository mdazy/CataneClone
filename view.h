#pragma once

#include <QtWidgets/QWidget>

#include <vector>

class QPaintEvent;

class View : public QWidget {
public:
	View( QWidget* parent  = Q_NULLPTR );

	void paintEvent( QPaintEvent* event ) override;

public:
	std::vector<std::vector<int>>* grid_;
};