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
	static int activeColor = 0x80ff0000;
	static int inactiveColor = 0x800000ff;

	QPen pen;
	pen.setWidth(0);
	
	m_item0 = new QGraphicsLineItem();
	pen.setColor(QColor(bezier->drag0() ? activeColor : inactiveColor));
	m_item0->setPen(pen);
	m_item0->setPos(0, 0);
	master->scene()->addItem(m_item0);

	m_item1 = new QGraphicsLineItem();
	pen.setColor(QColor(bezier->drag0() == false ? activeColor : inactiveColor));
	m_item1->setPen(pen);
	m_item1->setPos(0, 0);
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

	m_item0->setLine(QLineF(master->mapToScene(bezier->endpoint0()), master->mapToScene(bezier->cp0())));
	m_item1->setLine(QLineF(master->mapToScene(bezier->endpoint1()), master->mapToScene(bezier->cp1())));
	m_item0->setVisible(true);
	m_item1->setVisible(true);
}


