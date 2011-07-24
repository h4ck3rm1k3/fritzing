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

#include "graphicssvglineitem.h"
#include "../utils/misc.h"
#include "../utils/graphicsutils.h"
#include "../fsvgrenderer.h"

////////////////////////////////////////

GraphicsSvgLineItem::GraphicsSvgLineItem( QGraphicsItem * parent ) 
	: QGraphicsSvgItem(parent)
{
}

GraphicsSvgLineItem::~GraphicsSvgLineItem()
{
}

QRectF GraphicsSvgLineItem::boundingRect() const
{    
	FSvgRenderer * frenderer = dynamic_cast<FSvgRenderer *>(this->renderer());
	if (frenderer == NULL) {
		return QGraphicsSvgItem::boundingRect();
	}

	QSizeF s = frenderer->defaultSizeF();
	QRectF r(0,0, s.width(), s.height());
	return r;
}

QPainterPath GraphicsSvgLineItem::hoverShape() const
{	
	return shape();
}

void GraphicsSvgLineItem::qt_graphicsItem_highlightSelected(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRectF & boundingRect, const QPainterPath & path)
{
	/*
	QLineF l1 = item->line();
	QLineF l2 =l1.normalVector();
	l2.setLength(20);
	l2.translate(-l2.dx() / 2, -l2.dy() / 2);
	QLineF l3(l2);
	l3.translate(l1.dx(), l1.dy());
	
	QPointF points[5];
	points[0] = l2.p1();
	points[1] = l2.p2();
	points[2] = l3.p2();
	points[3] = l3.p1();
	points[4] = l2.p1();
	
    const qreal penWidth = 0; // cosmetic pen
	
    const QColor fgcolor = option->palette.windowText().color();
    const QColor bgcolor( // ensure good contrast against fgcolor
						 fgcolor.red()   > 127 ? 0 : 255,
						 fgcolor.green() > 127 ? 0 : 255,
						 fgcolor.blue()  > 127 ? 0 : 255);
		
    painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
    painter->setBrush(Qt::NoBrush);
	painter->drawPolyline(points, 5);
	
    painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
	painter->drawPolyline(points, 5);
	
	return;
	*/
	
    const QRectF murect = painter->transform().mapRect(QRectF(0, 0, 1, 1));
    if (qFuzzyCompare(qMax(murect.width(), murect.height()) + 1, 1))
        return;

    const QRectF mbrect = painter->transform().mapRect(boundingRect);
    if (qMin(mbrect.width(), mbrect.height()) < qreal(1.0))
        return;

    qreal itemPenWidth = 1.0;
	const qreal pad = itemPenWidth / 2;
    const qreal penWidth = 0; // cosmetic pen

    const QColor fgcolor = option->palette.windowText().color();
    const QColor bgcolor( // ensure good contrast against fgcolor
        fgcolor.red()   > 127 ? 0 : 255,
        fgcolor.green() > 127 ? 0 : 255,
        fgcolor.blue()  > 127 ? 0 : 255);

    painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
    painter->setBrush(Qt::NoBrush);
	if (path.isEmpty()) {
		painter->drawRect(boundingRect.adjusted(pad, pad, -pad, -pad));
	}
	else {
		painter->drawPath(path);
	}

	painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
	if (path.isEmpty()) {
		painter->drawRect(boundingRect.adjusted(pad, pad, -pad, -pad));
	}
	else {
		painter->drawPath(path);
	} 
}
