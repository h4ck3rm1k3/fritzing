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

#include <qmath.h>

#include "pcbsketchwidget.h"
#include "../debugdialog.h"
#include "../svg/svgfilesplitter.h"
#include "../items/tracewire.h"
#include "../items/virtualwire.h"
#include "../items/resizableboard.h"
#include "../waitpushundostack.h"
#include "../autoroute/autorouter1.h"
#include "../connectors/connectoritem.h"
#include "../items/moduleidnames.h"
#include "../help/sketchmainhelp.h"
#include "../utils/ratsnestcolors.h"
#include "../fsvgrenderer.h"
#include "../autoroute/autorouteprogressdialog.h"
#include "../items/groundplane.h"
#include "../utils/autoclosemessagebox.h"

#include <limits>
#include <QApplication>

static const int MAX_INT = std::numeric_limits<int>::max();

static QString PCBTraceColor1 = "trace1";
static QString PCBTraceColor = "trace";


struct DistanceThing {
	int distance;
	bool fromConnector0;
};

QHash <ConnectorItem *, DistanceThing *> distances;

bool bySize(QList<ConnectorItem *> * l1, QList<ConnectorItem *> * l2) {
	return l1->count() >= l2->count();
}

bool distanceLessThan(ConnectorItem * end0, ConnectorItem * end1) {
	if (end0->connectorType() == Connector::Male && end1->connectorType() == Connector::Female) {
		return true;
	}
	if (end1->connectorType() == Connector::Male && end0->connectorType() == Connector::Female) {
		return false;
	}

	DistanceThing * dt0 = distances.value(end0, NULL);
	DistanceThing * dt1 = distances.value(end1, NULL);
	if (dt0 && dt1) {
		return dt0->distance <= dt1->distance;
	}

	if (dt0) {
		return true;
	}

	if (dt1) {
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////

PCBSketchWidget::PCBSketchWidget(ViewIdentifierClass::ViewIdentifier viewIdentifier, QWidget *parent)
    : SketchWidget(viewIdentifier, parent)
{
	m_viewName = QObject::tr("PCB View");
	m_shortName = QObject::tr("pcb");
	m_standardBackgroundColor = QColor(160,168,179);
	initBackgroundColor();

	m_routingStatus.zero();
	m_jumperColor = "jumper";
	m_jumperWidth = 3;
	m_cleanType = noClean;
	m_addBoard = false;
}

void PCBSketchWidget::setWireVisible(Wire * wire)
{
	bool visible = wire->getRatsnest() || wire->getTrace() || wire->getJumper();
	wire->setVisible(visible);
	wire->setEverVisible(visible);
}

void PCBSketchWidget::addViewLayers() {
	addPcbViewLayers();

	// disable these for now
	//viewLayer = m_viewLayers.value(ViewLayer::Keepout);
	//viewLayer->action()->setEnabled(false);

	setBoardLayers(1, false);
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
				if (newWire != NULL) {
					ratsnestWires.append(newWire);
					if (source->wiredTo(dest, ViewGeometry::TraceFlag | ViewGeometry::JumperFlag)) {
						newWire->setRouted(true);
					}
				}

			}
			else {
				modelWire = tempWire;
			}
		}
	}
}


ViewLayer::ViewLayerID PCBSketchWidget::multiLayerGetViewLayerID(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerSpec viewLayerSpec, QDomElement & layers, QString & layerName) {
	Q_UNUSED(modelPart);
	Q_UNUSED(viewIdentifier);

	if (viewLayerSpec == ViewLayer::GroundPlane_Bottom) return ViewLayer::GroundPlane0;
	else if (viewLayerSpec == ViewLayer::GroundPlane_Top) return ViewLayer::GroundPlane1;

	// priviledge Copper if it's available
	ViewLayer::ViewLayerID wantLayer = modelPart->flippedSMD() && viewLayerSpec == ViewLayer::ThroughHoleThroughTop_TwoLayers ? ViewLayer::Copper1 : ViewLayer::Copper0;
	QDomElement layer = layers.firstChildElement("layer");
	while (!layer.isNull()) {
		QString lName = layer.attribute("layerId");
		if (ViewLayer::viewLayerIDFromXmlString(lName) == wantLayer) {
			return wantLayer;
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
	createJumperOrTrace(commandString, ViewGeometry::JumperFlag);
	ensureJumperLayerVisible();
}

void PCBSketchWidget::createTrace() {
	QString commandString = tr("Create Trace from this Wire");
	createJumperOrTrace(commandString, ViewGeometry::TraceFlag);
	ensureTraceLayerVisible();
}

void PCBSketchWidget::createJumperOrTrace(const QString & commandString, ViewGeometry::WireFlag flag)
{
	QList<Wire *> done;
	QUndoCommand * parentCommand = NULL;
	foreach (QGraphicsItem * item, scene()->selectedItems()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		if (done.contains(wire)) continue;

		createOneJumperOrTrace(wire, flag, false, done, parentCommand, commandString);
	}

	if (parentCommand == NULL) return;

	new CleanUpWiresCommand(this, false, parentCommand);
	m_undoStack->push(parentCommand);
}

void PCBSketchWidget::createOneJumperOrTrace(Wire * wire, ViewGeometry::WireFlag flag, bool allowAny, QList<Wire *> & done, 
											 QUndoCommand * & parentCommand, const QString & commandString) 
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
		removeWire(jumperOrTrace, ends, done, parentCommand);
	}

	QString colorString;
	if (flag == ViewGeometry::JumperFlag) {
		colorString = m_jumperColor;
	}
	else {
		ConnectorItem * toConnectorItem = ends[0]->connectedToItems()[0];
		colorString = traceColor(toConnectorItem);
	}
	long newID = createWire(ends[0], ends[1], flag, false, false, BaseCommand::SingleView, parentCommand);
	new WireColorChangeCommand(this, newID, colorString, colorString, getRatsnestOpacity(false), getRatsnestOpacity(false), parentCommand);
	new WireWidthChangeCommand(this, newID, Wire::STANDARD_TRACE_WIDTH, Wire::STANDARD_TRACE_WIDTH, parentCommand);
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
			makeChangeRoutedCommand(r, true, getRatsnestOpacity(true), parentCommand);
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

	//kill copperfill here
	//Autorouter1::clearTraces(this, true, parentCommand);
	//updateRatsnestStatus(NULL, parentCommand);
}

void PCBSketchWidget::updateRatsnestStatus(CleanUpWiresCommand* command, QUndoCommand * undoCommand, RoutingStatus & routingStatus)
{
	//DebugDialog::debug("update ratsnest status");

	QHash<ConnectorItem *, int> indexer;
	QList< QList<ConnectorItem *>* > allPartConnectorItems;
	collectAllNets(indexer, allPartConnectorItems, false);
	routingStatus.zero();

	// TODO:  handle delete in updateRatsnestColors...
	if (command) {
		removeRatsnestWires(allPartConnectorItems, command);
	}

	foreach (QList<ConnectorItem *>* list, allPartConnectorItems) {
		delete list;
	}

	updateRatsnestColors(command, undoCommand, false, routingStatus);

	if (routingStatus != m_routingStatus) {
		if (command) {
			// changing state after the command has already been executed
			command->addRoutingStatus(this, m_routingStatus, routingStatus);
		}
		if (undoCommand) {
			// the command is still to be executed
			new RoutingStatusCommand(this, m_routingStatus, routingStatus, undoCommand);
		}

		emit routingStatusSignal(this, routingStatus);

		m_routingStatus = routingStatus;
	}
}

void PCBSketchWidget::forwardRoutingStatus(const RoutingStatus & routingStatus) 
{
	m_routingStatus = routingStatus;
	SketchWidget::forwardRoutingStatus(routingStatus);
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
	if (fromConnectorItem == toConnectorItem) {
		// can't drag a wire to itself
		return false;
	}

	bool result = false;

	QList<ConnectorItem *> connectorItems;
	connectorItems.append(fromConnectorItem);
	ConnectorItem::collectEqualPotential(connectorItems, true, ViewGeometry::TraceJumperRatsnestFlags);
	if (connectorItems.contains(toConnectorItem)) {
		// don't generate a wire in bb view if the connectors are already connected
		result = true;
	}
	else {
		if (fromConnectorItem->attachedToItemType() == ModelPart::Symbol ||
			toConnectorItem->attachedToItemType() == ModelPart::Symbol) 
		{
			// set up extra connection for breadboard view (for now; won't be necessary if breadboard gets ratsnest)
			connectSymbols(fromConnectorItem, toConnectorItem, parentCommand);
		}

		bool fromWire = fromConnectorItem->attachedToItemType() == ModelPart::Wire;
		bool toWire = toConnectorItem->attachedToItemType() == ModelPart::Wire;

		if (fromWire && toWire)
		{
			ConnectorItem * originalFromConnectorItem = fromConnectorItem;
			ConnectorItem * originalToConnectorItem = toConnectorItem;
			ConnectorItem * newFromConnectorItem = lookForBreadboardConnection(fromConnectorItem);
			ConnectorItem * newToConnectorItem = lookForBreadboardConnection(toConnectorItem);
			if (newFromConnectorItem->attachedToItemType() == ModelPart::Breadboard &&
				newToConnectorItem->attachedToItemType() == ModelPart::Breadboard)
			{
				// connection can be made with one wire
				makeModifiedWire(newFromConnectorItem, newToConnectorItem, BaseCommand::CrossView, 0, parentCommand);
			}
			else if (newToConnectorItem->attachedToItemType() == ModelPart::Breadboard) {
				makeTwoWires(originalToConnectorItem, newToConnectorItem, originalFromConnectorItem, newFromConnectorItem, parentCommand);
			}
			else {
				makeTwoWires(originalFromConnectorItem, newFromConnectorItem, originalToConnectorItem, newToConnectorItem, parentCommand);
			}
			result = true;
		}
		else if (fromWire) {
			modifyNewWireConnectionsAux(fromConnectorItem, toConnectorItem, parentCommand);
			result = true;
		}
		else if (toWire) {
			modifyNewWireConnectionsAux(toConnectorItem, fromConnectorItem, parentCommand);
			result = true;
		}
	}

	dragWire->connector0()->tempConnectTo(fromConnectorItem, false);
	dragWire->connector1()->tempConnectTo(toConnectorItem, false);
	QList<ConnectorItem *> ends;
	Wire * jumperOrTrace = dragWire->findJumperOrTraced(ViewGeometry::JumperFlag | ViewGeometry::TraceFlag, ends);
	dragWire->connector0()->tempRemove(fromConnectorItem, false);
	dragWire->connector1()->tempRemove(toConnectorItem, false);

	if (jumperOrTrace == NULL) {
		long newID = makeModifiedWire(fromConnectorItem, toConnectorItem, BaseCommand::SingleView, ViewGeometry::TraceFlag, parentCommand);
		QString tc = traceColor(fromConnectorItem);
		new WireColorChangeCommand(this, newID, tc, tc, 1.0, 1.0, parentCommand);
		new WireWidthChangeCommand(this, newID, Wire::STANDARD_TRACE_WIDTH, Wire::STANDARD_TRACE_WIDTH, parentCommand);
		if (fromConnectorItem->attachedToItemType() == ModelPart::Wire) {
			foreach (ConnectorItem * connectorItem, fromConnectorItem->connectedToItems()) {
				if (connectorItem->attachedToItemType() == ModelPart::Wire) {
					// not sure about the wire type check
					new ChangeConnectionCommand(this, BaseCommand::SingleView,
							newID, "connector0",
							connectorItem->attachedToID(), connectorItem->connectorSharedID(),
							ViewLayer::specFromID(connectorItem->attachedToViewLayerID()),
							true, parentCommand);
				}
			}
		}
		if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			foreach (ConnectorItem * connectorItem, toConnectorItem->connectedToItems()) {
				if (connectorItem->attachedToItemType() == ModelPart::Wire) {
					// not sure about the wire type check
					new ChangeConnectionCommand(this, BaseCommand::SingleView,
							newID, "connector1",
							connectorItem->attachedToID(), connectorItem->connectorSharedID(),
							ViewLayer::specFromID(connectorItem->attachedToViewLayerID()),
							true, parentCommand);
				}
			}
		}
	}

	return result;
}

void PCBSketchWidget::connectSymbols(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, QUndoCommand * parentCommand) {
	ConnectorItem * target1 = NULL;
	ConnectorItem * target2 = NULL;
	if (fromConnectorItem->attachedToItemType() == ModelPart::Symbol && toConnectorItem->attachedToItemType() == ModelPart::Symbol) {
		QList<ConnectorItem *> connectorItems;
		connectorItems.append(fromConnectorItem);
		ConnectorItem::collectEqualPotential(connectorItems, false, ViewGeometry::TraceJumperRatsnestFlags);
		foreach (ConnectorItem * c, connectorItems) {
			if (c->attachedToItemType() == ModelPart::Part) {
				target1 = c; 
				break;
			}
		}
		connectorItems.clear();
		connectorItems.append(toConnectorItem);
		ConnectorItem::collectEqualPotential(connectorItems, false, ViewGeometry::TraceJumperRatsnestFlags);
		foreach (ConnectorItem * c, connectorItems) {
			if (c->attachedToItemType() == ModelPart::Part) {
				target2 = c; 
				break;
			}
		}
	}
	else if (fromConnectorItem->attachedToItemType() == ModelPart::Symbol) {
		connectSymbolPrep(fromConnectorItem, toConnectorItem, target1, target2);
	}
	else if (toConnectorItem->attachedToItemType() == ModelPart::Symbol) {
		connectSymbolPrep(toConnectorItem, fromConnectorItem, target1, target2);
	}

	if (target1 == NULL) return;
	if (target2 == NULL) return;

	makeModifiedWire(target1, target2, BaseCommand::CrossView, 0, parentCommand);
}

void PCBSketchWidget::connectSymbolPrep(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, ConnectorItem * & target1, ConnectorItem * & target2) {
	QList<ConnectorItem *> connectorItems;
	connectorItems.append(fromConnectorItem);
	ConnectorItem::collectEqualPotential(connectorItems, false, ViewGeometry::TraceJumperRatsnestFlags);
	foreach (ConnectorItem * c, connectorItems) {
		if (c->attachedToItemType() == ModelPart::Part) {
			target1 = c;
			break;
		}
	}
	if (target1 == NULL) return;

	if (toConnectorItem->attachedToItemType() == ModelPart::Part) {
		target2 = toConnectorItem;
	}
	else if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
		target2 = findNearestPartConnectorItem(toConnectorItem);
	}
}

void PCBSketchWidget::addBoard() {
	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(QPointF(0, 0));
	m_addedBoard = addItem(paletteModel()->retrieveModelPart(ModuleIDNames::rectangleModuleIDName), defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);

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
			p.setX((int) ((vp.width() - helpsize.width()) / 2.0));
			p.setY((int) ((vp.height() - helpsize.height()) / 2.0));

			// TODO: make these constants less arbitrary (get the size and location of the icon which the board is replacing)
			p += QPointF(10, 30);

			// add a board to the empty sketch, and place it in the help area.

			m_addedBoard->setPos(mapToScene(p.toPoint()));
			qobject_cast<ResizableBoard *>(m_addedBoard)->resizePixels(95, helpsize.height() - 30 - 30, m_viewLayers);
		}
	}
}

void PCBSketchWidget::setClipEnds(ClipableWire * vw, bool clipEnds) {
	vw->setClipEnds(clipEnds);
}

ViewLayer::ViewLayerID PCBSketchWidget::getDragWireViewLayerID(ConnectorItem * connectorItem) {
	switch (connectorItem->attachedToViewLayerID()) {
		case ViewLayer::Copper1:
		case ViewLayer::Copper1Trace:
		case ViewLayer::GroundPlane1:
			return ViewLayer::Copper1Trace;
		default:
			return ViewLayer::Copper0Trace;
	}
}

ViewLayer::ViewLayerID PCBSketchWidget::getWireViewLayerID(const ViewGeometry & viewGeometry, ViewLayer::ViewLayerSpec viewLayerSpec) {
	if (viewGeometry.getJumper()) {
		return ViewLayer::Jumperwires;
	}

	if (viewGeometry.getRatsnest()) {
		return ViewLayer::Ratsnest;
	}

	if (viewGeometry.getTrace()) {
		switch (viewLayerSpec) {
			case ViewLayer::WireOnTop_TwoLayers:
			case ViewLayer::GroundPlane_Top:
				return ViewLayer::Copper1Trace;
			default:
				return ViewLayer::Copper0Trace;
		}
	}

	switch (viewLayerSpec) {
		case ViewLayer::WireOnTop_TwoLayers:
			return ViewLayer::Copper1Trace;
		default:
			return m_wireViewLayerID;
	}
}

void PCBSketchWidget::initWire(Wire * wire, int penWidth) {
	Q_UNUSED(penWidth);
	wire->setColorString("unrouted", 1.0);
	wire->setPenWidth(1, this);
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

const QString & PCBSketchWidget::traceColor(ConnectorItem * forColor) {
	switch(forColor->attachedToViewLayerID()) {
		case ViewLayer::Copper1:
		case ViewLayer::Copper1Trace:
		case ViewLayer::GroundPlane1:
			return PCBTraceColor1;
		default:
			return PCBTraceColor;
	}
	
}

const QString & PCBSketchWidget::jumperColor() {
	return m_jumperColor;
}

qreal PCBSketchWidget::jumperWidth() {
	return m_jumperWidth;
}

PCBSketchWidget::CleanType PCBSketchWidget::cleanType() {
	return m_cleanType;
}

void PCBSketchWidget::ensureTraceLayersVisible() {
	ensureLayerVisible(ViewLayer::Copper0);
	ensureLayerVisible(ViewLayer::Copper0Trace);
	ensureLayerVisible(ViewLayer::GroundPlane0);
	ensureLayerVisible(ViewLayer::Jumperwires);
	if (m_boardLayers == 2) {
		ensureLayerVisible(ViewLayer::Copper1);
		ensureLayerVisible(ViewLayer::Copper1Trace);
		ensureLayerVisible(ViewLayer::GroundPlane1);
	}
}

void PCBSketchWidget::ensureTraceLayerVisible() {
	ensureLayerVisible(ViewLayer::Copper0);
	ensureLayerVisible(ViewLayer::Copper0Trace);
}

void PCBSketchWidget::ensureJumperLayerVisible() {
	ensureLayerVisible(ViewLayer::Jumperwires);
}

bool PCBSketchWidget::canChainMultiple() {
	return false;
}

void PCBSketchWidget::setNewPartVisible(ItemBase * itemBase) {
	if (itemBase->itemType() == ModelPart::Breadboard  || itemBase->itemType() == ModelPart::Symbol) {
		// don't need to see the breadboard in the other views
		// but it's there so connections can be more easily synched between views
		itemBase->setVisible(false);
		itemBase->setEverVisible(false);
	}
}

bool PCBSketchWidget::canDropModelPart(ModelPart * modelPart) {
	switch (modelPart->itemType()) {
		case ModelPart::Jumper:
		case ModelPart::Logo:
		case ModelPart::Ruler:
			return true;
		case ModelPart::Wire:
		case ModelPart::Breadboard:
		case ModelPart::Symbol:
		case ModelPart::CopperFill:
			// can't drag and drop these parts in this view
			return false;
		case ModelPart::Board:
		case ModelPart::ResizableBoard:
			if (!matchesLayer(modelPart)) return false;
			
			if (findBoard() != NULL) {
				// don't allow multiple boards
				AutoCloseMessageBox::showMessage(this->window(), tr("Fritzing only allows one board part per sketch. Either delete the current board, or select it and swap it for a different one."));
				return false;
			}

			return true;
		default:
			return true;
	}

	return true;
}

bool PCBSketchWidget::alreadyRatsnest(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem) {
	if (fromConnectorItem->attachedToItemType() == ModelPart::Wire) {
		Wire * wire = dynamic_cast<Wire *>(fromConnectorItem->attachedTo());
		if (wire->getRatsnest() || wire->getJumper() || wire->getTrace()) {
			// don't make further ratsnest's from ratsnest
			return true;
		}
	}

	if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
		Wire * wire = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
		if (wire->getRatsnest() || wire->getJumper() || wire->getTrace()) {
			// don't make further ratsnest's from ratsnest
			return true;
		}
	}

	return false;
}

void PCBSketchWidget::dealWithRatsnest(long fromID, const QString & fromConnectorID, 
								  long toID, const QString & toConnectorID,
								  ViewLayer::ViewLayerSpec viewLayerSpec,
								  bool connect, class RatsnestCommand * ratsnestCommand, bool doEmit)

{
	if (!connect) {
		return;
	}

	ConnectorItem * fromConnectorItem = NULL;
	ConnectorItem * toConnectorItem = NULL;
	if (dealWithRatsnestAux(fromConnectorItem, toConnectorItem, fromID, fromConnectorID, 
							toID, toConnectorID,
							viewLayerSpec,
							connect, ratsnestCommand, doEmit)) 
	{
		return;
	}

	DebugDialog::debug(QString("deal with ratsnest %7 %8: %1 %2 %3, %4 %5 %6 ")
		.arg(fromConnectorItem->attachedToTitle())
		.arg(fromConnectorItem->attachedToID())
		.arg(fromConnectorItem->connectorSharedID())
		.arg(toConnectorItem->attachedToTitle())
		.arg(toConnectorItem->attachedToID())
		.arg(toConnectorItem->connectorSharedID())
		.arg(m_viewIdentifier)
		.arg(fromConnectorItem->attachedToViewLayerID())
	);

	QList<ConnectorItem *> connectorItems;
	QList<ConnectorItem *> partsConnectorItems;
	connectorItems.append(fromConnectorItem);
	ConnectorItem::collectEqualPotential(connectorItems, true, ViewGeometry::TraceJumperRatsnestFlags);
	ConnectorItem::collectParts(connectorItems, partsConnectorItems, includeSymbols(), viewLayerSpec);

	QList <Wire *> ratsnestWires;
	Wire * modelWire = NULL;

	makeWires(partsConnectorItems, ratsnestWires, modelWire, ratsnestCommand);
	if (ratsnestWires.count() > 0) {
		QColor color;
		if (modelWire) {
			color = modelWire->color();
		}
		else {
			color = RatsnestColors::netColor(m_viewIdentifier);
		}
		foreach (Wire * wire, ratsnestWires) {
			wire->setColor(color, getRatsnestOpacity(wire));
			checkSticky(wire->id(), false, false, NULL);
		}
	}

	return;
}

void PCBSketchWidget::removeRatsnestWires(QList< QList<ConnectorItem *>* > & allPartConnectorItems, CleanUpWiresCommand * command)
{
	/*
	DebugDialog::debug("----------");
	foreach (QList<ConnectorItem *>* list, allPartConnectorItems) {
		foreach (ConnectorItem * ci, *list) {
			DebugDialog::debug(QString("%1 %2 %3")
				.arg(ci->attachedToTitle())
				.arg(ci->attachedTo()->instanceTitle())
				.arg(ci->connectorSharedName()));
		}
		DebugDialog::debug("-----");
	}
	*/

	QSet<Wire *> deleteWires;
	QSet<Wire *> visitedWires;
	foreach (QGraphicsItem * item, scene()->items()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		if (visitedWires.contains(wire)) continue;

		ViewGeometry::WireFlags flag = wire->wireFlags() & (ViewGeometry::RatsnestFlag | ViewGeometry::TraceFlag | ViewGeometry::JumperFlag);
		if (flag == 0) continue;

		// if a ratsnest is connecting two items that aren't connected any longer
		// delete the ratsnest

		QList<Wire *> wires;
		QList<ConnectorItem *> ends;
		QList<ConnectorItem *> uniqueEnds;
		wire->collectChained(wires, ends, uniqueEnds);
		foreach (Wire * w, wires) {
			visitedWires.insert(w);
		}

		QList<Wire *> wiresCopy(wires);

		// this is ugly, but for the moment I can't think of anything better.
		// it prevents disconnected deleted traces (traces which have been directly deleted,
		// as opposed to traces that are indirectly deleted by deleting or disconnecting parts)
		// from being deleted twice on the undo stack
		// and therefore added twice, and causing other problems
		if (flag == ViewGeometry::TraceFlag || flag == ViewGeometry::JumperFlag) {
			if (ends.count() == 0)
			{
				continue;
			}
		}

		foreach (QList<ConnectorItem *>* list, allPartConnectorItems) {
			foreach (ConnectorItem * ci, ends) {
				if (!list->contains(ci)) continue;

				foreach (ConnectorItem * tci, ci->connectedToItems()) {
					if (tci->attachedToItemType() != ModelPart::Wire) continue;

					Wire * w = dynamic_cast<Wire *>(tci->attachedTo());
					if (!wires.contains(w)) continue;  // already been tested and removed so keep going

					ViewGeometry::WireFlags wflag = w->wireFlags() & (ViewGeometry::RatsnestFlag | ViewGeometry::TraceFlag | ViewGeometry::JumperFlag);
					if (wflag != flag) continue;

					// assumes one end is connected to a part, checks to see if the other end is also, possibly indirectly, connected
					bothEndsConnected(w, flag, tci, wires, *list);

				}
			}
			if (wires.count() == 0) break;
		}


		foreach (Wire * w, wires) {
			deleteWires.insert(w);
		}

	}

	foreach (Wire * wire, deleteWires) {
		command->addWire(this, wire);
		deleteItem(wire, true, false, false);
	}
}

bool PCBSketchWidget::bothEndsConnected(Wire * wire, ViewGeometry::WireFlags flag, ConnectorItem * oneEnd, QList<Wire *> & wires, QList<ConnectorItem *> & partConnectorItems)
{
	QList<Wire *> visited;
	return bothEndsConnectedAux(wire, flag, oneEnd, wires, partConnectorItems, visited);
}


bool PCBSketchWidget::bothEndsConnectedAux(Wire * wire, ViewGeometry::WireFlags flag, ConnectorItem * oneEnd, QList<Wire *> & wires, QList<ConnectorItem *> & partConnectorItems, QList<Wire *> & visited)
{
	if (visited.contains(wire)) return false;
	visited.append(wire);

	bool result = false;
	ConnectorItem * otherEnd = wire->otherConnector(oneEnd);
	foreach (ConnectorItem * toConnectorItem, otherEnd->connectedToItems()) {
		if (partConnectorItems.contains(toConnectorItem)) {
			result = true;
			continue;
		}

		if (toConnectorItem->attachedToItemType() != ModelPart::Wire) continue;

		Wire * w = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
		ViewGeometry::WireFlags wflag = w->wireFlags() & (ViewGeometry::RatsnestFlag | ViewGeometry::TraceFlag | ViewGeometry::JumperFlag);
		if (wflag != flag) continue;

		result = bothEndsConnectedAux(w, flag, toConnectorItem, wires, partConnectorItems, visited) || result;   // let it recurse
	}

	if (result) {
		wires.removeOne(wire);
	}

	return result;
}

bool PCBSketchWidget::reviewDeletedConnections(QSet<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash *> & deletedConnections, QUndoCommand * parentCommand)
{
	Q_UNUSED(parentCommand);
	Q_UNUSED(deletedItems);

	foreach (ConnectorPairHash * connectorHash, deletedConnections.values())
	{
		QList <ConnectorItem *> removeKeys;
		foreach (ConnectorItem * fromConnectorItem,  connectorHash->uniqueKeys()) {
			if (fromConnectorItem->attachedTo()->getVirtual()) {
				removeKeys.append(fromConnectorItem);
				continue;
			}

			QList<ConnectorItem *> removeValues;
			foreach (ConnectorItem * toConnectorItem, connectorHash->values(fromConnectorItem)) {
				if (toConnectorItem->attachedTo()->getVirtual()) {
					removeValues.append(toConnectorItem);
				}
			}
			foreach (ConnectorItem * toConnectorItem, removeValues) {
				connectorHash->remove(fromConnectorItem, toConnectorItem);
			}
		}
		foreach (ConnectorItem * fromConnectorItem, removeKeys) {
			connectorHash->remove(fromConnectorItem);
		}
	}

	return false;
}

bool PCBSketchWidget::canCreateWire(Wire * dragWire, ConnectorItem * from, ConnectorItem * to)
{
	Q_UNUSED(dragWire);
	return ((from != NULL) && (to != NULL));
}

Wire * PCBSketchWidget::makeOneRatsnestWire(ConnectorItem * source, ConnectorItem * dest, RatsnestCommand * ratsnestCommand, bool select) {
	if (source->attachedTo() == dest->attachedTo()) {
		if (source == dest) return NULL;

		if (source->bus() == dest->bus() && dest->bus() != NULL) {
			return NULL;				// don't draw a wire within the same part on the same bus
		}
	}
	
	long newID = ItemBase::getNextID();

	ViewGeometry viewGeometry;
	makeRatsnestViewGeometry(viewGeometry, source, dest);

	/*
	 DebugDialog::debug(QString("creating ratsnest %10: %1, from %6 %7, to %8 %9, frompos: %2 %3, topos: %4 %5")
	 .arg(newID)
	 .arg(fromPos.x()).arg(fromPos.y())
	 .arg(toPos.x()).arg(toPos.y())
	 .arg(source->attachedToTitle()).arg(source->connectorSharedID())
	 .arg(dest->attachedToTitle()).arg(dest->connectorSharedID())
	 .arg(m_viewIdentifier)
	 );
	 */

	ItemBase * newItemBase = addItem(m_paletteModel->retrieveModelPart(ModuleIDNames::wireModuleIDName), source->attachedTo()->viewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);		
	Wire * wire = dynamic_cast<Wire *>(newItemBase);
	tempConnectWire(wire, source, dest);
	if (!select) {
		wire->setSelected(false);
	}

	Wire * tempWire = source->wiredTo(dest, ViewGeometry::TraceFlag);
	if (tempWire) {
		wire->setOpacity(getRatsnestOpacity(true));
	}

	if (ratsnestCommand) {
		ratsnestCommand->addWire(this, wire, source, dest, select);
	}
	return wire ;
}

void PCBSketchWidget::makeRatsnestViewGeometry(ViewGeometry & viewGeometry, ConnectorItem * source, ConnectorItem * dest) 
{
	QPointF fromPos = source->sceneAdjustedTerminalPoint(NULL);
	viewGeometry.setLoc(fromPos);
	QPointF toPos = dest->sceneAdjustedTerminalPoint(NULL);
	QLineF line(0, 0, toPos.x() - fromPos.x(), toPos.y() - fromPos.y());
	viewGeometry.setLine(line);
	viewGeometry.setWireFlags(ViewGeometry::RatsnestFlag | ViewGeometry::VirtualFlag);
}


bool PCBSketchWidget::dealWithRatsnestAux(ConnectorItem * & fromConnectorItem, ConnectorItem * & toConnectorItem, 
						long fromID, const QString & fromConnectorID, 
						long toID, const QString & toConnectorID,
						ViewLayer::ViewLayerSpec viewLayerSpec,
						bool connect, class RatsnestCommand * ratsnestCommand, bool doEmit) 
{
	SketchWidget::dealWithRatsnest(fromID, fromConnectorID, toID, toConnectorID, viewLayerSpec, connect, ratsnestCommand, doEmit);

	ItemBase * from = findItem(fromID);
	if (from == NULL) return true;

	fromConnectorItem = findConnectorItem(from, fromConnectorID, viewLayerSpec);
	if (fromConnectorItem == NULL) return true;

	ItemBase * to = findItem(toID);
	if (to == NULL) return true;

	toConnectorItem = findConnectorItem(to, toConnectorID, viewLayerSpec);
	if (toConnectorItem == NULL) return true;

	return alreadyRatsnest(fromConnectorItem, toConnectorItem);
}

bool PCBSketchWidget::doRatsnestOnCopy() 
{
	return true;
}

qreal PCBSketchWidget::getRatsnestOpacity(Wire * wire) {
	return getRatsnestOpacity(wire->getRouted());
}

qreal PCBSketchWidget::getRatsnestOpacity(bool routed) {
	return (routed ? 0.2 : 1.0);
}

ConnectorItem * PCBSketchWidget::lookForBreadboardConnection(ConnectorItem * connectorItem) 
{
	Wire * wire = dynamic_cast<Wire *>(connectorItem->attachedTo());
	QList<ConnectorItem *> ends;
	if (wire != NULL) {
		QList<ConnectorItem *> uniqueEnds;
		QList<Wire *> wires;
		wire->collectChained(wires, ends, uniqueEnds);
		foreach (ConnectorItem * end, ends) {
			foreach (ConnectorItem * toConnectorItem, end->connectedToItems()) {
				if (toConnectorItem->attachedToItemType() == ModelPart::Breadboard) {
					return findEmptyBusConnectorItem(toConnectorItem);
				}
			}
		}
	}

	ends.clear();
	ends.append(connectorItem);
	ConnectorItem::collectEqualPotential(ends, true, ViewGeometry::TraceJumperRatsnestFlags);
	foreach (ConnectorItem * end, ends) {
		if (end->attachedToItemType() == ModelPart::Breadboard) {
			return findEmptyBusConnectorItem(end);
		}
	}

	return connectorItem;
}

ConnectorItem * PCBSketchWidget::findEmptyBusConnectorItem(ConnectorItem * busConnectorItem) {
	Bus * bus = busConnectorItem->bus();
	if (bus == NULL) return busConnectorItem;

	QList<ConnectorItem *> connectorItems;
	busConnectorItem->attachedTo()->busConnectorItems(bus, connectorItems);
	foreach (ConnectorItem * connectorItem, connectorItems) {
		if (connectorItem == busConnectorItem) continue;

		if (connectorItem->connectionsCount() == 0) {
			return connectorItem;
		}
	}

	return busConnectorItem;
}


long PCBSketchWidget::makeModifiedWire(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, BaseCommand::CrossViewType cvt, ViewGeometry::WireFlags wireFlags, QUndoCommand * parentCommand) 
{
	// create a new real wire
	long newID = ItemBase::getNextID();
	DebugDialog::debug(QString("new real wire %1").arg(newID));
	ViewGeometry viewGeometry;
	makeRatsnestViewGeometry(viewGeometry, fromConnectorItem, toConnectorItem);
	viewGeometry.setWireFlags(wireFlags);
	ViewLayer::ViewLayerSpec viewLayerSpec = wireViewLayerSpec(fromConnectorItem);
	new AddItemCommand(this, cvt, ModuleIDNames::wireModuleIDName, viewLayerSpec, viewGeometry, newID, true, -1, parentCommand);
	new CheckStickyCommand(this, cvt, newID, false, parentCommand);

	new ChangeConnectionCommand(this, cvt,
								newID, "connector0",
								fromConnectorItem->attachedToID(), fromConnectorItem->connectorSharedID(),
								ViewLayer::specFromID(fromConnectorItem->attachedToViewLayerID()),
								true, parentCommand);
	new ChangeConnectionCommand(this, cvt,
								newID, "connector1",
								toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
								ViewLayer::specFromID(toConnectorItem->attachedToViewLayerID()),
								true, parentCommand);

	if (wireFlags == 0) {
		new RatsnestCommand(this, cvt,
							newID, "connector0",
							fromConnectorItem->attachedToID(), fromConnectorItem->connectorSharedID(),
							ViewLayer::specFromID(fromConnectorItem->attachedToViewLayerID()),
							true, parentCommand);
		new RatsnestCommand(this, cvt,
							newID, "connector1",
							toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
							ViewLayer::specFromID(toConnectorItem->attachedToViewLayerID()),
							true, parentCommand);
	}
	return newID;
}

void PCBSketchWidget::modifyNewWireConnectionsAux(ConnectorItem * fromConnectorItem,
												  ConnectorItem * toConnectorItem, 
												  QUndoCommand * parentCommand)
{
	ConnectorItem * originalFromConnectorItem = fromConnectorItem;
	fromConnectorItem = lookForBreadboardConnection(fromConnectorItem);		
	if (fromConnectorItem->attachedToItemType() == ModelPart::Breadboard) {
		makeModifiedWire(fromConnectorItem, toConnectorItem, BaseCommand::CrossView, 0, parentCommand);
		return;
	}

	makeTwoWires(originalFromConnectorItem, fromConnectorItem, toConnectorItem, toConnectorItem, parentCommand);
}

void PCBSketchWidget::makeTwoWires(ConnectorItem * originalFromConnectorItem, ConnectorItem * fromConnectorItem,
									ConnectorItem * originalToConnectorItem, ConnectorItem * toConnectorItem, 
									QUndoCommand * parentCommand) 
{	
	ItemBase * newBreadboard = NULL;
	if (!(fromConnectorItem->attachedToItemType() == ModelPart::Breadboard)) {
		// find an empty bus on a breadboard
		fromConnectorItem = lookForNewBreadboardConnection(fromConnectorItem, newBreadboard);
		if (fromConnectorItem == NULL) {
			// this shouldn't happen
			return;
		}

		if (newBreadboard) {
			new AddItemCommand(this, BaseCommand::CrossView, newBreadboard->modelPart()->moduleID(), originalFromConnectorItem->attachedTo()->viewLayerSpec(), newBreadboard->getViewGeometry(), newBreadboard->id(), true, -1, parentCommand);
			m_temporaries.append(newBreadboard);			// puts it on a list to be deleted
		}
	}

	ConnectorItem * nearestPartConnectorItem = findNearestPartConnectorItem(originalFromConnectorItem);
	if (nearestPartConnectorItem == NULL) return;

	// make a wire, from the part nearest to fromConnectorItem, to the breadboard
	toConnectorItem = nearestPartConnectorItem;
	makeModifiedWire(fromConnectorItem, toConnectorItem, BaseCommand::CrossView, 0, parentCommand);

	if (originalToConnectorItem->attachedToItemType() == ModelPart::Wire) {
		originalToConnectorItem = findNearestPartConnectorItem(originalToConnectorItem);
		if (originalToConnectorItem == NULL) return;
	}

	// draw a wire from that bus on the breadboard to the other part (toConnectorItem)
	ConnectorItem * otherPartBusConnectorItem = findEmptyBusConnectorItem(fromConnectorItem);
	makeModifiedWire(otherPartBusConnectorItem, originalToConnectorItem, BaseCommand::CrossView, 0, parentCommand);
}

ConnectorItem * PCBSketchWidget::lookForNewBreadboardConnection(ConnectorItem * connectorItem,  ItemBase * & newBreadboard) {
	Q_UNUSED(connectorItem);
	
	newBreadboard = NULL;
	QList<ItemBase *> breadboards;
	qreal maxY = 0;
	foreach (QGraphicsItem * item, scene()->items()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;

		if (itemBase->itemType() == ModelPart::Breadboard) {
			breadboards.append(itemBase);
			break;
		}

		qreal y = itemBase->pos().y() + itemBase->size().height();
		if (y > maxY) {
			maxY = y;
		}
	}

	ConnectorItem * busConnectorItem = NULL;
	foreach (ItemBase * breadboard, breadboards) {
		busConnectorItem = findEmptyBus(breadboard);
		if (busConnectorItem != NULL) return busConnectorItem;
	}

	ViewGeometry vg;
	vg.setLoc(QPointF(0, maxY + 50));

	long id = ItemBase::getNextID();
	newBreadboard = this->addItem(ModuleIDNames::tinyBreadboardModuleIDName, defaultViewLayerSpec(), BaseCommand::SingleView, vg, id, -1, NULL);
	busConnectorItem = findEmptyBus(newBreadboard);
	return busConnectorItem;
}

ConnectorItem * PCBSketchWidget::findEmptyBus(ItemBase * breadboard) {
	foreach (Bus * bus, breadboard->buses().values()) {
		QList<ConnectorItem *> busConnectorItems;
		breadboard->busConnectorItems(bus, busConnectorItems);
		bool allEmpty = true;
		foreach (ConnectorItem * busConnectorItem, busConnectorItems) {
			if (busConnectorItem->connectionsCount() > 0) {
				allEmpty = false;
				break;
			}
		}
		if (allEmpty && busConnectorItems.count() > 0) {
			return busConnectorItems[0];
		}
	}
	return NULL;
}

ConnectorItem * PCBSketchWidget::findNearestPartConnectorItem(ConnectorItem * fromConnectorItem) {
	// find the nearest part to fromConnectorItem
	Wire * wire = dynamic_cast<Wire *>(fromConnectorItem->attachedTo());
	if (wire == NULL) return NULL;

	QList<ConnectorItem *> ends;
	calcDistances(wire, ends);
	clearDistances();
	if (ends.count() < 1) return NULL;

	return ends[0];
}

void PCBSketchWidget::calcDistances(Wire * wire, QList<ConnectorItem *> & ends) {
	QList<Wire *> chained;
	QList<ConnectorItem *> uniqueEnds;
	wire->collectChained(chained, ends, uniqueEnds);
	if (ends.count() < 2) return;

	clearDistances();
	foreach (ConnectorItem * end, ends) {
		bool fromConnector0;
		QList<Wire *> distanceWires;
		int distance = calcDistance(wire, end, 0, distanceWires, fromConnector0);
		DistanceThing * dt = new DistanceThing;
		dt->distance = distance;
		dt->fromConnector0 = fromConnector0;
		DebugDialog::debug(QString("distance %1 %2 %3, %4 %5")
			.arg(end->attachedToID()).arg(end->attachedToTitle()).arg(end->connectorSharedID())
			.arg(distance).arg(fromConnector0 ? "connector0" : "connector1"));
		distances.insert(end, dt);
	}
	qSort(ends.begin(), ends.end(), distanceLessThan);

}

void PCBSketchWidget::clearDistances() {
	foreach (ConnectorItem * c, distances.keys()) {
		DistanceThing * dt = distances.value(c, NULL);
		if (dt) delete dt;
	}
	distances.clear();
}

int PCBSketchWidget::calcDistanceAux(ConnectorItem * from, ConnectorItem * to, int distance, QList<Wire *> & distanceWires) {
	//DebugDialog::debug(QString("calc distance aux: %1 %2, %3 %4, %5").arg(from->attachedToID()).arg(from->connectorSharedID())
		//.arg(to->attachedToTitle()).arg(to->connectorSharedID()).arg(distance));

	foreach (ConnectorItem * toConnectorItem, from->connectedToItems()) {
		if (toConnectorItem == to) {
			return distance;
		}
	}

	int result = MAX_INT;
	foreach (ConnectorItem * toConnectorItem, from->connectedToItems()) {
		if (toConnectorItem->attachedToItemType() != ModelPart::Wire) continue;

		Wire * w = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
		if (distanceWires.contains(w)) continue;

		bool fromConnector0;
		int temp = calcDistance(w, to, distance + 1, distanceWires, fromConnector0);
		if (temp < result) {
			result = temp;
		}
	}

	return result;
}

int PCBSketchWidget::calcDistance(Wire * wire, ConnectorItem * end, int distance, QList<Wire *> & distanceWires, bool & fromConnector0) {
	//DebugDialog::debug(QString("calc distance wire: %1 rat:%2 to %3 %4, %5").arg(wire->id()).arg(wire->getRatsnest())
		//.arg(end->attachedToTitle()).arg(end->connectorSharedID()).arg(distance));
	
	distanceWires.append(wire);
	int d0 = calcDistanceAux(wire->connector0(), end, distance, distanceWires);
	if (d0 == distance) {
		fromConnector0 = true;
		return d0;
	}

	int d1 = calcDistanceAux(wire->connector1(), end, distance, distanceWires);
	if (d0 <= d1) {
		fromConnector0 = true;
		return d0;
	}

	fromConnector0 = false;
	return d1;
}

void PCBSketchWidget::setJumperFlags(ViewGeometry & vg) {
	vg.setJumper(true);
}

bool PCBSketchWidget::usesJumperItem() {
	return true;
}

void PCBSketchWidget::showGroundTraces(bool show) {
	foreach (QGraphicsItem * item, scene()->items()) {
		TraceWire * trace = dynamic_cast<TraceWire *>(item);
		if (trace == NULL) continue;

		if (trace->isGrounded()) {
			trace->setVisible(show);
		}
	}
}

void PCBSketchWidget::getLabelFont(QFont & font, QColor & color, ViewLayer::ViewLayerSpec viewLayerSpec) {
	font.setFamily("OCRA");			// ocra10
	font.setPointSize(getLabelFontSizeMedium());
	color.setAlpha(255);

	switch (viewLayerSpec) {
		case ViewLayer::WireOnTop_TwoLayers:
		case ViewLayer::WireOnBottom_OneLayer:
		case ViewLayer::WireOnBottom_TwoLayers:
			DebugDialog::debug("bad viewLayerSpec in getLabelFont");
			break;

		case ViewLayer::ThroughHoleThroughTop_OneLayer:
		case ViewLayer::ThroughHoleThroughTop_TwoLayers:
		case ViewLayer::GroundPlane_Top:
			color.setNamedColor(ViewLayer::Silkscreen1Color);
			break;
		case ViewLayer::ThroughHoleThroughBottom_TwoLayers:
		case ViewLayer::GroundPlane_Bottom:
			color.setNamedColor(ViewLayer::Silkscreen0Color);
			break;
		case ViewLayer::SMDOnTop_TwoLayers:
			color.setNamedColor(ViewLayer::Silkscreen1Color);
			break;
		case ViewLayer::SMDOnBottom_TwoLayers:
		case ViewLayer::SMDOnBottom_OneLayer:
			color.setNamedColor(ViewLayer::Silkscreen0Color);
			break;
	}
}

void PCBSketchWidget::makeWiresChangeConnectionCommands(const QList<Wire *> & wires, QUndoCommand * parentCommand)
{
	QStringList alreadyList;
	foreach (Wire * wire, wires) {
		QList<ConnectorItem *> wireConnectorItems;
		wireConnectorItems << wire->connector0() << wire->connector1();
		foreach (ConnectorItem * fromConnectorItem, wireConnectorItems) {
			foreach(ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
				QString already = ((fromConnectorItem->attachedToID() <= toConnectorItem->attachedToID()) ? QString("%1.%2.%3.%4") : QString("%3.%4.%1.%2"))
					.arg(fromConnectorItem->attachedToID()).arg(fromConnectorItem->connectorSharedID())
					.arg(toConnectorItem->attachedToID()).arg(toConnectorItem->connectorSharedID());
				if (alreadyList.contains(already)) continue;

				alreadyList.append(already);
				new ChangeConnectionCommand(this, BaseCommand::SingleView,
											fromConnectorItem->attachedToID(), fromConnectorItem->connectorSharedID(),
											toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
											ViewLayer::specFromID(toConnectorItem->attachedToViewLayerID()),
											false, parentCommand);
			}
		}
	}
}

qreal PCBSketchWidget::getLabelFontSizeSmall() {
	return 5;
}

qreal PCBSketchWidget::getLabelFontSizeMedium() {
	return 7;
}

qreal PCBSketchWidget::getLabelFontSizeLarge() {
	return 12;
}

void PCBSketchWidget::resizeBoard(qreal mmW, qreal mmH, bool doEmit)
{
	Q_UNUSED(doEmit);

	PaletteItem * item = getSelectedPart();
	if (item == NULL) return;

	switch (item->itemType()) {
		case ModelPart::ResizableBoard:
		case ModelPart::Logo:
			break;
		default:
			return SketchWidget::resizeBoard(mmW, mmH, doEmit);
	}

	qreal origw = item->modelPart()->prop("width").toDouble();
	qreal origh = item->modelPart()->prop("height").toDouble();

	if (mmH == 0 || mmW == 0) {
		dynamic_cast<ResizableBoard *>(item)->setInitialSize();
		qreal w = item->modelPart()->prop("width").toDouble();
		qreal h = item->modelPart()->prop("height").toDouble();
		if (origw == w && origh == h) {
			// no change
			return;
		}

		viewItemInfo(item);
		mmW = w;
		mmH = h;
	}

	QUndoCommand * parentCommand = new QUndoCommand(tr("Resize board to %1 %2").arg(mmW).arg(mmH));
	new ResizeBoardCommand(this, item->id(), origw, origh, mmW, mmH, parentCommand);
	new CheckStickyCommand(this, BaseCommand::SingleView, item->id(), true, parentCommand);
	m_undoStack->push(parentCommand);
}

void PCBSketchWidget::showLabelFirstTime(long itemID, bool show, bool doEmit) {
	SketchWidget::showLabelFirstTime(itemID, show, doEmit);
	ItemBase * itemBase = findItem(itemID);
	if (itemBase == NULL) return;

	switch (itemBase->itemType()) {
		case ModelPart::Part:
			itemBase->showPartLabel(true, m_viewLayers.value(getLabelViewLayerID(itemBase->viewLayerSpec())));
			break;
		default:
			break;
	}

}

ItemBase * PCBSketchWidget::findBoard() {
    foreach (QGraphicsItem * childItem, items()) {
        ItemBase * board = dynamic_cast<ItemBase *>(childItem);
        if (board == NULL) continue;

        //for now take the first board you find
        if (board->itemType() == ModelPart::ResizableBoard || board->itemType() == ModelPart::Board) {
            return board;
        }
    }

	return NULL;
}

void PCBSketchWidget::collectConnectorNames(QList<ConnectorItem *> & connectorItems, QStringList & connectorNames) 
{
	foreach(ConnectorItem * connectorItem, connectorItems) {
		if (!connectorNames.contains(connectorItem->connectorSharedName())) {
			connectorNames.append(connectorItem->connectorSharedName());
			//DebugDialog::debug("name " + connectorItem->connectorSharedName());
		}
	}
}

void PCBSketchWidget::updateRatsnestColors(BaseCommand * command, QUndoCommand * parentCommand, bool forceUpdate, RoutingStatus & routingStatus) 
{
	//DebugDialog::debug("update ratsnest colors");
	// TODO: think about ways to optimize this...

	QList<ConnectorItem *> virtualWireConnectors;
	foreach (QGraphicsItem * item, items()) {
		VirtualWire * vw = dynamic_cast<VirtualWire *>(item);
		if (vw == NULL) continue;

		virtualWireConnectors.append(vw->connector0());
		virtualWireConnectors.append(vw->connector1());
	}

	while (virtualWireConnectors.count() > 0) {
		QList<ConnectorItem *> connectorItems;
		connectorItems.append(virtualWireConnectors.takeFirst());
		ConnectorItem::collectEqualPotential(connectorItems, true, ViewGeometry::NormalFlag | ViewGeometry::TraceFlag | ViewGeometry::JumperFlag);
		for (int i = 1; i < connectorItems.count(); i++) {
			virtualWireConnectors.removeOne(connectorItems[i]);
		}

		scoreOneNet(connectorItems, routingStatus);

		recolor(connectorItems, command, parentCommand, forceUpdate);
	}

	routingStatus.m_jumperWireCount /= 2;			// since we counted each connector
	routingStatus.m_jumperItemCount /= 2;			// since we counted each connector
}

void traceAdjacency(QVector< QVector<bool> > & adjacency, int count)
{
	QString string = "\n";
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < count; j++) {
			string += adjacency[i][j] ? "1" : "0";
		}
		string += "\n";
	}
	DebugDialog::debug(string);
}

void PCBSketchWidget::scoreOneNet(QList<ConnectorItem *> & connectorItems, RoutingStatus & routingStatus) 
{
	routingStatus.m_netCount++;
	QList<ConnectorItem *> partConnectorItems;
	ConnectorItem::collectParts(connectorItems, partConnectorItems, includeSymbols(), ViewLayer::TopAndBottom);
	int count = partConnectorItems.count();
	// want adjacency[count][count] but some C++ compilers don't like it
	QVector< QVector<bool> > adjacency(count);
	for (int i = 0; i < count; i++) {
		QVector<bool> row(count, false);
		adjacency[i] = row;
	}

	// initialize adjaceny
	for (int i = 0; i < count; i++) {
		adjacency[i][i] = true;
		ConnectorItem * from = partConnectorItems[i];
		for (int j = i + 1; j < count; j++) {
			if (j == i) continue;

			ConnectorItem * to = partConnectorItems[j];
			if (to->attachedTo() != from->attachedTo()) continue;
			if (to->bus() == NULL) continue;
			if (to->bus() != from->bus()) continue;
				
			adjacency[i][j] = true;
			adjacency[j][i] = true;
		}
	}

	//traceAdjacency(adjacency, count);

	for (int i = 0; i < count; i++) {
		ConnectorItem * fromConnectorItem = partConnectorItems[i];
		if (fromConnectorItem->attachedToItemType() == ModelPart::Jumper) {
			routingStatus.m_jumperItemCount++;				
		}
		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
			if (toConnectorItem->attachedToItemType() != ModelPart::Wire) {
				continue;
			}

			Wire * wire = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
			if (wire == NULL) continue;

			if (!(wire->getJumper() || wire->getTrace())) continue;

			if (wire->getJumper()) {
				routingStatus.m_jumperWireCount++;
			}

			QList<Wire *> wires;
			QList<ConnectorItem *> ends;
			QList<ConnectorItem *> uniqueEnds;
			wire->collectChained(wires, ends, uniqueEnds);
			foreach (ConnectorItem * end, ends) {
				if (end == fromConnectorItem) continue;

				int j = partConnectorItems.indexOf(end);
				if (j >= 0) {
					adjacency[i][j] = true;
					adjacency[j][i] = true;
				}
			}
		}
	}

	//traceAdjacency(adjacency, count);

	transitiveClosure(adjacency, count);

	//traceAdjacency(adjacency, count);

	int todo = countMissing(adjacency, count);
	if (todo == 0) {
		routingStatus.m_netRoutedCount++;
	}
	else {
		routingStatus.m_connectorsLeftToRoute += todo;
	}
}

int PCBSketchWidget::countMissing(QVector< QVector<bool> > & adjacency, int count)
{
	QVector<bool> check(count, true);
	int missing = 0;
	for (int i = 0; i < count; i++) {
		if (!check[i]) continue;

		check[i] = false;
		bool missingOne = false;
		for (int j = 0; j < count; j++) {
			if (!check[j]) continue;
			if (i == j) continue;

			if (adjacency[i][j]) {
				check[j] = false;
				continue;
			}

			missingOne = true;							// we can minimally span the set with n-1 wires, so even if multiple connections are missing from a given connector, count it as one
		}

		if (missingOne) missing++;
	}

	return missing;
}


void PCBSketchWidget::transitiveClosure(QVector< QVector<bool> > & adjacency, int count)
{
	// TODO: is there a faster implementation?
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < count; j++) {
			if (adjacency[i][j]) {
				for (int k = 0; k < count; k++) {
					if (adjacency[j][k]) {
						adjacency[i][k] = true;
						adjacency[k][i] = true;
					}
				}
			}
		}
	}
}

void PCBSketchWidget::recolor(QList<ConnectorItem *> & connectorItems, BaseCommand * command, QUndoCommand * parentCommand, bool forceUpdate) 
{
	QColor standardColor = RatsnestColors::netColor(m_viewIdentifier);

	QStringList connectorNames;
	collectConnectorNames(connectorItems, connectorNames);
	QColor color;
	bool gotColor = RatsnestColors::findConnectorColor(connectorNames, color);

	QList<VirtualWire *> virtualWires;

	foreach(ConnectorItem * connectorItem, connectorItems) {
		if (connectorItem->attachedToItemType() != ModelPart::Wire) continue;

		VirtualWire * vw = dynamic_cast<VirtualWire *>(connectorItem->attachedTo());
		if (vw == NULL) continue;
		if (virtualWires.contains(vw)) continue;

		virtualWires.append(vw);

		bool routed = false;
		qreal opacity = vw->opacity();
		ConnectorItem * from = vw->connector0()->firstConnectedToIsh();
		if (from) {
			ConnectorItem * to = vw->connector1()->firstConnectedToIsh();
			if (to) {
				QList<ConnectorItem *> traceConnectorItems;
				traceConnectorItems.append(from);
				ConnectorItem::collectEqualPotential(traceConnectorItems, true, ViewGeometry::RatsnestFlag | ViewGeometry::NormalFlag);
				routed = traceConnectorItems.contains(to);
			}
		}
		qreal newOpacity = getRatsnestOpacity(routed);

		QColor currentColor = vw->color();
		bool isC = RatsnestColors::isConnectorColor(m_viewIdentifier, currentColor);
		QColor useColor;
		if (gotColor && isC) {
			if (color == currentColor) {
				// no change necessary
				if (!forceUpdate && (newOpacity == opacity)) continue;
			}
			useColor = color;
		}
		else if (gotColor) {
			useColor = color;
		}
		else if (isC) {
			// this shouldn't happen, or at least not often
			useColor = standardColor;
		}
		else {
			// no change necessary
			if (!forceUpdate && (newOpacity == opacity)) continue;
			useColor = forceUpdate ? standardColor : currentColor;
		}

		WireColorChangeCommand * cmd = new WireColorChangeCommand(this, vw->id(), currentColor.name(), useColor.name(), vw->opacity(), newOpacity, parentCommand);
		if (command) {
			command->addSubCommand(cmd);
		}
		vw->setColor(useColor, newOpacity);
	}
}

double PCBSketchWidget::defaultGridSizeInches() {
	return 0.1;
}

bool PCBSketchWidget::canAlignToTopLeft(ItemBase * itemBase) 
{
	switch (itemBase->itemType()) {
		case ModelPart::Board:
		case ModelPart::ResizableBoard:
		case ModelPart::Ruler:
			return true;
		default:
			return false;
	}
}

ViewLayer::ViewLayerID PCBSketchWidget::getLabelViewLayerID(ViewLayer::ViewLayerSpec viewLayerSpec) {
	switch (viewLayerSpec) {
		case ViewLayer::WireOnTop_TwoLayers:
		case ViewLayer::WireOnBottom_OneLayer:
		case ViewLayer::WireOnBottom_TwoLayers:
			DebugDialog::debug("bad viewLayerSpec in getLabelViewLayerID");
			return ViewLayer::Silkscreen1Label;

		case ViewLayer::ThroughHoleThroughTop_OneLayer:
		case ViewLayer::ThroughHoleThroughTop_TwoLayers:
			return ViewLayer::Silkscreen1Label;
		case ViewLayer::ThroughHoleThroughBottom_TwoLayers:
			return ViewLayer::Silkscreen0Label;
		case ViewLayer::SMDOnTop_TwoLayers:
			return ViewLayer::Silkscreen1Label;
		case ViewLayer::SMDOnBottom_OneLayer:
		case ViewLayer::SMDOnBottom_TwoLayers:
			return ViewLayer::Silkscreen0Label;
		default:
			return ViewLayer::Silkscreen1Label;
	}

}

void PCBSketchWidget::cancelDRC() {
	m_cancelDRC = true;
}

void PCBSketchWidget::stopDRC() {
}

int PCBSketchWidget::designRulesCheck() 
{
	// TODO: 
	//	what about ground plane?

	m_cancelDRC = false;
	QUndoCommand * parentCommand = new QUndoCommand(QObject::tr("Design Rule Check (select items that are too close together)"));
	SelectItemCommand * selectItemCommand = stackSelectionState(false, parentCommand);
	setIgnoreSelectionChangeEvents(true);

	AutorouteProgressDialog progress(tr("Design Rules Check Progress..."), false, false, this, this->window());
	progress.setModal(true);
	progress.show();

	connect(&progress, SIGNAL(cancel()), this, SLOT(cancelDRC()), Qt::DirectConnection);
	connect(&progress, SIGNAL(stop()), this, SLOT(stopDRC()), Qt::DirectConnection);
	connect(this, SIGNAL(setMaximumDRCProgress(int)), &progress, SLOT(setMaximum(int)), Qt::DirectConnection);
	connect(this, SIGNAL(setDRCProgressValue(int)), &progress, SLOT(setValue(int)), Qt::DirectConnection);
	QApplication::processEvents();

	emit setMaximumDRCProgress(1000);
	
	scene()->clearSelection();
	QColor color;
	color = this->background();
	this->setBackground(QColor::fromRgb(255,255,255,255));
	this->saveLayerVisibility();
	this->setAllLayersVisible(false);
	bool copper0Active = layerIsActive(ViewLayer::Copper0);
	bool copper1Active = layerIsActive(ViewLayer::Copper1);
	setLayerActive(ViewLayer::Copper0, true);
	setLayerActive(ViewLayer::Copper1, true);
	this->setLayerVisible(ViewLayer::Copper0, true);
	QSet<ItemBase *> collidingItems;
	if (m_boardLayers == 1) {
		drcLayer(collidingItems, 0, 1.0, 1000);
	}
	else {
		drcLayer(collidingItems, 0, 0.5, 1000);
		this->setLayerVisible(ViewLayer::Copper0, false);
		this->setLayerVisible(ViewLayer::Copper1, true);
		drcLayer(collidingItems, 500, 0.5, 1000);
	}

	this->setBackground(color);
	this->restoreLayerVisibility();
	setLayerActive(ViewLayer::Copper0, copper0Active);
	setLayerActive(ViewLayer::Copper1, copper1Active);

	if (m_cancelDRC) {
		selectItemCommand->undo();
		delete parentCommand;
		setIgnoreSelectionChangeEvents(false);
		return -1;
	}
	else {
		setIgnoreSelectionChangeEvents(false);
		SelectItemCommand * selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
		foreach (ItemBase * itemBase, collidingItems) {
			selectItemCommand->addRedo(itemBase->layerKinChief()->id());
		}

		m_undoStack->push(parentCommand);
		return collidingItems.count();
	}
}

void PCBSketchWidget::drcLayer(QSet<ItemBase *> & collidingItems, int progressOffset, qreal progressRange, int progressGoal) {
	QSet<QGraphicsItem *> checkItems;

	foreach (QGraphicsItem * item, scene()->items()) {
		if (!item->isVisible()) {
			continue;
		}

		TraceWire * tw = dynamic_cast<TraceWire *>(item);
		if (tw != NULL) {
			checkItems.insert(item);
			continue;
		}

		GroundPlane * gp = dynamic_cast<GroundPlane *>(item);
		if (gp != NULL) {
			// skip these (just for now?)
			continue;
		}

		NonConnectorItem * nonConnectorItem = dynamic_cast<NonConnectorItem *>(item);
		if (nonConnectorItem != NULL) {
			if (nonConnectorItem->hidden()) continue;

			// if we're attached to a trace or groundplane no need to do a separate check on the connector
			TraceWire * tw = dynamic_cast<TraceWire *>(nonConnectorItem->attachedTo());
			if (tw != NULL) continue;

			GroundPlane * gp = dynamic_cast<GroundPlane *>(nonConnectorItem->attachedTo());
			if (gp != NULL) continue;

			checkItems.insert(item);
		}
	}

	qreal progressSoFar = checkItems.count() / 10;
	int maxProgress = checkItems.count() + progressSoFar;

	emit setDRCProgressValue(progressOffset + (progressGoal * progressRange * progressSoFar / maxProgress));

	int imageCount = 0;
	qreal expandBy = .01 * FSvgRenderer::printerScale();
	foreach (QGraphicsItem * checkItem, checkItems) {
		if (m_cancelDRC) break;

		QRectF r = checkItem->boundingRect();
		r.adjust(-expandBy, -expandBy, expandBy, expandBy);
		QPolygonF expandedCheckItemPoly = checkItem->mapToScene(r);						// mapToScene does take transforms into account

		QList<ConnectorItem *> equipotentialConnectorItems;
		TraceWire * checkTraceWire = NULL;
		QGraphicsItem * checkItemParent = checkItem->parentItem();
		ConnectorItem * checkConnectorItem = dynamic_cast<ConnectorItem *>(checkItem);
		if (checkConnectorItem) {
			if (checkConnectorItem->attachedToItemType() == ModelPart::CopperFill) {
				// skip these
				continue;
			}

			equipotentialConnectorItems.append(checkConnectorItem);
		}
		else {
			checkTraceWire = dynamic_cast<TraceWire *>(checkItem);
			if (checkTraceWire != NULL) {
				equipotentialConnectorItems.append(checkTraceWire->connector0());
			}
		}
	
		ConnectorItem::collectEqualPotential(equipotentialConnectorItems, false, ViewGeometry::NoFlag);
		for (int i = equipotentialConnectorItems.count() - 1; i >= 0; i--) {
			if (!equipotentialConnectorItems.at(i)->isVisible()) {
				//DebugDialog::debug(QString("not visible %1").arg(ViewLayer::viewLayerNameFromID(equipotentialConnectorItems.at(i)->attachedToViewLayerID())));
				equipotentialConnectorItems.removeAt(i);
			}
		}

		QSet<ItemBase *> intersectingItems;
		foreach (QGraphicsItem * candidate, scene()->items(expandedCheckItemPoly)) {
			if (m_cancelDRC) break;
			QApplication::processEvents();

			if (!candidate->isVisible()) {
				continue;
			}

			if (candidate == checkItem) continue;
			if (candidate == checkItemParent) continue;

			ItemBase * itemBase = dynamic_cast<ItemBase *>(candidate);
			if (itemBase == NULL) continue;
			if (itemBase->hidden()) continue;

			TraceWire * tw = dynamic_cast<TraceWire *>(candidate);
			if (tw != NULL) {
				if (equipotentialConnectorItems.contains(tw->connector0())) continue;			// part of the same net; skip it

				intersectingItems.insert(tw);
				continue;
			}

			GroundPlane * gp = dynamic_cast<GroundPlane *>(candidate);
			if (gp != NULL) {
				// skip these
				continue;
			}

			intersectingItems.insert(itemBase);
		}

		if (m_cancelDRC) break;

		if (intersectingItems.count() == 0) {
			emit setDRCProgressValue(progressOffset + (progressGoal * progressRange * ++progressSoFar / maxProgress));
			continue;
		}

		// take equipotential connectors out of the picture; unfortunately the visible part of these connectors
		// is actually part of the svg of the parent, so this is a hack to remove them completely from the "other" image
		// if the connector isn't well-behaved (a nice circle or rect) then this step may remove too much
		// would a graphical xor within a clipping region do the job?
		foreach (ConnectorItem * connectorItem, equipotentialConnectorItems) {
			if (connectorItem->attachedToItemType() == ModelPart::Wire) {
				connectorItem->setVisible(false);
			}
			else {
				connectorItem->setWhite(true);
			}
		}

		QApplication::processEvents();
		QRectF expandedPolyBounds = expandedCheckItemPoly.boundingRect();
		QHash<QGraphicsItem *, bool> visibility;
		// hide checkItem and any non-intersecting items so they don't show up in the "other" image
		// also hide the connectors of the intersecting items so they don't show up in the other image (just the svg of their parent)
		foreach (QGraphicsItem * item, scene()->items(expandedPolyBounds)) {
			if (!item->isVisible()) continue;
			
			ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
			if (itemBase) {
				if (itemBase->hidden()) continue;

				if (intersectingItems.contains(itemBase)) continue;
			}

			setDRCVisibility(item, equipotentialConnectorItems, visibility);
		}

		QSize sz(qCeil(expandedPolyBounds.width()), qCeil(expandedPolyBounds.height()));
		QImage otherImage(sz, QImage::Format_ARGB32);
		QPainter painter;
		painter.begin(&otherImage);
		scene()->render(&painter, otherImage.rect(), expandedPolyBounds);
		painter.end();

		for (int x = 0; x < sz.width(); x++) {
			for (int y = 0; y < sz.height(); y++) {
				QRgb p = otherImage.pixel(x, y);
				if (p == 0xffffffff) {
					otherImage.setPixel(x, y, 0x00000000);
				}
				else {
					otherImage.setPixel(x, y, 0xffffffff);
				}
			}
		}

		foreach (ItemBase * itemBase, intersectingItems) {
			itemBase->setVisible(false);
			visibility.insert(itemBase, true);
		}

		checkItem->setVisible(true);
		if (checkConnectorItem) {
			if (checkConnectorItem->attachedToItemType() == ModelPart::Wire) {
				checkConnectorItem->setVisible(true);
			}
			else {
				checkConnectorItem->setWhite(false);
			}
		}
		if (checkItemParent) {
			checkItemParent->setVisible(true);
		}

		QPolygonF poly = checkItem->mapToScene(checkItem->boundingRect());						
		QImage selfImage(sz, QImage::Format_ARGB32);
		painter.begin(&selfImage);
		scene()->render(&painter, selfImage.rect(), expandedPolyBounds);
		painter.end();
		for (int x = 0; x < sz.width(); x++) {
			for (int y = 0; y < sz.height(); y++) {
				if (poly.containsPoint(QPointF(expandedPolyBounds.left() + x, expandedPolyBounds.top() + y), Qt::OddEvenFill)) {
					QRgb p = selfImage.pixel(x, y);
					if (p == 0xffffffff) {
						// white is background in the original image
						selfImage.setPixel(x, y, 0x00000000);
					}
					else {
						selfImage.setPixel(x, y, 0xffffffff);
					}
				}
				else {
					selfImage.setPixel(x, y, 0x00000000);
				}
			}
		}

		QRectF polyBounds = poly.boundingRect();
		qreal growAmountX = (expandBy + expandBy + polyBounds.width()) * sz.width() / polyBounds.width();
		qreal growAmountY = (expandBy + expandBy + polyBounds.height()) * sz.height() / polyBounds.height();
		QImage scaledImage = selfImage.scaled(growAmountX, growAmountY, Qt::KeepAspectRatio);
		

#ifndef QT_NO_DEBUG
		//selfImage.save("testDesignRulePolySelf.png", "png");
		scaledImage.save("testDesignRuleSelfScaled" + QString::number(imageCount) + ".png", "png");
		otherImage.save("testDesignRulePolyOther" + QString::number(imageCount) + ".png", "png");
#endif
		imageCount++;
		
		foreach (ConnectorItem * connectorItem, equipotentialConnectorItems) {
			if (connectorItem->attachedToItemType() == ModelPart::Wire) {
				connectorItem->setVisible(true);
			}
			else {
				connectorItem->setWhite(false);
			}
		}
		foreach (QGraphicsItem * item, visibility.keys()) {
			item->setVisible(visibility.value(item, true));
		}

		int dx = (scaledImage.width() - sz.width()) / 2;
		int dy = (scaledImage.height() - sz.height()) / 2;
		for (int x = 0; x < sz.width(); x++) {
			for (int y = 0; y < sz.height(); y++) {
				QRgb c = otherImage.pixel(x, y);
				if (c != 0xffffffff) continue;

				c = scaledImage.pixel(x + dx, y + dy);
				if (c != 0xffffffff) continue;

				QPointF p(x + expandedPolyBounds.left(), y + expandedPolyBounds.y());
				foreach (QGraphicsItem * item, scene()->items(p)) {
					ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
					if (itemBase == NULL) continue;
					if (!intersectingItems.contains(itemBase)) continue;


					itemBase->setSelected(true);
					collidingItems.insert(itemBase);
					itemBase = dynamic_cast<ItemBase *>(checkItemParent ? checkItemParent : checkItem);
					collidingItems.insert(itemBase);
					itemBase->setSelected(true);
				}
			}
		}
		emit setDRCProgressValue(progressOffset + (progressGoal * progressRange * ++progressSoFar / maxProgress));
		QApplication::processEvents();
	}
}


void PCBSketchWidget::setDRCVisibility(QGraphicsItem * item, QList<ConnectorItem *> & equipotentialConnectorItems, QHash<QGraphicsItem *, bool> & visibility)
{
	ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
	if (connectorItem && equipotentialConnectorItems.contains(connectorItem)) return;

	visibility.insert(item, true);
	item->setVisible(false);
}

ViewLayer::ViewLayerSpec PCBSketchWidget::wireViewLayerSpec(ConnectorItem * connectorItem) {
	switch (connectorItem->attachedToViewLayerID()) {
		case ViewLayer::Copper1:
		case ViewLayer::Copper1Trace:
		case ViewLayer::GroundPlane1:
			return ViewLayer::WireOnTop_TwoLayers;
		default:
			return (m_boardLayers == 1) ?  ViewLayer::WireOnBottom_OneLayer : ViewLayer::WireOnBottom_TwoLayers;
	}
}

void PCBSketchWidget::setBoardLayers(int layers, bool redraw) {
	SketchWidget::setBoardLayers(layers, redraw);

	QList <ViewLayer::ViewLayerID> viewLayerIDs;
	viewLayerIDs << ViewLayer::Copper1 << ViewLayer::Copper1Trace;
	foreach (ViewLayer::ViewLayerID viewLayerID, viewLayerIDs) {
		ViewLayer * layer = m_viewLayers.value(viewLayerID, NULL);
		if (layer) {
			layer->action()->setEnabled(layers == 2);
			layer->setVisible(layers == 2);
			if (redraw) {
				setLayerVisible(layer, layers == 2);
				if (layers == 2) {
					layer->action()->setChecked(true);
				}
			}
		}
	}
}

long PCBSketchWidget::setUpSwap(ItemBase * itemBase, long newModelIndex, const QString & newModuleID, ViewLayer::ViewLayerSpec viewLayerSpec, bool master, QUndoCommand * parentCommand)
{
	long result = SketchWidget::setUpSwap(itemBase, newModelIndex, newModuleID, viewLayerSpec, master, parentCommand);

	int newLayers = isBoardLayerChange(itemBase, newModuleID, master);
	if (newLayers == m_boardLayers) return result;

	QList<ItemBase *> smds;
	QList<Wire *> already;
	ChangeBoardLayersCommand * changeBoardCommand = new ChangeBoardLayersCommand(this, m_boardLayers, newLayers, parentCommand);

	// disconnect and flip smds
	foreach (QGraphicsItem * item, scene()->items()) {
		ItemBase * smd = dynamic_cast<ItemBase *>(item);
		if (smd == NULL) continue;
		if (!smd->modelPart()->flippedSMD()) continue;

		smd = smd->layerKinChief();
		if (smds.contains(smd)) continue;

		foreach (QGraphicsItem * child, smd->childItems()) {
			ConnectorItem * ci = dynamic_cast<ConnectorItem *>(child);
			if (ci == NULL) continue;

			foreach (ConnectorItem * toci, ci->connectedToItems()) {
				TraceWire * tw = qobject_cast<TraceWire *>(toci->attachedTo());
				if (tw == NULL) continue;
				if (already.contains(tw)) continue;

				QList<ConnectorItem *> ends;
				removeWire(tw, ends, already, changeBoardCommand);

				// remove these connections so they're not added back in when the part is swapped
				foreach (ConnectorItem * end, ends) {
					foreach (ConnectorItem * endTo, end->connectedToItems()) {
						end->tempRemove(endTo, false);
						endTo->tempRemove(end, false);
					}
				}
			}
		}

		smds.append(smd);
	}

	if (newLayers == 1) {
		// remove traces on the top layer
		foreach (QGraphicsItem * item, scene()->items()) {
			TraceWire * tw = dynamic_cast<TraceWire *>(item);
			if (tw == NULL) continue;
			if (tw->viewLayerID() != ViewLayer::Copper1Trace) continue;
			if (already.contains(tw)) continue;
				
			QList<ConnectorItem *> ends;
			removeWire(tw, ends, already, changeBoardCommand);
		}
	}

	// TODO need to disconnect first?

	foreach (ItemBase * smd, smds) {
		emit subSwapSignal(this, smd, (newLayers == 1) ? ViewLayer::ThroughHoleThroughTop_OneLayer : ViewLayer::ThroughHoleThroughTop_TwoLayers, changeBoardCommand);
	}

	return result;
}

int PCBSketchWidget::isBoardLayerChange(ItemBase * itemBase, const QString & newModuleID, bool master)
{							
	if (!master) return m_boardLayers;

	switch (itemBase->itemType()) {
		case ModelPart::Board:
		case ModelPart::ResizableBoard:
			// maybe a change
			break;
		default: 
			// no change
			return m_boardLayers;
	}

	ModelPart * modelPart = paletteModel()->retrieveModelPart(newModuleID);
	if (modelPart == NULL) {
		// shouldn't happen
		return m_boardLayers;
	}

	QString slayers = modelPart->properties().value("layers", "");
	if (slayers.isEmpty()) {
		// shouldn't happen
		return m_boardLayers;
	}

	bool ok;
	int layers = slayers.toInt(&ok);
	if (!ok) {
		// shouldn't happen
		return m_boardLayers;
	}

	return layers;
}


void PCBSketchWidget::changeBoardLayers(int layers, bool doEmit) {
	setBoardLayers(layers, true);
	SketchWidget::changeBoardLayers(layers, doEmit);
	if (layers == 1) {
		this->setLayerActive(ViewLayer::Copper0, true);
	}
	emit updateLayerMenuSignal();
}

void PCBSketchWidget::removeWire(Wire * w, QList<ConnectorItem *> & ends, QList<Wire *> & done, QUndoCommand * parentCommand) 
{
	QList<Wire *> chained;
	QList<ConnectorItem *> uniqueEnds;
	w->collectChained(chained, ends, uniqueEnds);
	makeWiresChangeConnectionCommands(chained, parentCommand);
	foreach (Wire * c, chained) {
		makeDeleteItemCommand(c, BaseCommand::SingleView, parentCommand);
		done.append(c);
	}
}

void PCBSketchWidget::loadFromModelParts(QList<ModelPart *> & modelParts, BaseCommand::CrossViewType crossViewType, QUndoCommand * parentCommand, bool doRatsnest, bool offsetPaste) {
	if (parentCommand == NULL) {
		bool done = false;
		foreach (ModelPart * modelPart, modelParts) {
			switch (modelPart->itemType()) {
				case ModelPart::Board:
				case ModelPart::ResizableBoard:
					{
						done = true;		// not allowed to have multiple boards, so we're almost done

						QString slayers = modelPart->properties().value("layers", "");
						if (slayers.isEmpty()) {
							// shouldn't happen
							break;
						}
						bool ok;
						int layers = slayers.toInt(&ok);
						if (!ok) {
							// shouldn't happen
							break;
						}
						if (layers != m_boardLayers) {
							changeBoardLayers(layers, true);
							break;
						}		
					}
					break;
				default: 
					break;
			}

			if (done) break;
		}
	}

	SketchWidget::loadFromModelParts(modelParts, crossViewType, parentCommand, doRatsnest, offsetPaste);
}
