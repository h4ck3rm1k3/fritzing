/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2009 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 2148 $:
$Author: cohen@irascible.com $:
$Date: 2009-01-13 05:46:37 +0100 (Tue, 13 Jan 2009) $

********************************************************************/

#ifndef CORNERHANDLER_H_
#define CORNERHANDLER_H_

#include <QGraphicsPixmapItem>
#include <QCursor>

class ConnectorRectangle;

class CornerHandler : public QGraphicsRectItem {
public:
	CornerHandler(ConnectorRectangle *parent, QGraphicsRectItem* parentItem, Qt::Corner corner);
	void resize(const QPointF &mousePos);
	Qt::Corner corner();
	bool isBeingDragged();
	void setPixmap(const QPixmap &pixmap);
	void setVisible1(bool visible);
	void setRectAux(QRectF newRect) {
		setFlag(QGraphicsItem::ItemIgnoresTransformations,false);
		setRect(mapFromParent(newRect).boundingRect());
		//setFlag(QGraphicsItem::ItemIgnoresTransformations,true);
	}

protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
	//void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	Qt::CursorShape cursorForCorner(Qt::Corner);

	void initPixmapHash();

	ConnectorRectangle *m_parent;
	Qt::Corner m_corner;
	QCursor m_cursorBU;
	QGraphicsPixmapItem *m_child;
	volatile bool m_resizing;

public:
	static QHash<Qt::Corner,QPixmap> pixmapHash;
};

#endif /* CORNERHANDLER_H_ */
