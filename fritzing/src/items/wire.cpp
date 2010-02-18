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
#include <QComboBox>

#include "../debugdialog.h"
#include "../sketch/infographicsview.h"
#include "../connectors/connectoritem.h"
#include "../connectors/connectorshared.h"
#include "../layerattributes.h"
#include "../fsvgrenderer.h"
#include "../labels/partlabel.h"
#include "../model/modelpart.h"
#include "../utils/graphicsutils.h"

#include <stdlib.h>

QHash<QString, QString> Wire::colors;
QHash<QString, QString> Wire::shadowColors;
QHash<QString, QString> Wire::colorTrans;
QStringList Wire::colorNames;
QHash<long, QString> Wire::widthTrans;
QList<long> Wire::widths;
qreal Wire::STANDARD_TRACE_WIDTH;

////////////////////////////////////////////////////////////

bool alphaLessThan(QColor * c1, QColor * c2)
{
	return c1->alpha() < c2->alpha();
}

/////////////////////////////////////////////////////////////

Wire::Wire( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu* itemMenu)
	: ItemBase(modelPart, viewIdentifier, viewGeometry, id, itemMenu)
{
	m_connector0 = m_connector1 = NULL;
	m_partLabel = new PartLabel(this, NULL);
	m_canChainMultiple = false;
    setFlag(QGraphicsItem::ItemIsSelectable, true );
	m_connectorHover = NULL;
	m_opacity = 1.0;
	m_ignoreSelectionChange = false;

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

FSvgRenderer * Wire::setUp(ViewLayer::ViewLayerID viewLayerID, const LayerHash &  viewLayers, InfoGraphicsView * infoGraphicsView ) {
	ItemBase::setViewLayerID(viewLayerID, viewLayers);
	FSvgRenderer * svgRenderer = setUpConnectors(m_modelPart, m_viewIdentifier);
	if (svgRenderer != NULL) {
		initEnds(m_viewGeometry, svgRenderer->viewBox(), infoGraphicsView);
		setConnectorTooltips();
	}
	setZValue(this->z());

	return svgRenderer;
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

void Wire::initEnds(const ViewGeometry & vg, QRectF defaultRect, InfoGraphicsView * infoGraphicsView) {
	bool gotOne = false;
	bool gotTwo = false;
	qreal penWidth = 1;
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
			penWidth = item->rect().width();
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
	if (infoGraphicsView != NULL) {
		infoGraphicsView->initWire(this, penWidth);
	}

	prepareGeometryChange();
}

void Wire::paint (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) {
	if (m_hidden) return;

	painter->setOpacity(m_opacity);
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

void Wire::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
	//DebugDialog::debug("checking press event");
	emit wireSplitSignal(this, event->scenePos(), this->pos(), this->line());
}

void Wire::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	ItemBase::mousePressEvent(event);
}

void Wire::initDragEnd(ConnectorItem * connectorItem, QPointF scenePos) {
	Q_UNUSED(scenePos);
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


void Wire::mouseReleaseConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	Q_UNUSED(event);
	Q_UNUSED(connectorItem);
	releaseDrag();
}

void Wire::mouseMoveConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	mouseMoveEventAux(this->mapFromItem(connectorItem, event->pos()), (event->modifiers() & Qt::ShiftModifier) != 0);
}


void Wire::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	mouseMoveEventAux(event->pos(), (event->modifiers() & Qt::ShiftModifier) != 0);
}

void Wire::mouseMoveEventAux(QPointF eventPos, bool shiftModifier) {
	if (m_spaceBarWasPressed) {
		return;
	}

	if (m_dragEnd == false) {
		return;
	}

	ConnectorItem * whichConnectorItem;
	ConnectorItem * otherConnectorItem;
	if (m_drag0) {
		whichConnectorItem = m_connector0;
		otherConnectorItem = m_connector1;
	}
	else {
		whichConnectorItem = m_connector1;
		otherConnectorItem = m_connector0;
	}

	if (shiftModifier) {
		QPointF initialPos = mapFromScene(otherConnectorItem->sceneAdjustedTerminalPoint(NULL));  
		bool bendpoint = whichConnectorItem->connectionsCount() > 0;
		if (bendpoint) {
			foreach (ConnectorItem * ci, whichConnectorItem->connectedToItems()) {
				if (ci->attachedToItemType() != ModelPart::Wire) {
					bendpoint = false;
					break;
				}
			}
		}
		if (bendpoint) {
			bendpoint = false;
			foreach (ConnectorItem * ci, whichConnectorItem->connectedToItems()) {
				Wire * w = dynamic_cast<Wire *>(ci->attachedTo());
				ConnectorItem * oci = w->otherConnector(ci);
				QPointF otherInitialPos = mapFromScene(oci->sceneAdjustedTerminalPoint(NULL));
				QPointF p1(initialPos.x(), otherInitialPos.y());
				qreal d = ((p1.x() - eventPos.x()) * (p1.x() - eventPos.x())) +  ((p1.y() - eventPos.y()) * (p1.y() - eventPos.y()));
				if (d <= 144) {
					bendpoint = true;
					eventPos = p1;
					break;
				}
				p1.setX(otherInitialPos.x());
				p1.setY(initialPos.y());
				d = ((p1.x() - eventPos.x()) * (p1.x() - eventPos.x())) +  ((p1.y() - eventPos.y()) * (p1.y() - eventPos.y()));
				if (d <= 144) {
					bendpoint = true;
					eventPos = p1;
					break;
				}				

			}
		}

		if (!bendpoint) {
			eventPos = GraphicsUtils::calcConstraint(initialPos, eventPos);
		}

	}

	QPointF temp = this->mapToScene(eventPos);
	DebugDialog::debug(QString("wire move event %1,%2  %3").arg(temp.x()).arg(temp.y()).arg(m_drag0));


	if (m_drag0) {
		QPointF r = this->mapToScene(eventPos);
		this->setPos(r.x(), r.y());
		this->setLine(0, 0, m_wireDragOrigin.x() - r.x() + m_viewGeometry.loc().x(),
							m_wireDragOrigin.y() - r.y() + m_viewGeometry.loc().y() );
	}
	else {
		this->setLine(m_wireDragOrigin.x(), m_wireDragOrigin.y(), eventPos.x(), eventPos.y());
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
		// don't allow wire to connect back to something the other end is already directly connected to
		QList<Wire *> wires;
		QList<ConnectorItem *> ends;
		collectChained(otherConnectorItem, wires, ends);
		for (int i = 0; i < wires.count(); i++) {
			Wire * w = wires[i];
			collectChained(w->m_connector1, wires, ends);
			collectChained(w->m_connector0, wires, ends);
		}
		ends.append(otherConnectorItem);
		foreach (Wire * w, wires) {
			ends.append(w->connector0());
			ends.append(w->connector1());
		}
		foreach (ConnectorItem * toConnectorItem, whichConnectorItem->connectedToItems()) {
			ends.removeOne(toConnectorItem);
		}

		whichConnectorItem->setOverConnectorItem(
					findConnectorUnder(whichConnectorItem,  whichConnectorItem->overConnectorItem(), false, true, ends));
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
}

void Wire::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	if (m_spaceBarWasPressed) {
		return;
	}

	if (releaseDrag()) return;

	ItemBase::mouseReleaseEvent(event);
}

bool Wire::releaseDrag() {
	if (m_dragEnd == false) return false;

	m_dragEnd = false;

	ConnectorItem * from = (m_drag0) ? m_connector0 : m_connector1;
	ConnectorItem * to = from->overConnectorItem();
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

	return true;
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
	streamWriter.writeAttribute("mils", QString::number(mils()));
	streamWriter.writeAttribute("color", m_pen.brush().color().name());
	streamWriter.writeAttribute("opacity", QString::number(m_opacity));
	streamWriter.writeEndElement();
}

void Wire::setExtras(QDomElement & element, InfoGraphicsView * infoGraphicsView)
{
	if (element.isNull()) return;

	bool ok;
	qreal w = element.attribute("width").toDouble(&ok);
	if (ok) {
		setWireWidth(w, infoGraphicsView);
	}
	else {
		w = element.attribute("mils").toDouble(&ok);
		if (ok) {
			setWireWidth(GraphicsUtils::mils2pixels(w), infoGraphicsView);
		}
	}

	setColor(element);
}

void Wire::setColor(QDomElement & element) {
	QString colorString = element.attribute("color");
	if (colorString.isNull() || colorString.isEmpty()) return;

	bool ok;
	qreal op = element.attribute("opacity").toDouble(&ok);
	if (!ok) {
		op = 1.0;
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

void Wire::connectionChange(ConnectorItem * onMe, ConnectorItem * onIt, bool connect) {
	Q_UNUSED(onMe);
	if (connect && !onIt->attachedTo()->isVisible()) {
		this->setVisible(false);
	}

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

void Wire::mouseDoubleClickConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	Q_UNUSED(event);
	int chained = 0;
	foreach (ConnectorItem * toConnectorItem, connectorItem->connectedToItems()) {
		if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			chained++;
		}
		else {
			return;
		}
	}

	if (chained == 1) {
		emit wireJoinSignal(this, connectorItem);
	}
}

void Wire::mousePressConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	//DebugDialog::debug("checking press connector event");

	if (m_canChainMultiple && event->modifiers() == Qt::AltModifier) {
		// dragging a wire out of a bendpoint
		InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
		if (infoGraphicsView != NULL) {
			infoGraphicsView->mousePressConnectorEvent(connectorItem, event);
		}

		return;
	}


	connectorItem->setOverConnectorItem(NULL);
	initDragEnd(connectorItem, event->scenePos());

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
		p1 = from->sceneAdjustedTerminalPoint(to);
		ConnectorItem * otherFrom = m_connector1->firstConnectedToIsh();
		if (otherFrom == NULL) {
			p2 = m_connector1->mapToScene(m_connector1->rect().center());
		}
		else {
			p2 = otherFrom->sceneAdjustedTerminalPoint(m_connector1);
		}
	}
	else {
		p2 = from->sceneAdjustedTerminalPoint(to);
		ConnectorItem * otherFrom = m_connector0->firstConnectedToIsh();
		if (otherFrom == NULL) {
			p1 = m_connector0->mapToScene(m_connector0->rect().center());
		}
		else {
			p1 = otherFrom->sceneAdjustedTerminalPoint(m_connector0);
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
			p1 = from->sceneAdjustedTerminalPoint(m_connector0);
			p2 = this->line().p2() + p1;
		}
		else {
			p2 = from->sceneAdjustedTerminalPoint(m_connector1);
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
				findConnectorUnder(connectorItem,  connectorItem->overConnectorItem(), true, false, ConnectorItem::emptyConnectorItemList));
	}
}

void Wire::collectChained(QList<Wire *> & chained, QList<ConnectorItem *> & ends, QList<ConnectorItem *> & uniqueEnds ) {
	chained.append(this);
	for (int i = 0; i < chained.count(); i++) {
		Wire * wire = chained[i];
		collectChained(wire->m_connector1, chained, ends);
		collectChained(wire->m_connector0, chained, ends);
		if ((wire->m_connector0 != NULL) && wire->m_connector0->chained()) {
			uniqueEnds.append(wire->m_connector0);
		}
	}
}

void Wire::collectChained(ConnectorItem * connectorItem, QList<Wire *> & chained, QList<ConnectorItem *> & ends) {
	if (connectorItem == NULL) return;

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

bool Wire::stickyEnabled()
{
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

void Wire::setColor(const QColor & color, qreal op) {
	m_pen.setBrush(QBrush(color));
	m_opacity = op;
	m_colorName = color.name();
	this->update();
}

void Wire::setShadowColor(QColor & color) {
	m_shadowBrush = QBrush(color);
	m_shadowPen.setBrush(m_shadowBrush);
	m_bendpointPen.setBrush(m_shadowBrush);
	m_bendpoint2Pen.setBrush(m_shadowBrush);
	if (m_connector0) m_connector0->restoreColor(false, 0);
	if (m_connector0) m_connector1->restoreColor(false, 0);
	this->update();
}

const QColor & Wire::color() {
	return m_pen.brush().color();
}

void Wire::setWireWidth(qreal width, InfoGraphicsView * infoGraphicsView) {
	if (m_pen.widthF() == width) return;

	prepareGeometryChange();
	setPenWidth(width, infoGraphicsView);
	if (m_connector0) m_connector0->restoreColor(false, 0);
	if (m_connector1) m_connector1->restoreColor(false, 0);
	update();
}

qreal Wire::width() {
	return m_pen.widthF();
}

qreal Wire::mils() {
	return 1000 * m_pen.widthF() / FSvgRenderer::printerScale();
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

	widths << 16 << 24 << 32 << 48;
	widthTrans.insert(widths[0], tr("thin (16 mil)"));
	widthTrans.insert(widths[1], tr("standard (24 mil)"));
	widthTrans.insert(widths[2], tr("thick (32 mil)"));
	widthTrans.insert(widths[3], tr("extra thick (48 mil)"));

	STANDARD_TRACE_WIDTH = GraphicsUtils::mils2pixels(widths[1]);

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
	colors.insert("blackblack", "#000000");
	colors.insert("schematicGrey", "#9d9d9d");
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
	shadowColors.insert("trace",	"#d69b00");
	shadowColors.insert("unrouted", "#000000");
	shadowColors.insert("schematicGrey", "#1d1d1d");
	shadowColors.insert("blackblack", "#000000");
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

bool Wire::draggingEnd() {
	return m_dragEnd;
}

void Wire::setCanChainMultiple(bool can) {
	m_canChainMultiple = can;
}

bool Wire::canChangeColor() {
	if (getTrace() || getRatsnest()) return false;

	return true;
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
		if (m_partLabel) {
			m_partLabel->update();
		}

		if (!m_ignoreSelectionChange) {
			QList<Wire *> chained;
			QList<ConnectorItem *> ends;
			QList<ConnectorItem *> uniqueEnds;
			collectChained(chained, ends, uniqueEnds);
			InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
			if (infoGraphicsView) {
				infoGraphicsView->setIgnoreSelectionChangeEvents(true);
			}
			DebugDialog::debug(QString("original wire selected %1 %2").arg(value.toBool()).arg(this->id()));
			foreach (Wire * wire, chained) {
				if (wire != this ) {
					wire->setIgnoreSelectionChange(true);
					wire->setSelected(value.toBool());
					wire->setIgnoreSelectionChange(false);
					DebugDialog::debug(QString("wire selected %1 %2").arg(value.toBool()).arg(wire->id()));
				}
			}
			if (infoGraphicsView) {
				infoGraphicsView->setIgnoreSelectionChangeEvents(false);
			}
		}
    }
    return ItemBase::itemChange(change, value);
}

void Wire::cleanup() {
}

void Wire::getConnectedColor(ConnectorItem * connectorItem, QBrush * &brush, QPen * &pen, qreal & opacity, qreal & negativePenWidth) {

	int count = 0;
	bool bendpoint = true;
	foreach (ConnectorItem * toConnectorItem, connectorItem->connectedToItems()) {
		if (toConnectorItem->attachedToItemType() != ModelPart::Wire) {
			bendpoint = false;
			if (toConnectorItem->connectionsCount() > 1) {	
				InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
				if (infoGraphicsView != NULL && infoGraphicsView->hasBigDots()) {
					int c = 0;
					foreach (ConnectorItem * totoConnectorItem, toConnectorItem->connectedToItems()) {
						if (totoConnectorItem->attachedToItemType() == ModelPart::Wire) {
							Wire * w = dynamic_cast<Wire *>(totoConnectorItem->attachedTo());
							if (w && w->getTrace()) {
								c++;
							}
						}
					}
					if (c > 1) {
						count = 2;
						break;
					}
				}
			}

			ItemBase::getConnectedColor(connectorItem, brush, pen, opacity, negativePenWidth);
			return;
		}

		count++;
	}

	if (count == 0) {
		ItemBase::getConnectedColor(connectorItem, brush, pen, opacity, negativePenWidth);
		return;
	}
	
	// connectorItem is a bendpoint or connects to a multiply connected connector

	if (!bendpoint) {
		//DebugDialog::debug(QString("big dot %1 %2 %3").arg(this->id()).arg(connectorItem->connectorSharedID()).arg(count));
	}

	brush = &m_shadowBrush;
	opacity = 1.0;
	if (count > 1) {
		pen = &m_bendpoint2Pen;
		negativePenWidth = m_bendpoint2Width;
	}
	else {
		negativePenWidth = m_bendpointWidth;
		pen = &m_bendpointPen;

	}
}

void Wire::setPenWidth(qreal w, InfoGraphicsView * infoGraphicsView) {
	m_pen.setWidthF(w);
	infoGraphicsView->getBendpointWidths(this, w, m_bendpointWidth, m_bendpoint2Width);
	m_bendpointPen.setWidthF(qAbs(m_bendpointWidth));
	m_bendpoint2Pen.setWidthF(qAbs(m_bendpoint2Width));
	m_shadowPen.setWidthF(w + 2);
}

void Wire::getColor(QColor & color, const QString & name) {
	color.setNamedColor(colors.value(name));
}

bool Wire::connectionIsAllowed(ConnectorItem * to) {
	Wire * w = dynamic_cast<Wire *>(to->attachedTo());
	if (w == NULL) return true;

	if (w->getVirtual()) return false;

	return m_viewIdentifier != ViewIdentifierClass::BreadboardView;
}

bool Wire::isGrounded() {
	return ConnectorItem::isGrounded(connector0(), connector1());
}

bool Wire::acceptsMouseDoubleClickConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *) {
	return true;
}

bool Wire::acceptsMouseMoveConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *) {
	return true;
}

bool Wire::acceptsMouseReleaseConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *) {
	return true;
}

void Wire::setIgnoreSelectionChange(bool ignore) {
	m_ignoreSelectionChange = ignore;
}



bool Wire::collectExtraInfoHtml(const QString & family, const QString & prop, const QString & value, bool collectValues, QString & returnProp, QString & returnValue) {
	if (prop.compare("width", Qt::CaseInsensitive) == 0) {
		// don't display width property
		return false;
	}

	if (prop.compare("color", Qt::CaseInsensitive) == 0) {
		returnProp = tr("color");
		if (canChangeColor()) {
			returnValue = "<object type='application/x-qt-plugin' classid='WireColorInput' width='100%' height='22px'></object>";
		}
		else {
			returnValue = colorString();
		}
		return true;
	}

	return ItemBase::collectExtraInfoHtml(family, prop, value, collectValues, returnProp, returnValue);
}

QObject * Wire::createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues) {
	Q_UNUSED(url);

	if (classid.compare("WireColorInput", Qt::CaseInsensitive) != 0) {
		return ItemBase::createPlugin(parent, classid, url, paramNames, paramValues);
	}

	QComboBox * comboBox = new QComboBox(parent);
	comboBox->setEditable(false);
	
	int ix = 0;
	QString englishCurrColor = colorString();
	foreach(QString transColorName, Wire::colorNames) {
		QString englishColorName = Wire::colorTrans.value(transColorName);
		comboBox->addItem(transColorName, QVariant(englishColorName));
		if (englishColorName.compare(englishCurrColor, Qt::CaseInsensitive) == 0) {
			comboBox->setCurrentIndex(ix);
		}
		ix++;
	}

	connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(colorEntry(const QString &)));
	return comboBox;
}

void Wire::colorEntry(const QString & text) {
	Q_UNUSED(text);

	QComboBox * comboBox = dynamic_cast<QComboBox *>(sender());
	if (comboBox == NULL) return;

	QString color = comboBox->itemData(comboBox->currentIndex()).toString();

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->changeWireColor(color);
	}
}

bool Wire::hasPartLabel() {
	
	return false;
}
