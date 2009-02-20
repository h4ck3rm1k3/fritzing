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

#include "note.h"
#include "../graphicssvglineitem.h"
#include "../debugdialog.h"
#include "../infographicsview.h"
 
#include <QTextFrame>
#include <QTextFrameFormat>

// TODO:
//		** search for ModelPart:: and fix up
//		check which menu items don't apply
//		** selection
//		** delete
//		** move
//		** undo delete + text
//		** resize
//		** undo resize
//		anchor	
//		** undo change text
//		** undo selection
//		** undo move
//		** layers and z order
//		** hide and show layer
//		** save and load
//		format: bold, italic, size (small normal large huge), color?, 
//		undo format
//		heads-up controls
//		copy/paste
//		** z-order manipulation
//		hover
//		** multiple selection
//		** icon in taskbar (why does it show up as text until you update it?)

QString Note::moduleIDName = "NoteModuleID";
const int Note::emptyMinWidth = 150;
const int Note::emptyMinHeight = 150;
const int borderWidth = 3;

QString Note::initialTextString;

/////////////////////////////////////////////

Note::Note( ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu* itemMenu)
	: ItemBase(modelPart, viewIdentifier, viewGeometry, id, itemMenu)
{
	if (initialTextString.isEmpty()) {
		initialTextString = tr("[write your note here]");
	}

	m_inResize = false;
	this->setCursor(Qt::CrossCursor);

    setFlags(QGraphicsItem::ItemIsSelectable );

	m_rect.setRect(0, 0, viewGeometry.rect().width(), viewGeometry.rect().height());
	m_pen.setWidth(borderWidth);
	m_pen.setBrush(QColor(0xff, 0xd5, 0x0e));

	m_brush.setColor(QColor(0xfb, 0xf7, 0xab));
	m_brush.setStyle(Qt::SolidPattern);

	setPos(m_viewGeometry.loc());

	m_resizeGrip = new QGraphicsPixmapItem(QPixmap(":/resources/images/icons/noteResizeGrip.png"));
	m_resizeGrip->setParentItem(this);
	m_resizeGrip->setCursor(Qt::SizeFDiagCursor);
	m_resizeGrip->setVisible(true);

	m_graphicsTextItem = new QGraphicsTextItem();
	m_graphicsTextItem->setParentItem(this);
	m_graphicsTextItem->setVisible(true);
	m_graphicsTextItem->setPlainText(initialTextString);
	m_graphicsTextItem->setTextInteractionFlags(Qt::TextEditorInteraction);
	m_graphicsTextItem->setCursor(Qt::IBeamCursor);

	connect(m_graphicsTextItem->document(), SIGNAL(contentsChanged()), 
		this, SLOT(contentsChangedSlot()), Qt::DirectConnection);

	positionGrip();	

	setAcceptHoverEvents(true);
}

void Note::saveGeometry() {
	m_viewGeometry.setRect(boundingRect());
	m_viewGeometry.setLoc(this->pos());
	m_viewGeometry.setSelected(this->isSelected());
	m_viewGeometry.setZ(this->zValue());
}

bool Note::itemMoved() {
	return (this->pos() != m_viewGeometry.loc());
}

void Note::saveInstanceLocation(QXmlStreamWriter & streamWriter) {
	QRectF rect = m_viewGeometry.rect();
	QPointF loc = m_viewGeometry.loc();
	streamWriter.writeAttribute("x", QString::number(loc.x()));
	streamWriter.writeAttribute("y", QString::number(loc.y()));
	streamWriter.writeAttribute("width", QString::number(rect.width()));
	streamWriter.writeAttribute("height", QString::number(rect.height()));
}

void Note::moveItem(ViewGeometry & viewGeometry) {
	this->setPos(viewGeometry.loc());
}

void Note::findConnectorsUnder() {
}

void Note::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(widget);

	if (m_hidden) return;

	painter->setPen(m_pen);
	painter->setBrush(m_brush);
    painter->drawRect(m_rect);

	if (option->state & QStyle::State_Selected) {	
		GraphicsSvgLineItem::qt_graphicsItem_highlightSelected(this, painter, option, boundingRect(), QPainterPath(), NULL);
    }
}

QRectF Note::boundingRect() const
{
	return m_rect;
}

QPainterPath Note::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void Note::positionGrip() {
	QSizeF gripSize = m_resizeGrip->boundingRect().size();
	QSizeF sz = this->boundingRect().size() - gripSize;
	QPointF p(sz.width(), sz.height());
	m_resizeGrip->setPos(p);
	m_graphicsTextItem->setPos(gripSize.width(), gripSize.height());
	m_graphicsTextItem->setTextWidth(sz.width() - gripSize.width());
}

void Note::mousePressEvent(QGraphicsSceneMouseEvent * event) {
	QPointF p = m_resizeGrip->mapFromParent(event->pos());
	if (m_resizeGrip->boundingRect().contains(p)) {
		saveGeometry();
		m_inResize = true;
		m_resizePos = event->pos();
		event->accept();
		return;
	}

	m_inResize = false;
	ItemBase::mousePressEvent(event);
}

void Note::mouseMoveEvent(QGraphicsSceneMouseEvent * event) {
	if (m_inResize) {
		QRectF r = m_rect;
		r.setWidth(m_viewGeometry.rect().width() + event->pos().x() - m_resizePos.x());
		r.setHeight(m_viewGeometry.rect().height() + event->pos().y() - m_resizePos.y());
		qreal minWidth = emptyMinWidth;
		qreal minHeight = emptyMinHeight;
		QSizeF gripSize = m_resizeGrip->boundingRect().size();
		QSizeF minSize = m_graphicsTextItem->document()->size() + gripSize + gripSize;
		if (minSize.height() > minHeight) minHeight = minSize.height();

		if (r.width() < minWidth) {
			r.setWidth(minWidth);
		}
		if (r.height() < minHeight) {
			r.setHeight(minHeight);
		}

		prepareGeometryChange();
		m_rect = r;
		positionGrip();

		event->accept();
		return;
	}

	ItemBase::mouseMoveEvent(event);
}

void Note::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
	if (m_inResize) {
		m_inResize = false;
		InfoGraphicsView *infoGraphicsView = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
		if (infoGraphicsView != NULL) {
			infoGraphicsView->noteSizeChanged(this, m_viewGeometry.rect(), m_rect);
		}
		event->accept();
		return;
	}

	ItemBase::mouseReleaseEvent(event);
}

bool Note::resizing() {
	return m_inResize;
}

void Note::contentsChangedSlot() {
	InfoGraphicsView *infoGraphicsView = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
	if (infoGraphicsView != NULL) {
		QString oldText;
		if (m_modelPart) {
			oldText = m_modelPart->instanceText();
		}

		QSizeF oldSize = m_rect.size();
		QSizeF newSize = oldSize;
		QSizeF gripSize = m_resizeGrip->boundingRect().size();
		QSizeF size = m_graphicsTextItem->document()->size();
		if (size.height() + gripSize.height() + gripSize.height() > m_rect.height()) {
			prepareGeometryChange();
			m_rect.setHeight(size.height() + gripSize.height() + gripSize.height());
			newSize.setHeight(m_rect.height());
			positionGrip();
			this->update();
		}

		infoGraphicsView->partLabelChanged(this, oldText, m_graphicsTextItem->document()->toPlainText(), oldSize, newSize);
	}
	if (m_modelPart) {
		m_modelPart->setInstanceText(m_graphicsTextItem->document()->toPlainText());
	}
}

void Note::setText(const QString & text) {
	// disconnect the signal so it doesn't fire recursively
	disconnect(m_graphicsTextItem->document(), SIGNAL(contentsChanged()), 
			this, SLOT(contentsChangedSlot()));

	QString oldText = text;
	m_graphicsTextItem->document()->setPlainText(text);

	connect(m_graphicsTextItem->document(), SIGNAL(contentsChanged()), 
		this, SLOT(contentsChangedSlot()), Qt::DirectConnection);

}

QString Note::text() {
	return m_graphicsTextItem->document()->toPlainText();
}

void Note::setSize(const QSizeF & size) 
{
	prepareGeometryChange();
	m_rect.setWidth(size.width());
	m_rect.setHeight(size.height());
	positionGrip();
}

void Note::setText(const QDomElement & textElement) 
{
	QString t = textElement.text();
	if (t.isEmpty()) {
		t = initialTextString;
	}
	setText(t);
}

void Note::setHidden(bool hide) 
{
	ItemBase::setHidden(hide);
	m_graphicsTextItem->setVisible(!hide);
	m_resizeGrip->setVisible(!hide);
}
