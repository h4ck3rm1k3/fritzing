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
#include "../debugdialog.h"
#include "../infographicsview.h"

#include <QGraphicsScene>
#include <QTextDocument>
#include <QTextFrameFormat>
#include <QTextFrame>
#include <QStyle>
#include <QMenu>
#include <QApplication>

// TODO:
//		** selection: coordinate with part selection: it's a layerkin
//		** select a part, highlight its label; click a label, highlight its part
//		** viewinfo update when selected
//		** viewinfo for wires
//		** graphics (esp. drag area vs. edit area) 
//		** html info box needs to update when view switches
//		** undo delete text
//		** undo change text
//		** undo move
//		** layers and z order
//		** hide and show layer
//		** sync hide/show checkbox with visibility state
//		** save and load
//		** text color needs to be separate in separate views
//		** hide silkscreen should hide silkscreen label
//		** delete owner: delete label
//		** make label single-line (ignore enter key)

//		** rotate/flip 
//		** undo rotate/flip
//		format: bold, italic, size (small normal large huge), color?, 
//		undo format
//		heads-up controls

//		rotate/flip may need to be relative to part?
//		copy/paste?
//		z-order manipulation?
//		hover?
//		show = autoselect?
//		undo delete show?
//		close focus on enter/return?

//		-- multiple selection?
//		-- undo select
//		-- export to svg for export diy (silkscreen layer is not exported)


enum PartLabelTransformation {
	PartLabelRotate90CW = 1,
	PartLabelRotate180,
	PartLabelRotate90CCW,
	PartLabelFlipHorizontal,
	PartLabelFlipVertical	
};

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
	m_spaceBarWasPressed = false;

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
	InfoGraphicsView *infographics = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
	if (infographics != NULL && infographics->spaceBarIsPressed()) {
		m_spaceBarWasPressed = true;
		event->ignore();
		return;
	}

	m_spaceBarWasPressed = false;
	scene()->clearSelection();
    m_owner->setSelected(true);

	m_doDrag = false;

	// don't seem to get a contextMenuEvent in the drag area of the label, so fake it for now
	// (since this is all only temporary anyway)
	if (event->button() == Qt::RightButton && event->pos().y() <= 0) {
		temporaryMenuEvent(event);
		return;
	}

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
	if (m_spaceBarWasPressed) {
		event->ignore();
		return;
	}

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
	if (m_spaceBarWasPressed) {
		event->ignore();
		return;
	}

	if (m_doDrag) {
		m_owner->partLabelMoved(m_initialPosition, m_initialOffset, pos(), m_offset);
	}

	QGraphicsTextItem::mouseReleaseEvent(event);
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

void PartLabel::keyPressEvent(QKeyEvent * event)
{
	switch (event->key()) {
		case Qt::Key_Return:
		case Qt::Key_Enter:
			event->ignore();
			return;
		default:
			QGraphicsTextItem::keyPressEvent(event);
			break;
	}
}

void PartLabel::keyReleaseEvent(QKeyEvent * event)
{
	switch (event->key()) {
		case Qt::Key_Return:
		case Qt::Key_Enter:
			event->ignore();
			return;
		default:
			QGraphicsTextItem::keyPressEvent(event);
			break;
	}
}

void PartLabel::temporaryMenuEvent(QGraphicsSceneMouseEvent * event) {

	QMenu menu;
    QAction *rotate90cwAct = menu.addAction(tr("&Rotate 90\x00B0 Clockwise"));
	rotate90cwAct->setData(QVariant(PartLabelRotate90CW));
	rotate90cwAct->setStatusTip(tr("Rotate the selected parts by 90 degrees clockwise"));

 	QAction *rotate180Act = menu.addAction(tr("&Rotate 180\x00B0"));
	rotate180Act->setData(QVariant(PartLabelRotate180));
	rotate180Act->setStatusTip(tr("Rotate the selected parts by 180 degrees"));
   
	QAction *rotate90ccwAct = menu.addAction(tr("&Rotate 90\x00B0 Counter Clockwise"));
	rotate90ccwAct->setData(QVariant(PartLabelRotate90CCW));
	rotate90ccwAct->setStatusTip(tr("Rotate current selection 90 degrees counter clockwise"));
	
	QAction *flipHorizontalAct = menu.addAction(tr("&Flip Horizontal"));
	flipHorizontalAct->setData(QVariant(PartLabelFlipHorizontal));
	flipHorizontalAct->setStatusTip(tr("Flip current selection horizontally"));

	QAction *flipVerticalAct = menu.addAction(tr("&Flip Vertical"));
	flipVerticalAct->setData(QVariant(PartLabelFlipVertical));
	flipVerticalAct->setStatusTip(tr("Flip current selection vertically"));

	//menu.addSeparator();


	
	QAction *selectedAction = menu.exec(event->screenPos());
	if (selectedAction == NULL) return;

	Qt::Orientations orientation = 0;
	qreal degrees = 0;
	switch ((PartLabelTransformation) selectedAction->data().toInt()) {
		case PartLabelRotate90CW:
			degrees = 90;
			break;
		case PartLabelRotate90CCW:
			degrees = 270;
			break;
		case PartLabelRotate180:
			degrees = 180;
			break;
		case PartLabelFlipHorizontal:
			orientation = Qt::Horizontal;
			break;
		case PartLabelFlipVertical:
			orientation = Qt::Vertical;
			break;
	}

	m_owner->rotateFlipPartLabel(degrees, orientation);
}

void PartLabel::rotateFlipLabel(qreal degrees, Qt::Orientations orientation) {
	if (degrees != 0) {
		transformLabel(QTransform().rotate(degrees));
	}
	else {
		int xScale, yScale;
		if (orientation == Qt::Vertical) {
			xScale = 1;
			yScale = -1;
		} 
		else if(orientation == Qt::Horizontal) {
			xScale = -1;
			yScale = 1;
		}
		else return;
		transformLabel(QTransform().scale(xScale,yScale));
	}
}

void PartLabel::transformLabel(QTransform currTransf) 
{
	QRectF rect = this->boundingRect();
	qreal x = rect.width() / 2;
	qreal y = rect.height() / 2;
	QTransform transf = transform() * QTransform().translate(-x, -y) * currTransf * QTransform().translate(x, y);
	setTransform(transf);
}

void PartLabel::focusInEvent(QFocusEvent * event) {
	QApplication::instance()->installEventFilter(this);
	QGraphicsTextItem::focusInEvent(event);
}

void PartLabel::focusOutEvent(QFocusEvent * event) {
	QApplication::instance()->removeEventFilter(this);
	QGraphicsTextItem::focusOutEvent(event);
}

bool PartLabel::eventFilter(QObject * object, QEvent * event) 
{
	if (event->type() == QEvent::Shortcut || event->type() == QEvent::ShortcutOverride)
	{
		if (!object->inherits("QGraphicsView"))
		{
			event->accept();
			return true;
		}
	}
	return false;
}