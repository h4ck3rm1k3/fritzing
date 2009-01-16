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
//		** html info box needs to update when view switches
//		-- multiple selection?
//		** undo delete text
//		undo delete show?
//		-- undo select
//		** undo change text
//		undo move
//		** layers and z order
//		** hide and show layer
//		tools (bold, italic, color, size)?
//		** sync hide/show checkbox with visibility state
//		-- export to svg for export diy (silkscreen layer is not exported)
//		** save and load
//		** text color needs to be separate in separate views
//		** hide silkscreen should hide silkscreen label
//		** delete owner: delete label
//		rotate/flip (where is the control?)--heads up?  label menu for the time being
//		make label single-line (ignore or trigger edit-done on pressing enter)
//		undo rotate/flip
//		copy/paste?
//		z-order manipulation?

/////////////////////////////////////////////

PartLabelTextDocument::PartLabelTextDocument(long id, QObject * parent) : QTextDocument(parent) 
{
	m_id = id;
	m_refCount = 0;
}

void PartLabelTextDocument::addRef() {
	m_refCount++;
}

void PartLabelTextDocument::decRef() {
	if (--m_refCount <= 0) {
		AllTextDocuments.remove(m_id);
		deleteLater();
	}
}

QHash<long, PartLabelTextDocument *> PartLabelTextDocument::AllTextDocuments;

///////////////////////////////////////////

PartLabel::PartLabel(ItemBase * owner, const QString & text, QGraphicsItem * parent)
	: QGraphicsTextItem(text, parent)
{
	m_owner = owner;

	PartLabelTextDocument * doc = PartLabelTextDocument::AllTextDocuments.value(owner->id());
	if (doc == NULL) {
		doc = new PartLabelTextDocument(owner->id(), NULL);
		PartLabelTextDocument::AllTextDocuments.insert(owner->id(), doc);
		doc->setUndoRedoEnabled(true);							
	}
	doc->addRef();
	setDocument(doc);
	connect(doc, SIGNAL(contentsChanged()), this, SLOT(contentsChangedSlot()), Qt::DirectConnection);

	m_hidden = m_initialized = false;
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsMovable, false);					// don't move this in the standard QGraphicsItem way
	setTextInteractionFlags(Qt::TextEditorInteraction);
	setVisible(false);
	m_viewLayerID = ViewLayer::UnknownLayer;
	setAcceptHoverEvents(true);
}

PartLabel::~PartLabel() 
{
	PartLabelTextDocument * doc = dynamic_cast<PartLabelTextDocument *>(document());
	if (doc) {
		doc->decRef();
	}
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
		m_initialOffset = m_offset;
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

void PartLabel::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if (m_doDrag) {
		m_owner->partLabelMoved(m_initialPosition, m_initialOffset, pos(), m_offset);
	}
}

void PartLabel::contentsChangedSlot() {
	if (m_owner) {
		m_owner->partLabelChanged(document()->toPlainText());
	}
}

void PartLabel::setPlainText(const QString & text) 
{
	// prevent unnecessary contentsChanged signals
	if (text.compare(document()->toPlainText()) == 0) return;

	document()->blockSignals(true);
	QGraphicsTextItem::setPlainText(text);
	document()->blockSignals(false);
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

void PartLabel::saveInstance(QXmlStreamWriter & streamWriter) {
	if (!m_initialized) return;

	streamWriter.writeStartElement("titleGeometry");
	streamWriter.writeAttribute("visible", isVisible() ? "true" : "false");
	streamWriter.writeAttribute("x", QString::number(pos().x()));
	streamWriter.writeAttribute("y", QString::number(pos().y()));
	streamWriter.writeAttribute("z", QString::number(zValue()));
	streamWriter.writeAttribute("xOffset", QString::number(m_offset.x()));
	streamWriter.writeAttribute("yOffset", QString::number(m_offset.y()));
	streamWriter.writeAttribute("textColor", defaultTextColor().name());
	streamWriter.writeEndElement();
}

void PartLabel::restoreLabel(QDomElement & labelGeometry, ViewLayer::ViewLayerID viewLayerID) 
{
	m_viewLayerID = viewLayerID;
	m_initialized = true;
	m_owner->scene()->addItem(this);
	setVisible(labelGeometry.attribute("visible").compare("true") == 0);
	QPointF p = pos();
	bool ok = false;
	qreal x = labelGeometry.attribute("x").toDouble(&ok);
	if (ok) p.setX(x);
	qreal y = labelGeometry.attribute("y").toDouble(&ok);
	if (ok) p.setY(y);
	setPos(p);
	x = labelGeometry.attribute("xOffset").toDouble(&ok);
	if (ok) m_offset.setX(x);
	y = labelGeometry.attribute("yOffset").toDouble(&ok);
	if (ok) m_offset.setY(y);
	qreal z = labelGeometry.attribute("z").toDouble(&ok);
	if (ok) this->setZValue(z);

	QColor c;
	c.setNamedColor(labelGeometry.attribute("textColor"));
	setDefaultTextColor(c);
}

void PartLabel::moveLabel(QPointF newPos, QPointF newOffset) 
{
	this->setPos(newPos);
	m_offset = newOffset;
}

ItemBase * PartLabel::owner() {
	return m_owner;
}


