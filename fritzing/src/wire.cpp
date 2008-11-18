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

#include "wire.h"

#include <QLineF>
#include <QPen>
#include <QRadialGradient>
#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
#include <QList>
#include <QGraphicsItem>

#include "debugdialog.h"
#include "infographicsview.h"
#include "connectoritem.h"
#include "connectorstuff.h"
#include "layerattributes.h"
#include "busconnectoritem.h"

#include <stdlib.h>

QString Wire::moduleIDName = "WireModuleID";
QHash<QString, QString> Wire::colors;
QHash<QString, QString> Wire::shadowColors;
QHash<QString, QString> Wire::colorTrans;
QList<QString> Wire::colorNames;
QList<QColor *> ratsnestColors;

////////////////////////////////////////////////////////////

static QHash<ItemBase::ViewIdentifier, int> colorStringIndex;

bool alphaLessThan(QColor * c1, QColor * c2)
{
	return c1->alpha() < c2->alpha();
}

/////////////////////////////////////////////////////////////

Wire::Wire( ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu* itemMenu)
	: ItemBase(modelPart, viewIdentifier, viewGeometry, id, true, itemMenu)
{
    setFlags(QGraphicsItem::ItemIsSelectable );
	m_grabbedMouse = false;
	m_connectorHover = NULL;
	m_autoroutable = true;
	m_opacity = 1.0;

	//DebugDialog::debug(QObject::tr("aix line %1 %2 %3 %4").arg(this->viewGeometry().line().x1())
													//.arg(this->viewGeometry().line().y1())
													//.arg(this->viewGeometry().line().x2())
													//.arg(this->viewGeometry().line().y2()) );
	//DebugDialog::debug(QObject::tr("aix loc %1 %2").arg(this->viewGeometry().loc().x())
														//.arg(this->viewGeometry().loc().y()) );

	setPos(m_viewGeometry.loc());

	m_dragEnd = false;
}

Wire::~Wire() {

}

void Wire::setUp(ViewLayer::ViewLayerID viewLayerID, const LayerHash &  viewLayers ) {
	ItemBase::setViewLayerID(viewLayerID, viewLayers);
	QSvgRenderer * svgRenderer = setUpConnectors(m_modelPart, m_viewIdentifier);
	if (svgRenderer != NULL) {
		initEnds(m_viewGeometry, svgRenderer->viewBox());
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

void Wire::rotateItem(qreal /* degrees */) {
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
		case ItemBase::BreadboardView:
			m_pen.setWidth(penWidth - 2);
			m_shadowPen.setWidth(penWidth);
			setColorString("red", 1.0);
			break;
		case ItemBase::SchematicView:
			setColorString("routed", 1.0);
			m_pen.setWidth(2);
			break;
		case ItemBase::PCBView:
			setColorString("unrouted", 1.0);
			m_pen.setWidth(1);
			break;
		default:
			break;
	}
}

void Wire::paint (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) {
	if (m_hidden) return;

	painter->setOpacity(m_opacity);
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

}

void Wire::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	ItemBase::mousePressEvent(event);

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
 		//DebugDialog::debug(QObject::tr("drag near origin %1 %2").arg(m_wireDragOrigin.x()).arg(m_wireDragOrigin.y()) );
	}
	else {
		m_wireDragOrigin = line.p1();
 		//DebugDialog::debug(QObject::tr("drag far origin %1 %2").arg(m_wireDragOrigin.x()).arg(m_wireDragOrigin.y()) );
 		//DebugDialog::debug(QObject::tr("drag far other %1 %2").arg(line.p2().x()).arg(line.p2().y()) );
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
	if (m_dragEnd == false) {
		if (event->modifiers() & Qt::ShiftModifier) {
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

	if (whichConnectorItem->chained()) {
		updateConnections(whichConnectorItem);
	}
	else {
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
	if (m_grabbedMouse) {
		m_grabbedMouse = false;
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
		op = 1.0;
	}

	foreach (QString colorName, colors.keys()) {
		if (colors.value(colorName).compare(colorString) == 0) {
			setColorString(colorName, op);
			return;
		}
	}

	QColor c;
	c.setNamedColor(colorString);
	setColor(c, op);
	m_colorName = "";
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
	this->setFlag(QGraphicsItem::ItemIsMovable, movable);
}

void Wire::mousePressConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	if (event->modifiers() == Qt::ShiftModifier) {
		if (connectorItem->chained()) {
			if (connectorItem->connectionsCount() == 1) {
				emit wireJoinSignal(this, connectorItem);
			}
			return;
		}
	}


	connectorItem->setOverConnectorItem(NULL);
	initDragEnd(connectorItem);

	// connector item currently has the mouse, so call grab mouse to get mouse events to wire
	m_grabbedMouse = true;
	this->grabMouse();
}

void Wire::connectedMoved(ConnectorItem * from, ConnectorItem * to) {
	// "from" is the connector on the part
	// "to" is the connector on the wire


	//DebugDialog::debug(QObject::tr("connected moved %1 %2 ")
	//.arg(m_connector0->connectionsCount())
	//.arg(m_connector1->connectionsCount()));


	if (false && ((m_connector1->connectionsCount() == 0) || (m_connector0->connectionsCount() == 0))) {
		// if only one connector is attached move the wire
		QPointF p1 = from->sceneAdjustedTerminalPoint();
		QPointF p2 = to->sceneAdjustedTerminalPoint();

		this->moveBy(p1.x() - p2.x(), p1.y() - p2.y());
	}
	else {
		QPointF p1, p2;
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
			QPointF temp = from->adjustedTerminalPoint();
			//DebugDialog::debug(QString("from tp %1 %2, %3 %4").arg(p2.x()).arg(p2.y()).arg(temp.x()).arg(temp.y()) );

			ConnectorItem * otherFrom = m_connector0->firstConnectedToIsh();
			if (otherFrom == NULL) {
				p1 = m_connector0->mapToScene(m_connector0->rect().center());
			}
			else {
				p1 = otherFrom->sceneAdjustedTerminalPoint();
			}

		}
		this->setPos(p1);
		this->setLine(0,0, p2.x() - p1.x(), p2.y() - p1.y() );
		//DebugDialog::debug(QString("set line %5: %1 %2, %3 %4, vis:%6 lyr:%7").arg(p1.x()).arg(p1.y()).arg(p2.x()).arg(p2.y()).arg(id()).arg(isVisible()).arg(m_viewIdentifier) );
		setConnector1Rect();
	}


}


QSvgRenderer * Wire::setUpConnectors(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier) {

	LayerAttributes layerAttributes;
	QSvgRenderer * renderer = PaletteItemBase::setUpImage(modelPart, viewIdentifier, m_viewLayerID, layerAttributes);
	if (renderer == NULL) {
		return NULL;
	}

	foreach (Connector * connector, m_modelPart->connectors().values()) {
		if (connector == NULL) continue;

		QRectF connectorRect;
		QPointF terminalPoint;
		bool result = connector->setUpConnector(renderer, m_viewIdentifier, m_viewLayerID, connectorRect, terminalPoint, false);
		if (!result) continue;

		ConnectorItem * connectorItem = newConnectorItem(connector);

		connectorItem->setRect(connectorRect);
		connectorItem->setTerminalPoint(terminalPoint);

		connectorItem->setCircular(true);
		//DebugDialog::debug(tr("terminal point %1 %2").arg(terminalPoint.x()).arg(terminalPoint.y()) );
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

ItemBase * Wire::layerKinChief()
{
	return this;
}

void Wire::findConnectorsUnder() {
	for (int i = 0; i < childItems().count(); i++) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItems()[i]);
		if (connectorItem == NULL) continue;

		connectorItem->setOverConnectorItem(
				findConnectorUnder(connectorItem,  connectorItem->overConnectorItem(), true));
	}
}

void Wire::updateConnections(ConnectorItem * item) {
	item->attachedMoved();
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
		if (connectedToItem->chained()) {
			Wire * wire = dynamic_cast<Wire *>(connectedToItem->attachedTo());
			if (wire == NULL) continue;
			if (chained.contains(wire)) continue;
			chained.append(wire);
		}
		else {
			if (!ends.contains(connectedToItem)) {
				ends.append(connectedToItem);
			}
		}
	}
}

void Wire::collectWires(QList<Wire *> & wires, bool includeBusConnections) {
	if (wires.contains(this)) return;

	wires.append(this);
	//DebugDialog::debug(QString("collecting wire %1").arg(this->id()) );
	collectWiresAux(wires, m_connector0, includeBusConnections);
	collectWiresAux(wires, m_connector1, includeBusConnections);
}

void Wire::collectWiresAux(QList<Wire *> & wires, ConnectorItem * start, bool includeBusConnections) {
	foreach (ConnectorItem * toConnectorItem, start->connectedToItems()) {
		if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			dynamic_cast<Wire *>(toConnectorItem->attachedTo())->collectWires(wires, includeBusConnections);
		}
		else if (includeBusConnections) {
			BusConnectorItem * bci = dynamic_cast<BusConnectorItem *>(toConnectorItem);
			if (bci == NULL) continue;

			foreach(ConnectorItem * bConnectorItem, bci->connectedToItems()) {
				if (bConnectorItem->attachedToItemType() == ModelPart::Wire) {
					dynamic_cast<Wire *>(bConnectorItem->attachedTo())->collectWires(wires, includeBusConnections);
				}
			}
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

bool Wire::connectedToBreadboard() {
	foreach (ConnectorItem * connectorItem, connector0()->connectedToItems()) {
		if (connectorItem->attachedToItemType() == ModelPart::Breadboard) return true;
	}
	foreach (ConnectorItem * connectorItem, connector1()->connectedToItems()) {
		if (connectorItem->attachedToItemType() == ModelPart::Breadboard) return true;
	}

	// TODO: check chains?

	return false;
}

void Wire::setColor(QColor & color, qreal op) {
	m_pen.setBrush(QBrush(color));
	m_opacity = op;
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
	m_pen.setWidth(width);
	m_shadowPen.setWidth(width + 2);
}

int Wire::width() {
	return m_pen.width();
}

void Wire::setColorString(QString colorName, qreal op) {
	// sets a color using the name (.e. "red") 
	// note: colorName is associated with a Fritzing color, not a Qt color

	m_colorName = colorName;
	QString colorString = colors.value(colorName);
	if (colorString.isEmpty() || colorString.isNull()) return;

	QColor c;
	c.setNamedColor(colorString);
	setColor(c, op);

	colorString = shadowColors.value(colorName);
	if (colorString.isEmpty() || colorString.isNull()) {
		return;
	}

	c.setNamedColor(colorString);
	setShadowColor(c);
}

QString Wire::colorString() {
	return m_colorName;
}

void Wire::initNames() {
	if (colors.count() > 0) return;

	// need a list because a hash table doesn't guarantee order
	colorNames.append(tr("red"));
	colorNames.append(tr("black"));
	colorNames.append(tr("blue"));
	colorNames.append(tr("yellow"));
	colorNames.append(tr("green"));
	colorNames.append(tr("white"));

	// need this hash table to translate from user's language to internal color name
	colorTrans.insert(tr("red"), "red");
	colorTrans.insert(tr("black"), "black");
	colorTrans.insert(tr("blue"), "blue");
	colorTrans.insert(tr("yellow"), "yellow");
	colorTrans.insert(tr("green"), "green");
	colorTrans.insert(tr("white"), "white");

	colors.insert("red",	"#cc1f1f");
	colors.insert("black",	"#4d4d4d");
	colors.insert("blue",	"#71a4d6");
	colors.insert("yellow", "#ffe666");
	colors.insert("green",	"#52cc80");
	colors.insert("white",	"#e6e6e6");
	colors.insert("jumper", "#ff0000");
	colors.insert("trace",  "#ffbf00");
	colors.insert("unrouted", "#000000");
	colors.insert("routed", "#7d7d7d");
	colors.insert("purple", "#b673e6");
	colors.insert("orange", "#ff7033");
	colors.insert("brown", "#8c3b00");
	
	shadowColors.insert("red",	"#990000");
	shadowColors.insert("black",	"#363636");
	shadowColors.insert("blue",	"#357dcc");
	shadowColors.insert("yellow", "#d9ad20");
	shadowColors.insert("green", "#00b342");
	shadowColors.insert("white",	"#b3b3b3");
	shadowColors.insert("jumper", "#ff0000");
	shadowColors.insert("trace", "#ffbf00");
	shadowColors.insert("unrouted", "#000000");
	shadowColors.insert("routed", "#7d7d7d");

	colorStringIndex.insert(ItemBase::BreadboardView, 0);
	colorStringIndex.insert(ItemBase::SchematicView, 0);
	colorStringIndex.insert(ItemBase::PCBView, 0);
	makeHues(80, 340, 5, 0, ratsnestColors);
	qSort(ratsnestColors.begin(), ratsnestColors.end(), alphaLessThan);
	foreach (QColor * c, ratsnestColors) {
		c->setAlpha(255);
	}
	DebugDialog::debug(QString("hues total %1").arg(ratsnestColors.count()) );

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

const QColor * Wire::netColor(ItemBase::ViewIdentifier viewIdentifier) {
	int csi = colorStringIndex.value(viewIdentifier);
	QColor * c = ratsnestColors[csi];
	csi = (csi + 1) % ratsnestColors.count();
	colorStringIndex.insert(viewIdentifier, csi);
	if (viewIdentifier == ItemBase::PCBView) {
		DebugDialog::debug(QString("wire hue %1").arg(c->hue()) );
	}
	return c;
}
