/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/

#ifndef MINIVIEWCONTAINER_H
#define MINIVIEWCONTAINER_H

#include <QWidget>
#include <QPaintEvent>

#include "miniview.h"

class MiniViewContainer : public QWidget
{
	Q_OBJECT
	
public:
	MiniViewContainer(QWidget * parent = 0);
	void setView(QGraphicsView *);	
	void resizeEvent ( QResizeEvent * event ); 
	void mousePressEvent(QMouseEvent *event);
	
protected slots:
	void updateFrame();
	
protected:
	MiniView * m_miniView;
	class MiniViewFrame * m_frame;
	class MiniViewFrame * m_outerFrame;
	QWidget * m_mask;
	
};


class MiniViewFrame : public QFrame
{
	Q_OBJECT
	
public:
	MiniViewFrame(QBrush &, bool draggable, QWidget * parent = 0);
	
	void setMaxDrag(int x, int y);
	
protected:
	void paintEvent(QPaintEvent * event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	
protected:
	QBrush m_brush;
	QPen m_pen;
	QPoint m_dragOffset;
	QPoint m_originalPos;
	bool m_inDrag;
	bool m_draggable;
	QSize m_maxDrag;
	
signals:
	void scrollChangeSignal(double x, double y);
};

#endif
