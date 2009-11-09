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

$Revision$:
$Author$:
$Date$

********************************************************************/

#ifndef INFOGRAPHICSVIEW_H
#define INFOGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMenu>
#include <QHash>
#include <QList>

#include "items/itembase.h"
#include "zoomablegraphicsview.h"

class InfoGraphicsView : public ZoomableGraphicsView
{
	Q_OBJECT

public:
	InfoGraphicsView(QWidget* parent = 0);

	void viewItemInfo(ItemBase * item);
	virtual void hoverEnterItem(QGraphicsSceneHoverEvent * event, ItemBase * item);
	virtual void hoverEnterItem(ModelPart * modelPart);
	virtual void hoverLeaveItem(QGraphicsSceneHoverEvent * event, ItemBase * item);
	virtual void hoverLeaveItem(ModelPart * modelPart);

	virtual bool swappingEnabled(ItemBase *) = 0;

	void viewConnectorItemInfo(ConnectorItem * item);
	virtual void hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	virtual void hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item);

	void setInfoView(class HtmlInfoView *);
	class HtmlInfoView * infoView();

	virtual void mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);

	virtual void partLabelChanged(ItemBase *, const QString &oldText, const QString & newText, QSizeF oldSize, QSizeF newSize, bool isLabel);
	virtual void partLabelMoved(ItemBase *, QPointF oldPos, QPointF oldOffset, QPointF newPos, QPointF newOffset);
	virtual void rotateFlipPartLabel(ItemBase *, qreal degrees, Qt::Orientations flipDirection);
	virtual void noteSizeChanged(ItemBase * itemBase, const QRectF & oldRect, const QRectF & newRect);

	virtual bool spaceBarIsPressed(); 
	virtual void initWire(class Wire *, int penWidth);

	QVariant evaluateJavascript(const QString &);

	virtual void setIgnoreSelectionChangeEvents(bool) {}
	virtual void getBendpointWidths(class Wire *, qreal w, qreal & w1, qreal & w2);
	virtual void getLabelFont(QFont &, QColor &);
	virtual qreal getLabelFontSizeSmall();
	virtual qreal getLabelFontSizeMedium();
	virtual qreal getLabelFontSizeLarge();
	virtual bool hasBigDots();

public:
	static InfoGraphicsView * getInfoGraphicsView(QGraphicsItem *);

protected:
	QGraphicsItem *selectedAux();
	class HtmlInfoView *m_infoView;

};

#endif
