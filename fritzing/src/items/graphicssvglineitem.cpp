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
#include "../fsvgrenderer.h"


// note: copied most of this code directly from Qt's QGraphicsLineItem code
// so it may need updated with new versions > 4.4.0

////////////////////////////////////////

GraphicsSvgLineItem::GraphicsSvgLineItem( QGraphicsItem * parent ) 
	: QGraphicsSvgItem(parent)
{
	m_hasLine = false;
}

GraphicsSvgLineItem::~GraphicsSvgLineItem()
{
}

/*!
    Returns the item's pen, or a black solid 0-width pen if no pen has
    been set.

    \sa setPen()
*/
QPen GraphicsSvgLineItem::pen() const
{
    return m_pen;
}

/*!
    Sets the item's pen to \a pen. If no pen is set, the line will be painted
    using a black solid 0-width pen.

    \sa pen()
*/
void GraphicsSvgLineItem::setPen(const QPen &pen)
{
    prepareGeometryChange();
    m_pen = pen;
    update();
}

/*!
    Returns the item's line, or a null line if no line has been set.

    \sa setLine()
*/
QLineF GraphicsSvgLineItem::line() const
{
    return m_line;
}

/*!
    Sets the item's line to be the given \a line.

    \sa line()
*/
void GraphicsSvgLineItem::setLine(const QLineF &line)
{
	m_hasLine = true;
    if (m_line == line)
        return;
    prepareGeometryChange();
    m_line = line;
    update();
}

/*!
    \fn void GraphicsSvgLineItem::setLine(qreal x1, qreal y1, qreal x2, qreal y2)
    \overload

    Sets the item's line to be the line between (\a x1, \a y1) and (\a
    x2, \a y2).

    This is the same as calling \c {setLine(QLineF(x1, y1, x2, y2))}.
*/

/*!
    \reimp
*/
QRectF GraphicsSvgLineItem::boundingRect() const
{
	if (m_hasLine) {
	    if (m_pen.widthF() == 0.0) {
	        const qreal x1 = m_line.p1().x();
	        const qreal x2 = m_line.p2().x();
	        const qreal y1 = m_line.p1().y();
	        const qreal y2 = m_line.p2().y();
	        qreal lx = qMin(x1, x2);
	        qreal rx = qMax(x1, x2);
	        qreal ty = qMin(y1, y2);
	        qreal by = qMax(y1, y2);
	        return QRectF(lx, ty, rx - lx, by - ty);
	    }
	    return hoverShape().controlPointRect();
    }
    
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
	if (m_hasLine) {
	    QPainterPath path;
	    if (m_line == QLineF()) {
	        return path;
		}
				
	    path.moveTo(m_line.p1());
	    path.lineTo(m_line.p2());
	    return qt_graphicsItem_shapeFromPath(path, m_pen, 4);
	}
	
	return QGraphicsSvgItem::shape();
}


/*!
    \reimp
*/
QPainterPath GraphicsSvgLineItem::shape() const
{
	if (m_hasLine) {
	    QPainterPath path;
	    if (m_line == QLineF()) {
	        return path;
		}
	
	    path.moveTo(m_line.p1());
	    path.lineTo(m_line.p2());
	    return qt_graphicsItem_shapeFromPath(path, m_pen, 1);
    }


	if (!m_shape.isEmpty()) {
		return m_shape;
	}
    
    return QGraphicsSvgItem::shape();
}

/*!
    \reimp
*/
void GraphicsSvgLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	if (m_hasLine) {
	    if (option->state & QStyle::State_Selected) {	
			// draw this first because otherwise it seems to draw a dashed line down the middle
	        qt_graphicsItem_highlightSelected(this, painter, option, boundingRect(), hoverShape(), NULL);
        }
	    painter->setPen(m_pen);
	    painter->drawLine(getPaintLine());
	
    }
    else {
    	//QGraphicsSvgItem::paint(painter, option, widget);

		// Qt's SVG renderer's defaultSize is not correct when the svg has a fractional pixel size
		Q_UNUSED(widget);
		renderer()->render(painter, boundingRect());

		if (option->state & QStyle::State_Selected) {
			qt_graphicsItem_highlightSelected(this, painter, option, boundingRect(), hoverShape(), NULL);
		}
   	}
}

const QLineF & GraphicsSvgLineItem::getPaintLine() {
	return m_line;
}

QPainterPath GraphicsSvgLineItem::qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen, int multiplier)
{
    // We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
    // if we pass a value of 0.0 to QPainterPathStroker::setWidth()
    const qreal penWidthZero = qreal(0.00000001);

    if (path == QPainterPath())
        return path;
    QPainterPathStroker ps;
    ps.setCapStyle(pen.capStyle());
    //ps.setCapStyle(Qt::FlatCap);
    if (pen.widthF() <= 0.0)
        ps.setWidth(penWidthZero);
    else
        ps.setWidth(pen.widthF() * multiplier);

    ps.setJoinStyle(pen.joinStyle());
    ps.setMiterLimit(pen.miterLimit());
    QPainterPath p = ps.createStroke(path);
    p.addPath(path);
    return p;
}

void GraphicsSvgLineItem::qt_graphicsItem_highlightSelected(QGraphicsItem * item, QPainter *painter, const QStyleOptionGraphicsItem *option, const QRectF & boundingRect, const QPainterPath & path,
															HighlightSelectedCallback callback)
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
	if (callback) {
		(*callback)(item, painter, 0);
	}
	if (path.isEmpty()) {
		painter->drawRect(boundingRect.adjusted(pad, pad, -pad, -pad));
	}
	else {
		painter->drawPath(path);
	}

	painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
	if (callback) {
		(*callback)(item, painter, 1);
	}
	if (path.isEmpty()) {
		painter->drawRect(boundingRect.adjusted(pad, pad, -pad, -pad));
	}
	else {
		painter->drawPath(path);
	} 
}

bool GraphicsSvgLineItem::hasLine() {
	return m_hasLine;
}

void GraphicsSvgLineItem::setShape(QPainterPath & pp) {
	m_shape = qt_graphicsItem_shapeFromPath(pp, pen(), 1);
}
