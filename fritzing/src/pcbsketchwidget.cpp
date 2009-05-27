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

$Revision: 1617 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-22 20:32:44 +0100 (Sat, 22 Nov 2008) $

********************************************************************/


#include "pcbsketchwidget.h"
#include "debugdialog.h"
#include "svg/svgfilesplitter.h"
#include "items/tracewire.h"
#include "items/virtualwire.h"
#include "items/resizableboard.h"
#include "waitpushundostack.h"
#include "autoroute/autorouter1.h"
#include "connectoritem.h"
#include "help/sketchmainhelp.h"

static QColor labelTextColor = Qt::white;

PCBSketchWidget::PCBSketchWidget(ViewIdentifierClass::ViewIdentifier viewIdentifier, QWidget *parent)
    : PCBSchematicSketchWidget(viewIdentifier, parent)
{
	m_addBoard = false;
	m_viewName = QObject::tr("PCB View");
	m_netCount = m_netRoutedCount = m_connectorsLeftToRoute = m_jumperCount = 0;
	m_traceColor = "trace";
	m_jumperColor = "jumper";
}

void PCBSketchWidget::setWireVisible(Wire * wire)
{
	wire->setVisible(wire->getRatsnest() || wire->getTrace() || wire->getJumper());
}

void PCBSketchWidget::addViewLayers() {
	addPcbViewLayers();

	// disable these for now
	ViewLayer * viewLayer = m_viewLayers.value(ViewLayer::Vias);
	viewLayer->action()->setEnabled(false);
	viewLayer = m_viewLayers.value(ViewLayer::Copper1);
	viewLayer->action()->setEnabled(false);
	//viewLayer = m_viewLayers.value(ViewLayer::Keepout);
	//viewLayer->action()->setEnabled(false);
}


void PCBSketchWidget::makeWires(QList<ConnectorItem *> & partsConnectorItems, QList <Wire *> & ratsnestWires, Wire * & modelWire, RatsnestCommand * ratsnestCommand)
{
	int count = partsConnectorItems.count();
	for (int i = 0; i < count - 1; i++) {
		ConnectorItem * source = partsConnectorItems[i];
		for (int j = i + 1; j < count; j++) {
			ConnectorItem * dest = partsConnectorItems[j];
			// if you can't get from i to j via wires, then add a virtual ratsnest wire
			Wire* tempWire = source->wiredTo(dest, ViewGeometry::RatsnestFlag);
			if (tempWire == NULL) {
				Wire * newWire = makeOneRatsnestWire(source, dest, ratsnestCommand, source->wiredTo(dest, ViewGeometry::TraceFlag | ViewGeometry::JumperFlag) == NULL);
				ratsnestWires.append(newWire);
				if (source->wiredTo(dest, ViewGeometry::TraceFlag | ViewGeometry::JumperFlag)) {
					newWire->setRouted(true);
				}

			}
			else {
				modelWire = tempWire;
			}
		}
	}
}


ViewLayer::ViewLayerID PCBSketchWidget::multiLayerGetViewLayerID(ModelPart * modelPart, QDomElement & layers, QString & layerName) {
	Q_UNUSED(modelPart);

	// priviledge Copper0 if it's available
	QDomElement layer = layers.firstChildElement("layer");
	while (!layer.isNull()) {
		QString lName = layer.attribute("layerId");
		if (ViewLayer::viewLayerIDFromXmlString(lName) == ViewLayer::Copper0) {
			return ViewLayer::Copper0;
		}

		layer = layer.nextSiblingElement("layer");
	}

	return ViewLayer::viewLayerIDFromXmlString(layerName);
}

bool PCBSketchWidget::canDeleteItem(QGraphicsItem * item)
{
	VirtualWire * wire = dynamic_cast<VirtualWire *>(item);
	if (wire != NULL) return false;

	return SketchWidget::canDeleteItem(item);
}

bool PCBSketchWidget::canCopyItem(QGraphicsItem * item)
{
	VirtualWire * wire = dynamic_cast<VirtualWire *>(item);
	if (wire != NULL) {
		if (wire->getRatsnest()) return false;
	}

	return SketchWidget::canDeleteItem(item);
}

bool PCBSketchWidget::canChainWire(Wire * wire) {
	bool result = SketchWidget::canChainWire(wire);
	if (!result) return result;

	if (wire->getRatsnest()) return false;

	return result;
}

void PCBSketchWidget::createJumper() {
	QString commandString = tr("Create Jumper from this Wire");
	createJumperOrTrace(commandString, ViewGeometry::JumperFlag, m_jumperColor);
	ensureLayerVisible(ViewLayer::Jumperwires);
}

void PCBSketchWidget::createTrace() {
	QString commandString = tr("Create Trace from this Wire");
	createJumperOrTrace(commandString, ViewGeometry::TraceFlag, m_traceColor);
	ensureLayerVisible(ViewLayer::Copper0);
}

void PCBSketchWidget::createJumperOrTrace(const QString & commandString, ViewGeometry::WireFlag flag, const QString & colorString)
{
	QList<Wire *> done;
	QUndoCommand * parentCommand = NULL;
	foreach (QGraphicsItem * item, scene()->selectedItems()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		if (done.contains(wire)) continue;

		createOneJumperOrTrace(wire, flag, false, done, parentCommand, commandString, colorString);
	}

	if (parentCommand == NULL) return;

	new CleanUpWiresCommand(this, false, parentCommand);
	m_undoStack->push(parentCommand);
}

void PCBSketchWidget::createOneJumperOrTrace(Wire * wire, ViewGeometry::WireFlag flag, bool allowAny, QList<Wire *> & done, 
											 QUndoCommand * & parentCommand, const QString & commandString, const QString & colorString) 
{
	QList<ConnectorItem *> ends;
	Wire * jumperOrTrace = NULL;
	if (wire->getRatsnest()) {
		jumperOrTrace = wire->findJumperOrTraced(ViewGeometry::JumperFlag | ViewGeometry::TraceFlag, ends);
	}
	else if (wire->getJumper()) {
		jumperOrTrace = wire;
	}
	else if (wire->getTrace()) {
		jumperOrTrace = wire;
	}
	else if (!allowAny) {
		// not eligible
		return;
	}
	else {
		jumperOrTrace = wire->findJumperOrTraced(ViewGeometry::JumperFlag | ViewGeometry::TraceFlag, ends);
	}

	if (jumperOrTrace && jumperOrTrace->hasFlag(flag)) {
		return;
	}

	if (parentCommand == NULL) {
		parentCommand = new QUndoCommand(commandString);
	}

	if (jumperOrTrace != NULL) {
		QList<Wire *> chained;
		QList<ConnectorItem *> uniqueEnds;
		jumperOrTrace->collectChained(chained, ends, uniqueEnds);
		makeWiresChangeConnectionCommands(chained, parentCommand);
		foreach (Wire * c, chained) {
			makeDeleteItemCommand(c, BaseCommand::SingleView, parentCommand);
			done.append(c);
		}
	}

	long newID = createWire(ends[0], ends[1], flag, false, false, BaseCommand::SingleView, parentCommand);
	new WireColorChangeCommand(this, newID, colorString, colorString, Wire::UNROUTED_OPACITY, Wire::UNROUTED_OPACITY, parentCommand);
	new WireWidthChangeCommand(this, newID, 3, 3, parentCommand);
	Wire* rat = NULL;
	if (wire->getRatsnest()) {
		rat = wire;
	}
	else {
		rat = ends[0]->wiredTo(ends[1], ViewGeometry::RatsnestFlag);
	}

	if (rat != NULL) {
		QList<ConnectorItem *> ends;
		QList<Wire *> rats;
		QList<ConnectorItem *> uniqueEnds;
		rat->collectChained(rats, ends, uniqueEnds);
		foreach (Wire * r, rats) {
			makeChangeRoutedCommand(r, true, Wire::ROUTED_OPACITY, parentCommand);
		}
	}

}


void PCBSketchWidget::excludeFromAutoroute(bool exclude)
{
	foreach (QGraphicsItem * item, scene()->selectedItems()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;

		if (wire->getTrace() || wire->getJumper()) {
			QList<Wire *> wires;
			QList<ConnectorItem *> ends;
			QList<ConnectorItem *> uniqueEnds;
			wire->collectChained(wires, ends, uniqueEnds);
			foreach (Wire * w, wires) {
				w->setAutoroutable(!exclude);
			}
		}
	}
}

bool PCBSketchWidget::ratsAllRouted() {
	foreach (QGraphicsItem * item, scene()->items()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		if (!wire->getRatsnest()) continue;

		if (!wire->getRouted()) return false;
	}

	return true;
}

void PCBSketchWidget::makeChangeRoutedCommand(Wire * wire, bool routed, qreal opacity, QUndoCommand * parentCommand) {
	new WireColorChangeCommand(this, wire->id(), wire->colorString(), wire->colorString(), wire->opacity(), opacity, parentCommand);
	ViewGeometry::WireFlags wireFlags = wire->wireFlags();
	wireFlags |= ViewGeometry::RoutedFlag;
	if (!routed) {
		wireFlags &= ~ViewGeometry::RoutedFlag;
	}
	new WireFlagChangeCommand(this, wire->id(), wire->wireFlags(), wireFlags, parentCommand);
}

void PCBSketchWidget::clearRouting(QUndoCommand * parentCommand) {
	Q_UNUSED(parentCommand);
	//Autorouter1::clearTraces(this, true, parentCommand);
	//updateRatsnestStatus(NULL, parentCommand);
}

void PCBSketchWidget::updateRatsnestStatus(CleanUpWiresCommand* command, QUndoCommand * undoCommand)
{
	QHash<ConnectorItem *, int> indexer;
	QList< QList<ConnectorItem *>* > allPartConnectorItems;
	Autorouter1::collectAllNets(this, indexer, allPartConnectorItems);
	int netCount = 0;
	int netRoutedCount = 0;
	int connectorsLeftToRoute = 0;
	int jumperCount = 0;
	foreach (QList<ConnectorItem *>* netList, allPartConnectorItems) {
		if (netList->count() <= 1) continue;			// nets with a single part are not worth counting

		int selfConnections = 0;
		QVector<bool> self(netList->count(), true);
		for (int i = 0; i < netList->count() - 1; i++) {
			for (int j = i + 1; j < netList->count(); j++) {
				ConnectorItem * ci = netList->at(i);
				ConnectorItem * cj = netList->at(j);
				if (ci->bus() && ci->attachedTo() == cj->attachedTo() && ci->bus() == cj->bus()) {
					// if connections are on the same bus on a given part
					self[i] = false;
					self[j] = false;
					selfConnections++;
				}
			}
		}

		int useIndex = 0;
		bool bail = true;
		foreach (bool v, self) {
			if (v) {
				bail = false;
				break;
			}
			useIndex++;
		}

		if (bail) {
			continue;			// only have a net on the same part on the same bus
		}

		netCount++;
		ConnectorItem * connectorItem = netList->at(useIndex);

		// figure out how many parts are connected via jumpers or traces
		QList<ConnectorItem *> partConnectorItems;
		partConnectorItems.append(connectorItem);
		ConnectorItem::collectEqualPotentialParts(partConnectorItems, ViewGeometry::JumperFlag | ViewGeometry::TraceFlag);
		foreach (ConnectorItem * jConnectorItem, partConnectorItems) {
			foreach (ConnectorItem * kConnectorItem, jConnectorItem->connectedToItems()) {
				if (kConnectorItem->attachedToItemType() == ModelPart::Wire) {
					Wire * wire = dynamic_cast<Wire *>(kConnectorItem->attachedTo());
					if (wire->getJumper()) {
						jumperCount++;
					}
				}
			}
		}
		int todo = netList->count() - partConnectorItems.count() - selfConnections;
		if (todo <= 0) {
			netRoutedCount++;
		}
		else {
			connectorsLeftToRoute += (todo + 1);
		}
	}

	if (command) {
		removeRatsnestWires(allPartConnectorItems, command);
	}


	foreach (QList<ConnectorItem *>* list, allPartConnectorItems) {
		delete list;
	}

	// divide jumpercount by two since we counted both ends of each jumper wire
	jumperCount /= 2;

	if (netCount != m_netCount || jumperCount != m_jumperCount || netRoutedCount != m_netRoutedCount || connectorsLeftToRoute != m_connectorsLeftToRoute) {
		if (command) {
			// changing state after the command has already been executed
			command->addRoutingStatus(this, m_netCount, m_netRoutedCount, m_connectorsLeftToRoute, m_jumperCount,
											netCount, netRoutedCount, connectorsLeftToRoute, jumperCount);
		}
		if (undoCommand) {
			// the command is still to be executed
			new RoutingStatusCommand(this, m_netCount, m_netRoutedCount, m_connectorsLeftToRoute, m_jumperCount,
											netCount, netRoutedCount, connectorsLeftToRoute, jumperCount, undoCommand);
		}

		emit routingStatusSignal(this, netCount, netRoutedCount, connectorsLeftToRoute, jumperCount);

		m_netCount = netCount;
		m_jumperCount = jumperCount;
		m_connectorsLeftToRoute = connectorsLeftToRoute;
		m_netRoutedCount = netRoutedCount;
	}
}

void PCBSketchWidget::forwardRoutingStatusSignal(int netCount, int netRoutedCount, int connectorsLeftToRoute, int jumperCount) 
{
	m_netCount = netCount;
	m_netRoutedCount = netRoutedCount;
	m_connectorsLeftToRoute = connectorsLeftToRoute;
	m_jumperCount = jumperCount;
	SketchWidget::forwardRoutingStatusSignal(netCount, netRoutedCount, connectorsLeftToRoute, jumperCount);
}

const QColor & PCBSketchWidget::getLabelTextColor() {
	return labelTextColor;
}

void PCBSketchWidget::selectAllExcludedTraces() 
{
	QList<Wire *> wires;
	foreach (QGraphicsItem * item, scene()->items()) {
		TraceWire * wire = dynamic_cast<TraceWire *>(item);
		if (wire == NULL) continue;

		if (wire->parentItem() != NULL) {
			// skip module wires
			continue;
		}

		if (!wire->getAutoroutable()) {
			wires.append(wire);
		}
	}

	QUndoCommand * parentCommand = new QUndoCommand(QObject::tr("Select all traces marked \"Don't autoroute\""));

	stackSelectionState(false, parentCommand);
	SelectItemCommand * selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
	foreach (Wire * wire, wires) {
		selectItemCommand->addRedo(wire->id());
	}

	scene()->clearSelection();
	m_undoStack->push(parentCommand);
}

const QString & PCBSketchWidget::hoverEnterPartConnectorMessage(QGraphicsSceneHoverEvent * event, ConnectorItem * item)
{
	Q_UNUSED(event);
	Q_UNUSED(item);

	static QString message = tr("Click this connector to drag out a new trace.");

	return message;
}

bool PCBSketchWidget::modifyNewWireConnections(Wire * dragWire, ConnectorItem * fromDragWire, ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, QUndoCommand * parentCommand)
{	
	Q_UNUSED(fromDragWire);
	if (fromConnectorItem->attachedToItemType() != ModelPart::Wire && 
		toConnectorItem->attachedToItemType() != ModelPart::Wire)
	{
		QList<Wire *> done;
		dragWire->connector0()->tempConnectTo(fromConnectorItem, false);
		dragWire->connector1()->tempConnectTo(toConnectorItem, false);
		createOneJumperOrTrace(dragWire, ViewGeometry::TraceFlag, true, done, parentCommand, ___emptyString___, m_traceColor);
		dragWire->connector0()->tempRemove(fromConnectorItem, false);
		dragWire->connector1()->tempRemove(toConnectorItem, false);
	}

	return false;
}

void PCBSketchWidget::addBoard() {
	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(QPointF(0, 0));
	m_addedBoard = addItem(paletteModel()->retrieveModelPart(ItemBase::rectangleModuleIDName), BaseCommand::SingleView, viewGeometry, newID, -1, -1, NULL, NULL);

	// have to put this off until later, because positioning the item doesn't work correctly until the view is visible
	// so position it in setCurrent()
	m_addBoard = true;
}

void PCBSketchWidget::setCurrent(bool current) {
	SketchWidget::setCurrent(current);
	if (current && m_addBoard && (m_addedBoard != NULL)) {
		m_addBoard = false;

		if (m_fixedToCenterItem != NULL) {
			QSizeF helpsize = m_fixedToCenterItem->size();
			QSizeF vp = this->viewport()->size();

			QPointF p;
			p.setX((int) ((vp.width() - helpsize.width()) / 2));
			p.setY((int) ((vp.height() - helpsize.height()) / 2));

			// TODO: make these constants less arbitrary
			p += QPointF(10, 30);

			m_addedBoard->setPos(p);
			dynamic_cast<ResizableBoard *>(m_addedBoard)->resizePixels(110, helpsize.height() - 30 - 30, m_viewLayers);
		}
	}
}

void PCBSketchWidget::setClipEnds(VirtualWire * vw) {
	vw->setClipEnds(true);
}

ViewLayer::ViewLayerID PCBSketchWidget::getWireViewLayerID(const ViewGeometry & viewGeometry) {
	if (viewGeometry.getJumper()) {

		return ViewLayer::Jumperwires;
	}

	if (viewGeometry.getTrace()) {
		return ViewLayer::Copper0Trace;
	}

	return SketchWidget::getWireViewLayerID(viewGeometry);
}

void PCBSketchWidget::initWire(Wire * wire, int penWidth) {
	Q_UNUSED(penWidth);
	wire->setColorString("unrouted", Wire::UNROUTED_OPACITY);
	wire->setPenWidth(1);
}

bool PCBSketchWidget::autorouteNeedsBounds() {
	return true;
}

bool PCBSketchWidget::autorouteCheckWires() {
	return true;
}

bool PCBSketchWidget::autorouteCheckConnectors() {
	return true;
}

bool PCBSketchWidget::autorouteCheckParts() {
	return false;
}

const QString & PCBSketchWidget::traceColor() {
	return m_traceColor;
}

const QString & PCBSketchWidget::jumperColor() {
	return m_jumperColor;
}
