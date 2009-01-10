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

#include "partlabel.h"
#include "../itembase.h"
#include "../viewgeometry.h"

#include <QGraphicsScene>
#include <QTextDocument>

PartLabel::PartLabel(ItemBase * owner, const QString & text, QGraphicsItem * parent)
	: QGraphicsTextItem(text, parent)
{
	m_owner = owner;
	m_initialized = false;
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setTextInteractionFlags(Qt::TextEditorInteraction);
	connect(document(), SIGNAL(contentsChanged()), this, SLOT(contentsChangedSlot()));
}

void PartLabel::showLabel(bool showIt) {
	if (showIt == this->isVisible()) return;

	if (showIt && !m_initialized) {
		if (m_owner == NULL) return;
		if (m_owner->scene() == NULL) return;
		m_owner->scene()->addItem(this);
		QRectF br = m_owner->boundingRect();
		QPointF initial = m_owner->pos() + QPointF(br.width(), -QGraphicsTextItem::boundingRect().height());
		this->setPos(initial);
		m_offset = initial - m_owner->pos();
		m_initialized = true;
	}

	setVisible(showIt);
}

QRectF PartLabel::boundingRect() const
{
	QRectF br = QGraphicsTextItem::boundingRect();
	br.adjust(-8, -8, 8, 8);
	return br;
}

QPainterPath PartLabel::shape() const
{
	QRectF t = boundingRect();
	QPainterPath path;
	path.addRect(t);
    return path;
}

void PartLabel::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	m_doDrag = false;
	QRectF br = QGraphicsTextItem::boundingRect();
	QPointF p = event->pos();
	if (!br.contains(p)) {
		m_doDrag = true;
		m_initialPosition = pos();
		return;
	}

	QGraphicsTextItem::mousePressEvent(event);
}

void PartLabel::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (m_doDrag) {
		QPointF currentParentPos = mapToParent(mapFromScene(event->scenePos()));
		QPointF buttonDownParentPos = mapToParent(mapFromScene(event->buttonDownScenePos(Qt::LeftButton)));
		setPos(m_initialPosition + currentParentPos - buttonDownParentPos);
		m_offset = this->pos() - m_owner->pos();
		return;
	}

	QGraphicsTextItem::mouseMoveEvent(event);
}

void PartLabel::contentsChangedSlot() {
	if (m_owner) {
		m_owner->partLabelChanged(document()->toPlainText());
	}
}

void PartLabel::setPlainText(const QString & text) {
	// prevent unnecessary contentsChanged signals
	if (text.compare(document()->toPlainText()) == 0) return;

	QGraphicsTextItem::setPlainText(text);
}

bool PartLabel::initialized() {
	return m_initialized;
}

void PartLabel::ownerMoved(QPointF newPos) {
	this->setPos(m_offset + newPos);
}
