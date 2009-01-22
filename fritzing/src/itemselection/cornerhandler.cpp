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

#include <QHash>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "cornerhandler.h"
#include "connectorrectangle.h"
#include "../debugdialog.h"

QHash<Qt::Corner,QPixmap> CornerHandler::pixmapHash;

CornerHandler::CornerHandler(ConnectorRectangle* parent, Qt::Corner corner)
	: QGraphicsPixmapItem(parent)
{
	m_parent = parent;
	m_corner = corner;
	m_resizing = false;

	initPixmapHash();

	setFlag(QGraphicsItem::ItemIgnoresTransformations);
	setPixmap(pixmapHash[m_corner]);
}

void CornerHandler::initPixmapHash() {
	if(pixmapHash.isEmpty()) {
		pixmapHash[Qt::TopLeftCorner] =
			QPixmap(":/resources/images/itemselection/cornerHandlerActiveTopLeft.png");
		pixmapHash[Qt::TopRightCorner] =
		 	QPixmap(":/resources/images/itemselection/cornerHandlerActiveTopRight.png");
		pixmapHash[Qt::BottomRightCorner] =
		 	QPixmap(":/resources/images/itemselection/cornerHandlerActiveBottomRight.png");
		pixmapHash[Qt::BottomLeftCorner] =
		 	QPixmap(":/resources/images/itemselection/cornerHandlerActiveBottomLeft.png");
	}
}


void CornerHandler::resize(const QPointF &mousePos) {
	m_parent->prepareForChange();
	QRectF rect = m_parent->itemRect();

	qreal oldX1 = rect.x();
	qreal oldY1 = rect.y();
	qreal oldX2 = oldX1+rect.width();
	qreal oldY2 = oldY1+rect.height();
	qreal newX = mousePos.x();
	qreal newY = mousePos.y();

	DebugDialog::debug(QString("mouse pos x=%1 y=%2").arg(newX).arg(newY));

	switch(m_corner) {
		case Qt::TopLeftCorner:
			m_parent->resizeRect(newX,newY,oldX2,oldY2); break;
		case Qt::BottomLeftCorner:
			m_parent->resizeRect(newX,oldY1,oldX1,newY); break;
		case Qt::TopRightCorner:
			m_parent->resizeRect(oldX1,newY,newX,oldX2); break;
		case Qt::BottomRightCorner:
			m_parent->resizeRect(oldX1,oldY1,newX,newY); break;
		default: break;
	}
}

Qt::Corner CornerHandler::corner() {
	return m_corner;
}

void CornerHandler::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	if(m_parent->isResizable() && m_resizing) {
		resize(event->pos());
		scene()->update();
	} else {
		QGraphicsPixmapItem::mouseMoveEvent(event);
	}
}

void CornerHandler::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if(m_parent->isResizable()) {
		m_resizing = true;
		m_parent->resizingStarted();
		m_mousePressedPos = event->pos();
	} else {
		QGraphicsPixmapItem::mousePressEvent(event);
	}
}

void CornerHandler::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	if(m_parent->isResizable()) {
		m_parent->resizingFinished();
		m_resizing = false;
	}
	QGraphicsPixmapItem::mouseReleaseEvent(event);
}

bool CornerHandler::isBeingDragged() {
	return m_resizing;
}
