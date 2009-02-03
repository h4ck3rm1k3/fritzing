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
#include <QPainter>
#include <QHash>

#include "connectorrectangle.h"
#include "../sketchwidget.h"
#include "../debugdialog.h"

ConnectorRectangle::ConnectorRectangle(QGraphicsRectItem* owner, bool withHandlers)
{
	m_owner = owner;
	m_firstPaint = true;

	if(withHandlers) {
		m_topLeftHandler     = new CornerHandler(this, owner, Qt::TopLeftCorner);
		m_topRightHandler    = new CornerHandler(this, owner, Qt::TopRightCorner);
		m_bottomRightHandler = new CornerHandler(this, owner, Qt::BottomRightCorner);
		m_bottomLeftHandler  = new CornerHandler(this, owner, Qt::BottomLeftCorner);

		m_cornerHandlers
			<< m_topLeftHandler << m_topRightHandler
			<< m_bottomRightHandler << m_bottomLeftHandler;

		setHandlersVisible(false);
	} else {
		m_topLeftHandler = m_topRightHandler =
		m_bottomRightHandler = m_bottomLeftHandler = NULL;
	}
}

QGraphicsRectItem *ConnectorRectangle::owner() {
	return m_owner;
}

void ConnectorRectangle::setHandlersVisible(bool visible) {
	foreach(CornerHandler* handler, m_cornerHandlers) {
		handler->setVisible(visible);
	}
}

void ConnectorRectangle::prepareForChange() {
	//m_owner->prepareGeometryChange();
}

void ConnectorRectangle::resizeRect(qreal x1, qreal y1, qreal x2, qreal y2) {
	qreal minWidth = resizableOwner()->minWidth();
	qreal minHeight = resizableOwner()->minHeight();

	qreal width = x2-x1 < minWidth ? minWidth : x2-x1;
	qreal height = y2-y1 < minHeight ? minHeight : y2-y1;

	//if(width != m_owner->boundingRect().width()
	//   && height != m_owner->boundingRect().height()) {
		resizableOwner()->resizeRect(x1,y1,width,height);
	//}
}

bool ConnectorRectangle::isResizable() {
	return resizableOwner()->isResizable();
}

void ConnectorRectangle::setState(State state) {
	m_state = state;
}

void ConnectorRectangle::paint(QPainter *painter) {
	QRectF rect = m_owner->boundingRect();

	if(m_firstPaint && rect.width() > 0 && rect.height() > 0) {
		placeHandlersInto(rect);
		m_firstPaint = false;
	}

	bool beingResized = false;
	foreach(CornerHandler* handler, m_cornerHandlers) {
		if(handler->isBeingDragged()) {
			beingResized = true;
			break;
		}
	}

	if(beingResized) {
		prepareForChange();
		foreach(CornerHandler* handler, m_cornerHandlers) {
			Qt::Corner corner = handler->corner();
			QPixmap pm = CornerHandler::pixmapHash[corner];
			qreal scale = currentScale();
			QPointF hPos = posForHandlerIn(corner, rect);
			painter->drawPixmap(hPos.x(),hPos.y(),pm.width()/scale,pm.height()/scale,pm);
		}
	}
}

void ConnectorRectangle::resizingStarted() {
	foreach(CornerHandler* handler, m_cornerHandlers) {
		handler->setPixmap(0);
	}
}

void ConnectorRectangle::resizingFinished() {
	foreach(CornerHandler* handler, m_cornerHandlers) {
		QRectF rect = m_owner->boundingRect();
		Qt::Corner corner = handler->corner();
		handler->setPos(posForHandlerIn(corner,rect));
		handler->setPixmap(CornerHandler::pixmapHash[corner]);
	}
}

void ConnectorRectangle::placeHandlersInto(const QRectF &rect) {
	foreach(CornerHandler* handler, m_cornerHandlers) {
		handler->setPos(posForHandlerIn(handler->corner(), rect));
	}
}

QPointF ConnectorRectangle::posForHandlerIn(Qt::Corner corner, const QRectF &rect) {
	qreal xaux = offsetX();
	qreal yaux = offsetY();
	switch(corner) {
		case Qt::TopLeftCorner:
			return QPointF(rect.x()-xaux,rect.y()-yaux);
		case Qt::TopRightCorner:
			return QPointF(rect.x()+rect.width()-xaux,rect.y()-yaux);
		case Qt::BottomRightCorner:
			return QPointF(rect.x()+rect.width()-xaux,rect.y()+rect.height()-yaux);
		case Qt::BottomLeftCorner:
			return QPointF(rect.x()-xaux,rect.y()+rect.height()-yaux);
		default: Q_ASSERT(false);
	}
	return QPointF(-1000,-1000);
}

qreal ConnectorRectangle::offsetX() {
	//return QPixmap("resources/images/itemselection/selectionRingLeft.png").width()/currentScale();
	return 2/currentScale();
}

qreal ConnectorRectangle::offsetY() {
	//return QPixmap("resources/images/itemselection/selectionRingTop.png").height()/currentScale();
	return offsetX();
}

qreal ConnectorRectangle::currentScale() {
	if(m_owner->scene()) {
		SketchWidget *sw = dynamic_cast<SketchWidget*>(m_owner->scene()->parent());
		if(sw) {
			return sw->currentZoom()/100;
		}
	}
	return 1;
}

ResizableRectItem* ConnectorRectangle::resizableOwner() {
	return dynamic_cast<ResizableRectItem*>(m_owner);
}
