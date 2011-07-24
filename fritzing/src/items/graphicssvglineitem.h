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

#ifndef GRAPHICSSVGLINEITEM_H
#define GRAPHICSSVGLINEITEM_H

#include <QGraphicsSvgItem>
#include <QPen>
#include <QLine>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

// combines QGraphicsLineItem and QGraphicsSvgItem so all parts and wires can inherit from the same class

class GraphicsSvgLineItem : public QGraphicsSvgItem
{
Q_OBJECT
public:
	GraphicsSvgLineItem(QGraphicsItem * parent = 0);
    ~GraphicsSvgLineItem();

    QRectF boundingRect() const;
    virtual QPainterPath hoverShape() const;

public:
	static void qt_graphicsItem_highlightSelected(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRectF & boundingRect, const QPainterPath & path);

};



#endif
