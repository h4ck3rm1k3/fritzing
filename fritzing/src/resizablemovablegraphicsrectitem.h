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

$Revision: 2102 $:
$Author: merunga $:
$Date: 2009-01-07 17:51:32 +0100 (Wed, 07 Jan 2009) $

********************************************************************/

#ifndef RESIZABLEMOVABLEGRAPHICSRECTITEM_H_
#define RESIZABLEMOVABLEGRAPHICSRECTITEM_H_

#include <QGraphicsRectItem>
#include "abstractresizablemovablegraphicsitem.h"

class ResizableMovableGraphicsRectItem : public QGraphicsRectItem, public AbstractResizableMovableGraphicsItem {
public:
	ResizableMovableGraphicsRectItem(QGraphicsItem *parent=0);

protected:
	QPointF map(const QPointF &point) const;
	QRectF rectAux() const;
	void doMoveBy(qreal dx, qreal dy);
	void prepareForChange();
	void setCursorAux(const QCursor &cursor);
};

#endif /* RESIZABLEMOVABLEGRAPHICSRECTITEM_H_ */
