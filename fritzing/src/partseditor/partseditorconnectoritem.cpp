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


#include "partseditorconnectoritem.h"
#include "../itemselection/connectorrectangle.h"
#include "../debugdialog.h"

QColor PartsEditorConnectorItem::selectedColor(131,224,179);
QColor PartsEditorConnectorItem::notSelectedColor(131,224,179);
QColor PartsEditorConnectorItem::selectedPenColor(52, 128, 92);
qreal PartsEditorConnectorItem::selectedPenWidth = 1.5;

qreal PartsEditorConnectorItem::MinWidth = 2;
qreal PartsEditorConnectorItem::MinHeight = MinWidth;

PartsEditorConnectorItem::PartsEditorConnectorItem(Connector * conn, ItemBase* attachedTo, bool showsTerminalPoints, bool showingTerminalPoint)
	: ConnectorItem(conn, attachedTo)
{
	init(false,false);
	m_terminalPointItem = NULL;
	m_showsTerminalPoint = showsTerminalPoints;
	m_showingTerminalPoint = showingTerminalPoint;
}

PartsEditorConnectorItem::PartsEditorConnectorItem(Connector * conn, ItemBase* attachedTo, bool showsTerminalPoints, bool showingTerminalPoint, const QRectF &bounds)
	: ConnectorItem(conn, attachedTo)
{
	init(true,true);

	setRect(bounds);
	removeBorder();

	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsMovable);

	m_geometryHasChanged = false;
	m_terminalPointItem = NULL;
	m_showsTerminalPoint = showsTerminalPoints;
	m_showingTerminalPoint = showingTerminalPoint;
	updateTerminalPoint();
}

void PartsEditorConnectorItem::init(bool resizable, bool movable) {
	setAcceptsHoverEvents(resizable);
	setAcceptHoverEvents(resizable);
	m_withBorder = false;
	m_errorIcon = NULL;

	if(resizable) new ConnectorRectangle(this);
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
	//ConnectorItem::paint(painter, option, widget);
}


void PartsEditorConnectorItem::setParentDragMode(QGraphicsView::DragMode dragMode) {
	QGraphicsView *prnt = dynamic_cast<QGraphicsView*>(scene()->parent());
	if(prnt) prnt->setDragMode(dragMode);
}


void PartsEditorConnectorItem::setShowTerminalPoint(bool show) {
	if(m_showsTerminalPoint) {
		m_showingTerminalPoint = show;
		if(m_terminalPointItem) {
			if(!m_geometryHasChanged) {
				m_terminalPointItem->setVisible(show);
			} else if(show) {
				resetTerminalPoint();
				m_geometryHasChanged = false;
			}
		}
	}
}

bool PartsEditorConnectorItem::showingTerminalPoint() {
	if(m_showsTerminalPoint) {
		return m_showingTerminalPoint;
	}
	return false;
}

void PartsEditorConnectorItem::setTerminalPoint(QPointF point) {
	ConnectorItem::setTerminalPoint(point);
	m_terminalPointItem = new TerminalPointItem(this,m_showsTerminalPoint&&m_showingTerminalPoint,point);
}

void PartsEditorConnectorItem::resizeRect(qreal x, qreal y, qreal width, qreal height) {
	setRect(x,y,width,height);
	if(showingTerminalPoint()) {
		updateTerminalPoint();
	} else {
		m_geometryHasChanged = true;
	}
}

void PartsEditorConnectorItem::resetTerminalPoint() {
	scene()->removeItem(m_terminalPointItem);
	//delete m_terminalPointItem; // already deleted or what?
	m_terminalPointItem = NULL;
	updateTerminalPoint();
}


void PartsEditorConnectorItem::updateTerminalPoint() {
	if(!m_terminalPointItem) {
		m_terminalPointItem = new TerminalPointItem(this,m_showsTerminalPoint&&m_showingTerminalPoint);
	} else {
		m_terminalPointItem->setParentItem(this);
		m_terminalPointItem->updatePoint();
	}
}


TerminalPointItem *PartsEditorConnectorItem::terminalPointItem() {
	return m_terminalPointItem;
}
