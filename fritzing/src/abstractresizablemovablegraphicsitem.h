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


#ifndef ABSTRACTRESIZABLEMOVABLEGRAPHICSITEM_H_
#define ABSTRACTRESIZABLEMOVABLEGRAPHICSITEM_H_

#include <QPointF>
#include <QRectF>
#include <QCursor>

class AbstractResizableMovableGraphicsItem {
public:
	enum Position {
		TopLeftCorner = 0x00000,
		TopRightCorner = 0x00001,
		BottomLeftCorner = 0x00002,
		BottomRightCorner = 0x00003,
		Inside = 0x00004,
		Outside = 0x00005
	};

	AbstractResizableMovableGraphicsItem();

	void setResizable(bool resizable);
	void setMovable(bool movable);

protected:
	void resize(const QPointF &mousePos);
	void move(const QPointF &newPos);

	Position updateCursor(const QPointF &mousePos, const QCursor &defaultCursor=QCursor());
	Position closeToCorner(const QPointF &pos);

	virtual void doMoveBy(qreal dx, qreal dy) = 0;
	virtual QPointF map(const QPointF &point) const = 0;
	virtual void prepareForChange() = 0;
	virtual QRectF rectAux() const = 0;
	virtual void setRectAux(qreal x1, qreal y1, qreal x2, qreal y2);
	virtual void setCursorAux(const QCursor &cursor) = 0;

protected:
	bool m_resizable;
	bool m_movable;
	volatile bool m_resizing;
	volatile bool m_moving;
	volatile Position m_mousePosition;
	QPointF m_mousePressedPos;
};

#endif /* ABSTRACTRESIZABLEMOVABLEGRAPHICSITEM_H_ */
