/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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
	setAcceptsHoverEvents(false);
	this->setCursor(Qt::ArrowCursor);
	m_withBorder = false;
	m_errorIcon = NULL;

	m_resizable = false;
	m_resizing = false;
}

PartsEditorConnectorItem::PartsEditorConnectorItem(Connector * conn, ItemBase* attachedTo, const QRectF &bounds)
	: ConnectorItem(conn, attachedTo)
{
	setAcceptsHoverEvents(true);
	this->setCursor(Qt::ArrowCursor);
	m_withBorder = false;
	m_errorIcon = NULL;

	setFlag(QGraphicsItem::ItemIsMovable);
	setRect(bounds);
	removeBorder();

	m_resizable = true;
	m_resizing = false;
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
	DebugDialog::debug("<<< hoverenter");
	if(m_resizable) {
		updateCursor(event->pos());
	}
}

void PartsEditorConnectorItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	DebugDialog::debug("<<< hoverleave");
	if(m_resizable) {
		updateCursor(event->pos());
	}
}

void PartsEditorConnectorItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	updateCursor(event->pos());
	if(m_resizable && m_resizing) {
		prepareGeometryChange();
		DebugDialog::debug("<<< moveevent");
		int corner = closeToCorner(event->pos());
		if(corner > -1) {
			qreal oldX1 = pos().x();
			qreal oldY1 = pos().y();
			qreal oldX2 = pos().x()+boundingRect().width();
			qreal oldY2 = pos().y()+boundingRect().height();
			qreal newX = event->pos().x();
			qreal newY = event->pos().y();
			switch(corner) {
				case Qt::TopLeftCorner:
					//setRect(newX,newY,oldX-newX+w, oldY-newY+h);
					setRect(newX,newY,oldX2,oldY2);
					break;
				case Qt::BottomLeftCorner:
					//setRect(newX,oldY,oldX-newX+w, newY-oldY+w);
					setRect(newX,oldY1,oldX1,newY);
					break;
				case Qt::TopRightCorner:
					//setRect(oldX,newY,newX-oldX+w, oldY-newY+w);
					setRect(oldX1,newY,newX,oldX2);
					break;
				case Qt::BottomRightCorner:
					//setRect(oldX,oldY,newX-oldX+w, newY-oldY+h);
					setRect(oldX1,oldY1,newX,newY);
					break;
				default: break;
			}

			DebugDialog::debug(QString("<<<< new rect %1 %2 %3 %4")
				.arg(rect().x()).arg(rect().y())
				.arg(rect().width()).arg(rect().height())
				);
		}
	} else {
		ConnectorItem::mouseMoveEvent(event);
	}
	//ConnectorItem::mouseMoveEvent(event);
}

void PartsEditorConnectorItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if(m_resizable && cursor().shape() != QCursor().shape()) {
		m_resizing = true;
	}
	ConnectorItem::mousePressEvent(event);
}

void PartsEditorConnectorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	if(m_resizable) {
		m_resizing = false;
		setCursor(QCursor());
	}
	ConnectorItem::mouseReleaseEvent(event);
}

void PartsEditorConnectorItem::updateCursor(const QPointF &mousePos) {
	QCursor cursor;
	int corner = closeToCorner(mousePos);
	switch(corner) {
		case Qt::TopLeftCorner:
			cursor = QCursor(Qt::SizeFDiagCursor); break;
		case Qt::BottomRightCorner:
			cursor = QCursor(Qt::SizeFDiagCursor); break;
		case Qt::TopRightCorner:
			cursor = QCursor(Qt::SizeBDiagCursor); break;
		case Qt::BottomLeftCorner:
			cursor = QCursor(Qt::SizeBDiagCursor); break;
		case -1:
			cursor = QCursor(); break;
	}
	setCursor(cursor);
}

/*Qt::Corner*/ int PartsEditorConnectorItem::closeToCorner(const QPointF &pos) {
	qreal x1 = this->pos().x();
	qreal y1 = this->pos().y();
	qreal x2 = this->pos().x()+boundingRect().width();
	qreal y2 = this->pos().y()+boundingRect().height();

	// TODO: trasladar las lineas al origen
	QPair<qreal,Qt::Corner> tl(QLineF(QPointF(x1,y1),pos).length(), Qt::TopLeftCorner);
	QPair<qreal,Qt::Corner> tr(QLineF(QPointF(x2,y1),pos).length(), Qt::TopRightCorner);
	QPair<qreal,Qt::Corner> br(QLineF(QPointF(x2,y2),pos).length(), Qt::BottomRightCorner);
	QPair<qreal,Qt::Corner> bl(QLineF(QPointF(x1,y2),pos).length(), Qt::BottomLeftCorner);

	QPair<qreal,Qt::Corner> min = tl.first < tr.first ? tl : tr;
	min = min.first < br.first ? min : br;
	min = min.first < bl.first ? min : bl;

	DebugDialog::debug(QString("<<<<< min %1 %2").arg(min.first).arg(min.second));
	if(min.first <= 2.5) return min.second;
	else return -1;
}
