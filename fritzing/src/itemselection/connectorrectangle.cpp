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
#include "../partseditor/partseditorconnectorsconnectoritem.h"

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

void ConnectorRectangle::setHandlersVisible(bool visible) {
	foreach(CornerHandler* handler, m_cornerHandlers) {
		handler->doSetVisible(visible);
	}
}

void ConnectorRectangle::prepareForChange() {
	resizableOwner()->doPrepareGeometryChange();
}

void ConnectorRectangle::resizeRect(qreal x1, qreal y1, qreal x2, qreal y2) {
	qreal minWidth = resizableOwner()->minWidth();
	qreal minHeight = resizableOwner()->minHeight();

	qreal width = x2-x1 < minWidth ? minWidth : x2-x1;
	qreal height = y2-y1 < minHeight ? minHeight : y2-y1;

	if(width != owner()->rect().width() && height != owner()->rect().height()) {
		resizableOwner()->resizeRect(x1,y1,width,height);
	}
}

bool ConnectorRectangle::isResizable() {
	return resizableOwner()->isResizable();
}

void ConnectorRectangle::paint(QPainter *painter) {
	QRectF rect = m_owner->boundingRect();

	if(m_firstPaint && rect.width() > 0 && rect.height() > 0) {
		placeHandlers();
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
			handler->doPaint(painter);
		}
	}
}

void ConnectorRectangle::resizingStarted() {
	foreach(CornerHandler* handler, m_cornerHandlers) {
		handler->doSetVisible(false);
	}
}

void ConnectorRectangle::resizingFinished() {
	foreach(CornerHandler* handler, m_cornerHandlers) {
		handler->doSetVisible(true);
		setHandlerRect(handler);
	}
}

void ConnectorRectangle::placeHandlers() {
	foreach(CornerHandler* handler, m_cornerHandlers) {
		setHandlerRect(handler);
	}
}

void ConnectorRectangle::setHandlerRect(CornerHandler* handler) {
	handler->doSetRect(handlerRect(handler->corner()));
}

QRectF ConnectorRectangle::handlerRect(Qt::Corner corner) {
	QRectF rect = m_owner->boundingRect();
	qreal scale = currentScale();
	QPointF offset(CornerHandler::Size/scale,CornerHandler::Size/scale);
	QPointF cornerPoint;
	switch(corner) {
		case Qt::TopLeftCorner:
			cornerPoint=rect.topLeft();
			break;
		case Qt::TopRightCorner:
			cornerPoint=rect.topRight();
			break;
		case Qt::BottomRightCorner:
			cornerPoint=rect.bottomRight();
			break;
		case Qt::BottomLeftCorner:
			cornerPoint=rect.bottomLeft();
			break;
		default: Q_ASSERT(false);
	}
	return QRectF(cornerPoint-offset,cornerPoint+offset);
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

QGraphicsRectItem *ConnectorRectangle::owner() {
	return m_owner;
}

ResizableRectItem* ConnectorRectangle::resizableOwner() {
	return dynamic_cast<ResizableRectItem*>(m_owner);
}
