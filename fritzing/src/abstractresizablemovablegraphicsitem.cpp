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

$Revision: 2130 $:
$Author: merunga $:
$Date: 2009-01-09 12:26:13 +0100 (Fri, 09 Jan 2009) $

********************************************************************/


#include <QLineF>
#include <QPair>
#include "abstractresizablemovablegraphicsitem.h"

AbstractResizableMovableGraphicsItem::AbstractResizableMovableGraphicsItem() {
	m_resizable = false;
	m_resizing = false;

	m_movable = false;
	m_moving = false;
}

void AbstractResizableMovableGraphicsItem::setResizable(bool resizable) {
	m_resizable = resizable;
}

void AbstractResizableMovableGraphicsItem::setMovable(bool movable) {
	m_movable = movable;
}

void AbstractResizableMovableGraphicsItem::resize(const QPointF &mousePos) {
	prepareForChange();

	qreal oldX1 = rectAux().x();
	qreal oldY1 = rectAux().y();
	qreal oldX2 = oldX1+rectAux().width();
	qreal oldY2 = oldY1+rectAux().height();
	qreal newX = mousePos.x();
	qreal newY = mousePos.y();

	switch(m_mouseRelativePosition) {
		case TopLeftCorner:
			setRectAux(newX,newY,oldX2,oldY2);
//			DebugDialog::debug(QString("<<< from top left new rect (%1,%2)  (%3,%4)")
//				.arg(newX).arg(newY).arg(oldX2).arg(oldY2));
			break;
		case BottomLeftCorner:
			setRectAux(newX,oldY1,oldX1,newY);
//			DebugDialog::debug(QString("<<< from bottom left new rect (%1,%2)  (%3,%4)")
//				.arg(newX).arg(oldY1).arg(oldX1).arg(newY));
			break;
		case TopRightCorner:
			setRectAux(oldX1,newY,newX,oldX2);
//			DebugDialog::debug(QString("<<< from top right new rect (%1,%2)  (%3,%4)")
//				.arg(oldX1).arg(newY).arg(newX).arg(oldX2));
			break;
		case BottomRightCorner:
			setRectAux(oldX1,oldY1,newX,newY);
//			DebugDialog::debug(QString("<<< from bottom right new rect (%1,%2)  (%3,%4)")
//				.arg(oldX1).arg(oldY1).arg(newX).arg(newY));
			break;
		default: break;
	}
}

void AbstractResizableMovableGraphicsItem::move(const QPointF &newPos) {
	QPointF currentParentPos = map(newPos);
	QPointF buttonDownParentPos = map(m_mousePressedPos);
	QPointF aux = currentParentPos - buttonDownParentPos;
	doMoveBy(aux.x(),aux.y());
	m_mousePressedPos = newPos;
}


AbstractResizableMovableGraphicsItem::Position AbstractResizableMovableGraphicsItem::updateCursor(const QPointF &mousePos, const QCursor &defaultCursor) {
	QCursor cursor;
	m_mouseRelativePosition = closeToCorner(mousePos);
	switch(m_mouseRelativePosition) {
		case TopLeftCorner:
			cursor = QCursor(Qt::SizeFDiagCursor);
			break;
		case BottomRightCorner:
			cursor = QCursor(Qt::SizeFDiagCursor);
			break;
		case TopRightCorner:
			cursor = QCursor(Qt::SizeBDiagCursor);
			break;
		case BottomLeftCorner:
			cursor = QCursor(Qt::SizeBDiagCursor);
			break;
		case Inside:
			cursor = QCursor(Qt::SizeAllCursor);
			break;
		case Outside:
			cursor = defaultCursor;
			break;
	}
	setCursorAux(cursor);
	return m_mouseRelativePosition;
}


AbstractResizableMovableGraphicsItem::Position AbstractResizableMovableGraphicsItem::closeToCorner(const QPointF &pos) {
	qreal x1 = rectAux().x();
	qreal y1 = rectAux().y();
	qreal x2 = x1+rectAux().width();
	qreal y2 = y1+rectAux().height();

	bool mouseOutOfRect = pos.x() < x1 || pos.y() < y1 || pos.x() > x2 || pos.y() > y2;

	QPair<qreal,Position> tl(QLineF(QPointF(x1,y1),pos).length(), TopLeftCorner);
	QPair<qreal,Position> tr(QLineF(QPointF(x2,y1),pos).length(), TopRightCorner);
	QPair<qreal,Position> br(QLineF(QPointF(x2,y2),pos).length(), BottomRightCorner);
	QPair<qreal,Position> bl(QLineF(QPointF(x1,y2),pos).length(), BottomLeftCorner);

	QPair<qreal,Position> min = tl.first < tr.first ? tl : tr;
	min = min.first < br.first ? min : br;
	min = min.first < bl.first ? min : bl;

	if(min.first <= 3)
		return min.second;
	else if(mouseOutOfRect) return Outside;
	else return Inside;
}


void AbstractResizableMovableGraphicsItem::setRectAux(qreal x1, qreal y1, qreal x2, qreal y2) {

}
