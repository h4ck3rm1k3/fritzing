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

$Revision: 1886 $:
$Author: merunga $:
$Date: 2008-12-18 19:17:13 +0100 (Thu, 18 Dec 2008) $

********************************************************************/

#include "terminalpointitem.h"
#include "../partseditor/partseditorconnectorsconnectoritem.h"
#include "../debugdialog.h"

QHash<ConnectorRectangle::State, QPixmap> TerminalPointItem::m_pixmapHash;

TerminalPointItem::TerminalPointItem(PartsEditorConnectorsConnectorItem *parent, bool visible)
	: QGraphicsRectItem(parent), ResizableRectItem()
{
	Q_ASSERT(parent);
	m_parent = parent;
	m_hasBeenMoved = false;
	m_pressed = false;

	init(visible);
}

TerminalPointItem::TerminalPointItem(PartsEditorConnectorsConnectorItem *parent, bool visible, const QPointF &point)
	: QGraphicsRectItem(parent), ResizableRectItem()
{
	Q_ASSERT(parent);
	m_parent = parent;
	m_hasBeenMoved = false;
	m_point = point;

	init(visible);
}

TerminalPointItem::~TerminalPointItem()
{
	if (m_handlers) {
		delete m_handlers;
	}
}

void TerminalPointItem::init(bool visible) {
	initPixmapHash();

	m_handlers = new ConnectorRectangle(this,false);

	QPen pen = QPen();
	pen.setWidth(0);
	pen.setBrush(QBrush());
	setPen(pen);

	m_cross = NULL;
	setVisible(visible);
	updatePoint();
}

void TerminalPointItem::initPixmapHash() {
	if(m_pixmapHash.isEmpty()) {
		m_pixmapHash[ConnectorRectangle::Normal] =
			QPixmap(":/resources/images/itemselection/crosshairHandlerNormal.png");
		m_pixmapHash[ConnectorRectangle::Hover] =
		 	QPixmap(":/resources/images/itemselection/crosshairHandlerHover.png");
		m_pixmapHash[ConnectorRectangle::Selected] =
		 	QPixmap(":/resources/images/itemselection/crosshairHandlerActive.png");
	}
}

void TerminalPointItem::updatePoint() {
	setRect(m_parent->boundingRect());
	drawCross();
}

void TerminalPointItem::drawCross() {
	if(!m_cross) {
		m_cross = new QGraphicsPixmapItem(this);
		m_cross->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	}

	m_cross->setPixmap(m_pixmapHash[ConnectorRectangle::Normal]);
}

void TerminalPointItem::posCross() {
	QRectF pRect = parentItem()->boundingRect();
	QPointF crossCenter = m_point == QPointF()?
							pRect.center():
							QPointF(pRect.x(),pRect.y()) + m_point;

	qreal transf = 2*m_handlers->currentScale();
	qreal x = crossCenter.x() -m_cross->boundingRect().width()/transf;
	qreal y = crossCenter.y() -m_cross->boundingRect().height()/transf;

	m_cross->setPos(x,y);
}

void TerminalPointItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	if(isVisible()) m_cross->setPixmap(m_pixmapHash[ConnectorRectangle::Hover]);
	QGraphicsItem::hoverEnterEvent(event);
}

void TerminalPointItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	if(isVisible()) m_cross->setPixmap(m_pixmapHash[ConnectorRectangle::Normal]);
	QGraphicsItem::hoverLeaveEvent(event);
}

void TerminalPointItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	if(isVisible()) {
		if(isOutsideConnector()) {
			setCursor(QCursor(Qt::ForbiddenCursor));
		} else {
			m_hasBeenMoved = m_pressed;
			setCursor(QCursor());
		}
	}
	QGraphicsItem::mouseMoveEvent(event);
}

void TerminalPointItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if(isVisible()) {
		m_cross->setPixmap(m_pixmapHash[ConnectorRectangle::Selected]);
		m_pressed = true;
	}
	QGraphicsItem::mousePressEvent(event);
}

void TerminalPointItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	if(isVisible()) {
		if(isOutsideConnector()) {
			m_parent->resetTerminalPoint();
			setCursor(QCursor());
			return;
		} else {
			m_cross->setPixmap(m_pixmapHash[ConnectorRectangle::Hover]);
			m_pressed = false;
		}
	}
	QGraphicsItem::mouseReleaseEvent(event);
}

bool TerminalPointItem::isOutsideConnector() {
	/*QPointF myCenter = mapToParent(rect().center());
	//QPointF myCenter = mapToParent(m_cross->boundingRect().center());
	QRectF pRect = m_parent->rect();

	QPointF aux = m_cross->boundingRect().center();

	return myCenter.x()<pRect.x()
		|| myCenter.y()<pRect.y()
		|| myCenter.x()>pRect.x()+pRect.width()
		|| myCenter.y()>pRect.y()+pRect.height();
	*/
	return false;
}

QPointF TerminalPointItem::mappedToScenePoint() {
	return mapToScene(point());
}

QPointF TerminalPointItem::point() {
	return rect().center();
}

bool TerminalPointItem::hasBeenMoved() {
	return m_hasBeenMoved;
}

void TerminalPointItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	posCross();
	QGraphicsRectItem::paint(painter,option,widget);
}

qreal TerminalPointItem::minHeight() {
	return 1;
}

qreal TerminalPointItem::minWidth() {
	return 1;
}

void TerminalPointItem::resizeRect(qreal x, qreal y, qreal w, qreal h) {
	setRect(x,y,w,h);
}

void TerminalPointItem::doPrepareGeometryChange() {
	prepareGeometryChange();
}
