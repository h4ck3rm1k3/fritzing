/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision$:
$Author$:
$Date$

********************************************************************/

#ifndef MINIVIEW_H
#define MINIVIEW_H

#include <QGraphicsView>
#include <QBrush>
#include <QPainter>
#include <QGraphicsItem>
#include <QStyleOptionGraphicsItem>

class MiniView : public QGraphicsView
{
	Q_OBJECT
	
public:
	MiniView(QWidget *parent=0);
	virtual ~MiniView(); 

	void setView(QGraphicsView *);	
	QGraphicsView* view();
	void setTitle(const QString & title);
	
protected:
	void resizeEvent ( QResizeEvent * event ); 
	void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *);
	void mouseMoveEvent(QMouseEvent * event);
	bool viewportEvent(QEvent *event);
	void drawBackground(QPainter * painter, const QRectF & rect);

public slots:
	void updateSceneRect ( const QRectF & rect );
	void navigatorMousePressedSlot(class MiniViewContainer *);
	void navigatorMouseEnterSlot(class MiniViewContainer *);
	void navigatorMouseLeaveSlot(class MiniViewContainer *);

signals:
	void rectChangedSignal();
	void miniViewMousePressedSignal();
	
protected:
	QGraphicsView * m_otherView;
	QString m_title;
	int m_titleWeight;
	QColor m_titleColor;
	bool m_selected;
	int m_lastHeight;

};

#endif
