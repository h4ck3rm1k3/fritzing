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

#ifndef RESIZABLERECTITEM_H_
#define RESIZABLERECTITEM_H_

#include <QGraphicsRectItem>

class ResizableRectItem : public QGraphicsRectItem {
friend class ConnectorRectangle;
public:
	ResizableRectItem(QGraphicsItem *parent=0);

	virtual qreal minWidth();
	virtual qreal minHeight();

	virtual void resizeRect(qreal x, qreal y, qreal width, qreal height);
	virtual bool isResizable();
};

#endif /* RESIZABLERECTITEM_H_ */
