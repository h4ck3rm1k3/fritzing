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

$Revision: 1886 $:
$Author: merunga $:
$Date: 2008-12-18 19:17:13 +0100 (Thu, 18 Dec 2008) $

********************************************************************/

#include "terminalpointitem.h"
#include "partseditorconnectoritem.h"
#include "../debugdialog.h"

const qreal TerminalPointItem::size = 3;

TerminalPointItem::TerminalPointItem(PartsEditorConnectorItem *parent, bool movable)
	: ResizableMovableGraphicsRectItem(parent)
{
	Q_ASSERT(parent);
	m_parent = parent;

	initPen();
	m_vLine = NULL;
	m_hLine = NULL;
	updatePoint();
	setFlag(QGraphicsItem::ItemIsMovable);

	setResizable(false);
	setMovable(movable);
}

void TerminalPointItem::updatePoint() {
	/*QPointF newPos = mapToParent(pos());
	QRectF pRect = parentItem()->boundingRect();
	QRectF newRect(newPos.x(),newPos.y(),pRect.width(),pRect.height());*/

	// this maps a moved tpoint correctly to its parent, after a
	// transformation of the later, but breaks the back-to-center-if-outside behaviour
	//setTransform(m_parent->transform());

	setRect(m_parent->boundingRect());
	drawCross();

	//after transformation, move back to center
	//moveBackToConnectorCenter();
}

void TerminalPointItem::initPen() {
	m_linePen = QPen(QColor::fromRgb(0,0,0));
	m_linePen.setWidth(1);
}

void TerminalPointItem::drawCross() {
	QRectF pRect = parentItem()->boundingRect();
	QPointF topPoint(pRect.x()+pRect.width()/2,pRect.y()+pRect.height()/2-size);
	QPointF bottomPoint(pRect.x()+pRect.width()/2,pRect.y()+pRect.height()/2+size);
	QPointF rightPoint(pRect.x()+pRect.width()/2+size,pRect.y()+pRect.height()/2);
	QPointF leftPoint(pRect.x()+pRect.width()/2-size,pRect.y()+pRect.height()/2);
	//m_point = QPointF(pRect.x()+pRect.width()/2,pRect.y()+pRect.height()/2);

	if(!m_vLine) m_vLine = new QGraphicsLineItem(this);
	m_vLine->setLine(QLineF(topPoint,bottomPoint));

	if(!m_hLine) m_hLine = new QGraphicsLineItem(this);
	m_hLine->setLine(QLineF(leftPoint,rightPoint));
}

void TerminalPointItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	if(isVisible() && m_movable) {
		grabMouse();
		updateCursor(event->pos(),QCursor(Qt::SizeAllCursor));
	} else {
		//QGraphicsItem::hoverEnterEvent(event);
	}
}

void TerminalPointItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	if(isVisible() && m_movable) {
		updateCursor(event->pos());
		ungrabMouse();
	} else {
		//QGraphicsItem::hoverEnterEvent(event);
	}
}

void TerminalPointItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	if(isVisible() && m_movable && m_moving && m_mousePosition != Outside) {
		move(event->scenePos());
		if(isOutSideConnector()) {
			setCursor(QCursor(Qt::ForbiddenCursor));
		} else {
			updateCursor(event->pos());
		}
		scene()->update();
	} else {
		ResizableMovableGraphicsRectItem::mouseMoveEvent(event);
	}
}

void TerminalPointItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	//setParentDragMode(QGraphicsView::NoDrag);
	if(isVisible() && m_movable) {
		m_mousePosition = closeToCorner(event->pos());
		m_moving = m_mousePosition != Outside;
		if(m_moving) {
			m_mousePressedPos = event->buttonDownScenePos(Qt::LeftButton);
		}
	} else {
		ResizableMovableGraphicsRectItem::mousePressEvent(event);
	}
}

void TerminalPointItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	if(isVisible() && m_movable) {
		m_moving = false;
		//setParentDragMode(QGraphicsView::ScrollHandDrag);
		setCursor(QCursor());
		if(isOutSideConnector()) {
			m_parent->resetTerminalPoint();
			return;
		}
	}
	ResizableMovableGraphicsRectItem::mouseReleaseEvent(event);
}

bool TerminalPointItem::isOutSideConnector() {
	QPointF myCenter = point();
	QPointF pPos = m_parent->mapToScene(m_parent->pos());
	QRectF pRectAux = m_parent->boundingRect();
	qreal magicNumber = 2; //?
	QRectF pRect(pPos.x()-magicNumber,pPos.y()-magicNumber,pPos.x()+pRectAux.width(),pPos.y()+pRectAux.height());

	DebugDialog::debug(QString("<<<< center %1 %2").arg(myCenter.x()).arg(myCenter.y()));
	DebugDialog::debug(QString("<<<< parent %1 %2 - %3 %4")
			.arg(pRect.x()).arg(pRect.y()).arg(pRect.width()).arg(pRect.height()));
	DebugDialog::debug("");
	return myCenter.x()<pRect.x() || myCenter.y()<pRect.y()
		|| myCenter.x()>pRect.width() || myCenter.y()>pRect.height();
}

QPointF TerminalPointItem::point() {
	scene()->update();
	QPointF pos = mapToScene(this->pos());
	QPointF size = mapToScene(QPointF(boundingRect().width(),boundingRect().height()));
	qreal lineWx2 = m_linePen.widthF()*2;
	return QPointF((pos.x()+size.x()-lineWx2)/2,(pos.y()+size.y()-lineWx2)/2);
}
