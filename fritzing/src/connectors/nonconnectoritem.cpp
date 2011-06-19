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

#include "nonconnectoritem.h"

#include <QBrush>
#include <QPen>
#include <QColor>
#include <limits>

#include "../sketch/infographicsview.h"
#include "../debugdialog.h"
#include "../utils/graphicsutils.h"

//static const qreal EffectiveAdjustment = 1.25;
static const qreal EffectiveAdjustmentFactor = 5.0 / 15.0;

/////////////////////////////////////////////////////////

NonConnectorItem::NonConnectorItem(ItemBase * attachedTo) : QGraphicsRectItem(attachedTo)
{
	m_radius = m_strokeWidth = 0;
	m_effectivelyCircular = m_effectivelyRectangular = m_circular = false;
	m_inactive = m_hidden = false;
	m_attachedTo = attachedTo;
    setAcceptHoverEvents(false);
	setAcceptedMouseButtons(Qt::NoButton);
	setFlag(QGraphicsItem::ItemIsMovable, false);
	setFlag(QGraphicsItem::ItemIsSelectable, false);
	setFlag(QGraphicsItem::ItemIsFocusable, false);
}

NonConnectorItem::~NonConnectorItem() {
}

ItemBase * NonConnectorItem::attachedTo() {
	return m_attachedTo;
}

void NonConnectorItem::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) {

	if (m_hidden || m_inactive || !m_paint) return;

	painter->setOpacity(m_opacity);

	/*
	DebugDialog::debug(QString("id:%1 %2 w:%3 %4 c:%5 ec:%6 er:%7 neg:%8 w:%9")
		.arg(attachedToID())
		.arg(attachedToTitle())
		.arg(pen().width())
		.arg(pen().color().name()) 
		.arg(m_circular)
		.arg(m_effectivelyCircular)
		.arg(m_effectivelyRectangular)
		.arg(m_negativePenWidth)
		.arg(rect().width())
		);
	*/

	if (m_circular) {
		painter->setBrush(brush());
		if (m_negativePenWidth < 0) {
			// for wires
			painter->setPen(Qt::NoPen);
			if (!m_negativeOffsetRect) {
				painter->drawEllipse(rect().center(), m_negativePenWidth, m_negativePenWidth);
			}
			else {
				int pw = m_negativePenWidth + 1;
				painter->drawEllipse(rect().adjusted(-pw, -pw, pw, pw));
			}
		}
		else
		{
			// for parts
			QRectF r = rect();
			qreal delta = .66 * m_strokeWidth;
			painter->setPen(pen());
			painter->drawEllipse(r.adjusted(delta, delta, -delta, -delta));
		}
	}
	else if (!m_shape.isEmpty()) {
		painter->setBrush(brush());  
		painter->setPen(pen());
		painter->drawPath(m_shape);
	}
	else if (m_effectivelyCircular) {
		painter->setBrush(brush());  
		painter->setPen(pen());
		QRectF r = rect();
		qreal delta = r.width() * EffectiveAdjustmentFactor;
		painter->drawEllipse(r.adjusted(delta, delta, -delta, -delta));
	}
	else if (m_effectivelyRectangular) {
		painter->setBrush(brush());  
		painter->setPen(pen());
		QRectF r = rect();
		qreal delta = qMin(r.width(), r.height()) * EffectiveAdjustmentFactor;
		painter->drawRect(r.adjusted(delta, delta, -delta, -delta));
	}
	else {
		QGraphicsRectItem::paint(painter, option, widget);
	}

}

void NonConnectorItem::setHidden(bool hide) {
	m_hidden = hide;
	this->update();
}

bool NonConnectorItem::hidden() {
	return m_hidden;
}

void NonConnectorItem::setInactive(bool inactivate) {
	m_inactive = inactivate;
	this->update();
}

bool NonConnectorItem::inactive() {
	return m_inactive;
}

long NonConnectorItem::attachedToID() {
	if (attachedTo() == NULL) return -1;
	return attachedTo()->id();
}

const QString & NonConnectorItem::attachedToTitle() {
	if (attachedTo() == NULL) return ___emptyString___;
	return attachedTo()->title();
}

const QString & NonConnectorItem::attachedToInstanceTitle() {
	if (attachedTo() == NULL) return ___emptyString___;
	return attachedTo()->instanceTitle();
}

void NonConnectorItem::setCircular(bool circular) {
	m_circular = circular;
}

void NonConnectorItem::setRadius(qreal radius, qreal strokeWidth) {
	m_radius = radius;
	m_strokeWidth = strokeWidth;
	m_circular = (m_radius > 0);
}

qreal NonConnectorItem::radius() {
	return m_radius;
}

qreal NonConnectorItem::strokeWidth() {
	return m_strokeWidth;
}

QPainterPath NonConnectorItem::shape() const
{
	if (m_circular || m_effectivelyCircular) {
		QPainterPath path;
		path.addEllipse(rect());
		return GraphicsSvgLineItem::qt_graphicsItem_shapeFromPath(path, pen(), 1);
	}
	else if (!m_shape.isEmpty()) {
		return m_shape;
	}

	return QGraphicsRectItem::shape();
}

void NonConnectorItem::setShape(QPainterPath & pp) {
	// so far only used by GroundPlane
	m_shape = GraphicsSvgLineItem::qt_graphicsItem_shapeFromPath(pp, pen(), 1);
}
