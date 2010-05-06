/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include <QTimer>

#include "partseditorconnectorsconnectoritem.h"
#include "partseditorview.h"
#include "../itemselection/connectorrectangle.h"
#include "../debugdialog.h"


qreal PartsEditorConnectorsConnectorItem::MinWidth = 1;
qreal PartsEditorConnectorsConnectorItem::MinHeight = MinWidth;

#define NON_DEFINED_TERMINAL_POINT QPointF(-1,-1)

PartsEditorConnectorsConnectorItem::PartsEditorConnectorsConnectorItem(Connector * conn, ItemBase* attachedTo, bool showingTerminalPoint)
	: PartsEditorConnectorItem(conn, attachedTo)
{
	init(true);

	m_inFileDefined = true;
	m_terminalPoint = NON_DEFINED_TERMINAL_POINT;
	m_terminalPointItem = NULL;
	m_showingTerminalPoint = showingTerminalPoint;

	QTimer::singleShot(300, this, SLOT(createTerminalPoint()));
}

PartsEditorConnectorsConnectorItem::PartsEditorConnectorsConnectorItem(Connector * conn, ItemBase* attachedTo, bool showingTerminalPoint, const QRectF &bounds)
	: PartsEditorConnectorItem(conn, attachedTo)
{
	init(true);

	setRect(bounds);

	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsMovable);

	m_inFileDefined = false;
	m_showingTerminalPoint = showingTerminalPoint;
	m_terminalPoint = NON_DEFINED_TERMINAL_POINT;
	createTerminalPoint();
}

PartsEditorConnectorsConnectorItem::~PartsEditorConnectorsConnectorItem()
{
	if (m_handlers) {
		delete m_handlers;
	}
}

void PartsEditorConnectorsConnectorItem::createTerminalPoint() {
	m_terminalPointItem = newTerminalPointItem();
}

TerminalPointItem* PartsEditorConnectorsConnectorItem::newTerminalPointItem() {
	TerminalPointItem * terminalPointItem = m_terminalPoint == NON_DEFINED_TERMINAL_POINT?
			new TerminalPointItem(this,m_showingTerminalPoint):
			new TerminalPointItem(this,m_showingTerminalPoint,m_terminalPoint);
	terminalPointItem->updatePoint();
	return terminalPointItem;
}

void PartsEditorConnectorsConnectorItem::resizeRect(qreal x, qreal y, qreal width, qreal height) {
	m_resized = true;

	setRect(x,y,width,height);
	m_resizedRect = QRectF(x,y,width,height);
	informChange();
	m_geometryHasChanged = true;
	m_geometryHasChangedAlLeastOnce = true;
	m_terminalPointItem->reset();
	scene()->update();
}

void PartsEditorConnectorsConnectorItem::init(bool resizable) {
	m_resized = false;

	setFlag(QGraphicsItem::ItemIsMovable, true);
#if QT_VERSION >= 0x040600
		this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
#endif

	setAcceptHoverEvents(resizable);
	m_showErrorIcon = false;
	m_geometryHasChanged = false;
	m_geometryHasChangedAlLeastOnce = false;
	m_resizedRect = QRectF();
	m_initialPos = pos();

	setResizable(resizable);
	if(m_resizable) {
		m_handlers = new ConnectorRectangle(this);
		m_handlers->setMinSize(MinWidth, MinHeight);

		connect(m_handlers, SIGNAL(resizeSignal(qreal, qreal, qreal, qreal)),
				this, SLOT(resizeSlot(qreal, qreal, qreal, qreal)));

		// isResizableSignal MUST be connected with Qt::DirectConnection
		connect(m_handlers, SIGNAL(isResizableSignal(bool &)),
				this, SLOT(isResizableSlot(bool &)),
				Qt::DirectConnection);
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
	showErrorIcon(isMismatching);
}

void PartsEditorConnectorsConnectorItem::showErrorIcon(bool showIt) {
	m_showErrorIcon = showIt;
}

void PartsEditorConnectorsConnectorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	painter->save();
	drawDottedRect(painter,Qt::black,Qt::white,rect());
	m_handlers->paint(painter);
	if(m_showErrorIcon) {
		paintErrorIcon(painter);
	}
	painter->restore();
}

void PartsEditorConnectorsConnectorItem::paintErrorIcon(QPainter *painter) {
	QRectF trgRect = m_handlers->errorIconRect();
	QPixmap pm(":resources/images/error_x_mini.png");
	QRectF srcRect = QRectF(pm.rect());
	painter->drawPixmap(trgRect,pm,srcRect);
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
	if(!m_terminalPointItem) m_terminalPointItem = newTerminalPointItem();

	if(m_geometryHasChanged && show) {
		m_terminalPointItem->reset();
		m_geometryHasChanged = false;
	}
	m_terminalPointItem->doSetVisible(show);

	// if we're showing the terminal points, then the connector
	// is not movable
	setFlag(QGraphicsItem::ItemIsMovable,!show);
	m_terminalPointItem->setMovable(show);
}

bool PartsEditorConnectorsConnectorItem::isShowingTerminalPoint() {
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
	if(m_inFileDefined && !m_geometryHasChangedAlLeastOnce) {
		PartsEditorView *gv = dynamic_cast<PartsEditorView*>(scene()->parent());
		if(gv) {
			gv->inFileDefinedConnectorChanged(this);
		}
	}
	//m_geometryHasChanged = true;
}

QRectF PartsEditorConnectorsConnectorItem::mappedRect() {
	return mapToParent(rect()).boundingRect();
}

void PartsEditorConnectorsConnectorItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	highlight(connectorSharedID());
	PartsEditorView *gv = dynamic_cast<PartsEditorView*>(scene()->parent());
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
	if(!m_showingTerminalPoint) {
		setCursor(QCursor(Qt::SizeAllCursor));
	}
}

void PartsEditorConnectorsConnectorItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	Q_UNUSED(event);
	unsetCursor();
}

void PartsEditorConnectorsConnectorItem::setResizable(bool resizable) {
	m_resizable = resizable;
}

void PartsEditorConnectorsConnectorItem::isResizableSlot(bool & resizable) {
	resizable = m_resizable && !isShowingTerminalPoint();
}

void PartsEditorConnectorsConnectorItem::resizeSlot(qreal x, qreal y, qreal width, qreal height) {
	prepareGeometryChange();
	resizeRect(x, y, width, height);
}

bool PartsEditorConnectorsConnectorItem::hasBeenMoved() const {
	return m_initialPos != pos();
}

bool PartsEditorConnectorsConnectorItem::hasBeenResized() const {
	return m_resized;
}

QPointF PartsEditorConnectorsConnectorItem::initialPos() const {
	return m_initialPos;
}
