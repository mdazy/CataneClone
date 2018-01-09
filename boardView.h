#pragma once

#include <QtWidgets/QWidget>

#include "board.h"

class QMouseEvent;
class QPaintEvent;

class BoardView : public QWidget {
	Q_OBJECT;
public:
	enum SelectionMode { None = 0, Hex, Node, Road };
	BoardView( QWidget* parent  = Q_NULLPTR );

	void setSelectionMode( SelectionMode mode );
	
protected:
	// from QWidget
	void mouseMoveEvent( QMouseEvent* event ) override;
	void mouseReleaseEvent( QMouseEvent* event ) override;
	void paintEvent( QPaintEvent* event ) override;

	// custom
	QPointF nodeCenter( unsigned int nx, unsigned int ny ) const;
	void drawHexes( QPainter& p, Hex::Type type = Hex::Any );
	void drawNodes( QPainter& p, bool drawHarbors = false );
	void drawRoads( QPainter& p ) const;

signals:
	void hexSelected( unsigned int hx, unsigned int hy );
	void nodeSelected( unsigned int nx, unsigned int ny );
	void roadSelected( unsigned int fromX, unsigned int fromY, unsigned int toX, unsigned int toY );

public:
	Board* board_;
	int fromX_;
	int fromY_;

private:
	// for selection
	SelectionMode selectionMode_;
	int mouseX_;
	int mouseY_;
	int nodeX_;
	int nodeY_;
	int hexX_;
	int hexY_;

	// for painting
	float radius_;
	float innerRadius_;
	float nodeRadius_;
	float nodeDiag_;
	float centerShiftX_;
	float centerShiftY_;
	float textSize_;
};