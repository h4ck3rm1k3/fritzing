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
#include "../items/jumperitem.h"
#include "../utils/autoclosemessagebox.h"
#include "../utils/graphicsutils.h"
#include "../utils/graphutils.h"

#include <limits>
#include <QApplication>

static const int MAX_INT = std::numeric_limits<int>::max();

static QString PCBTraceColor1 = "trace1";
static QString PCBTraceColor = "trace";

static bool AlreadyDidJumperHack = false;

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
	m_resizingBoard = NULL;
	m_resizingJumperItem = NULL;
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

bool PCBSketchWidget::canDeleteItem(QGraphicsItem * item, int count)
{
	VirtualWire * wire = dynamic_cast<VirtualWire *>(item);
	if (wire != NULL && count > 1) return false;

	return SketchWidget::canDeleteItem(item, count);
}

bool PCBSketchWidget::canCopyItem(QGraphicsItem * item, int count)
{
	VirtualWire * wire = dynamic_cast<VirtualWire *>(item);
	if (wire != NULL) {
		if (wire->getRatsnest()) return false;
	}

	return SketchWidget::canDeleteItem(item, count);
}

bool PCBSketchWidget::canChainWire(Wire * wire) {
	bool result = SketchWidget::canChainWire(wire);
	if (!result) return result;

	if (wire->getRatsnest()) {
		ConnectorItem * c0 = wire->connector0()->firstConnectedToIsh();
		if (c0 == NULL) return false;

		ConnectorItem * c1 = wire->connector1()->firstConnectedToIsh();
		if (c1 == NULL) return false;

		return !c0->wiredTo(c1, ViewGeometry::NotTraceFlags); 
	}

	return result;
}

void PCBSketchWidget::createJumper(Wire * wire) {
	QString commandString = tr("Create Jumper from this Wire");
	createJumperOrTrace(wire, commandString, ViewGeometry::JumperFlag);
	ensureJumperLayerVisible();
}

void PCBSketchWidget::createTrace(Wire * wire) {
	QString commandString = tr("Create Trace from this Wire");
	createJumperOrTrace(wire, commandString, ViewGeometry::TraceFlag);
	ensureTraceLayerVisible();
}

void PCBSketchWidget::createJumperOrTrace(Wire * fromWire, const QString & commandString, ViewGeometry::WireFlag flag)
{
	QList<Wire *> done;
	QUndoCommand * parentCommand = NULL;
	if (fromWire == NULL) {
		foreach (QGraphicsItem * item, scene()->selectedItems()) {
			Wire * wire = dynamic_cast<Wire *>(item);
			if (wire == NULL) continue;
			if (done.contains(wire)) continue;

			createOneJumperOrTrace(wire, flag, false, done, parentCommand, commandString);
		}
	}
	else {
		createOneJumperOrTrace(fromWire, flag, false, done, parentCommand, commandString);
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

void PCBSketchWidget::updateRoutingStatus(CleanUpWiresCommand* command, QUndoCommand * undoCommand, RoutingStatus & routingStatus, bool manual)
{
	//DebugDialog::debug("update ratsnest status");

	if (!manual && m_manualRoutingStatusUpdate) return;

	routingStatus.zero();
	updateRoutingStatus(routingStatus);

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
			case ViewLayer::Top:
			case ViewLayer::WireOnTop_TwoLayers:
			case ViewLayer::GroundPlane_Top:
				return ViewLayer::Copper1Trace;
			default:
				return ViewLayer::Copper0Trace;
		}
	}

	switch (viewLayerSpec) {
		case ViewLayer::Top:
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

const QString & PCBSketchWidget::traceColor(ViewLayer::ViewLayerSpec viewLayerSpec) {
	if (viewLayerSpec == ViewLayer::Top) {
		return PCBTraceColor1;
	}

	return PCBTraceColor;
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
	Q_UNUSED(deletedItems);
	Q_UNUSED(parentCommand);

	// keeps virtual wire connections off the undo list
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

void PCBSketchWidget::getRatsnestColor(QColor & color) 
{
	//RatsnestColors::reset(m_viewIdentifier);
	color = RatsnestColors::netColor(m_viewIdentifier);
}


VirtualWire * PCBSketchWidget::makeOneRatsnestWire(ConnectorItem * source, ConnectorItem * dest, bool routed, QColor color) {
	if (source->attachedTo() == dest->attachedTo()) {
		if (source == dest) return NULL;

		if (source->bus() == dest->bus() && dest->bus() != NULL) {
			return NULL;				// don't draw a wire within the same part on the same bus
		}
	}
	
	long newID = ItemBase::getNextID();

	ViewGeometry viewGeometry;
	makeRatsnestViewGeometry(viewGeometry, source, dest);
	viewGeometry.setRouted(routed);

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
	VirtualWire * wire = dynamic_cast<VirtualWire *>(newItemBase);
	tempConnectWire(wire, source, dest);

	if (!source->attachedTo()->isVisible() || !dest->attachedTo()->isVisible()) {
		wire->setVisible(false);
	}

	qreal opacity = getRatsnestOpacity(routed);
	wire->setColor(color, opacity);

	return wire;
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
	new CheckStickyCommand(this, cvt, newID, false, CheckStickyCommand::RemoveOnly, parentCommand);

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
	}
	return newID;
}

void PCBSketchWidget::modifyNewWireConnectionsAux(ConnectorItem * fromConnectorItem,
												  ConnectorItem * toConnectorItem, 
												  QUndoCommand * parentCommand)
{
	// fromConnectorItem is connected to a wire
	Wire * wire = qobject_cast<Wire *>(fromConnectorItem->attachedTo());
	QList<Wire *> wires;
	QList<ConnectorItem *> ends;
	QList<ConnectorItem *> uniqueEnds;
	wire->collectChained(wires, ends, uniqueEnds);
	ConnectorItem::collectEqualPotential(ends, true, ViewGeometry::TraceJumperRatsnestFlags);
	if (ends.contains(toConnectorItem)) {
		// don't need a new wire
		return;
	}

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
                case ViewLayer::Top:
                case ViewLayer::Bottom:
                case ViewLayer::TopAndBottom:
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
	rememberSticky(item->id(), parentCommand);
	new ResizeBoardCommand(this, item->id(), origw, origh, mmW, mmH, parentCommand);
	new CheckStickyCommand(this, BaseCommand::SingleView, item->id(), true, CheckStickyCommand::RedoOnly, parentCommand);
	m_undoStack->push(parentCommand);
}

void PCBSketchWidget::showLabelFirstTime(long itemID, bool show, bool doEmit) {
	SketchWidget::showLabelFirstTime(itemID, show, doEmit);
	ItemBase * itemBase = findItem(itemID);
	if (itemBase == NULL) return;

	switch (itemBase->itemType()) {
		case ModelPart::Part:
		case ModelPart::Hole:
		case ModelPart::Via:
			itemBase->showPartLabel(itemBase->isVisible(), m_viewLayers.value(getLabelViewLayerID(itemBase->viewLayerSpec())));
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

void PCBSketchWidget::updateRoutingStatus(RoutingStatus & routingStatus) 
{
	DebugDialog::debug(QString("update routing status %1").arg(m_viewIdentifier) );
	// TODO: think about ways to optimize this...

	QList< QList<ConnectorItem *> > ratnestsToUpdate;
	QList<ConnectorItem *> visited;
	foreach (QGraphicsItem * item, scene()->items()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
		if (connectorItem == NULL) continue;
		if (visited.contains(connectorItem)) continue;

		QList<ConnectorItem *> connectorItems;
		connectorItems.append(connectorItem);
		ConnectorItem::collectEqualPotential(connectorItems, true, ViewGeometry::RatsnestFlag);
		visited.append(connectorItems);

		bool doRatsnest = checkUpdateRatsnest(connectorItems);
		if (!doRatsnest && connectorItems.count() <= 1) continue;

		QList<ConnectorItem *> partConnectorItems;
		ConnectorItem::collectParts(connectorItems, partConnectorItems, includeSymbols(), ViewLayer::TopAndBottom);
		if (partConnectorItems.count() < 1) continue;
		if (!doRatsnest && partConnectorItems.count() <= 1) continue;

		for (int i = partConnectorItems.count() - 1; i >= 0; i--) {
			ConnectorItem * ci = partConnectorItems[i];
			DebugDialog::debug(QString("pc '%1' id:%2 cid:%3 vid:%4 vlid:%5 vis:%6")
				.arg(ci->attachedToTitle())
				.arg(ci->attachedToID())
				.arg(ci->connectorSharedID())
				.arg(m_viewIdentifier)
				.arg(ci->attachedToViewLayerID())
				.arg(ci->attachedTo()->isEverVisible())
				);
			if (!ci->attachedTo()->isEverVisible()) {
				// may not be necessary when views are brought completely into sync
				partConnectorItems.removeAt(i);
			}
		}

		if (partConnectorItems.count() < 1) continue;

		if (doRatsnest) {
			ratnestsToUpdate.append(partConnectorItems);
		}

		if (partConnectorItems.count() <= 1) continue;

		GraphUtils::scoreOneNet(partConnectorItems, routingStatus);
	}

	routingStatus.m_jumperWireCount /= 2;			// since we counted each connector twice
	routingStatus.m_jumperItemCount /= 4;			// since we counted each connector twice on two layers (4 connectors per jumper item)

	// can't do this in the above loop since VirtualWires and ConnectorItems are added and deleted
	foreach (QList<ConnectorItem *> partConnectorItems, ratnestsToUpdate) {
		partConnectorItems.at(0)->displayRatsnest(partConnectorItems);
	}
}

bool PCBSketchWidget::checkUpdateRatsnest(QList<ConnectorItem *> & connectorItems) {
	bool doRatsnest = false;
	for (int i = m_ratsnestUpdateConnect.count() - 1; i >= 0; i--) {
		ConnectorItem * ci = m_ratsnestUpdateConnect[i];
		bool remove = false;
		if (ci == NULL) {
			remove = true;
			DebugDialog::debug(QString("rem rat null %1 con:true").arg(m_viewIdentifier));
		}
		else if (connectorItems.contains(ci)) {
			remove = true;
			DebugDialog::debug(QString("rem rat '%1' id:%2 cid:%3 vid:%4 vlid:%5 con:true")
				.arg(ci->attachedToTitle())
				.arg(ci->attachedToID())
				.arg(ci->connectorSharedID())
				.arg(m_viewIdentifier)
				.arg(ci->attachedToViewLayerID())
				);
			doRatsnest = true;
		}
		if (remove) m_ratsnestUpdateConnect.removeAt(i);
	}
	for (int i = m_ratsnestUpdateDisconnect.count() - 1; i >= 0; i--) {
		ConnectorItem * ci = m_ratsnestUpdateDisconnect[i];
		bool remove = false;
		if (ci == NULL) {
			remove = true;
			DebugDialog::debug(QString("rem rat null %1 con:false").arg(m_viewIdentifier));
		}
		else if (connectorItems.contains(ci)) {
			remove = true;
			DebugDialog::debug(QString("rem rat '%1' id:%2 cid:%3 vid:%4 vlid:%5 false")
				.arg(ci->attachedToTitle())
				.arg(ci->attachedToID())
				.arg(ci->connectorSharedID())
				.arg(m_viewIdentifier)
				.arg(ci->attachedToViewLayerID())
				);
			doRatsnest = true;
		}
		if (remove) m_ratsnestUpdateDisconnect.removeAt(i);
	}

	return doRatsnest;
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
	saveLayerVisibility();
	setAllLayersVisible(true);
	bool copper0Active = layerIsActive(ViewLayer::Copper0);
	bool copper1Active = layerIsActive(ViewLayer::Copper1);
	setLayerActive(ViewLayer::Copper0, true);
	setLayerActive(ViewLayer::Copper1, true);
	QSet<ItemBase *> collidingItems;
	QList<ItemBase *> itemBases;
	foreach (QGraphicsItem * item, scene()->items()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;

		if (!ViewLayer::copperLayers(ViewLayer::Bottom).contains(itemBase->viewLayerID())) {
			if (itemBase->isVisible()) {
				itemBase->setVisible(false);
				itemBases.append(itemBase);
			}
		}
	}
	if (m_boardLayers == 1) {
		drcLayer(collidingItems, 0, 1.0, 1000);
	}
	else {
		drcLayer(collidingItems, 0, 0.5, 1000);
		foreach (ItemBase * itemBase, itemBases) {
			itemBase->setVisible(true);
		}
		itemBases.clear();
		foreach (QGraphicsItem * item, scene()->items()) {
			ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
			if (itemBase == NULL) continue;

			if (!ViewLayer::copperLayers(ViewLayer::Top).contains(itemBase->viewLayerID())) {
				if (itemBase->isVisible()) {
					itemBase->setVisible(false);
					itemBases.append(itemBase);
				}
			}
		}
		drcLayer(collidingItems, 500, 0.5, 1000);
	}
	foreach (ItemBase * itemBase, itemBases) {
		itemBase->setVisible(true);
	}

	this->setBackground(color);
	restoreLayerVisibility();
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

bool PCBSketchWidget::drcLayer(QSet<ItemBase *> & collidingItems, int progressOffset, qreal progressRange, int progressGoal) {
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


	bool retval = false;
	int imageCount = 0;
	qreal expandBy = .01 * FSvgRenderer::printerScale();
	foreach (QGraphicsItem * checkItem, checkItems) {
		if (m_cancelDRC) break;

		if (drcLayerItem(checkItem, collidingItems, progressOffset, progressRange, progressGoal, 
			imageCount, expandBy, progressSoFar, maxProgress)) 
		{
			retval = true;
		}
	}

	return retval;
}

bool PCBSketchWidget::drcLayerItem(QGraphicsItem * checkItem, QSet<ItemBase *> & collidingItems, int progressOffset, qreal progressRange, int progressGoal,
								   int & imageCount, qreal expandBy, qreal & progressSoFar, int maxProgress) 
{
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
			return false;
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

	if (m_cancelDRC) return false;

	if (intersectingItems.count() == 0) {
		emit setDRCProgressValue(progressOffset + (progressGoal * progressRange * ++progressSoFar / maxProgress));
		return false;
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

	QRectF expandedPolyBounds = expandedCheckItemPoly.boundingRect();
	QHash<QGraphicsItem *, bool> visibility;

	// hide checkItem and any non-intersecting items so they don't show up in the "other" image
	// also hide the connectors of the intersecting items so they don't show up in the other image (just the svg of their parent)
	if (checkItemParent) {
		checkItemParent->setVisible(false);
	}
	else {
		checkItem->setVisible(false);
	}

	QApplication::processEvents();

	foreach (QGraphicsItem * item, scene()->items(expandedPolyBounds)) {
		if (!item->isVisible()) continue;
		
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase) {
			if (itemBase->hidden()) continue;
			if (intersectingItems.contains(itemBase)) continue;
		}

		setDRCVisibility(item, equipotentialConnectorItems, visibility);
	}

	QApplication::processEvents();
	if (m_cancelDRC) return false;

	QSize sz(qCeil(expandedPolyBounds.width()), qCeil(expandedPolyBounds.height()));
	QImage otherImage(sz, QImage::Format_ARGB32);
	otherImage.fill(0);
	QPainter painter;
	painter.begin(&otherImage);
	scene()->render(&painter, otherImage.rect(), expandedPolyBounds);
	painter.end();

#ifndef QT_NO_DEBUG
	QString suffix = QString::number(imageCount) + "_" + QString::number(progressOffset) + ".png";
	imageCount++;
	otherImage.save("pretestDesignRulePolyOther" + suffix, "png");
#else
	Q_UNUSED(imageCount);
#endif

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

	QApplication::processEvents();
	if (m_cancelDRC) return false;

	QPolygonF poly = checkItem->mapToScene(checkItem->boundingRect());						
	QImage selfImage(sz, QImage::Format_ARGB32);
	selfImage.fill(0);
	painter.begin(&selfImage);
	scene()->render(&painter, selfImage.rect(), expandedPolyBounds);
	painter.end();

#ifndef QT_NO_DEBUG
	selfImage.save("pretestDesignRuleSelfScaled" + suffix, "png");
#endif

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
	//scaledImage.save("posttestDesignRuleSelfScaled" + suffix, "png");
	//otherImage.save("posttestDesignRulePolyOther" + suffix, "png");
#endif
	
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

	bool colliding = false;

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

				collidingItems.insert(itemBase);
				itemBase = dynamic_cast<ItemBase *>(checkItemParent ? checkItemParent : checkItem);
				collidingItems.insert(itemBase);
				colliding = true;
				break;
			}
		}
	}

#ifndef QT_NO_DEBUG
	if (!colliding) {
		//QFile::remove("posttestDesignRuleSelfScaled" + suffix);
		//QFile::remove("posttestDesignRulePolyOther" + suffix);
		QFile::remove("pretestDesignRuleSelfScaled" + suffix);
		QFile::remove("pretestDesignRulePolyOther" + suffix);
	}
#endif

	emit setDRCProgressValue(progressOffset + (progressGoal * progressRange * ++progressSoFar / maxProgress));
	QApplication::processEvents();

	return colliding;
}
	

void PCBSketchWidget::setDRCVisibility(QGraphicsItem * item, QList<ConnectorItem *> & equipotentialConnectorItems, QHash<QGraphicsItem *, bool> & visibility)
{
	ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
	if (connectorItem && equipotentialConnectorItems.contains(connectorItem)) {
		// equipotential connector items have already been set white so don't hide them
		return;
	}

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
	long newID = SketchWidget::setUpSwap(itemBase, newModelIndex, newModuleID, viewLayerSpec, master, parentCommand);

	if (itemBase->viewIdentifier() != m_viewIdentifier) {
		itemBase = findItem(itemBase->id());
		if (itemBase == NULL) return newID;
	}

	int newLayers = isBoardLayerChange(itemBase, newModuleID, master);
	if (newLayers == m_boardLayers) return newID;

	QList<ItemBase *> smds;
	QList<Wire *> already;
	ChangeBoardLayersCommand * changeBoardCommand = new ChangeBoardLayersCommand(this, m_boardLayers, newLayers, parentCommand);

	if (itemBase->itemType() == ModelPart::ResizableBoard) {
		// preserve the size if swapping rectangular board
		ResizableBoard * rb = qobject_cast<ResizableBoard *>(itemBase);
		QPointF p;
		QSizeF sz;
		rb->getParams(p, sz);
		new ResizeBoardCommand(this, newID, sz.width(), sz.height(), sz.width(), sz.height(), parentCommand);
		new CheckStickyCommand(this, BaseCommand::SingleView, newID, false, CheckStickyCommand::RemoveOnly, parentCommand);
	}
 

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

	return newID;
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

void PCBSketchWidget::loadFromModelParts(QList<ModelPart *> & modelParts, BaseCommand::CrossViewType crossViewType, QUndoCommand * parentCommand, bool offsetPaste, const QRectF * boundingRect) {
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

	SketchWidget::loadFromModelParts(modelParts, crossViewType, parentCommand, offsetPaste, boundingRect);
}

bool PCBSketchWidget::isInLayers(ConnectorItem * connectorItem, ViewLayer::ViewLayerSpec viewLayerSpec) {
	return connectorItem->isInLayers(viewLayerSpec);
}

bool PCBSketchWidget::routeBothSides() {
	return m_boardLayers > 1;
}

bool PCBSketchWidget::sameElectricalLayer(ViewLayer::ViewLayerID id1, ViewLayer::ViewLayerID id2) {
	// assumes both ids are in a copper layer or one id is a wildcard (UnknownLayer)

	if (id1 == id2) return true;
	if (id1 == ViewLayer::UnknownLayer) return true;
	if (id2 == ViewLayer::UnknownLayer) return true;

	LayerList copperBottom = ViewLayer::copperLayers(ViewLayer::Bottom);

	bool c1 = copperBottom.contains(id1);
	bool c2 = copperBottom.contains(id2);

	return c1 == c2;
}

void PCBSketchWidget::changeTraceLayer() {
	QList<Wire *> visitedWires;
	QList<Wire *> changeWires;
	foreach (QGraphicsItem * item, scene()->selectedItems()) {
		TraceWire * tw = dynamic_cast<TraceWire *>(item);
		if (tw == NULL) continue;
		if (visitedWires.contains(tw)) continue;

		QList<Wire *> wires;
		QList<ConnectorItem *> uniqueEnds;
		QList<ConnectorItem *> ends;
		tw->collectChained(wires, ends, uniqueEnds);
		visitedWires.append(wires);
		if (ends.count() < 2) continue;   // should never happen, since traces have to be connected at both ends

		bool canChange = true;
		foreach(ConnectorItem * end, ends) {
			if (end->getCrossLayerConnectorItem() == NULL) {
				canChange = false;
				break;
			}
		}
		if (!canChange) continue;

		changeWires.append(tw);
	}

	if (changeWires.count() == 0) return;

	QUndoCommand * parentCommand = new QUndoCommand(tr("Change trace layer"));

	foreach (Wire * wire, changeWires) {
		QList<Wire *> wires;
		QList<ConnectorItem *> uniqueEnds;
		QList<ConnectorItem *> ends;
		wire->collectChained(wires, ends, uniqueEnds);

		// probably safest to disconnect change the layers and reconnect, so that's why the redundant looping

		foreach (ConnectorItem * end, ends) {
			ConnectorItem * targetConnectorItem = NULL;
			foreach (ConnectorItem * toConnectorItem, end->connectedToItems()) {
				Wire * w = qobject_cast<Wire *>(toConnectorItem->attachedTo());
				if (w == NULL) continue;

				if (wires.contains(w)) {
					targetConnectorItem = toConnectorItem;
					break;
				}
			}

			new ChangeConnectionCommand(this, BaseCommand::SingleView,
								targetConnectorItem->attachedToID(), targetConnectorItem->connectorSharedID(),
								end->attachedToID(), end->connectorSharedID(),
								ViewLayer::specFromID(end->attachedToViewLayerID()), 
								false, parentCommand);
		}


		foreach (Wire * w, wires) {
			ViewLayer::ViewLayerID newViewLayerID = w->viewLayerID() == ViewLayer::Copper0Trace ? ViewLayer::Copper1Trace : ViewLayer::Copper0Trace;
			new ChangeLayerCommand(this, w->id(), w->zValue(), m_viewLayers.value(newViewLayerID)->nextZ(), w->viewLayerID(), newViewLayerID, parentCommand);
		}

		foreach (ConnectorItem * end, ends) {
			ConnectorItem * targetConnectorItem = NULL;
			foreach (ConnectorItem * toConnectorItem, end->connectedToItems()) {
				Wire * w = qobject_cast<Wire *>(toConnectorItem->attachedTo());
				if (w == NULL) continue;

				if (wires.contains(w)) {
					targetConnectorItem = toConnectorItem;
					break;
				}
			}

			new ChangeConnectionCommand(this, BaseCommand::SingleView,
								targetConnectorItem->attachedToID(), targetConnectorItem->connectorSharedID(),
								end->attachedToID(), end->connectorSharedID(),
								ViewLayer::specFromID(end->getCrossLayerConnectorItem()->attachedToViewLayerID()), 
								true, parentCommand);
		}
	}

	m_undoStack->push(parentCommand);
}

void PCBSketchWidget::changeLayer(long id, qreal z, ViewLayer::ViewLayerID viewLayerID) {
	ItemBase * itemBase = findItem(id);
	if (itemBase == NULL) return;

	itemBase->setViewLayerID(viewLayerID, m_viewLayers);
	itemBase->setZValue(z);
	itemBase->saveGeometry();

	TraceWire * tw = qobject_cast<TraceWire *>(itemBase);
	if (tw != NULL) {
		tw->setColorString(traceColor(ViewLayer::specFromID(viewLayerID)), 1.0);
		ViewLayer * viewLayer = m_viewLayers.value(viewLayerID);
		tw->setActive(viewLayer->isActive());
		tw->setHidden(!viewLayer->visible());
	}
}

bool PCBSketchWidget::resizingJumperItemPress(QGraphicsItem * item) {
	JumperItem * jumperItem = dynamic_cast<JumperItem *>(item);
	if (jumperItem == NULL) return false;

	if (jumperItem->inDrag()) {
		m_resizingJumperItem = jumperItem;
		m_resizingJumperItem->saveParams();
		if (m_alignToGrid) {
			m_alignmentStartPoint = QPointF(0,0);
			ItemBase * board = findBoard();
			QSet<ItemBase *> savedItems;
			QHash<Wire *, ConnectorItem *> savedWires;
			if (board == NULL) {
				foreach (QGraphicsItem * item, scene()->items()) {
					PaletteItemBase * itemBase = dynamic_cast<PaletteItemBase *>(item);
					if (itemBase->itemType() == ModelPart::Jumper) continue;

					savedItems.insert(itemBase);
				}
			}
			findAlignmentAnchor(board, savedItems, savedWires);
			m_jumperDragOffset = jumperItem->dragOffset();
			connect(m_resizingJumperItem, SIGNAL(alignMe(JumperItem *, QPointF &)), this, SLOT(alignJumperItem(JumperItem *, QPointF &)), Qt::DirectConnection);
		}
		return true;
	}

	return false;
}

void PCBSketchWidget::alignJumperItem(JumperItem * jumperItem, QPointF & loc) {
	Q_UNUSED(jumperItem);
	if (!m_alignToGrid) return;

	QPointF newPos = loc - m_jumperDragOffset - m_alignmentStartPoint;
	qreal ny = GraphicsUtils::getNearestOrdinate(newPos.y(), gridSizeInches() * FSvgRenderer::printerScale());
	qreal nx = GraphicsUtils::getNearestOrdinate(newPos.x(), gridSizeInches() * FSvgRenderer::printerScale());
	loc.setX(loc.x() + nx - newPos.x());
	loc.setY(loc.y() + ny - newPos.y());
}

bool PCBSketchWidget::resizingJumperItemRelease() {
	if (m_resizingJumperItem == NULL) return false;

	if (m_alignToGrid) {
		disconnect(m_resizingJumperItem, SIGNAL(alignMe(JumperItem *, QPointF &)), this, SLOT(alignJumperItem(JumperItem *, QPointF &)));
	}
	resizeJumperItem();
	return true;
}

void PCBSketchWidget::resizeJumperItem() {
	QPointF oldC0, oldC1;
	QPointF oldPos;
	m_resizingJumperItem->getParams(oldPos, oldC0, oldC1);
	QPointF newC0, newC1;
	QPointF newPos;
	m_resizingJumperItem->saveParams();
	m_resizingJumperItem->getParams(newPos, newC0, newC1);
	QUndoCommand * cmd = new ResizeJumperItemCommand(this, m_resizingJumperItem->id(), oldPos, oldC0, oldC1, newPos, newC0, newC1, NULL);
	cmd->setText("Resize Jumper");
	m_undoStack->waitPush(cmd, 10);
	m_resizingJumperItem = NULL;
}

bool PCBSketchWidget::resizingBoardPress(QGraphicsItem * item) {
	// board's child items (at the moment) are the resize grips
	m_resizingBoard = dynamic_cast<ResizableBoard *>(item->parentItem());
	if (m_resizingBoard == NULL) return false;

	m_resizingBoard->saveParams();
	return true;
}

bool PCBSketchWidget::resizingBoardRelease() {

	if (m_resizingBoard == NULL) return false;

	resizeBoard();
	return true;
}

void PCBSketchWidget::resizeBoard() {
	QSizeF oldSize;
	QPointF oldPos;
	m_resizingBoard->getParams(oldPos, oldSize);
	QSizeF newSize;
	QPointF newPos;
	m_resizingBoard->saveParams();
	m_resizingBoard->getParams(newPos, newSize);
	QUndoCommand * parentCommand = new QUndoCommand(tr("Resize board to %1 %2").arg(newSize.width()).arg(newSize.height()));
	rememberSticky(m_resizingBoard->id(), parentCommand);
	new ResizeBoardCommand(this, m_resizingBoard->id(), oldSize.width(), oldSize.height(), newSize.width(), newSize.height(), parentCommand);
	if (oldPos != newPos) {
		m_resizingBoard->saveGeometry();
		ViewGeometry vg1 = m_resizingBoard->getViewGeometry();
		ViewGeometry vg2 = vg1;
		vg1.setLoc(oldPos);
		vg2.setLoc(newPos);
		new MoveItemCommand(this, m_resizingBoard->id(), vg1, vg2, parentCommand);
	}
	new CheckStickyCommand(this, BaseCommand::SingleView, m_resizingBoard->id(), true, CheckStickyCommand::RedoOnly, parentCommand);
	m_undoStack->waitPush(parentCommand, 10);
	m_resizingBoard = NULL;
}

void PCBSketchWidget::deleteSelected(Wire * wire) {
	QSet<ItemBase *> itemBases;
	if (wire) {
		itemBases << wire;
	}
	else {
		foreach (QGraphicsItem * item, scene()->selectedItems()) {
			ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
			if (itemBase == NULL) continue;

			itemBase = itemBase->layerKinChief();
			itemBases.insert(itemBase);
		}
	}

	// assumes ratsnest is not mixed with other itembases
	bool rats = true;
	foreach (ItemBase * itemBase, itemBases) {
		Wire * wire = qobject_cast<Wire *>(itemBase);
		if (wire == NULL) {
			rats = false;
			break;
		}
		if (!wire->getRatsnest()) {
			rats = false;
			break;
		}
	}

	if (!rats) {
		SketchWidget::deleteSelected(NULL);			// wire is selected in this case, so don't bother sending it along
		return;
	}

	// probably deal with connections to ground symbol or ground plane here?

	emit disconnectWireSignal(itemBases);
}

bool PCBSketchWidget::canDragWire(Wire * wire) {
	if (wire == NULL) return false;

	if (wire->getRatsnest()) return false;

	return true;
}

void PCBSketchWidget::dragWireChanged(Wire* wire, ConnectorItem * fromOnWire, ConnectorItem * to)
{
	if (m_bendpointWire == NULL || !wire->getRatsnest()) {
		SketchWidget::dragWireChanged(wire, fromOnWire, to);
		return;
	}

	// m_bendpointWire is the original wire
	// m_connectorDragWire is temporary
	// wire == m_connectorDragWire
	// m_connectorDragConnector is from original wire

	BaseCommand::CrossViewType crossViewType = BaseCommand::SingleView;

	QUndoCommand * parentCommand = new QUndoCommand();
	parentCommand->setText(tr("Create and connect trace"));

	//SelectItemCommand * selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);

	m_connectorDragWire->saveGeometry();
	m_bendpointWire->saveGeometry();

	ViewLayer::ViewLayerSpec viewLayerSpec = layerIsActive(ViewLayer::Copper0) ? ViewLayer::Bottom : ViewLayer::Top;

	long newID1 = ItemBase::getNextID();
	ViewGeometry vg1 = m_connectorDragWire->getViewGeometry();
	vg1.setRatsnest(false);
	vg1.setVirtual(false);
	vg1.setTrace(true);
	new AddItemCommand(this, crossViewType, m_connectorDragWire->modelPart()->moduleID(), viewLayerSpec, vg1, newID1, true, -1, parentCommand);
	new CheckStickyCommand(this, crossViewType, newID1, false, CheckStickyCommand::RemoveOnly, parentCommand);
	new WireColorChangeCommand(this, newID1, traceColor(viewLayerSpec), traceColor(viewLayerSpec), 1.0, 1.0, parentCommand);
	new WireWidthChangeCommand(this, newID1, Wire::STANDARD_TRACE_WIDTH, Wire::STANDARD_TRACE_WIDTH, parentCommand);

	long newID2 = ItemBase::getNextID();
	ViewGeometry vg2 = m_bendpointWire->getViewGeometry();
	vg2.setRatsnest(false);
	vg2.setVirtual(false);
	vg2.setTrace(true);
	new AddItemCommand(this, crossViewType, m_bendpointWire->modelPart()->moduleID(), viewLayerSpec, vg2, newID2, true, -1, parentCommand);
	new CheckStickyCommand(this, crossViewType, newID2, false, CheckStickyCommand::RemoveOnly, parentCommand);
	new WireColorChangeCommand(this, newID2, traceColor(viewLayerSpec), traceColor(viewLayerSpec), 1.0, 1.0, parentCommand);
	new WireWidthChangeCommand(this, newID2, Wire::STANDARD_TRACE_WIDTH, Wire::STANDARD_TRACE_WIDTH, parentCommand);

	new ChangeConnectionCommand(this, BaseCommand::SingleView,
									newID2, m_connectorDragConnector->connectorSharedID(),
									newID1, m_connectorDragWire->connector0()->connectorSharedID(),
									ViewLayer::specFromID(wire->viewLayerID()),
									true, parentCommand);

	foreach (ConnectorItem * toConnectorItem, m_bendpointWire->connector0()->connectedToItems()) {
		new ChangeConnectionCommand(this, BaseCommand::SingleView,
									newID2, m_bendpointWire->connector0()->connectorSharedID(),
									toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
									ViewLayer::specFromID(toConnectorItem->attachedToViewLayerID()),
									true, parentCommand);
	}
	foreach (ConnectorItem * toConnectorItem, m_connectorDragWire->connector1()->connectedToItems()) {
		new ChangeConnectionCommand(this, BaseCommand::SingleView,
									newID1, m_connectorDragWire->connector1()->connectorSharedID(),
									toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
									ViewLayer::specFromID(toConnectorItem->attachedToViewLayerID()),
									true, parentCommand);
		m_connectorDragWire->connector1()->tempRemove(toConnectorItem, false);
		toConnectorItem->tempRemove(m_connectorDragWire->connector1(), false);
		m_bendpointWire->connector1()->tempConnectTo(toConnectorItem, false);
		toConnectorItem->tempConnectTo(m_bendpointWire->connector1(), false);
	}



	m_bendpointWire->setPos(m_bendpointVG.loc());
	m_bendpointWire->setLine(m_bendpointVG.line());
	m_connectorDragConnector->tempRemove(m_connectorDragWire->connector0(), false);
	m_connectorDragWire->connector0()->tempRemove(m_connectorDragConnector, false);
	m_bendpointWire = NULL;			// signal that we're done


	// remove the temporary wire
	this->scene()->removeItem(m_connectorDragWire);

	new CleanUpWiresCommand(this, false, parentCommand);
	m_undoStack->push(parentCommand);
}

void PCBSketchWidget::wire_wireSplit(Wire* wire, QPointF newPos, QPointF oldPos, QLineF oldLine) {
	if (!wire->getRatsnest()) {
		SketchWidget::wire_wireSplit(wire, newPos, oldPos, oldLine);
	}

	ConnectorItem * c0 = wire->connector0()->firstConnectedToIsh();
	if (c0 == NULL) return;

	ConnectorItem * c1 = wire->connector1()->firstConnectedToIsh();
	if (c1 == NULL) return;

	if (c1->wiredTo(c0, ViewGeometry::NotTraceFlags)) {
		return;
	}

	QUndoCommand * parentCommand = new QUndoCommand();
	parentCommand->setText(tr("Create trace"));

	wire->saveGeometry();

	ViewLayer::ViewLayerSpec viewLayerSpec = layerIsActive(ViewLayer::Copper0) ? ViewLayer::Bottom : ViewLayer::Top;
	BaseCommand::CrossViewType crossViewType = BaseCommand::SingleView;

	long newID1 = ItemBase::getNextID();
	ViewGeometry vg1 = wire->getViewGeometry();
	vg1.setRatsnest(false);
	vg1.setVirtual(false);
	vg1.setTrace(true);
	new AddItemCommand(this, crossViewType, wire->modelPart()->moduleID(), viewLayerSpec, vg1, newID1, true, -1, parentCommand);
	new CheckStickyCommand(this, crossViewType, newID1, false, CheckStickyCommand::RemoveOnly, parentCommand);
	new WireColorChangeCommand(this, newID1, traceColor(viewLayerSpec), traceColor(viewLayerSpec), 1.0, 1.0, parentCommand);
	new WireWidthChangeCommand(this, newID1, Wire::STANDARD_TRACE_WIDTH, Wire::STANDARD_TRACE_WIDTH, parentCommand);

	new ChangeConnectionCommand(this, crossViewType,
								newID1, wire->connector0()->connectorSharedID(),
								c0->attachedToID(), c0->connectorSharedID(),
								ViewLayer::specFromID(c0->attachedToViewLayerID()),
								true, parentCommand);

	new ChangeConnectionCommand(this, crossViewType,
								newID1, wire->connector1()->connectorSharedID(),
								c1->attachedToID(), c1->connectorSharedID(),
								ViewLayer::specFromID(c1->attachedToViewLayerID()),
								true, parentCommand);

	//new CleanUpWiresCommand(this, false, parentCommand);
	m_undoStack->push(parentCommand);
}


void PCBSketchWidget::jumperItemHack() {
	if (AlreadyDidJumperHack) return;

	// under linux32, Qt 4.6.1, 4.6.2, 4.6.3, certain files (StepperMotor.fz) cause a crash somehow related to JumperItems.
	// this hack prevents the crash, though I still haven't been able to figure out the root cause.

	// later note: may be jumper wires at fault, not jumper items, though the root cause is still unknown

	AlreadyDidJumperHack = true;
	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(QPointF(0, 0));
	ItemBase * itemBase = addItem(paletteModel()->retrieveModelPart(ModuleIDNames::jumperModuleIDName), defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
	if (itemBase) {
		deleteItem(itemBase, true, false, false);
	}
}

void PCBSketchWidget::updateNet(Wire * wire) {
	if (wire == NULL) return;

	QList<ConnectorItem *> connectorItems;
	connectorItems.append(wire->connector0());
	ConnectorItem::collectEqualPotential(connectorItems, true, ViewGeometry::NoFlag);

	QList<ConnectorItem *> partConnectorItems;
	ConnectorItem::collectParts(connectorItems, partConnectorItems, includeSymbols(), ViewLayer::TopAndBottom);
	if (partConnectorItems.count() < 1) return;

	partConnectorItems.at(0)->displayRatsnest(partConnectorItems);
}

bool PCBSketchWidget::hasAnyNets() {
	return m_routingStatus.m_netCount > 0;
}

