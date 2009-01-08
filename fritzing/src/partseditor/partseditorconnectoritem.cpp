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


#include <QLineF>
#include "partseditorconnectoritem.h"
#include "../debugdialog.h"

QColor PartsEditorConnectorItem::selectedColor(131,224,179);
QColor PartsEditorConnectorItem::notSelectedColor(131,224,179);
QColor PartsEditorConnectorItem::selectedPenColor(52, 128, 92);
qreal PartsEditorConnectorItem::selectedPenWidth = 1.5;

PartsEditorConnectorItem::PartsEditorConnectorItem(Connector * conn, ItemBase* attachedTo)
	: ConnectorItem(conn, attachedTo)
{
	init(false);
	m_terminalPointItem = NULL;
}

PartsEditorConnectorItem::PartsEditorConnectorItem(Connector * conn, ItemBase* attachedTo, const QRectF &bounds)
	: ConnectorItem(conn, attachedTo)
{
	init(true);

	setRect(bounds);
	removeBorder();

	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsMovable);
	m_terminalPointItem = new TerminalPointItem(this);
}

void PartsEditorConnectorItem::init(bool resizable) {
	setAcceptsHoverEvents(resizable);
	setAcceptHoverEvents(resizable);
	m_withBorder = false;
	m_errorIcon = NULL;

	m_resizable = resizable;
	m_resizing = false;
	m_moving = false;
	m_mousePosition = Outside;
}

void PartsEditorConnectorItem::setSelectedColor(const QColor &color) {
	setColorAux(color);
}

void PartsEditorConnectorItem::setNotSelectedColor(const QColor &color) {
	setColorAux(color);
}

void PartsEditorConnectorItem::highlight(const QString &connId) {
	if(m_connector->connectorStuffID() == connId) {
		//setSelectedColor(color);
		addBorder();
	} else {
		//setNotSelectedColor();
		removeBorder();
	}
}

void PartsEditorConnectorItem::addBorder() {
	this->setBrush(QBrush(selectedColor));
	QPen pen(selectedPenColor);
	pen.setWidth(selectedPenWidth);
	pen.setJoinStyle(Qt::MiterJoin);
	pen.setCapStyle(Qt::SquareCap);
	this->setPen(pen);
	m_withBorder = true;
	m_paint = true;
}

void PartsEditorConnectorItem::removeBorder() {
	this->setBrush(QBrush(notSelectedColor));
	this->setPen(Qt::NoPen);
	m_withBorder = false;
	m_paint = true;
}

void PartsEditorConnectorItem::removeFromModel() {
	m_connector->removeViewItem(this);
}

void PartsEditorConnectorItem::setConnector(Connector *connector) {
	m_connector = connector;
}

void PartsEditorConnectorItem::setMismatching(bool isMismatching) {
	if(!isMismatching) {
		removeErrorIcon();
	} else {
		addErrorIcon();
	}
}

void PartsEditorConnectorItem::addErrorIcon() {
	if(!m_errorIcon) {
		m_errorIcon = new QGraphicsSvgItem("resources/images/error_x_mini.svg",this);
		this->scene()->addItem(m_errorIcon);

		QRectF boundRect = boundingRect();

		qreal x = boundRect.x()+boundRect.width()-5;
		qreal y = boundRect.y()-m_errorIcon->boundingRect().height()+5;

		m_errorIcon->setPos(x,y);
		m_errorIcon->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	}
}

void PartsEditorConnectorItem::removeErrorIcon() {
	if(m_errorIcon) {
		this->scene()->removeItem(m_errorIcon);
		delete m_errorIcon;
		m_errorIcon = NULL;
	}
}

void PartsEditorConnectorItem::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (m_hidden  || !m_paint) return;

	painter->save();
	painter->setPen(pen());
	painter->setBrush(brush());

	QRectF rect = this->rect();
	qreal pw = selectedPenWidth;
	if(m_withBorder) {
		painter->drawRect(rect.x()-pw/2,rect.y()-pw/2,rect.width()+pw,rect.height()+pw);
	} else {
		painter->drawRect(this->rect());
	}

	painter->restore();
	//this->scene()->update();  // calling update here puts you in an infinite paint loop
}

void PartsEditorConnectorItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	if(m_resizable) {
		grabMouse();
		updateCursor(event->pos(),QCursor(Qt::SizeAllCursor));
	} else {
		//QGraphicsItem::hoverEnterEvent(event);
	}
}

void PartsEditorConnectorItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	if(m_resizable) {
		updateCursor(event->pos());
		ungrabMouse();
	} else {
		//QGraphicsItem::hoverEnterEvent(event);
	}
}

void PartsEditorConnectorItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	if(m_resizable) {
		if(m_resizing && m_mousePosition < Inside) {
			resize(event->pos());
		} else if(m_moving && m_mousePosition == Inside) {
			move(event->scenePos());
		} else {
			ConnectorItem::mouseMoveEvent(event);
		}
	} else {
		ConnectorItem::mouseMoveEvent(event);
	}
	updateCursor(event->pos());
}

void PartsEditorConnectorItem::resize(const QPointF &mousePos) {
	prepareGeometryChange();

	qreal oldX1 = boundingRect().x();
	qreal oldY1 = boundingRect().y();
	qreal oldX2 = oldX1+boundingRect().width();
	qreal oldY2 = oldY1+boundingRect().height();
	qreal newX = mousePos.x();
	qreal newY = mousePos.y();

	DebugDialog::debug(QString("<<< (%1,%2)  (%3,%4)")
			.arg(oldX1).arg(oldY1).arg(oldX2).arg(oldY2));

	DebugDialog::debug(QString("<<< mouse (%1,%2)")
						.arg(newX).arg(newY));

	switch(m_mousePosition) {
		case TopLeftCorner: 	setRectAux(newX,newY,oldX2,oldY2);
		DebugDialog::debug(QString("<<< from top left new rect (%1,%2)  (%3,%4)")
				.arg(newX).arg(newY).arg(oldX2).arg(oldY2));
		break;
		case BottomLeftCorner: 	setRectAux(newX,oldY1,oldX1,newY);
		DebugDialog::debug(QString("<<< from bottom left new rect (%1,%2)  (%3,%4)")
				.arg(newX).arg(oldY1).arg(oldX1).arg(newY));
		break;
		case TopRightCorner: 	setRectAux(oldX1,newY,newX,oldX2);
		DebugDialog::debug(QString("<<< from top right new rect (%1,%2)  (%3,%4)")
				.arg(oldX1).arg(newY).arg(newX).arg(oldX2));
		break;
		case BottomRightCorner: setRectAux(oldX1,oldY1,newX,newY);
		DebugDialog::debug(QString("<<< from bottom right new rect (%1,%2)  (%3,%4)")
				.arg(oldX1).arg(oldY1).arg(newX).arg(newY));
		break;
		default: break;
	}

	DebugDialog::debug("");
}

void PartsEditorConnectorItem::move(const QPointF &newPos) {
	DebugDialog::debug("move it!");

	QPointF currentParentPos = mapToParent(mapFromScene(newPos));
	QPointF buttonDownParentPos = mapToParent(mapFromScene(m_mousePressedPos));
	QPointF aux = currentParentPos - buttonDownParentPos;
	moveBy(aux.x(),aux.y());
	m_mousePressedPos = newPos;

	/*DebugDialog::debug(QString("original parent pos %1 %2").arg(origPos.x()).arg(origPos.y()));
	DebugDialog::debug(QString("original mapped pos %1 %2").arg(buttonDownParentPos.x()).arg(buttonDownParentPos.y()));
	DebugDialog::debug(QString("new parent pos %1 %2").arg(newPos.x()).arg(newPos.y()));
	DebugDialog::debug(QString("new mapped pos %1 %2").arg(currentParentPos.x()).arg(currentParentPos.y()));
	*/
	DebugDialog::debug(QString("move by %1 %2").arg(aux.x()).arg(aux.y()));
}

void PartsEditorConnectorItem::setRectAux(qreal x1, qreal y1, qreal x2, qreal y2) {
	setRect(x1,y1,x2-x1,y2-y1);
	if(m_terminalPointItem) {
		m_terminalPointItem->updatePoint();
	}
}

void PartsEditorConnectorItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	setParentDragMode(QGraphicsView::NoDrag);
	if(m_resizable) {
		setFlag(QGraphicsItem::ItemIsMovable,true);
		m_mousePosition = closeToCorner(event->pos());
		if(m_mousePosition != Outside) {
			m_resizing = m_mousePosition != Inside;
			m_moving = !m_resizing;
			m_mousePressedPos = event->buttonDownScenePos(Qt::LeftButton);
		}
	} else {
		ConnectorItem::mousePressEvent(event);
	}
}

void PartsEditorConnectorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	if(m_resizable) {
		m_resizing = false;
		m_moving = false;
		setParentDragMode(QGraphicsView::ScrollHandDrag);
		setCursor(QCursor());
	}
	ConnectorItem::mouseReleaseEvent(event);
}

PartsEditorConnectorItem::Position PartsEditorConnectorItem::updateCursor(const QPointF &mousePos, const QCursor &defaultCursor) {
	QCursor cursor;
	m_mousePosition = closeToCorner(mousePos);
	switch(m_mousePosition) {
		case TopLeftCorner:
			cursor = QCursor(Qt::SizeFDiagCursor);
			break;
		case BottomRightCorner:
			cursor = QCursor(Qt::SizeFDiagCursor);
			break;
		case TopRightCorner:
			cursor = QCursor(Qt::SizeBDiagCursor);
			break;
		case BottomLeftCorner:
			cursor = QCursor(Qt::SizeBDiagCursor);
			break;
		case Inside:
			cursor = QCursor(Qt::SizeAllCursor);
			break;
		case Outside:
			cursor = defaultCursor;
			break;
	}
	setCursor(cursor);
	return m_mousePosition;
}


PartsEditorConnectorItem::Position PartsEditorConnectorItem::closeToCorner(const QPointF &pos) {
	qreal x1 = boundingRect().x();
	qreal y1 = boundingRect().y();
	qreal x2 = x1+boundingRect().width();
	qreal y2 = y1+boundingRect().height();

	bool mouseOutOfRect = pos.x() < x1 || pos.y() < y1 || pos.x() > x2 || pos.y() > y2;

	QPair<qreal,Position> tl(QLineF(QPointF(x1,y1),pos).length(), TopLeftCorner);
	QPair<qreal,Position> tr(QLineF(QPointF(x2,y1),pos).length(), TopRightCorner);
	QPair<qreal,Position> br(QLineF(QPointF(x2,y2),pos).length(), BottomRightCorner);
	QPair<qreal,Position> bl(QLineF(QPointF(x1,y2),pos).length(), BottomLeftCorner);

	QPair<qreal,Position> min = tl.first < tr.first ? tl : tr;
	min = min.first < br.first ? min : br;
	min = min.first < bl.first ? min : bl;

	if(min.first <= 3)
		return min.second;
	else if(mouseOutOfRect) return Outside;
	else return Inside;
}


void PartsEditorConnectorItem::setParentDragMode(QGraphicsView::DragMode dragMode) {
	QGraphicsView *prnt = dynamic_cast<QGraphicsView*>(scene()->parent());
	if(prnt) prnt->setDragMode(dragMode);
}


void PartsEditorConnectorItem::showTerminalPoint(bool show) {
	if(m_terminalPointItem) {
		m_terminalPointItem->setVisible(show);
	}
}
