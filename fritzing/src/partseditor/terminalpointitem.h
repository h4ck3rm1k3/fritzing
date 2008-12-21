/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include <QGraphicsRectItem>

#include "../connectoritem.h"

#ifndef TERMINALPOINTITEM_H_
#define TERMINALPOINTITEM_H_

class TerminalPointItem : public QGraphicsRectItem {
public:
	TerminalPointItem(ConnectorItem *parent);
	QPointF point();
	void updatePoint();

protected:
	void initPen();
	void drawCross();

	QPointF m_point;
	QPen m_linePen;
	QGraphicsLineItem *m_hLine;
	QGraphicsLineItem *m_vLine;

	static const qreal size;
};

#endif /* TERMINALPOINTITEM_H_ */
