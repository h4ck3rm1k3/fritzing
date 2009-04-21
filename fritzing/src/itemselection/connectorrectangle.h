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

class PartsEditorConnectorsConnectorItem;

class ConnectorRectangle : public QObject {
	Q_OBJECT

public:
	enum State {
		Normal = 0x00000,
		Highlighted = 0x00001,
		Hover = 0x00002,
		Selected = 0x00003
	};

	ConnectorRectangle(QGraphicsItem* owner, bool withHandlers = true);
	QGraphicsItem *owner();
	void resizeRect(qreal x1, qreal y1, qreal x2, qreal y2);
	bool isResizable();

	void resizingStarted();
	void resizingFinished();

	qreal currentScale();
	void setMinSize(qreal minWidth, qreal minHeight);

	void setHandlersVisible(bool visible);
	QRectF handlerRect(Qt::Corner corner);
	QRectF errorIconRect();
	void paint(QPainter *painter);

signals:
	void resizeSignal(qreal x1, qreal y1, qreal x2, qreal y2);
	void isResizableSignal(bool & resizable);

protected:
	void setHandlerRect(CornerHandler* handler);
	void placeHandlers();

	QGraphicsItem *m_owner;

	CornerHandler *m_topLeftHandler;
	CornerHandler *m_topRightHandler;
	CornerHandler *m_bottomRightHandler;
	CornerHandler *m_bottomLeftHandler;
	QList<CornerHandler*> m_cornerHandlers;

	RectangleSide *m_topSide;
	RectangleSide *m_rightSide;
	RectangleSide *m_leftSide;
	RectangleSide *m_bottomSide;

	qreal m_minWidth;
	qreal m_minHeight;

	bool m_firstPaint;

public:
	static qreal ErrorIconSize;
};

#endif /* CONNECTORRECTANGLE_H_ */
