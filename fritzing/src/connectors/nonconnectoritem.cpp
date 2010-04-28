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

#include "nonconnectoritem.h"

#include <QBrush>
#include <QPen>
#include <QColor>
#include <limits>

#include "../sketch/infographicsview.h"
#include "../debugdialog.h"
#include "../utils/graphicsutils.h"


/////////////////////////////////////////////////////////

NonConnectorItem::NonConnectorItem(ItemBase * attachedTo) : QGraphicsRectItem(attachedTo)
{
	m_radius = m_strokeWidth = 0;
	m_circular = false;
	m_hidden = false;
	m_attachedTo = attachedTo;
    setAcceptHoverEvents(false);

}

NonConnectorItem::~NonConnectorItem() {
}

ItemBase * NonConnectorItem::attachedTo() {
	return m_attachedTo;
}

void NonConnectorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	Q_UNUSED(event);
}

void NonConnectorItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
	Q_UNUSED(event);
}

void NonConnectorItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	Q_UNUSED(event);
}

void NonConnectorItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	Q_UNUSED(event);
}

void NonConnectorItem::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) {
	if (m_hidden  || !m_paint) return;

	painter->setOpacity(m_opacity);
	if (m_circular) {
		//DebugDialog::debug(QString("id:%1 w:%2 %3").arg(attachedToID()).arg(pen().width()).arg(pen().color().name()) );
		painter->setBrush(brush());
		if (m_negativePenWidth < 0) {
			int pw = m_negativePenWidth + 1;
			painter->setPen(Qt::NoPen);
			painter->drawEllipse(rect().adjusted(-pw, -pw, pw, pw));
		}
		else
		{
			painter->setPen(pen());
			painter->drawEllipse(rect());
		}
	}
	else if (!m_shape.isEmpty()) {
		painter->setBrush(brush());  
		painter->setPen(pen());
		painter->drawPath(m_shape);
	}
	else {
		QGraphicsRectItem::paint(painter, option, widget);
	}

}

void NonConnectorItem::setHidden(bool hide) {
	m_hidden = hide;
	this->update();
}

long NonConnectorItem::attachedToID() {
	if (attachedTo() == NULL) return -1;
	return attachedTo()->id();
}

const QString & NonConnectorItem::attachedToTitle() {
	if (attachedTo() == NULL) return ___emptyString___;
	return attachedTo()->title();
}

void NonConnectorItem::setCircular(bool circular) {
	m_circular = circular;
}

void NonConnectorItem::setRadius(qreal radius, qreal strokeWidth) {
	m_radius = radius;
	m_strokeWidth = strokeWidth;
}

qreal NonConnectorItem::radius() {
	return m_radius;
}

qreal NonConnectorItem::strokeWidth() {
	return m_strokeWidth;
}

QPainterPath NonConnectorItem::shape() const
{
	if (m_circular) {
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
	m_shape = GraphicsSvgLineItem::qt_graphicsItem_shapeFromPath(pp, pen(), 1);
}

