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
#include "../partseditor/partseditorconnectorsview.h"
#include "../debugdialog.h"

QHash<ConnectorRectangle::State, QPixmap> TerminalPointItem::m_pixmapHash;

TerminalPointItem::TerminalPointItem(PartsEditorConnectorsConnectorItem *parent, bool visible, bool reseting)
	: QGraphicsRectItem(parent)
{
	Q_ASSERT(parent);
	m_parent = parent;
	m_reseting = reseting;

	init(visible);
}

TerminalPointItem::TerminalPointItem(PartsEditorConnectorsConnectorItem *parent, bool visible, const QPointF &point)
	: QGraphicsRectItem(parent)
{
	Q_ASSERT(parent);
	m_parent = parent;
	m_point = point;
	m_reseting = false;

	init(visible);
}

void TerminalPointItem::init(bool visible) {
	initPixmapHash();

	QPen pen = QPen();
	pen.setWidth(0);
	pen.setBrush(QBrush());
	setPen(pen);

	m_cross = NULL;
	setVisible(visible);
	updatePoint();

	setFlag(QGraphicsItem::ItemIsMovable,false);
	m_cross->setFlag(QGraphicsItem::ItemIsMovable,true);
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
	updateCrossView();
}

void TerminalPointItem::updateCrossView() {
	if(!m_cross) {
		m_cross = new TerminalPointItemPrivate(this);
		m_cross->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	}

	m_cross->setPixmap(m_pixmapHash[ConnectorRectangle::Normal]);
	posCross();
}

void TerminalPointItem::posCross() {
	QRectF pRect = parentItem()->boundingRect();
	m_point = m_point == QPointF()
			?pRect.center()
			:m_point;

	QPointF correction = transformedCrossCenter();
	QPointF point = m_point-correction+(m_reseting?QPointF():pRect.topLeft());
	m_reseting = false;
	m_cross->setPos(point);
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
	/*if(isVisible()) {
		if(isOutsideConnector()) {
			setCursor(QCursor(Qt::ForbiddenCursor));
		} else {
			m_hasBeenMoved = m_pressed;
			setCursor(QCursor());
		}
	}*/
	QGraphicsRectItem::mouseMoveEvent(event);
}

void TerminalPointItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	/*if(isVisible()) {
		m_cross->setPixmap(m_pixmapHash[ConnectorRectangle::Selected]);
		m_pressed = true;
	}*/
	QGraphicsRectItem::mousePressEvent(event);
}

void TerminalPointItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	/*if(isVisible()) {
		if(isOutsideConnector()) {
			m_parent->resetTerminalPoint();
			setCursor(QCursor());
			return;
		} else {
			m_cross->setPixmap(m_pixmapHash[ConnectorRectangle::Hover]);
			m_pressed = false;
		}
	}*/
	QGraphicsRectItem::mouseReleaseEvent(event);
}

void TerminalPointItem::setMovable(bool movable) {
	m_cross->setFlag(QGraphicsItem::ItemIsMovable, movable);
}

qreal TerminalPointItem::currentScale() {
	if(scene()) {
		PartsEditorConnectorsView* view = dynamic_cast<PartsEditorConnectorsView*>(scene()->parent());
		if(view) {
			return view->currentZoom()/100;
		}
	}
	return 1;
}

void TerminalPointItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	QGraphicsRectItem::paint(painter,option,widget);
}

QPointF TerminalPointItem::mappedPoint() {
	Q_ASSERT(m_cross->hasBeenMoved());
	return m_cross->mapToItem(m_parent,transformedCrossCenter());
}

QPointF TerminalPointItem::transformedCrossCenter() {
	return QMatrix()
			.scale(currentScale(),currentScale())
			.inverted()
			.map(m_cross->boundingRect().center());
}

bool TerminalPointItem::hasBeenMoved() {
	return m_cross->hasBeenMoved();
}

void TerminalPointItem::reset() {
	m_parent->resetTerminalPoint();
}

void TerminalPointItem::doSetVisible(bool visible) {
	if(visible && !m_cross->hasBeenMoved()) {
		updateCrossView();
	}
	setVisible(visible);
}


/////////////////////////////////////////////////////////////////////////

TerminalPointItemPrivate::TerminalPointItemPrivate(TerminalPointItem *parent)
	: QGraphicsPixmapItem(parent)
{
	m_parent = parent;
	m_pressed = false;
	m_hasBeenMoved = false;
}

bool TerminalPointItemPrivate::isPressed() {
	return m_pressed;
}

bool TerminalPointItemPrivate::isOutsideConnector() {
	bool outside = !m_parent->rect().contains(mapToParent(m_parent->transformedCrossCenter()));
	return outside;
}

void TerminalPointItemPrivate::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	if(isVisible()) {
		if(isOutsideConnector()) {
			setCursor(QCursor(Qt::ForbiddenCursor));
		} else {
			m_hasBeenMoved = m_pressed;
			setCursor(QCursor());
		}
	}
	QGraphicsPixmapItem::mouseMoveEvent(event);
}

void TerminalPointItemPrivate::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if(isVisible()) {
		setPixmap(m_parent->m_pixmapHash[ConnectorRectangle::Selected]);
		m_pressed = true;
	}
	QGraphicsPixmapItem::mousePressEvent(event);
}

void TerminalPointItemPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	if(isVisible()) {
		if(isOutsideConnector()) {
			setCursor(QCursor());
			m_parent->reset();
			return;
		} else {
			setPixmap(m_parent->m_pixmapHash[ConnectorRectangle::Hover]);
			//m_parent->setPoint(boundingRect().center());
			m_pressed = false;
		}
	}
	QGraphicsPixmapItem::mouseReleaseEvent(event);
}


bool TerminalPointItemPrivate::hasBeenMoved() {
	return m_hasBeenMoved;
}
