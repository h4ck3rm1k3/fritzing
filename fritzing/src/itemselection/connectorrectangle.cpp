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

ConnectorRectangle::ConnectorRectangle(ResizableRectItem* owner)
	: QGraphicsRectItem(owner)
{
	m_owner = owner;
	m_firstPaint = true;

	m_topLeftHandler = new CornerHandler(this, Qt::TopLeftCorner);
	//m_topLeftHandler->setOffset(QPointF(width(),height()));

	//m_topRightHandler = new CornerHandler(this, Qt::TopRightCorner);
	//m_topRightHandler->setPos(rect.width(),rect.y());

	//m_bottomRightHandler = new CornerHandler(this, Qt::BottomRightCorner);
	//m_bottomRightHandler->setPos(rect.width(),rect.height());

	//m_bottomLeftHandler = new CornerHandler(this, Qt::BottomLeftCorner);
	//m_bottomLeftHandler->setPos(rect.x(),rect.height());
}

void ConnectorRectangle::prepareForChange() {
	prepareGeometryChange();
	m_owner->prepareGeometryChange();
}

void ConnectorRectangle::resizeRect(qreal x1, qreal y1, qreal x2, qreal y2) {
	qreal minWidth = m_owner->minWidth();
	qreal minHeight = m_owner->minHeight();

	qreal width = x2-x1 < minWidth ? minWidth : x2-x1;
	qreal height = y2-y1 < minHeight ? minHeight : y2-y1;

	if(width != m_owner->boundingRect().width()
	   && height != m_owner->boundingRect().height()) {
		m_owner->resizeRect(x1,y1,width,height);
		setRect(x1,y1,width,height);
	}
}

QRectF ConnectorRectangle::itemRect() {
	return m_owner->boundingRect();
}

bool ConnectorRectangle::isResizable() {
	return m_owner->isResizable();
}

void ConnectorRectangle::setState(State state) {
	m_state = state;
}

void ConnectorRectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	QRectF rect = m_owner->boundingRect();
	if(m_firstPaint && rect.width() > 0 && rect.height() > 0) {
		m_topLeftHandler->setPos(rect.x(),rect.y());
		m_firstPaint = false;
	}

	QGraphicsRectItem::paint(painter,option,widget);

	if(m_topLeftHandler->isBeingDragged()) {
		prepareForChange();
		painter->save();
		Qt::Corner corner = m_topLeftHandler->corner();
		QPixmap pm = CornerHandler::pixmapHash[corner];
		qreal scale = currentScale();
		painter->drawPixmap(rect.x(),rect.y(),pm.width()/scale,pm.height()/scale,pm);
		painter->restore();
	}
}

qreal ConnectorRectangle::offsetX() {
	//return QPixmap("resources/images/itemselection/selectionRingLeft.png").width()/currentScale();
	return 4/currentScale();
}

qreal ConnectorRectangle::offsetY() {
	//return QPixmap("resources/images/itemselection/selectionRingTop.png").height()/currentScale();
	return offsetX();
}

qreal ConnectorRectangle::currentScale() {
	return dynamic_cast<SketchWidget*>(scene()->parent())->currentZoom()/100;
}
