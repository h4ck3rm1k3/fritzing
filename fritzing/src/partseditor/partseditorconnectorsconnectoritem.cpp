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

$Revision: 2248 $:
$Author: merunga $:
$Date: 2009-01-22 19:47:17 +0100 (Thu, 22 Jan 2009) $

********************************************************************/


#include "partseditorconnectorsconnectoritem.h"
#include "partseditorconnectorsview.h"
#include "../debugdialog.h"


qreal PartsEditorConnectorsConnectorItem::MinWidth = 1;
qreal PartsEditorConnectorsConnectorItem::MinHeight = MinWidth;

PartsEditorConnectorsConnectorItem::PartsEditorConnectorsConnectorItem(Connector * conn, ItemBase* attachedTo, bool showingTerminalPoint)
	: PartsEditorConnectorItem(conn, attachedTo), ResizableRectItem()
{
	init(true);

	m_inFileDefined = true;
	m_terminalPointItem = NULL;
	m_showingTerminalPoint = showingTerminalPoint;
}

PartsEditorConnectorsConnectorItem::PartsEditorConnectorsConnectorItem(Connector * conn, ItemBase* attachedTo, bool showingTerminalPoint, const QRectF &bounds)
	: PartsEditorConnectorItem(conn, attachedTo), ResizableRectItem()
{
	init(true);

	setRect(bounds);

	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsMovable);

	m_inFileDefined = false;
	m_centerHasChanged = false;
	m_terminalPointItem = NULL;
	m_showingTerminalPoint = showingTerminalPoint;
	updateTerminalPoint();
}

PartsEditorConnectorsConnectorItem::~PartsEditorConnectorsConnectorItem()
{
	if (m_handlers) {
		delete m_handlers;
	}
}

void PartsEditorConnectorsConnectorItem::resizeRect(qreal x, qreal y, qreal width, qreal height) {
	setRect(x,y,width,height);
	m_resizedRect = QRectF(x,y,width,height);
	informChange();
	if(showingTerminalPoint()) {
		updateTerminalPoint();
	} else {
		m_centerHasChanged = true;
	}
}

void PartsEditorConnectorsConnectorItem::init(bool resizable) {
	setFlag(QGraphicsItem::ItemIsMovable);

	setAcceptsHoverEvents(resizable);
	setAcceptHoverEvents(resizable);
	m_errorIcon = NULL;
	m_geometryHasChanged = false;
	m_resizedRect = QRectF();

	setResizable(resizable);
	if(m_resizable) {
		m_handlers = new ConnectorRectangle(this);
	} else {
		m_handlers = NULL;
	}
}

void PartsEditorConnectorsConnectorItem::highlight(const QString &connId) {
	if(m_connector->connectorSharedID() == connId) {
		if(m_resizable) m_handlers->setHandlersVisible(true);
	} else {
		if(m_resizable) m_handlers->setHandlersVisible(false);
	}
}

void PartsEditorConnectorsConnectorItem::setConnector(Connector *connector) {
	m_connector = connector;
}

void PartsEditorConnectorsConnectorItem::setMismatching(bool isMismatching) {
	if(!isMismatching) {
		removeErrorIcon();
	} else {
		addErrorIcon();
	}
}

void PartsEditorConnectorsConnectorItem::addErrorIcon() {
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

void PartsEditorConnectorsConnectorItem::removeErrorIcon() {
	if(m_errorIcon) {
		this->scene()->removeItem(m_errorIcon);
		delete m_errorIcon;
		m_errorIcon = NULL;
	}
}

void PartsEditorConnectorsConnectorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	painter->save();
	drawDottedRect(painter,Qt::black,Qt::white,rect());
	m_handlers->paint(painter);
	painter->restore();
}

void PartsEditorConnectorsConnectorItem::drawDottedRect(QPainter *painter, const QColor &color1, const QColor &color2, const QRectF &rect) {
	QPen pen1(color1);
	QPen pen2(color2);

	qreal x1 = rect.x();
	qreal y1 = rect.y();
	qreal x2 = x1+rect.width();
	qreal y2 = y1+rect.height();

	QPen lastPen = drawDottedLine(Qt::Horizontal,painter,pen1,pen2,x1,x2,y1);
	lastPen = drawDottedLine(Qt::Vertical,painter,pen2,pen1,y1,y2,x1,lastPen);
	lastPen = drawDottedLine(Qt::Horizontal,painter,pen1,pen2,x1,x2,y2,lastPen);
	drawDottedLine(Qt::Vertical,painter,pen2,pen1,y1,y2,x2,lastPen);

}

QPen PartsEditorConnectorsConnectorItem::drawDottedLine(
		Qt::Orientations orientation, QPainter *painter, const QPen &pen1, const QPen &pen2,
		qreal pos1, qreal pos2, qreal fixedAxis, const QPen &lastUsedPen
) {
	qreal dotSize = 1.5;
	qreal lineSize = pos2-pos1;
	qreal aux = pos1;
	int dotCount = 0;

	QPen firstPen;
	QPen secondPen;
	if(pen1.color() != lastUsedPen.color()) {
		firstPen = pen2;
		secondPen = pen1;
	} else {
		firstPen = pen1;
		secondPen = pen2;
	}

	QPen currentPen;
	while(lineSize > dotSize) {
		currentPen = drawDottedLineAux(
			orientation, painter, firstPen, secondPen,
			aux, fixedAxis, dotSize, dotCount
		);
		dotCount++;
		aux+=dotSize;
		lineSize-=dotSize;
	}
	if(lineSize > 0) {
		currentPen = drawDottedLineAux(
			orientation, painter, firstPen, secondPen,
			aux, fixedAxis, lineSize, dotCount
		);
	}

	return currentPen;
}

QPen PartsEditorConnectorsConnectorItem::drawDottedLineAux(
		Qt::Orientations orientation, QPainter *painter, const QPen &firstPen, const QPen &secondPen,
		qreal pos, qreal fixedAxis, qreal dotSize, int dotCount
) {
	QPen currentPen = dotCount%2 == 0? firstPen: secondPen;
	painter->setPen(currentPen);
	if(orientation == Qt::Horizontal) {
		painter->drawLine(QPointF(pos,fixedAxis),QPointF(pos+dotSize,fixedAxis));
	} else if(orientation == Qt::Vertical) {
		painter->drawLine(QPointF(fixedAxis,pos),QPointF(fixedAxis,pos+dotSize));
	}

	return currentPen;
}

void PartsEditorConnectorsConnectorItem::setShowTerminalPoint(bool show) {
	m_showingTerminalPoint = show;
	if(m_terminalPointItem) {
		if(!m_centerHasChanged) {
			m_terminalPointItem->setVisible(show);
		} else if(show) {
			resetTerminalPoint();
			m_centerHasChanged = false;
		}
	}

	// if we're showing the rerminal points, then the connector
	// is not movable
	setFlag(QGraphicsItem::ItemIsMovable,!show);
	m_terminalPointItem->setFlag(QGraphicsItem::ItemIsMovable,show);
}

bool PartsEditorConnectorsConnectorItem::showingTerminalPoint() {
	return m_showingTerminalPoint;
}

void PartsEditorConnectorsConnectorItem::setTerminalPoint(QPointF point) {
	ConnectorItem::setTerminalPoint(point);
	m_terminalPointItem = new TerminalPointItem(this,m_showingTerminalPoint,point);
}

qreal PartsEditorConnectorsConnectorItem::minWidth() {
	return MinWidth;
}

qreal PartsEditorConnectorsConnectorItem::minHeight() {
	return MinHeight;
}

void PartsEditorConnectorsConnectorItem::resetTerminalPoint() {
	scene()->removeItem(m_terminalPointItem);
	//delete m_terminalPointItem; // already deleted or what?
	m_terminalPointItem = NULL;
	updateTerminalPoint();
}


void PartsEditorConnectorsConnectorItem::updateTerminalPoint() {
	if(!m_terminalPointItem) {
		m_terminalPointItem = new TerminalPointItem(this,m_showingTerminalPoint);
	} else {
		m_terminalPointItem->setParentItem(this);
		m_terminalPointItem->updatePoint();
	}
}


TerminalPointItem *PartsEditorConnectorsConnectorItem::terminalPointItem() {
	return m_terminalPointItem;
}

QVariant PartsEditorConnectorsConnectorItem::itemChange(GraphicsItemChange change, const QVariant &value) {
	if(change == QGraphicsItem::ItemPositionChange) {
		informChange();
	}
	return ConnectorItem::itemChange(change,value);
}


void PartsEditorConnectorsConnectorItem::informChange() {
	if(m_inFileDefined && !m_geometryHasChanged) {
		PartsEditorConnectorsView *gv = dynamic_cast<PartsEditorConnectorsView*>(scene()->parent());
		if(gv) {
			gv->inFileDefinedConnectorChanged(this);
		}
	}
	m_geometryHasChanged = true;
}

QRectF PartsEditorConnectorsConnectorItem::mappedRect() {
	return mapToParent(rect()).boundingRect();
}

void PartsEditorConnectorsConnectorItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	highlight(connectorSharedID());
	PartsEditorConnectorsView *gv = dynamic_cast<PartsEditorConnectorsView*>(scene()->parent());
	if(gv) {
		gv->informConnectorSelectionFromView(connectorSharedID());
	}
	Q_UNUSED(event);
}

void PartsEditorConnectorsConnectorItem::doPrepareGeometryChange() {
	prepareGeometryChange();
}

void PartsEditorConnectorsConnectorItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	Q_UNUSED(event);
	setCursor(QCursor(Qt::SizeAllCursor));
}

void PartsEditorConnectorsConnectorItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	Q_UNUSED(event);
	unsetCursor();
}
