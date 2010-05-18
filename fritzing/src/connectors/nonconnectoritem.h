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

#ifndef NONCONNECTORITEM_H
#define NONCONNECTORITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneHoverEvent>
#include <QPen>
#include <QBrush>
#include <QXmlStreamWriter>
#include <QPointer>

#include "../items/itembase.h"

class NonConnectorItem : public QObject, public QGraphicsRectItem
{
Q_OBJECT

public:
	NonConnectorItem(ItemBase* attachedTo);
	~NonConnectorItem();

	ItemBase * attachedTo();
	virtual void setHidden(bool hidden);
	bool hidden();
	long attachedToID();
	const QString & attachedToTitle();
	void setCircular(bool);
	void setRadius(qreal radius, qreal strokeWidth);
	qreal radius();
	qreal strokeWidth();
	void setShape(QPainterPath &);
	void setWhite(bool);

protected:
	void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
	QPainterPath shape() const;

protected:
	QPointer<ItemBase> m_attachedTo;
	bool m_hidden;
	bool m_paint;
	qreal m_opacity;
	bool m_circular;
	qreal m_radius;
	qreal m_strokeWidth;
	qreal m_negativePenWidth;
	QPainterPath m_shape;
	bool m_white;
	
};

#endif
