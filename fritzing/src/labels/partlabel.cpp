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
#include <QTextFrameFormat>
#include <QTextFrame>
#include <QStyle>

// TODO:
//		** selection: coordinate with part selection: it's a layerkin
//		** select a part, highlight its label; click a label, highlight its part
//		** viewinfo update when selected
//		hover?
//		show = autoselect?
//		** viewinfo for wires
//		** graphics (esp. drag area vs. edit area) 
//		html info box needs to update when view switches
//		multiple selection?
//		undo delete
//		undo select
//		undo change text
//		** layers and z order
//		** hide and show layer
//		tools (bold, italic, color, size)?
//		** sync hide/show checkbox with visibility state
//		export to svg for export diy
//		save and load
//		** text color needs to be separate in separate views
//		** hide silkscreen should hide silkscreen label
//		** delete owner: delete label
//		rotate/flip (where is the control?)--heads up?

PartLabel::PartLabel(ItemBase * owner, const QString & text, QGraphicsItem * parent)
	: QGraphicsTextItem(text, parent)
{
	m_owner = owner;
	m_hidden = m_initialized = false;
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsMovable, false);					// don't move this in the standard QGraphicsItem way
	setTextInteractionFlags(Qt::TextEditorInteraction);
	setVisible(false);
	connect(document(), SIGNAL(contentsChanged()), this, SLOT(contentsChangedSlot()));
	m_viewLayerID = ViewLayer::UnknownLayer;
	setAcceptHoverEvents(true);
}

PartLabel::~PartLabel() {
	if (m_owner) {
		m_owner->clearPartLabel();
	}
}

void PartLabel::showLabel(bool showIt, ViewLayer * viewLayer, const QColor & textColor) {
	if (showIt == this->isVisible()) return;

	if (showIt && !m_initialized) {
		if (m_owner == NULL) return;
		if (m_owner->scene() == NULL) return;
		m_owner->scene()->addItem(this);
		this->setZValue(viewLayer->nextZ());
		m_viewLayerID = viewLayer->viewLayerID();
		QRectF br = m_owner->boundingRect();
		QPointF initial = m_owner->pos() + QPointF(br.width(), -QGraphicsTextItem::boundingRect().height());
		this->setPos(initial);
		m_offset = initial - m_owner->pos();
		setDefaultTextColor(textColor);
		m_initialized = true;
	}

	setVisible(showIt);
}

QRectF PartLabel::boundingRect() const
{
	QRectF br = QGraphicsTextItem::boundingRect();
	br.adjust(0, -5, 0, 0);
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
    scene()->clearSelection();
    m_owner->setSelected(true);

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

void PartLabel::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) 
{
	if (m_hidden) return;


	if (m_owner->isSelected()) {
		painter->save();
		QRectF r = boundingRect();
		QPen pen = painter->pen();
		pen.setWidth(1);
		pen.setColor(Qt::gray);
		painter->drawRect(r);
		r.setHeight(-r.top());
		painter->fillRect(r, QBrush(Qt::gray));
		painter->restore();
	}

	QStyleOptionGraphicsItem newOption(*option);
	newOption.state &= ~(QStyle::State_Selected | QStyle::State_HasFocus);
	QGraphicsTextItem::paint(painter, &newOption, widget);
}

void PartLabel::setHidden(bool hide) {
	if (!m_initialized) return;

	m_hidden = hide;
	setAcceptedMouseButtons(hide ? Qt::NoButton : Qt::LeftButton | Qt::MidButton | Qt::RightButton | Qt::XButton1 | Qt::XButton2);
	setAcceptHoverEvents(!hide);
	update();
}

ViewLayer::ViewLayerID PartLabel::viewLayerID() {
	return m_viewLayerID;
}

bool PartLabel::hidden() {
	return m_hidden;
}
