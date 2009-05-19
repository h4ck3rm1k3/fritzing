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

#include "wire.h"

#include <QLineF>
#include <QPen>
#include <QRadialGradient>
#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
#include <QList>
#include <QGraphicsItem>
#include <QSet>

#include "../debugdialog.h"
#include "../infographicsview.h"
#include "../connectoritem.h"
#include "../connectorshared.h"
#include "../layerattributes.h"
#include "../fsvgrenderer.h"
#include "../labels/partlabel.h"
#include "../modelpart.h"

#include <stdlib.h>

QString Wire::moduleIDName = "WireModuleID";
QHash<QString, QString> Wire::colors;
QHash<QString, QString> Wire::shadowColors;
QHash<QString, QString> Wire::colorTrans;
QList<QString> Wire::colorNames;
QHash<QString, qreal> Wire::widthTrans;
QList<QString> Wire::widthNames;
QList<QColor *> ratsnestColors;
QColor schematicColor;

struct ConnectThing {
	Wire * wire;
	bool hasNone0;
	bool hasNone1;
	bool connectedIn0;
	bool connectedIn1;
	bool connectedOut0;
	bool connectedOut1;
};

////////////////////////////////////////////////////////////

static QHash<ViewIdentifierClass::ViewIdentifier, int> netColorIndex;

bool alphaLessThan(QColor * c1, QColor * c2)
{
	return c1->alpha() < c2->alpha();
}

/////////////////////////////////////////////////////////////

Wire::Wire( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu* itemMenu)
	: ItemBase(modelPart, viewIdentifier, viewGeometry, id, itemMenu)
{
	m_partLabel = new PartLabel(this, "", NULL);
	m_canChainMultiple = false;
    setFlag(QGraphicsItem::ItemIsSelectable, true );
	m_connectorHover = NULL;
	m_opacity = UNROUTED_OPACITY;

	//DebugDialog::debug(QString("aix line %1 %2 %3 %4").arg(this->viewGeometry().line().x1())
													//.arg(this->viewGeometry().line().y1())
													//.arg(this->viewGeometry().line().x2())
													//.arg(this->viewGeometry().line().y2()) );
	//DebugDialog::debug(QString("aix loc %1 %2").arg(this->viewGeometry().loc().x())
														//.arg(this->viewGeometry().loc().y()) );

	setPos(m_viewGeometry.loc());

	m_dragEnd = false;
}

Wire::~Wire() {

}

void Wire::setUp(ViewLayer::ViewLayerID viewLayerID, const LayerHash &  viewLayers ) {
	ItemBase::setViewLayerID(viewLayerID, viewLayers);
	FSvgRenderer * svgRenderer = setUpConnectors(m_modelPart, m_viewIdentifier);
	if (svgRenderer != NULL) {
		initEnds(m_viewGeometry, svgRenderer->viewBox());
		setConnectorTooltips();
	}
	setZValue(this->z());
}

void Wire::saveGeometry() {
	m_viewGeometry.setSelected(this->isSelected());
	m_viewGeometry.setLine(this->line());
	m_viewGeometry.setLoc(this->pos());
	m_viewGeometry.setZ(this->zValue());
}


bool Wire::itemMoved() {
	if  (m_viewGeometry.loc() != this->pos()) return true;

	if (this->line().dx() != m_viewGeometry.line().dx()) return false;
	if (this->line().dy() != m_viewGeometry.line().dy()) return false;

	return (this->line() != m_viewGeometry.line());
}

void Wire::moveItem(ViewGeometry & viewGeometry) {
	this->setLine(viewGeometry.line());
	this->setPos(viewGeometry.loc());
}

void Wire::initEnds(const ViewGeometry & vg, QRectF defaultRect) {
	bool gotOne = false;
	bool gotTwo = false;
	int penWidth = 1;
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * item = dynamic_cast<ConnectorItem *>(childItem);
		if (item == NULL) continue;

		// check the name or is order good enough?

		if (gotOne) {
			gotTwo = true;
			m_connector1 = item;
			break;
		}
		else {
			penWidth = (int) item->rect().width();
			m_connector0 = item;
			gotOne = true;
		}
	}

	if (!gotTwo) {
		return;
	}

	if ((vg.line().length() == 0) && (vg.line().x1() == -1)) {
		this->setLine(defaultRect.left(), defaultRect.top(), defaultRect.right(), defaultRect.bottom());
	}
	else {
		this->setLine(vg.line());
	}

	setConnector0Rect();
	setConnector1Rect();
	m_viewGeometry.setLine(this->line());
	
   	QBrush brush(QColor(0, 0, 0));
	QPen pen(brush, penWidth, Qt::SolidLine, Qt::RoundCap);
	this->setPen(pen);

	m_pen.setCapStyle(Qt::RoundCap);
	m_shadowPen.setCapStyle(Qt::RoundCap);
	switch (m_viewIdentifier) {
		case ViewIdentifierClass::BreadboardView:
			m_pen.setWidth(penWidth - 2);
			m_shadowPen.setWidth(penWidth);
            setColorString("blue", UNROUTED_OPACITY);
			break;
		case ViewIdentifierClass::SchematicView:
			setColorString("routed", UNROUTED_OPACITY);
			m_pen.setWidth(2);
			break;
		case ViewIdentifierClass::PCBView:
			setColorString("unrouted", UNROUTED_OPACITY);
			m_pen.setWidth(1);
			break;
		default:
			break;
	}

	prepareGeometryChange();
}

void Wire::paint (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) {
	if (m_hidden) return;

	painter->setOpacity(m_opacity);
	/*
	switch (m_viewIdentifier) {
		case ItemBase::BreadboardView:
			{
			painter->save();
			painter->setPen(m_shadowPen);
			QLineF line = this->line();
			painter->drawLine(line);
			painter->restore();
			ItemBase::paint(painter, option, widget);
			}
			break;
		case ItemBase::PCBView:
		case ItemBase::SchematicView:
		default:
			// assumes all wires in these views are selectable: jumper, ratsnest, trace
			ItemBase::paint(painter, option, widget);
			break;
	}
	*/

	if (!getRatsnest() && !getTrace()) {
		painter->save();
		painter->setPen(m_shadowPen);
		QLineF line = this->line();
		painter->drawLine(line);
		painter->restore();
	}
	ItemBase::paint(painter, option, widget);
}

void Wire::paintHover(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) 
{
	Q_UNUSED(widget);
	Q_UNUSED(option);
	painter->save();
	if ((m_connectorHoverCount > 0 && !m_dragEnd) || m_connectorHoverCount2 > 0) {
		painter->setOpacity(.50);
		painter->fillPath(this->hoverShape(), QBrush(connectorHoverColor));
	}
	else {
		painter->setOpacity(hoverOpacity);
		painter->fillPath(this->hoverShape(), QBrush(hoverColor));
	}
	painter->restore();
}

void Wire::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	ItemBase::mousePressEvent(event);
	if (m_spaceBarWasPressed) return;

	if (event->modifiers() & Qt::ShiftModifier) {
		emit wireSplitSignal(this, event->scenePos(), this->pos(), this->line());
	}
}

void Wire::initDragEnd(ConnectorItem * connectorItem) {
	saveGeometry();
	QLineF line = this->line();
	m_drag0 = (connectorItem == m_connector0);
	m_dragEnd = true;
	if (m_drag0) {
		m_wireDragOrigin = line.p2();
 		//DebugDialog::debug(QString("drag near origin %1 %2").arg(m_wireDragOrigin.x()).arg(m_wireDragOrigin.y()) );
	}
	else {
		m_wireDragOrigin = line.p1();
 		//DebugDialog::debug(QString("drag far origin %1 %2").arg(m_wireDragOrigin.x()).arg(m_wireDragOrigin.y()) );
 		//DebugDialog::debug(QString("drag far other %1 %2").arg(line.p2().x()).arg(line.p2().y()) );
	}

	if (connectorItem->chained()) {
		QList<Wire *> chained;
		QList<ConnectorItem *> ends;
		QList<ConnectorItem *> uniqueEnds;
		collectChained(chained, ends, uniqueEnds);
		// already saved the first one
		for (int i = 1; i < chained.count(); i++) {
			chained[i]->saveGeometry();
		}
	}
}

void Wire::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	if (m_spaceBarWasPressed) {
		event->ignore();
		return;
	}

	if (m_dragEnd == false) {
		if (event->modifiers() & Qt::ShiftModifier) {
			// bendpoint
			return;
		}

		// can't move a connected wire
		if (m_connector0->connectionsCount() > 0) return;
		if (m_connector1->connectionsCount() > 0) return;

		ItemBase::mouseMoveEvent(event);
		//updateConnections(m_connector0);
		//updateConnections(m_connector1);
		return;
	}

	ConnectorItem * whichConnectorItem;
	QPointF q = event->pos();
	if (m_drag0) {
		QPointF r = this->mapToScene(q);
		this->setPos(r.x(), r.y());
		this->setLine(0, 0, m_wireDragOrigin.x() - r.x() + m_viewGeometry.loc().x(),
							m_wireDragOrigin.y() - r.y() + m_viewGeometry.loc().y() );
		whichConnectorItem = m_connector0;

	}
	else {
		this->setLine(m_wireDragOrigin.x(), m_wireDragOrigin.y(), q.x(), q.y());
		whichConnectorItem = m_connector1;
	}
	setConnector1Rect();

	bool chained = false;
	foreach (ConnectorItem * toConnectorItem, whichConnectorItem->connectedToItems()) {
		Wire * chainedWire = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
		if (chainedWire == NULL) continue;

		chainedWire->simpleConnectedMoved(whichConnectorItem, toConnectorItem);
		chained = true;
	}

	if (!chained) {
		whichConnectorItem->setOverConnectorItem(
					findConnectorUnder(whichConnectorItem,  whichConnectorItem->overConnectorItem(), false));
	}
}

void Wire::setConnector0Rect() {
	QRectF rect = m_connector0->rect();
	rect.moveTo(0 - (rect.width()  / 2.0)  ,
				0 - (rect.height()  / 2.0) );
	m_connector0->setRect(rect);
}


void Wire::setConnector1Rect() {
	QRectF rect = m_connector1->rect();
	rect.moveTo(this->line().dx() - (rect.width()  / 2.0)  ,
				this->line().dy() - (rect.height()  / 2.0) );
	m_connector1->setRect(rect);
	emit posChangedSignal();
}

void Wire::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	if (m_spaceBarWasPressed) {
		event->ignore();
		return;
	}

	if (this->scene()->mouseGrabberItem() == this) {
		this->ungrabMouse();
	}

	ConnectorItem * from = (m_drag0) ? m_connector0 : m_connector1;
	ConnectorItem * to = NULL;
	if (m_dragEnd == true) {
		m_dragEnd = false;

		to = from->overConnectorItem();
		if (to != NULL) {
			to->connectorHover(this, false);

			// clean up
			from->setOverConnectorItem(NULL);
			from->clearConnectorHover();
		}


		QLineF newLine = this->line();
		QLineF oldLine = m_viewGeometry.line();
		QPointF oldPos = m_viewGeometry.loc();
		QPointF newPos = this->pos();
		if (newLine != oldLine || oldPos != newPos) {
			emit wireChangedSignal(this, oldLine, newLine, oldPos, newPos, from, to);
		}

		// don't call base class, since the base class will try to do a move
		return;
	}

	ItemBase::mouseReleaseEvent(event);
}

void Wire::saveInstanceLocation(QXmlStreamWriter & streamWriter)
{
	QLineF line = m_viewGeometry.line();
	QPointF loc = m_viewGeometry.loc();
	streamWriter.writeAttribute("x", QString::number(loc.x()));
	streamWriter.writeAttribute("y", QString::number(loc.y()));
	streamWriter.writeAttribute("x1", QString::number(line.x1()));
	streamWriter.writeAttribute("y1", QString::number(line.y1()));
	streamWriter.writeAttribute("x2", QString::number(line.x2()));
	streamWriter.writeAttribute("y2", QString::number(line.y2()));
	streamWriter.writeAttribute("wireFlags", QString::number(m_viewGeometry.flagsAsInt()));
}

void Wire::writeGeometry(QXmlStreamWriter & streamWriter) {
	ItemBase::writeGeometry(streamWriter);
	streamWriter.writeStartElement("wireExtras");
	streamWriter.writeAttribute("width", QString::number(m_pen.width()));
	streamWriter.writeAttribute("color", m_pen.brush().color().name());
	streamWriter.writeAttribute("opacity", QString::number(m_opacity));
	streamWriter.writeEndElement();
}

void Wire::setExtras(QDomElement & element)
{
	if (element.isNull()) return;

	bool ok;
	int w = element.attribute("width").toInt(&ok);
	if (ok) {
		setWidth(w);
	}

	setColor(element);
}

void Wire::setColor(QDomElement & element) {
	QString colorString = element.attribute("color");
	if (colorString.isNull() || colorString.isEmpty()) return;

	bool ok;
	qreal op = element.attribute("opacity").toDouble(&ok);
	if (!ok) {
		op = UNROUTED_OPACITY;
	}

	setColorString(colorString, op);
}

void Wire::hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event , ConnectorItem * item) {
	m_connectorHover = item;
	ItemBase::hoverEnterConnectorItem(event, item);
}

void Wire::hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item) {
	m_connectorHover = NULL;
	ItemBase::hoverLeaveConnectorItem(event, item);
}

void Wire::connectionChange(ConnectorItem * ) {
	bool movable = true;
	foreach (ConnectorItem * connectedTo, m_connector0->connectedToItems()) {
		if (connectedTo->attachedToItemType() != ModelPart::Wire) {
			movable = false;
			break;
		}
	}
	if (movable) {
		foreach (ConnectorItem * connectedTo, m_connector1->connectedToItems()) {
			if (connectedTo->attachedToItemType() != ModelPart::Wire) {
				movable = false;
				break;
			}
		}
	}
}

void Wire::mousePressConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	if (event->modifiers() == Qt::ShiftModifier) {

		int chained = 0;
		foreach (ConnectorItem * toConnectorItem, connectorItem->connectedToItems()) {
			if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
				chained++;
			}
		}

		if (chained == 1) {
			emit wireJoinSignal(this, connectorItem);
			return;
		}
	}

	if (m_canChainMultiple && event->modifiers() == Qt::ControlModifier) {
		InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
		if (infoGraphicsView != NULL) {
			infoGraphicsView->mousePressConnectorEvent(connectorItem, event);
		}

		return;
	}


	connectorItem->setOverConnectorItem(NULL);
	initDragEnd(connectorItem);

	// connector item currently has the mouse, so call grab mouse to get mouse events to wire
	this->grabMouse();
}

void Wire::simpleConnectedMoved(ConnectorItem * to) {
	// to is this wire, from is something else
	simpleConnectedMoved(to->firstConnectedToIsh(), to);
}

void Wire::simpleConnectedMoved(ConnectorItem * from, ConnectorItem * to)
{
	if (from == NULL) return;

	// to is this wire, from is something else
	QPointF p1, p2;
	calcNewLine(from, to, p1, p2);

	/*
	QPointF oldPos = this->pos();
	QPointF newPos = p1;
	QLineF oldLine = this->line();
	QLineF newLine(0, 0,  p2.x() - p1.x(), p2.y() - p1.y());
	if (qAbs(oldPos.x() - newPos.x()) > 1.75 ||
		qAbs(oldPos.y() - newPos.y()) > 1.75 ||
		qAbs(oldLine.x1() - newLine.x1()) > 1.75 ||
		qAbs(oldLine.x2() - newLine.x2()) > 1.75 ||
		qAbs(oldLine.y1() - newLine.y1()) > 1.75 ||
		qAbs(oldLine.y2() - newLine.y2()) > 1.75
		)
	{
		DebugDialog::debug("line changed");
		calcNewLine(from,to,p1,p2);
	}
	*/

	this->setPos(p1);
	this->setLine(0,0, p2.x() - p1.x(), p2.y() - p1.y() );
	//DebugDialog::debug(QString("set line %5: %1 %2, %3 %4, vis:%6 lyr:%7").arg(p1.x()).arg(p1.y()).arg(p2.x()).arg(p2.y()).arg(id()).arg(isVisible()).arg(m_viewIdentifier) );
	setConnector1Rect();
}

void Wire::calcNewLine(ConnectorItem * from, ConnectorItem * to, QPointF & p1, QPointF & p2) {
	// to is this wire, from is something else
	if (to == m_connector0) {
		p1 = from->sceneAdjustedTerminalPoint();
		ConnectorItem * otherFrom = m_connector1->firstConnectedToIsh();
		if (otherFrom == NULL) {
			p2 = m_connector1->mapToScene(m_connector1->rect().center());
		}
		else {
			p2 = otherFrom->sceneAdjustedTerminalPoint();
		}
	}
	else {
		p2 = from->sceneAdjustedTerminalPoint();
		ConnectorItem * otherFrom = m_connector0->firstConnectedToIsh();
		if (otherFrom == NULL) {
			p1 = m_connector0->mapToScene(m_connector0->rect().center());
		}
		else {
			p1 = otherFrom->sceneAdjustedTerminalPoint();
		}

	}
}

void Wire::connectedMoved(ConnectorItem * from, ConnectorItem * to) {
	// "from" is the connector on the part
	// "to" is the connector on the wire

	simpleConnectedMoved(from, to);
	return;

	/*
	DebugDialog::debug(QString("connected moved %1 %2, %3 %4")
		.arg(from->attachedToID())
		.arg(from->attachedToTitle())
		.arg(to->attachedToID())
		.arg(to->attachedToTitle())
		);
	*/

	ConnectorItem * otherEnd = otherConnector(to);
	bool chained = otherEnd->chained();
	QPointF p1, p2;
	if (chained) {
		// move both ends
		if (to == m_connector0) {
			p1 = from->sceneAdjustedTerminalPoint();
			p2 = this->line().p2() + p1;
		}
		else {
			p2 = from->sceneAdjustedTerminalPoint();
			p1 = p2 - this->line().p2();
		}
	}
	else {
		calcNewLine(from, to, p1, p2);
	}
	this->setPos(p1);
	this->setLine(0,0, p2.x() - p1.x(), p2.y() - p1.y() );
	//DebugDialog::debug(QString("set line %5: %1 %2, %3 %4, vis:%6 lyr:%7").arg(p1.x()).arg(p1.y()).arg(p2.x()).arg(p2.y()).arg(id()).arg(isVisible()).arg(m_viewIdentifier) );
	setConnector1Rect();

	if (chained) {
		foreach (ConnectorItem * otherEndTo, otherEnd->connectedToItems()) {
			if (otherEndTo->attachedToItemType() == ModelPart::Wire) {
				otherEndTo->attachedTo()->connectedMoved(otherEnd, otherEndTo);
			}
		}
	}
}


FSvgRenderer * Wire::setUpConnectors(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier) {

	LayerAttributes layerAttributes;
	FSvgRenderer * renderer = ItemBase::setUpImage(modelPart, viewIdentifier, m_viewLayerID, layerAttributes);
	if (renderer == NULL) {
		return NULL;
	}

	foreach (Connector * connector, m_modelPart->connectors().values()) {
		if (connector == NULL) continue;

		QRectF connectorRect;
		QPointF terminalPoint;
		qreal radius, strokeWidth;
		bool result = connector->setUpConnector(renderer, m_modelPart->moduleID(), m_viewIdentifier, m_viewLayerID, connectorRect, terminalPoint, radius, strokeWidth, false);
		if (!result) continue;

		ConnectorItem * connectorItem = newConnectorItem(connector);

		connectorItem->setRect(connectorRect);
		connectorItem->setTerminalPoint(terminalPoint);

		connectorItem->setCircular(true);
		//DebugDialog::debug(QString("terminal point %1 %2").arg(terminalPoint.x()).arg(terminalPoint.y()) );


		Bus * bus = connectorItem->bus();
		if (bus != NULL) {
			addBusConnectorItem(bus, connectorItem);
		}
	}

	return renderer;
}


// helpful for debugging
void Wire::setLine(QLineF line) {
	GraphicsSvgLineItem::setLine(line);
	//DebugDialog::debug(QString("set line %5 %6, %7 %8, %1 %2 %3 %4").arg(line.x1()).arg(line.y1()).arg(line.x2()).arg(line.y2())
		//.arg(id()).arg(m_viewIdentifier).arg(this->pos().x()).arg(this->pos().y()) );
}

void Wire::setLine(qreal x1, qreal y1, qreal x2, qreal y2) {
	GraphicsSvgLineItem::setLine(x1, y1, x2, y2);
	//DebugDialog::debug(QString("set line %5 %6, %7 %8, %1 %2 %3 %4").arg(x1).arg(y1).arg(x2).arg(y2)
		//.arg(id()).arg(m_viewIdentifier).arg(this->pos().x()).arg(this->pos().y()) );
}


void Wire::setLineAnd(QLineF line, QPointF pos, bool useLine) {
	this->setPos(pos);
	if (useLine) this->setLine(line);

	setConnector1Rect();
}

ConnectorItem * Wire::otherConnector(ConnectorItem * oneConnector) {
	if (oneConnector == m_connector0) return m_connector1;

	return m_connector0;
}

ConnectorItem * Wire::connector0() {
	return m_connector0;
}

ConnectorItem * Wire::connector1() {
	return m_connector1;
}

void Wire::findConnectorsUnder() {
	for (int i = 0; i < childItems().count(); i++) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItems()[i]);
		if (connectorItem == NULL) continue;

		connectorItem->setOverConnectorItem(
				findConnectorUnder(connectorItem,  connectorItem->overConnectorItem(), true));
	}
}

void Wire::collectChained(QList<Wire *> & chained, QList<ConnectorItem *> & ends, QList<ConnectorItem *> & uniqueEnds ) {
	chained.append(this);
	for (int i = 0; i < chained.count(); i++) {
		Wire * wire = chained[i];
		collectChained(wire->m_connector1, chained, ends);
		collectChained(wire->m_connector0, chained, ends);
		if (wire->m_connector0->chained()) {
			uniqueEnds.append(wire->m_connector0);
		}
	}
}

void Wire::collectChained(ConnectorItem * connectorItem, QList<Wire *> & chained, QList<ConnectorItem *> & ends) {
	foreach (ConnectorItem * connectedToItem, connectorItem->connectedToItems()) {
		Wire * wire = dynamic_cast<Wire *>(connectedToItem->attachedTo());
		if (wire == NULL) {
			if (!ends.contains(connectedToItem)) {
				ends.append(connectedToItem);
			}
			continue;
		}

		if (chained.contains(wire)) continue;
		chained.append(wire);
	}
}

void Wire::collectWires(QList<Wire *> & wires) {
	if (wires.contains(this)) return;

	wires.append(this);
	//DebugDialog::debug(QString("collecting wire %1").arg(this->id()) );
	collectWiresAux(wires, m_connector0);
	collectWiresAux(wires, m_connector1);
}

void Wire::collectWiresAux(QList<Wire *> & wires, ConnectorItem * start) {
	foreach (ConnectorItem * toConnectorItem, start->connectedToItems()) {
		if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			dynamic_cast<Wire *>(toConnectorItem->attachedTo())->collectWires(wires);
		}
	}

}

bool Wire::stickyEnabled(ItemBase * stickTo)
{
	Q_UNUSED(stickTo);
	return (connector0()->connectionsCount() <= 0) && (connector1()->connectionsCount() <= 0);
}

void Wire::setTrace(bool trace) {
	m_viewGeometry.setTrace(trace);
}

bool Wire::getTrace() {
	return m_viewGeometry.getTrace();
}

bool Wire::getRouted() {
	return m_viewGeometry.getRouted();
}

void Wire::setRouted(bool routed) {
	m_viewGeometry.setRouted(routed);
}

bool Wire::getVirtual() {
	return m_viewGeometry.getVirtual();
}

void Wire::setJumper(bool jumper) {
	m_viewGeometry.setJumper(jumper);
}

bool Wire::getJumper() {
	return m_viewGeometry.getJumper();
}

void Wire::setRatsnest(bool ratsnest) {
	m_viewGeometry.setRatsnest(ratsnest);
}

bool Wire::getRatsnest() {
	return m_viewGeometry.getRatsnest();
}

void Wire::setAutoroutable(bool ratsnest) {
	m_viewGeometry.setAutoroutable(ratsnest);
}

bool Wire::getAutoroutable() {
	return m_viewGeometry.getAutoroutable();
}

void Wire::setNormal(bool normal) {
	m_viewGeometry.setNormal(normal);
}

bool Wire::getNormal() {
	return m_viewGeometry.getNormal();
}

void Wire::setColor(QColor & color, qreal op) {
	m_pen.setBrush(QBrush(color));
	m_opacity = op;
	m_colorName = color.name();
	this->update();
}

void Wire::setShadowColor(QColor & color) {
	m_shadowPen.setBrush(QBrush(color));
	this->update();
}

const QColor * Wire::color() {
	return &m_pen.brush().color();
}

void Wire::setWidth(int width) {
	if (m_pen.width() == width) return;

	prepareGeometryChange();
	m_pen.setWidth(width);
	m_shadowPen.setWidth(width + 2);
	update();
}

int Wire::width() {
	return m_pen.width();
}

void Wire::setColorString(QString colorName, qreal op) {
	// sets a color using the name (.e. "red")
	// note: colorName is associated with a Fritzing color, not a Qt color

	QString colorString = colors.value(colorName, "");
	if (colorString.isEmpty()) {
		colorString = colorName;

		foreach (QString c, colors.keys()) {
			if (colors.value(c).compare(colorName, Qt::CaseInsensitive) == 0) {
				colorName = c;
				break;
			}
		}
	}

	QColor c;
	c.setNamedColor(colorString);
	setColor(c, op);
	m_colorName = colorName;

	colorString = shadowColors.value(colorName, "");
	if (colorString.isEmpty()) {
		colorString = colorName;
	}

	c.setNamedColor(colorString);
	setShadowColor(c);
}

QString Wire::hexString() {
	return m_pen.brush().color().name();
}

QString Wire::colorString() {
	return m_colorName;
}

void Wire::initNames() {
	if (colors.count() > 0) return;

	widthNames.append(tr("thin"));
	widthNames.append(tr("medium"));
	widthNames.append(tr("wide"));

	widthTrans.insert(tr("thin"), 1);
	widthTrans.insert(tr("medium"), 3);
	widthTrans.insert(tr("wide"), 5);

    // need a list because a hash table doesn't guarantee order
    colorNames.append(tr("blue"));
	colorNames.append(tr("red"));
    colorNames.append(tr("black"));
	colorNames.append(tr("yellow"));
	colorNames.append(tr("green"));
	colorNames.append(tr("grey"));
	colorNames.append(tr("white"));
	colorNames.append(tr("orange"));

	// need this hash table to translate from user's language to internal color name
    colorTrans.insert(tr("blue"), "blue");
	colorTrans.insert(tr("red"), "red");
    colorTrans.insert(tr("black"), "black");
	colorTrans.insert(tr("yellow"), "yellow");
	colorTrans.insert(tr("green"), "green");
	colorTrans.insert(tr("grey"), "grey");
	colorTrans.insert(tr("white"), "white");
	colorTrans.insert(tr("orange"), "orange");

    colors.insert("blue",	"#418dd9");
	colors.insert("red",	"#cc1414");
    colors.insert("black",	"#404040");
	colors.insert("yellow", "#ffe24d");
	colors.insert("green",	"#47cc79");
	colors.insert("grey",	"#999999");
	colors.insert("white",	"#ffffff");
	colors.insert("orange", "#ff7033");
    colors.insert("jumper", "#6699cc");
	colors.insert("trace",  "#ffbf00");
	colors.insert("unrouted", "#000000");
	colors.insert("routed", "#7d7d7d");
	colors.insert("purple", "#b673e6");
	colors.insert("brown", "#8c3b00");

    shadowColors.insert("blue",		"#1b5bb3");
	shadowColors.insert("red",		"#8c0000");
    shadowColors.insert("black",	"#000000");
	shadowColors.insert("yellow",	"#e6ab00");
	shadowColors.insert("green",	"#00a63d");
	shadowColors.insert("grey",		"#666666");
	shadowColors.insert("white",	"#999999");
	shadowColors.insert("orange",	"#d95821");
    shadowColors.insert("jumper",	"#2d6563");
	shadowColors.insert("trace",	"#ffbf00");
	shadowColors.insert("unrouted", "#000000");
	shadowColors.insert("routed",	"#7d7d7d");

	netColorIndex.insert(ViewIdentifierClass::BreadboardView, 0);
	netColorIndex.insert(ViewIdentifierClass::SchematicView, 0);
	netColorIndex.insert(ViewIdentifierClass::PCBView, 0);

	QFile file(":/resources/ratsnestcolors.txt");
	file.open(QFile::ReadOnly);
	QTextStream stream( &file );
	while(!stream.atEnd()) {
		QString line = stream.readLine();
		if (line.contains(",")) {
			QStringList strings = line.split(",");
			if (strings.count() == 2) {
				QColor * c = new QColor;
				c->setNamedColor(strings[1]);
				ratsnestColors.append(c);
			}
		}
	}
	file.close();

	schematicColor.setNamedColor(colors.value("black"));

	/*
	makeHues(80, 340, 5, 0, ratsnestColors);
	qSort(ratsnestColors.begin(), ratsnestColors.end(), alphaLessThan);
	foreach (QColor * c, ratsnestColors) {
		c->setAlpha(255);
	}
	*/

}

void Wire::makeHues(int hue1, int hue2, int maxCount, int currentCount, QList<QColor *> & hues) {
	if (currentCount >= maxCount) return;

	int avg = (hue1 + hue2) / 2;
	//DebugDialog::debug(QString("making hue %1 from %2 %3").arg(avg).arg(hue1).arg(hue2) );
	makeHue(avg, hues, currentCount);
	makeHues(hue1, avg, maxCount, currentCount + 1, hues);
	makeHues(avg, hue2, maxCount, currentCount + 1, hues);
}

void Wire::makeHue(int hue, QList<QColor *> & hues, int currentCount) {
	QColor * c = new QColor();
	c->setHsv(hue, 40 * 255 / 100, 90 * 255 / 100);
	c->setAlpha(currentCount);							// this is a hack so we can sort them later
	hues.append(c);
}

bool Wire::hasFlag(ViewGeometry::WireFlag flag)
{
	return m_viewGeometry.hasFlag(flag);
}

bool Wire::hasAnyFlag(ViewGeometry::WireFlags flags)
{
	return m_viewGeometry.hasAnyFlag(flags);
}

Wire * Wire::findJumperOrTraced(ViewGeometry::WireFlags flags, QList<ConnectorItem *>  & ends) {
	QList<Wire *> chainedWires;
	QList<ConnectorItem *> uniqueEnds;
	this->collectChained(chainedWires, ends, uniqueEnds);
	if (ends.count() != 2) {
		DebugDialog::debug(QString("wire in jumper or trace must have two ends") );
		return NULL;
	}

	return ends[0]->wiredTo(ends[1], flags);
}

QRgb Wire::getRgb(const QString & name) {
	QString str = colors.value(name);
	QColor c;
	c.setNamedColor(str);
	return c.rgb();
}

ViewGeometry::WireFlags Wire::wireFlags() {
	return m_viewGeometry.wireFlags();
}

void Wire::setWireFlags(ViewGeometry::WireFlags wireFlags) {
	m_viewGeometry.setWireFlags(wireFlags);
}

qreal Wire::opacity() {
	return m_opacity;
}


void Wire::setOpacity(qreal opacity) {
	m_opacity = opacity;
	this->update();
}

const QColor * Wire::netColor(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
	if (viewIdentifier == ViewIdentifierClass::SchematicView) {
		return &schematicColor;
	}

	int csi = netColorIndex.value(viewIdentifier);
	QColor * c = ratsnestColors[csi];
	csi = (csi + 1) % ratsnestColors.count();
	netColorIndex.insert(viewIdentifier, csi);
	return c;
}

bool Wire::draggingEnd() {
	return m_dragEnd;
}

void Wire::connectsWithin(QSet<ItemBase *> & in, QHash<Wire *, ConnectorItem *> & out) {
	QList<Wire *> chained;
	QList<ConnectorItem *> ends;
	QList<ConnectorItem *> uniqueEnds;
	collectChained(chained, ends, uniqueEnds);
	bool selected = false;
	
	QVector<ConnectThing> connectThings(chained.count());

	int ix = 0;
	foreach (Wire * wire, chained) {
		if (wire->isSelected()) {
			// if one is selected, all are selected
			selected = true;
		}
		ConnectThing * ct = &connectThings[ix++];
		ct->wire = wire;
		wire->connectsWithin(wire->connector0(), in, chained, ct->hasNone0, ct->connectedIn0, ct->connectedOut0);
		wire->connectsWithin(wire->connector1(), in, chained, ct->hasNone1, ct->connectedIn1, ct->connectedOut1);
	}

	// do the easy case first
	bool hasNone = false;
	bool connectedOut = false;
	foreach (ConnectThing ct, connectThings) {
		if (ct.hasNone0 || ct.hasNone1) hasNone = true;
		if (ct.connectedOut0 || ct.connectedOut1) connectedOut = true;
	}

	if (!connectedOut) {
		if ((!hasNone) || selected) {
			// either the wires all connect to the parts, or there are some dangling ends, but the wires are all selected
			foreach (Wire * wire, chained) {
				in.insert(wire);
			}
			return;
		}
	}

	foreach (ConnectThing ct, connectThings) {
		if ((ct.connectedIn0 || (ct.hasNone0 && selected)) && (ct.connectedIn1 || (ct.hasNone1 && selected))) {
			// can drag this one
			in.insert(ct.wire);
			continue;
		}

		if (ct.connectedIn0) {
			out.insert(ct.wire, ct.wire->connector0());
			continue;
		}

		if (ct.connectedIn1) {
			out.insert(ct.wire, ct.wire->connector1());
			continue;
		}

		if (ct.connectedOut0 || (ct.hasNone0 && !selected)) {
			out.insert(ct.wire, ct.wire->connector1());
			continue;
		}

		if (ct.connectedOut1 || (ct.hasNone1 && !selected)) {
			out.insert(ct.wire, ct.wire->connector0());
			continue;
		}			
	}
}

void Wire::connectsWithin(ConnectorItem * connectorItem, QSet<ItemBase *> & in, QList<Wire *> & wires,
						  bool & hasNone, bool & connectedIn, bool & connectedOut) 
{
	hasNone = connectedIn = connectedOut = false;
	if (connectorItem->connectionsCount() == 0) {
		hasNone = true;
		return;
	}
	
	foreach (ConnectorItem * toConnectorItem, connectorItem->connectedToItems()) {
		ItemBase * attachedTo = toConnectorItem->attachedTo();
		while (attachedTo->parentItem() != NULL) {
			attachedTo = dynamic_cast<ItemBase *>(attachedTo->parentItem());
		}

		if (in.contains(attachedTo)) {
			connectedIn = true;
			continue;
		}

		if (attachedTo->itemType() == ModelPart::Wire) {
			if (wires.contains(dynamic_cast<Wire *>(attachedTo))) {
				// connected to another wire in our set, don't mark anything
				continue;
			}
		}

		connectedOut = true;
	}
}

void Wire::setCanChainMultiple(bool can) {
	m_canChainMultiple = can;
}

bool Wire::canChangeColor() {
	if (getTrace() || getRatsnest()) return false;

	return true;
}

bool Wire::canChangeWidth() {
	return getTrace();
}


void Wire::collectDirectWires(QList<Wire *> & wires) {
	if (!wires.contains(this)) {
		wires.append(this);
	}

	collectDirectWires(m_connector0, wires);
	collectDirectWires(m_connector1, wires);
}

void Wire::collectDirectWires(ConnectorItem * connectorItem, QList<Wire *> & wires) {
	if (connectorItem->connectionsCount() != 1) return;

	ConnectorItem * toConnectorItem = connectorItem->connectedToItems()[0];
	if (toConnectorItem->attachedToItemType() != ModelPart::Wire) return;

	Wire * nextWire = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
	if (wires.contains(nextWire)) return;

	wires.append(nextWire);
	nextWire->collectDirectWires(nextWire->otherConnector(toConnectorItem), wires);
}

QVariant Wire::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedChange) {
		if (m_partLabel) m_partLabel->update();
    }
    return ItemBase::itemChange(change, value);
}

void Wire::cleanup() {
	foreach (QColor * color, ratsnestColors) {
		delete color;
	}

	ratsnestColors.clear();
}
