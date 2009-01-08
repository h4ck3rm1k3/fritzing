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

const qreal TerminalPointItem::size = 3;

TerminalPointItem::TerminalPointItem(ConnectorItem *parent) : QGraphicsRectItem(parent) {
	initPen();
	m_vLine = NULL;
	m_hLine = NULL;
	updatePoint();
	setFlag(QGraphicsItem::ItemIsMovable);
}

QPointF TerminalPointItem::point() {
	return m_point;
}

void TerminalPointItem::updatePoint() {
	setRect(parentItem()->boundingRect());
	drawCross();
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
	m_point = QPointF(pRect.x()+pRect.width()/2,pRect.y()+pRect.height()/2);

	if(!m_vLine) m_vLine = new QGraphicsLineItem(this);
	m_vLine->setLine(QLineF(topPoint,bottomPoint));

	if(!m_hLine) m_hLine = new QGraphicsLineItem(this);
	m_hLine->setLine(QLineF(leftPoint,rightPoint));
}
