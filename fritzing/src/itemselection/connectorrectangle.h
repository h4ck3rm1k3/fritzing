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

#ifndef CONNECTORRECTANGLE_H_
#define CONNECTORRECTANGLE_H_

#include "cornerhandler.h"
#include "rectangleside.h"
#include "resizablerectitem.h"

class PartsEditorConnectorsConnectorItem;

class ConnectorRectangle {
public:
	enum State {
		Normal = 0x00000,
		Highlighted = 0x00001,
		Hover = 0x00002,
		Selected = 0x00003
	};

	ConnectorRectangle(PartsEditorConnectorsConnectorItem* owner, bool withHandlers = true);
	QGraphicsRectItem *owner();
	void prepareForChange();
	void resizeRect(qreal x1, qreal y1, qreal x2, qreal y2);
	bool isResizable();

	void resizingStarted();
	void resizingFinished();

	qreal currentScale();

	void setHandlersVisible(bool visible);
	void paint(QPainter *painter);

protected:
	void setHandlerRect(CornerHandler* handler);
	QRectF handlerRect(Qt::Corner corner);
	void placeHandlers();
	ResizableRectItem* resizableOwner();

	PartsEditorConnectorsConnectorItem *m_owner;

	CornerHandler *m_topLeftHandler;
	CornerHandler *m_topRightHandler;
	CornerHandler *m_bottomRightHandler;
	CornerHandler *m_bottomLeftHandler;
	QList<CornerHandler*> m_cornerHandlers;

	RectangleSide *m_topSide;
	RectangleSide *m_rightSide;
	RectangleSide *m_leftSide;
	RectangleSide *m_bottomSide;

	bool m_firstPaint;
	bool m_isVisible;
};

#endif /* CONNECTORRECTANGLE_H_ */
