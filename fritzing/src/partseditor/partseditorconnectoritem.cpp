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


#include "partseditorconnectoritem.h"

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
	this->scene()->update();
}
