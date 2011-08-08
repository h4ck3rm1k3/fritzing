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

#include "bezierdisplay.h"
#include "../viewlayer.h"
#include "graphicsutils.h"

#include <QPen>
#include <QGraphicsScene>
#include <QApplication>

BezierDisplay::BezierDisplay()
{
	m_item0 = m_item1 = NULL;
}

BezierDisplay::~BezierDisplay()
{
	if (m_item0) delete m_item0;
	if (m_item1) delete m_item1;
}

void BezierDisplay::initDisplay(QGraphicsItem * master, Bezier *bezier)
{
	static int activeColor =   0x00c080;
	static int inactiveColor = 0xa00000;

	QPen pen;
	pen.setWidth(0);

	QGraphicsItem * parent = master;
	while (parent->parentItem()) {
		parent = master->parentItem();
	}

	double z = parent->zValue() - (ViewLayer::getZIncrement() / 2);
	
	m_item0 = new QGraphicsLineItem();
	pen.setColor(QColor(bezier->drag0() ? activeColor : inactiveColor));
	m_item0->setPen(pen);
	m_item0->setPos(0, 0);
	m_item0->setZValue(z);
	master->scene()->addItem(m_item0);

	m_item1 = new QGraphicsLineItem();
	pen.setColor(QColor(bezier->drag0() == false ? activeColor : inactiveColor));
	m_item1->setPen(pen);
	m_item1->setPos(0, 0);
	m_item1->setZValue(z);
	master->scene()->addItem(m_item1);

	updateDisplay(master, bezier);
	QApplication::processEvents();
}

void BezierDisplay::updateDisplay(QGraphicsItem * master, Bezier *bezier)
{
	if (m_item0 == NULL) return;
	if (m_item1 == NULL) return;

	if (bezier == NULL || bezier->isEmpty()) {
		m_item0->setVisible(false);
		m_item1->setVisible(false);
		return;
	}

	QRectF sr = master->scene()->sceneRect();
	double x1, y1, x2, y2;

	QPointF p0 = master->mapToScene(bezier->endpoint0());
	QPointF p1 = master->mapToScene(bezier->cp0());
	GraphicsUtils::liangBarskyLineClip(p0.x(), p0.y(), p1.x(), p1.y(), sr.left(), sr.right(), sr.top(), sr.bottom(), x1, y1, x2, y2);
	m_item0->setLine(x1, y1, x2, y2);

	p0 = master->mapToScene(bezier->endpoint1());
	p1 = master->mapToScene(bezier->cp1());
	GraphicsUtils::liangBarskyLineClip(p0.x(), p0.y(), p1.x(), p1.y(), sr.left(), sr.right(), sr.top(), sr.bottom(), x1, y1, x2, y2);
	m_item1->setLine(x1, y1, x2, y2);

	m_item0->setVisible(true);
	m_item1->setVisible(true);
}


