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

#ifndef GRAPHICSSVGLINEITEM_H
#define GRAPHICSSVGLINEITEM_H

#include <QGraphicsSvgItem>
#include <QPen>
#include <QLine>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

typedef void (*HighlightSelectedCallback)(QGraphicsItem *, QPainter * painter, int step);

// combines QGraphicsLineItem and QGraphicsSvgItem so all parts and wires can inherit from the same class

class GraphicsSvgLineItem : public QGraphicsSvgItem
{
Q_OBJECT
public:
	GraphicsSvgLineItem(QGraphicsItem * parent = 0);
    ~GraphicsSvgLineItem();

    QPen pen() const;
    void setPen(const QPen &pen);

    QLineF line() const;
    virtual void setLine(const QLineF &line);
    inline void setLine(qreal x1, qreal y1, qreal x2, qreal y2)
    	{ setLine(QLineF(x1, y1, x2, y2)); }

    QRectF boundingRect() const;
    QPainterPath shape() const;
    QPainterPath hoverShape() const;
	void setShape(QPainterPath &);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

	bool hasLine();
	virtual const QLineF & getPaintLine();

public:
	static void qt_graphicsItem_highlightSelected(QGraphicsItem *, QPainter *painter, const QStyleOptionGraphicsItem *option, const QRectF & boundingRect, const QPainterPath & path, HighlightSelectedCallback);
	static QPainterPath qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen, int multiplier);

protected:
	QLineF	m_line;
	QPen	m_pen;	
	bool	m_hasLine;
	QPainterPath m_shape;
};



#endif
