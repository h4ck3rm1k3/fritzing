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

TerminalPointItem::TerminalPointItem(PartsEditorConnectorsConnectorItem *parent, bool visible, bool movable)
	: QGraphicsRectItem(parent), ResizableRectItem()
{
	Q_ASSERT(parent);
	m_parent = parent;
	m_hasBeenMoved = false;

	init(visible, movable);
}

TerminalPointItem::TerminalPointItem(PartsEditorConnectorsConnectorItem *parent, bool visible, const QPointF &point)
	: QGraphicsRectItem(parent), ResizableRectItem()
{
	Q_ASSERT(parent);
	m_parent = parent;
	m_hasBeenMoved = false;
	m_point = point;

	init(visible, false);
}

void TerminalPointItem::init(bool visible, bool movable) {
	initPixmapHash();

	m_handlers = new ConnectorRectangle(this,false);

	QPen pen = QPen();
	pen.setWidth(0);
	pen.setBrush(QBrush());
	setPen(pen);

	m_cross = NULL;
	m_movable = movable;
	setFlag(QGraphicsItem::ItemIsMovable, movable);
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

	/*DebugDialog::debug(QString("<<< cross size %1 %2")
			.arg(m_cross->boundingRect().width()).arg(m_cross->boundingRect().height()));
	DebugDialog::debug(QString("<<< connector center %1 %2")
				.arg(crossCenter.x()).arg(crossCenter.y()));
	*/

	qreal transf = 2*m_handlers->currentScale();
	qreal x = crossCenter.x() -m_cross->boundingRect().width()/transf;
	qreal y = crossCenter.y() -m_cross->boundingRect().height()/transf;
	//QPointF pos = mapToScene(x,y);

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
			setCursor(QCursor());
		}
	}
	QGraphicsItem::mouseMoveEvent(event);
}

void TerminalPointItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if(isVisible()) m_cross->setPixmap(m_pixmapHash[ConnectorRectangle::Selected]);
	QGraphicsItem::mousePressEvent(event);
}

void TerminalPointItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	if(isVisible()) {
		if(isOutsideConnector()) {
			m_parent->resetTerminalPoint();
			//scene()->update();
			setCursor(QCursor());
			return;
		} else {
			m_cross->setPixmap(m_pixmapHash[ConnectorRectangle::Hover]);
		}
	}
	QGraphicsItem::mouseReleaseEvent(event);
}

bool TerminalPointItem::isOutsideConnector() {
	/*QPointF myCenter = mapToItem(m_handlers,mappedToScenePoint());
	QPointF pPos = m_handlers->mapToScene(m_handlers->pos());
	QRectF pRectAux = m_handlers->boundingRect();
	QRectF pRect(pPos.x(),pPos.y(),pRectAux.width(),pRectAux.height());
	*/

	QPointF myCenter = mapToItem(m_parent,m_cross->boundingRect().center());
	QPointF newCenter = mapToItem(m_handlers,m_cross->boundingRect().center());
	QRectF pRect = m_parent->rect();
	QPointF pPos = mapToScene(QPointF(pRect.x(),pRect.y()));
	QPointF pPos2 = mapToScene(QPointF(pRect.x()+pRect.width(),pRect.y()+pRect.height()));
	//QPolygonF pRect = mapToScene(m_parent->boundingRect());
	DebugDialog::debug(QString("<<<< center %1 %2").arg(myCenter.x()).arg(myCenter.y()));
	DebugDialog::debug(QString("<<<< pos %1 %2").arg(pPos.x()).arg(pPos.y()));
	DebugDialog::debug(QString("<<<< pos2 %1 %2").arg(pPos2.x()).arg(pPos2.y()));
	DebugDialog::debug(QString("<<<< parent %1 %2 - %3 %4")
		.arg(pRect.x()).arg(pRect.y())
		.arg(pRect.width()).arg(pRect.height())
	);
	DebugDialog::debug(QString("<<<< newCenter %1 %2").arg(newCenter.x()).arg(newCenter.y()));
	//DebugDialog::debug(QString("<<<< newCenter scale %1 %2").arg(newCenter.x()/m_handlers->currentScale()).arg(newCenter.y()/m_handlers->currentScale()));
	//DebugDialog::debug(QString("<<<< pospos %1 %2").arg(newCenter.x()).arg(newCenter.y()));

	DebugDialog::debug(mapToScene(m_parent->boundingRect()).contains(myCenter)?"true":"false");

	DebugDialog::debug("");
	//return m_parent->boundingRect().contains(myCenter);

	return myCenter.x()<pRect.x()
		|| myCenter.y()<pRect.y()
		|| myCenter.x()>pRect.x()+pRect.width()
		|| myCenter.y()>pRect.y()+pRect.height();
}

QPointF TerminalPointItem::mappedToScenePoint() {
	/*scene()->update();
	QPointF pos = mapToScene(this->pos());
	QPointF size = mapToScene(QPointF(boundingRect().width(),boundingRect().height()));
	qreal lineWx2 = m_linePen.widthF()*2;
	return QPointF((pos.x()+size.x()-lineWx2)/2,(pos.y()+size.y()-lineWx2)/2);
	*/
	return mapToScene(point());
}

QPointF TerminalPointItem::point() {
	scene()->update();
	QPointF center = m_cross->boundingRect().center();
	QPointF pos = mapToScene(this->pos());
	//QPointF size = mapToScene(QPointF(boundingRect().width(),boundingRect().height()));
	return QPointF(pos.x()+center.x(),pos.y()+center.y());

	//return m_cross->boundingRect().center();
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
