#pragma once

#include <QtWidgets/QWidget>

#include "board.h"

class QMouseEvent;
class QPaintEvent;

class View : public QWidget {
public:
	View( QWidget* parent  = Q_NULLPTR );

protected:
	// from QWidget
	void mouseMoveEvent( QMouseEvent* event ) override;
	void paintEvent( QPaintEvent* event ) override;

	// custom
	QPointF nodeCenter( unsigned int nx, unsigned int ny ) const;
	void drawHexes( QPainter& p, Hex::Type type = Hex::Any ) const;
	void drawNodes( QPainter& p, bool drawHarbors = false ) const;
	void drawRoads( QPainter& p ) const;

public:
	Board* board_;

private:
	// for selection
	int mouseX_;
	int mouseY_;

	// for painting
	float radius_;
	float innerRadius_;
	float nodeRadius_;
	float nodeDiag_;
	float centerShiftX_;
	float centerShiftY_;
	float textSize_;
};