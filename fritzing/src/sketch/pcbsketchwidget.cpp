/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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
#include "../items/tracewire.h"
#include "../items/virtualwire.h"
#include "../items/resizableboard.h"
#include "../waitpushundostack.h"
#include "../connectors/connectoritem.h"
#include "../items/moduleidnames.h"
#include "../items/partlabel.h"
#include "../help/sketchmainhelp.h"
#include "../utils/ratsnestcolors.h"
#include "../fsvgrenderer.h"
#include "../autoroute/autorouteprogressdialog.h"
#include "../items/groundplane.h"
#include "../items/jumperitem.h"
#include "../utils/autoclosemessagebox.h"
#include "../utils/graphicsutils.h"
#include "../utils/textutils.h"
#include "../utils/graphutils.h"
#include "../processeventblocker.h"
#include "../autoroute/cmrouter/cmrouter.h"
#include "../autoroute/autoroutersettingsdialog.h"
#include "../svg/groundplanegenerator.h"

#include <limits>
#include <QApplication>
#include <QScrollBar>
#include <QDialog>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QSettings>
#include <QPushButton>
#include <QMessageBox>


static const int MAX_INT = std::numeric_limits<int>::max();

const char * PCBSketchWidget::FakeTraceProperty = "FakeTrace";

static QString PCBTraceColor1 = "trace1";
static QString PCBTraceColor = "trace";

static bool AlreadyDidJumperHack = false;
QSizeF PCBSketchWidget::m_jumperItemSize;

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
	m_cleanType = noClean;
}

void PCBSketchWidget::setWireVisible(Wire * wire)
{
	bool visible = wire->getRatsnest() || wire->getTrace();
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


void PCBSketchWidget::createTrace(Wire * wire) {
	QString commandString = tr("Create Trace from this Wire");
	createTrace(wire, commandString, getTraceFlag());
	ensureTraceLayerVisible();
}

void PCBSketchWidget::createTrace(Wire * fromWire, const QString & commandString, ViewGeometry::WireFlag flag)
{
	QList<Wire *> done;
	QUndoCommand * parentCommand = new QUndoCommand(commandString);

	new CleanUpWiresCommand(this, CleanUpWiresCommand::UndoOnly, parentCommand);

	bool gotOne = false;
	if (fromWire == NULL) {
		foreach (QGraphicsItem * item, scene()->selectedItems()) {
			Wire * wire = dynamic_cast<Wire *>(item);
			if (wire == NULL) continue;
			if (done.contains(wire)) continue;

			gotOne = createOneTrace(wire, flag, false, done, parentCommand);
		}
	}
	else {
		gotOne = createOneTrace(fromWire, flag, false, done, parentCommand);
	}

	if (!gotOne) {
		delete parentCommand;
		return;
	}

	new CleanUpWiresCommand(this, CleanUpWiresCommand::RedoOnly, parentCommand);
	m_undoStack->push(parentCommand);
}

bool PCBSketchWidget::createOneTrace(Wire * wire, ViewGeometry::WireFlag flag, bool allowAny, QList<Wire *> & done, QUndoCommand * parentCommand) 
{
	QList<ConnectorItem *> ends;
	Wire * trace = NULL;
	if (wire->getRatsnest()) {
		trace = wire->findTraced(getTraceFlag(), ends);
	}
	else if (wire->getTrace()) {
		trace = wire;
	}
	else if (!allowAny) {
		// not eligible
		return false;
	}
	else {
		trace = wire->findTraced(getTraceFlag(), ends);
	}

	if (trace && trace->hasFlag(flag)) {
		return false;
	}

	if (trace != NULL) {
		removeWire(trace, ends, done, parentCommand);
	}

	QString colorString = traceColor(createWireViewLayerSpec(ends[0], ends[1]));
	long newID = createWire(ends[0], ends[1], flag, false, BaseCommand::SingleView, parentCommand);
	new WireColorChangeCommand(this, newID, colorString, colorString, getRatsnestOpacity(false), getRatsnestOpacity(false), parentCommand);
	new WireWidthChangeCommand(this, newID, getTraceWidth(), getTraceWidth(), parentCommand);
	return true;
}

void PCBSketchWidget::excludeFromAutoroute(bool exclude)
{
	foreach (QGraphicsItem * item, scene()->selectedItems()) {
		TraceWire * wire = dynamic_cast<TraceWire *>(item);
		if (wire) {
			QList<Wire *> wires;
			QList<ConnectorItem *> ends;
			wire->collectChained(wires, ends);
			foreach (Wire * w, wires) {
				w->setAutoroutable(!exclude);
			}
			continue;
		}

		JumperItem * jumperItem = dynamic_cast<JumperItem *>(item);
		if (jumperItem) {
			jumperItem->setAutoroutable(!exclude);
			continue;
		}

		Via * via = dynamic_cast<Via *>(item);
		if (via) {
			via->setAutoroutable(!exclude);
			continue;
		}
	}
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

	QUndoCommand * parentCommand = new QUndoCommand(QObject::tr("Select all traces marked 'Don't autoroute'"));

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
	ConnectorItem::collectEqualPotential(connectorItems, true, ViewGeometry::TraceRatsnestFlags);
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
				makeModifiedWire(newFromConnectorItem, newToConnectorItem, BaseCommand::CrossView, ViewGeometry::NormalFlag, parentCommand);
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
	Wire * Trace = dragWire->findTraced(getTraceFlag(), ends);
	dragWire->connector0()->tempRemove(fromConnectorItem, false);
	dragWire->connector1()->tempRemove(toConnectorItem, false);

	if (Trace == NULL) {
		ViewGeometry::WireFlags flags =  getTraceFlag();
		long newID = makeModifiedWire(fromConnectorItem, toConnectorItem, BaseCommand::SingleView, flags, parentCommand);
		QString tc = traceColor(fromConnectorItem);
		new WireColorChangeCommand(this, newID, tc, tc, 1.0, 1.0, parentCommand);
		double traceWidth = getTraceWidth();
		if (autorouteTypePCB()) {
			double minDim = qMin(fromConnectorItem->minDimension(), toConnectorItem->minDimension());
			if (minDim < traceWidth) {
				traceWidth = getSmallerTraceWidth(minDim);  
			}
		}
		new WireWidthChangeCommand(this, newID, traceWidth, traceWidth, parentCommand);
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
		ConnectorItem::collectEqualPotential(connectorItems, false, ViewGeometry::TraceRatsnestFlags);
		foreach (ConnectorItem * c, connectorItems) {
			if (c->attachedToItemType() == ModelPart::Part) {
				target1 = c; 
				break;
			}
		}
		connectorItems.clear();
		connectorItems.append(toConnectorItem);
		ConnectorItem::collectEqualPotential(connectorItems, false, ViewGeometry::TraceRatsnestFlags);
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

	makeModifiedWire(target1, target2, BaseCommand::CrossView, ViewGeometry::NormalFlag, parentCommand);
}

void PCBSketchWidget::connectSymbolPrep(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, ConnectorItem * & target1, ConnectorItem * & target2) {
	QList<ConnectorItem *> connectorItems;
	connectorItems.append(fromConnectorItem);
	ConnectorItem::collectEqualPotential(connectorItems, false, ViewGeometry::TraceRatsnestFlags);
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

void PCBSketchWidget::addDefaultParts() {

	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(QPointF(0, 0));

	// have to put this off until later, because positioning the item doesn't work correctly until the view is visible
	m_addedDefaultPart = addItem(paletteModel()->retrieveModelPart(ModuleIDNames::TwoSidedRectangleModuleIDName), defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
	m_addDefaultParts = true;

	changeBoardLayers(2, true);
}

QPoint PCBSketchWidget::calcFixedToCenterItemOffset(const QRect & viewPortRect, const QSizeF & helpSize) {
	QPoint p((int) ((viewPortRect.width() - helpSize.width()) / 2.0),
			 30);
	return p;
}

void PCBSketchWidget::showEvent(QShowEvent * event) {
	SketchWidget::showEvent(event);
	dealWithDefaultParts();
}

void PCBSketchWidget::dealWithDefaultParts() {
	if (!m_addDefaultParts) return;
	if  (m_addedDefaultPart == NULL) return;

	m_addDefaultParts = false;

	if (m_fixedToCenterItem == NULL) return;

	// place the default rectangular board in relation to the first time help area

	QSizeF helpSize = m_fixedToCenterItem->size();
	QSizeF vpSize = this->viewport()->size();
	QSizeF partSize(600, 200);

	//if (vpSize.height() < helpSize.height() + 50 + partSize.height()) {
		//vpSize.setWidth(vpSize.width() - verticalScrollBar()->width());
	//}

	QPointF p;
	p.setX((int) ((vpSize.width() - partSize.width()) / 2.0));
	p.setY((int) helpSize.height());

	// TODO: make these constants less arbitrary (get the size and location of the icon which the board is replacing)
	p += QPointF(0, 50);

	// place it
	QPointF q = mapToScene(p.toPoint());
	m_addedDefaultPart->setPos(q);
	ResizableBoard * rb = qobject_cast<ResizableBoard *>(m_addedDefaultPart);
	if (rb) rb->resizePixels(partSize.width(), partSize.height(), m_viewLayers);
	QTimer::singleShot(10, this, SLOT(vScrollToZero()));

	setLayerActive(ViewLayer::Copper1, false);
	setLayerActive(ViewLayer::Copper0, true);
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
	if (viewGeometry.getRatsnest()) {
		return ViewLayer::Ratsnest;
	}

	if (viewGeometry.getAnyTrace()) {
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
	wire->setColorString("black", 1.0);
	wire->setPenWidth(1, this, 2);
}

bool PCBSketchWidget::autorouteTypePCB() {
	return true;
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

PCBSketchWidget::CleanType PCBSketchWidget::cleanType() {
	return m_cleanType;
}

void PCBSketchWidget::ensureTraceLayersVisible() {
	ensureLayerVisible(ViewLayer::Copper0);
	ensureLayerVisible(ViewLayer::Copper0Trace);
	ensureLayerVisible(ViewLayer::GroundPlane0);
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

bool PCBSketchWidget::canChainMultiple() {
	return false;
}

void PCBSketchWidget::setNewPartVisible(ItemBase * itemBase) {
	if (itemBase->itemType() == ModelPart::Breadboard  || 
		itemBase->itemType() == ModelPart::Symbol || 
		itemBase->moduleID().endsWith(ModuleIDNames::SchematicFrameModuleIDName)) 
	{
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
		case ModelPart::CopperFill:
			return true;
		case ModelPart::Wire:
		case ModelPart::Breadboard:
		case ModelPart::Symbol:
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
			return !modelPart->moduleID().endsWith(ModuleIDNames::SchematicFrameModuleIDName);
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

		Wire * w = qobject_cast<Wire *>(toConnectorItem->attachedTo());
		ViewGeometry::WireFlags wflag = w->wireFlags() & (ViewGeometry::RatsnestFlag | getTraceFlag());
		if (wflag != flag) continue;

		result = bothEndsConnectedAux(w, flag, toConnectorItem, wires, partConnectorItems, visited) || result;   // let it recurse
	}

	if (result) {
		wires.removeOne(wire);
	}

	return result;
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

double PCBSketchWidget::getRatsnestOpacity(Wire * wire) {
	return getRatsnestOpacity(wire->getRouted());
}

double PCBSketchWidget::getRatsnestOpacity(bool routed) {
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

	ItemBase * newItemBase = addItem(m_paletteModel->retrieveModelPart(ModuleIDNames::WireModuleIDName), source->attachedTo()->viewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);		
	VirtualWire * wire = qobject_cast<VirtualWire *>(newItemBase);
	tempConnectWire(wire, source, dest);

	if (!source->attachedTo()->isVisible() || !dest->attachedTo()->isVisible()) {
		wire->setVisible(false);
	}

	double opacity = getRatsnestOpacity(routed);
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
	viewGeometry.setWireFlags(ViewGeometry::RatsnestFlag);
}

ConnectorItem * PCBSketchWidget::lookForBreadboardConnection(ConnectorItem * connectorItem) 
{
	Wire * wire = qobject_cast<Wire *>(connectorItem->attachedTo());
	QList<ConnectorItem *> ends;
	if (wire != NULL) {
		QList<Wire *> wires;
		wire->collectChained(wires, ends);
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
	ConnectorItem::collectEqualPotential(ends, true, ViewGeometry::TraceRatsnestFlags);
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
	//DebugDialog::debug(QString("new real wire %1").arg(newID));
	//fromConnectorItem->debugInfo("\tfrom");
	//toConnectorItem->debugInfo("\tto");
	ViewGeometry viewGeometry;
	makeRatsnestViewGeometry(viewGeometry, fromConnectorItem, toConnectorItem);
	viewGeometry.setWireFlags(wireFlags);
	ViewLayer::ViewLayerSpec viewLayerSpec = wireViewLayerSpec(fromConnectorItem);
	new AddItemCommand(this, cvt, ModuleIDNames::WireModuleIDName, viewLayerSpec, viewGeometry, newID, true, -1, parentCommand);
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
	wire->collectChained(wires, ends);
	ConnectorItem::collectEqualPotential(ends, true, ViewGeometry::TraceRatsnestFlags);
	if (ends.contains(toConnectorItem)) {
		// don't need a new wire
		return;
	}

	ConnectorItem * originalFromConnectorItem = fromConnectorItem;
	fromConnectorItem = lookForBreadboardConnection(fromConnectorItem);		
	if (fromConnectorItem->attachedToItemType() == ModelPart::Breadboard) {
		makeModifiedWire(fromConnectorItem, toConnectorItem, BaseCommand::CrossView, ViewGeometry::NormalFlag, parentCommand);
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
			new AddItemCommand(this, BaseCommand::CrossView, newBreadboard->moduleID(), originalFromConnectorItem->attachedTo()->viewLayerSpec(), newBreadboard->getViewGeometry(), newBreadboard->id(), true, -1, parentCommand);
			m_temporaries.append(newBreadboard);			// puts it on a list to be deleted
		}
	}

	ConnectorItem * nearestPartConnectorItem = findNearestPartConnectorItem(originalFromConnectorItem);
	if (nearestPartConnectorItem == NULL) return;

	// make a wire, from the part nearest to fromConnectorItem, to the breadboard
	toConnectorItem = nearestPartConnectorItem;
	makeModifiedWire(fromConnectorItem, toConnectorItem, BaseCommand::CrossView, ViewGeometry::NormalFlag, parentCommand);

	if (originalToConnectorItem->attachedToItemType() == ModelPart::Wire) {
		originalToConnectorItem = findNearestPartConnectorItem(originalToConnectorItem);
		if (originalToConnectorItem == NULL) return;
	}

	// draw a wire from that bus on the breadboard to the other part (toConnectorItem)
	ConnectorItem * otherPartBusConnectorItem = findEmptyBusConnectorItem(fromConnectorItem);
	makeModifiedWire(otherPartBusConnectorItem, originalToConnectorItem, BaseCommand::CrossView, ViewGeometry::NormalFlag, parentCommand);
}

ConnectorItem * PCBSketchWidget::lookForNewBreadboardConnection(ConnectorItem * connectorItem,  ItemBase * & newBreadboard) {
	Q_UNUSED(connectorItem);
	
	newBreadboard = NULL;
	QList<ItemBase *> breadboards;
	double maxY = 0;
	foreach (QGraphicsItem * item, scene()->items()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;

		if (itemBase->itemType() == ModelPart::Breadboard) {
			breadboards.append(itemBase);
			break;
		}

		double y = itemBase->pos().y() + itemBase->size().height();
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
	newBreadboard = this->addItem(ModuleIDNames::TinyBreadboardModuleIDName, defaultViewLayerSpec(), BaseCommand::SingleView, vg, id, -1, NULL);
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
	Wire * wire = qobject_cast<Wire *>(fromConnectorItem->attachedTo());
	if (wire == NULL) return NULL;

	QList<ConnectorItem *> ends;
	calcDistances(wire, ends);
	clearDistances();
	if (ends.count() < 1) return NULL;

	return ends[0];
}

void PCBSketchWidget::calcDistances(Wire * wire, QList<ConnectorItem *> & ends) {
	QList<Wire *> chained;
	wire->collectChained(chained, ends);
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

		Wire * w = qobject_cast<Wire *>(toConnectorItem->attachedTo());
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
        font.setPointSize(getLabelFontSizeSmall());
	color.setAlpha(255);

	switch (viewLayerSpec) {
		case ViewLayer::WireOnTop_TwoLayers:
		case ViewLayer::WireOnBottom_OneLayer:
		case ViewLayer::WireOnBottom_TwoLayers:
                case ViewLayer::Top:
                case ViewLayer::Bottom:
                case ViewLayer::TopAndBottom:
                case ViewLayer::UnknownSpec:
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

				extendChangeConnectionCommand(BaseCommand::SingleView, fromConnectorItem, toConnectorItem,
											ViewLayer::specFromID(toConnectorItem->attachedToViewLayerID()),
											false, parentCommand);
			}
		}
	}
}

double PCBSketchWidget::getLabelFontSizeTiny() {
        return 3;
}

double PCBSketchWidget::getLabelFontSizeSmall() {
	return 5;
}

double PCBSketchWidget::getLabelFontSizeMedium() {
	return 7;
}

double PCBSketchWidget::getLabelFontSizeLarge() {
	return 12;
}

void PCBSketchWidget::resizeBoard(double mmW, double mmH, bool doEmit)
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

	double origw = item->modelPart()->prop("width").toDouble();
	double origh = item->modelPart()->prop("height").toDouble();

	if (mmH == 0 || mmW == 0) {
		dynamic_cast<ResizableBoard *>(item)->setInitialSize();
		double w = item->modelPart()->prop("width").toDouble();
		double h = item->modelPart()->prop("height").toDouble();
		if (origw == w && origh == h) {
			// no change
			return;
		}

		viewItemInfo(item);
		mmW = w;
		mmH = h;
	}

	QUndoCommand * parentCommand = new QUndoCommand(tr("Resize board to %1 %2").arg(mmW).arg(mmH));
	rememberSticky(item, parentCommand);
	new ResizeBoardCommand(this, item->id(), origw, origh, mmW, mmH, parentCommand);
	new CheckStickyCommand(this, BaseCommand::SingleView, item->id(), true, CheckStickyCommand::RedoOnly, parentCommand);
	m_undoStack->push(parentCommand);
}

void PCBSketchWidget::showLabelFirstTime(long itemID, bool show, bool doEmit) {
	// called when new item is added, to decide whether to show part label
	SketchWidget::showLabelFirstTime(itemID, show, doEmit);
	ItemBase * itemBase = findItem(itemID);
	if (itemBase == NULL) return;

	switch (itemBase->itemType()) {
		case ModelPart::Part:
		case ModelPart::Jumper:
			{
				if (itemBase->hasPartLabel()) {
					ViewLayer * viewLayer = m_viewLayers.value(getLabelViewLayerID(itemBase->viewLayerSpec()));
					itemBase->showPartLabel(itemBase->isVisible(), viewLayer);
					itemBase->partLabelSetHidden(!viewLayer->visible());
				}
			}
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
            return board->layerKinChief();
        }
    }

	return NULL;
}

void PCBSketchWidget::forwardRoutingStatus(const RoutingStatus & routingStatus) 
{
	m_routingStatus = routingStatus;
	SketchWidget::forwardRoutingStatus(routingStatus);
}

void PCBSketchWidget::updateRoutingStatus(CleanUpWiresCommand* command, RoutingStatus & routingStatus, bool manual)
{
	//DebugDialog::debug("update ratsnest status");


	checkDeleteTrace(command);

	routingStatus.zero();
	updateRoutingStatus(routingStatus, manual);

	if (routingStatus != m_routingStatus) {
		if (command) {
			// changing state after the command has already been executed
			command->addRoutingStatus(this, m_routingStatus, routingStatus);
		}

		emit routingStatusSignal(this, routingStatus);

		m_routingStatus = routingStatus;
	}
}

void PCBSketchWidget::updateRoutingStatus(RoutingStatus & routingStatus, bool manual) 
{
	DebugDialog::debug(QString("update routing status %1 %2 %3")
		.arg(m_viewIdentifier) 
		.arg(m_ratsnestUpdateConnect.count())
		.arg(m_ratsnestUpdateDisconnect.count())
		);

	// TODO: think about ways to optimize this...

	QList< QPointer<VirtualWire> > ratsToDelete;

	QList< QList<ConnectorItem *> > ratnestsToUpdate;
	QList<ConnectorItem *> visited;
	foreach (QGraphicsItem * item, scene()->items()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
		if (connectorItem == NULL) continue;
		if (visited.contains(connectorItem)) continue;

		//connectorItem->debugInfo("testing urs");

		VirtualWire * vw = qobject_cast<VirtualWire *>(connectorItem->attachedTo());
		if (vw != NULL) {
			if (vw->connector0()->connectionsCount() == 0 || vw->connector1()->connectionsCount() == 0) {
				ratsToDelete.append(vw);
			}
			visited << vw->connector0() << vw->connector1();
			continue;  
		}


		QList<ConnectorItem *> connectorItems;
		connectorItems.append(connectorItem);
		ConnectorItem::collectEqualPotential(connectorItems, true, ViewGeometry::RatsnestFlag | getTraceFlag());
		visited.append(connectorItems);

		//foreach (ConnectorItem * ci, connectorItems) ci->debugInfo("cep");

		bool doRatsnest = manual || checkUpdateRatsnest(connectorItems);
		if (!doRatsnest && connectorItems.count() <= 1) continue;

		QList<ConnectorItem *> partConnectorItems;
		ConnectorItem::collectParts(connectorItems, partConnectorItems, includeSymbols(), ViewLayer::TopAndBottom);
		if (partConnectorItems.count() < 1) continue;
		if (!doRatsnest && partConnectorItems.count() <= 1) continue;

		for (int i = partConnectorItems.count() - 1; i >= 0; i--) {
			ConnectorItem * ci = partConnectorItems[i];
			
			//ci->debugInfo("pc");

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

	routingStatus.m_jumperItemCount /= 4;			// since we counted each connector twice on two layers (4 connectors per jumper item)

	// can't do this in the above loop since VirtualWires and ConnectorItems are added and deleted
	foreach (QList<ConnectorItem *> partConnectorItems, ratnestsToUpdate) {
		partConnectorItems.at(0)->displayRatsnest(partConnectorItems);
	}

	foreach(QPointer<VirtualWire> vw, ratsToDelete) {
		if (vw != NULL) {
			vw->scene()->removeItem(vw);
			delete vw;
		}
	}

        /*
        // uncomment for live drc
        CMRouter cmRouter(this);
        QString message;
        bool result = cmRouter.drc(message);
        */
}

bool PCBSketchWidget::checkUpdateRatsnest(QList<ConnectorItem *> & connectorItems) {
	bool doRatsnest = false;
	for (int i = m_ratsnestUpdateConnect.count() - 1; i >= 0; i--) {
		ConnectorItem * ci = m_ratsnestUpdateConnect[i];
		bool remove = false;
		if (ci == NULL) {
			remove = true;
			//DebugDialog::debug(QString("rem rat null %1 con:true").arg(m_viewIdentifier));
		}
		else if (connectorItems.contains(ci)) {
			remove = true;
			/*
			DebugDialog::debug(QString("rem rat '%1' id:%2 cid:%3 vid:%4 vlid:%5 con:true")
				.arg(ci->attachedToTitle())
				.arg(ci->attachedToID())
				.arg(ci->connectorSharedID())
				.arg(m_viewIdentifier)
				.arg(ci->attachedToViewLayerID())
				);
				*/
			doRatsnest = true;
		}
		if (remove) m_ratsnestUpdateConnect.removeAt(i);
	}
	for (int i = m_ratsnestUpdateDisconnect.count() - 1; i >= 0; i--) {
		ConnectorItem * ci = m_ratsnestUpdateDisconnect[i];
		bool remove = false;
		if (ci == NULL) {
			remove = true;
			//DebugDialog::debug(QString("rem rat null %1 con:false").arg(m_viewIdentifier));
		}
		else if (connectorItems.contains(ci)) {
			remove = true;
			/*
			DebugDialog::debug(QString("rem rat '%1' id:%2 cid:%3 vid:%4 vlid:%5 false")
				.arg(ci->attachedToTitle())
				.arg(ci->attachedToID())
				.arg(ci->connectorSharedID())
				.arg(m_viewIdentifier)
				.arg(ci->attachedToViewLayerID())
				);
				*/
			doRatsnest = true;
		}
		if (remove) m_ratsnestUpdateDisconnect.removeAt(i);
	}

	return doRatsnest;
}

double PCBSketchWidget::defaultGridSizeInches() {
	return 0.1;
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

long PCBSketchWidget::setUpSwap(ItemBase * itemBase, long newModelIndex, const QString & newModuleID, ViewLayer::ViewLayerSpec viewLayerSpec, 
								bool master, bool noFinalChangeWiresCommand, QUndoCommand * parentCommand)
{
	Q_UNUSED(noFinalChangeWiresCommand);

	QList<ItemBase *> smds;
	QList<Wire *> already;

	int newLayers = isBoardLayerChange(itemBase, newModuleID, master);
	bool wasSMD = itemBase->modelPart()->flippedSMD();

	long newID = SketchWidget::setUpSwap(itemBase, newModelIndex, newModuleID, viewLayerSpec, master, newLayers != m_boardLayers, parentCommand);
	if (newLayers == m_boardLayers) {
		if (!wasSMD) {
			ModelPart * mp = paletteModel()->retrieveModelPart(newModuleID);
			if (mp->flippedSMD()) {
				smds.append(itemBase);
				clearSmdTraces(smds, already, parentCommand);
			}
		}
		return newID;
	}

	if (itemBase->viewIdentifier() != m_viewIdentifier) {
		itemBase = findItem(itemBase->id());
		if (itemBase == NULL) {
			if (master) {
				new CleanUpWiresCommand(this, CleanUpWiresCommand::RedoOnly, parentCommand);
			}
			return newID;
		}
	}

	ChangeBoardLayersCommand * changeBoardCommand = new ChangeBoardLayersCommand(this, m_boardLayers, newLayers, parentCommand);

	ModelPart * mp = paletteModel()->retrieveModelPart(newModuleID);
	if (mp->itemType() == ModelPart::ResizableBoard) {
		// preserve the size if swapping rectangular board
		ResizableBoard * rb = qobject_cast<ResizableBoard *>(itemBase);
		if (rb) {
			QPointF p;
			QSizeF sz;
			rb->getParams(p, sz);
			new ResizeBoardCommand(this, newID, sz.width(), sz.height(), sz.width(), sz.height(), parentCommand);
		}
		new CheckStickyCommand(this, BaseCommand::SingleView, newID, false, CheckStickyCommand::RemoveOnly, parentCommand);
	}


	// disconnect and flip smds
	foreach (QGraphicsItem * item, scene()->items()) {
		ItemBase * smd = dynamic_cast<ItemBase *>(item);
		if (smd == NULL) continue;
		if (!smd->modelPart()->flippedSMD()) continue;

		smd = smd->layerKinChief();
		if (smds.contains(smd)) continue;

		smds.append(smd);
	}

	clearSmdTraces(smds, already, changeBoardCommand);

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

	foreach (ItemBase * smd, smds) {
		emit subSwapSignal(this, smd, (newLayers == 1) ? ViewLayer::ThroughHoleThroughTop_OneLayer : ViewLayer::ThroughHoleThroughTop_TwoLayers, changeBoardCommand);
	}

	return newID;
}

void PCBSketchWidget::clearSmdTraces(QList<ItemBase *> & smds, 	QList<Wire *> & already, QUndoCommand * parentCommand) {

	foreach (ItemBase * smd, smds) {
		foreach (ConnectorItem * ci, smd->cachedConnectorItems()) {
			//ci->debugInfo("smd from");

			foreach (ConnectorItem * toci, ci->connectedToItems()) {
				//toci->debugInfo(" smd to");
				TraceWire * tw = qobject_cast<TraceWire *>(toci->attachedTo());
				if (tw == NULL) continue;
				if (already.contains(tw)) continue;

				QList<ConnectorItem *> ends;
				removeWire(tw, ends, already, parentCommand);		
			}
		}
	}

	// remove these trace connections now so they're not added back in when the part is swapped
	QList<ConnectorItem *> ends;
	foreach (Wire * wire, already) {
		ends.append(wire->connector0());				
		ends.append(wire->connector1());
	}
	foreach (ConnectorItem * end, ends) {
		//end->debugInfo("end from");
		foreach (ConnectorItem * endTo, end->connectedToItems()) {
			//endTo->debugInfo("   end to");
			end->tempRemove(endTo, false);
			endTo->tempRemove(end, false);
		}
	}
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
	w->collectChained(chained, ends);
	makeWiresChangeConnectionCommands(chained, parentCommand);
	foreach (Wire * c, chained) {
		makeDeleteItemCommand(c,  /* BaseCommand::SingleView  */ BaseCommand::CrossView, parentCommand);
		done.append(c);
	}
}

void PCBSketchWidget::loadFromModelParts(QList<ModelPart *> & modelParts, BaseCommand::CrossViewType crossViewType, QUndoCommand * parentCommand, 
						bool offsetPaste, const QRectF * boundingRect, bool seekOutsideConnections, QList<long> & newIDs) {
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

	SketchWidget::loadFromModelParts(modelParts, crossViewType, parentCommand, offsetPaste, boundingRect, seekOutsideConnections, newIDs);
}

bool PCBSketchWidget::isInLayers(ConnectorItem * connectorItem, ViewLayer::ViewLayerSpec viewLayerSpec) {
	return connectorItem->isInLayers(viewLayerSpec);
}

bool PCBSketchWidget::routeBothSides() {
	return m_boardLayers > 1;
}

bool PCBSketchWidget::sameElectricalLayer2(ViewLayer::ViewLayerID id1, ViewLayer::ViewLayerID id2) {
	switch (id1) {
		case ViewLayer::Copper0Trace:
			if (id1 == id2) return true;
			return (id2 == ViewLayer::Copper0);
		case ViewLayer::Copper0:
			if (id1 == id2) return true;
			return (id2 == ViewLayer::Copper0Trace);
		case ViewLayer::Copper1Trace:
			if (id1 == id2) return true;
			return (id2 == ViewLayer::Copper1);
		case ViewLayer::Copper1:
			if (id1 == id2) return true;
			return (id2 == ViewLayer::Copper1Trace);
                default:
                        break;
	}

	return false;
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
		QList<ConnectorItem *> ends;
		tw->collectChained(wires, ends);
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

	ViewLayer::ViewLayerID newViewLayerID = (changeWires.at(0)->viewLayerID() == ViewLayer::Copper0Trace) ? ViewLayer::Copper1Trace : ViewLayer::Copper0Trace;;
	foreach (Wire * wire, changeWires) {
		QList<Wire *> wires;
		QList<ConnectorItem *> ends;
		wire->collectChained(wires, ends);

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

			extendChangeConnectionCommand(BaseCommand::SingleView,
								targetConnectorItem, end,
								ViewLayer::specFromID(end->attachedToViewLayerID()), 
								false, parentCommand);
		}

		foreach (Wire * w, wires) {
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

void PCBSketchWidget::changeLayer(long id, double z, ViewLayer::ViewLayerID viewLayerID) {
	ItemBase * itemBase = findItem(id);
	if (itemBase == NULL) return;

	itemBase->setViewLayerID(viewLayerID, m_viewLayers);
	itemBase->setZValue(z);
	itemBase->saveGeometry();

	TraceWire * tw = qobject_cast<TraceWire *>(itemBase);
	if (tw != NULL) {
		ViewLayer::ViewLayerSpec viewLayerSpec = ViewLayer::specFromID(viewLayerID);
		tw->setViewLayerSpec(viewLayerSpec);
		tw->setColorString(traceColor(viewLayerSpec), 1.0);
		ViewLayer * viewLayer = m_viewLayers.value(viewLayerID);
		tw->setInactive(!viewLayer->isActive());
		tw->setHidden(!viewLayer->visible());
		tw->update();
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
			QHash<long, ItemBase *> savedItems;
			QHash<Wire *, ConnectorItem *> savedWires;
			if (board == NULL) {
				foreach (QGraphicsItem * item, scene()->items()) {
					PaletteItemBase * itemBase = dynamic_cast<PaletteItemBase *>(item);
					if (itemBase->itemType() == ModelPart::Jumper) continue;

					savedItems.insert(itemBase->layerKinChief()->id(), itemBase);
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
	double ny = GraphicsUtils::getNearestOrdinate(newPos.y(), gridSizeInches() * FSvgRenderer::printerScale());
	double nx = GraphicsUtils::getNearestOrdinate(newPos.x(), gridSizeInches() * FSvgRenderer::printerScale());
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
	rememberSticky(m_resizingBoard, parentCommand);
	new ResizeBoardCommand(this, m_resizingBoard->id(), oldSize.width(), oldSize.height(), newSize.width(), newSize.height(), parentCommand);
	if (oldPos != newPos) {
		m_resizingBoard->saveGeometry();
		ViewGeometry vg1 = m_resizingBoard->getViewGeometry();
		ViewGeometry vg2 = vg1;
		vg1.setLoc(oldPos);
		vg2.setLoc(newPos);
		new MoveItemCommand(this, m_resizingBoard->id(), vg1, vg2, false, parentCommand);
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

	QUndoCommand * parentCommand = new QUndoCommand(tr("Delete ratsnest"));
	QList<long> ids;
	emit disconnectWireSignal(itemBases, ids, parentCommand);
	m_undoStack->push(parentCommand);
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

	QList<Wire *> wires;
	QList<ConnectorItem *> ends;
	m_bendpointWire->collectChained(wires, ends);
	if (ends.count() != 2) {
		// ratsnest wires should always and only have two ends: we're screwed
		return;
	}

	BaseCommand::CrossViewType crossViewType = BaseCommand::SingleView;

	QUndoCommand * parentCommand = new QUndoCommand();
	parentCommand->setText(tr("Create and connect trace"));

	new CleanUpWiresCommand(this, CleanUpWiresCommand::UndoOnly, parentCommand);

	//SelectItemCommand * selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);

	m_connectorDragWire->saveGeometry();
	m_bendpointWire->saveGeometry();

	ViewLayer::ViewLayerSpec viewLayerSpec = createWireViewLayerSpec(ends[0], ends[1]);
	if (viewLayerSpec == ViewLayer::UnknownSpec) {
		// for now this should not be possible
		QMessageBox::critical(NULL, tr("Fritzing"), tr("This seems like an attempt to create a trace across layers. This circumstance should not arise: please contact the developers."));
	}
	else {
		long newID1 = ItemBase::getNextID();
		ViewGeometry vg1 = m_connectorDragWire->getViewGeometry();
		vg1.setWireFlags(getTraceFlag());
		new AddItemCommand(this, crossViewType, m_connectorDragWire->moduleID(), viewLayerSpec, vg1, newID1, true, -1, parentCommand);
		new CheckStickyCommand(this, crossViewType, newID1, false, CheckStickyCommand::RemoveOnly, parentCommand);
		new WireColorChangeCommand(this, newID1, traceColor(viewLayerSpec), traceColor(viewLayerSpec), 1.0, 1.0, parentCommand);
		new WireWidthChangeCommand(this, newID1, getTraceWidth(), getTraceWidth(), parentCommand);

		long newID2 = ItemBase::getNextID();
		ViewGeometry vg2 = m_bendpointWire->getViewGeometry();
		vg2.setWireFlags(getTraceFlag());
		new AddItemCommand(this, crossViewType, m_bendpointWire->moduleID(), viewLayerSpec, vg2, newID2, true, -1, parentCommand);
		new CheckStickyCommand(this, crossViewType, newID2, false, CheckStickyCommand::RemoveOnly, parentCommand);
		new WireColorChangeCommand(this, newID2, traceColor(viewLayerSpec), traceColor(viewLayerSpec), 1.0, 1.0, parentCommand);
		new WireWidthChangeCommand(this, newID2, getTraceWidth(), getTraceWidth(), parentCommand);

		new ChangeConnectionCommand(this, BaseCommand::SingleView,
										newID2, m_connectorDragConnector->connectorSharedID(),
										newID1, m_connectorDragWire->connector0()->connectorSharedID(),
										viewLayerSpec,					// ViewLayer::specFromID(wire->viewLayerID())
										true, parentCommand);

		foreach (ConnectorItem * toConnectorItem, m_bendpointWire->connector0()->connectedToItems()) {
			new ChangeConnectionCommand(this, BaseCommand::SingleView,
										newID2, m_bendpointWire->connector0()->connectorSharedID(),
										toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
										viewLayerSpec,					// ViewLayer::specFromID(toConnectorItem->attachedToViewLayerID())
										true, parentCommand);
		}
		foreach (ConnectorItem * toConnectorItem, m_connectorDragWire->connector1()->connectedToItems()) {
			new ChangeConnectionCommand(this, BaseCommand::SingleView,
										newID1, m_connectorDragWire->connector1()->connectorSharedID(),
										toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
										viewLayerSpec,					// ViewLayer::specFromID(toConnectorItem->attachedToViewLayerID())
										true, parentCommand);
			m_connectorDragWire->connector1()->tempRemove(toConnectorItem, false);
			toConnectorItem->tempRemove(m_connectorDragWire->connector1(), false);
			m_bendpointWire->connector1()->tempConnectTo(toConnectorItem, false);
			toConnectorItem->tempConnectTo(m_bendpointWire->connector1(), false);
		}
	}

	m_bendpointWire->setPos(m_bendpointVG.loc());
	m_bendpointWire->setLine(m_bendpointVG.line());
	m_connectorDragConnector->tempRemove(m_connectorDragWire->connector0(), false);
	m_connectorDragWire->connector0()->tempRemove(m_connectorDragConnector, false);
	m_bendpointWire = NULL;			// signal that we're done

	// remove the temporary wire
	this->scene()->removeItem(m_connectorDragWire);

	new CleanUpWiresCommand(this, CleanUpWiresCommand::RedoOnly, parentCommand);
	m_undoStack->push(parentCommand);
}

void PCBSketchWidget::wireSplitSlot(Wire* wire, QPointF newPos, QPointF oldPos, QLineF oldLine) {
	if (!wire->getRatsnest()) {
		SketchWidget::wireSplitSlot(wire, newPos, oldPos, oldLine);
	}

	createTrace(wire);
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
	ItemBase * itemBase = addItem(paletteModel()->retrieveModelPart(ModuleIDNames::JumperModuleIDName), defaultViewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
	if (itemBase) {
		JumperItem * jumperItem = qobject_cast<JumperItem *>(itemBase);
                if (jumperItem) {
                    m_jumperItemSize = jumperItem->connector0()->rect().size();
                    deleteItem(itemBase, true, false, false);
                }
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

QSizeF PCBSketchWidget::jumperItemSize() {
	return m_jumperItemSize;
}

double PCBSketchWidget::getKeepout() {
	return 0.015 * FSvgRenderer::printerScale();  // mils converted to pixels
}

bool PCBSketchWidget::acceptsTrace(const ViewGeometry & viewGeometry) {
	return !viewGeometry.getSchematicTrace();
}

struct BestPlace
{
	Tile * bestTile;
	TileRect bestTileRect;
	int width;
	int height;
};

int placeBestFit(Tile * tile, UserData userData) {
	if (TiGetType(tile) != Tile::SPACE) return 0;

	BestPlace * bestPlace = (BestPlace *) userData;
	TileRect tileRect;
	TiToRect(tile, &tileRect);
	if (tileRect.xmaxi - tileRect.xmini < bestPlace->width) return 0;
	if (tileRect.ymaxi - tileRect.ymini < bestPlace->height) return 0;

	if (bestPlace->bestTile == NULL) {
		bestPlace->bestTile = tile;
		bestPlace->bestTileRect = tileRect;
		return 0;
	}

	double bestArea = ((double) (bestPlace->bestTileRect.xmaxi - bestPlace->bestTileRect.xmini)) * (bestPlace->bestTileRect.ymaxi - bestPlace->bestTileRect.ymini);
	double area =  ((double) (tileRect.xmaxi - tileRect.xmini)) * (tileRect.ymaxi - tileRect.ymini);
	if (area < bestArea) {
		bestPlace->bestTile = tile;
		bestPlace->bestTileRect = tileRect;
	}

	return 0;
}

ItemBase * PCBSketchWidget::placePartDroppedInOtherView(ModelPart * modelPart, ViewLayer::ViewLayerSpec viewLayerSpec, const ViewGeometry & viewGeometry, long id, SketchWidget * dropOrigin) 
{
	ItemBase * newItem = SketchWidget::placePartDroppedInOtherView(modelPart, viewLayerSpec, viewGeometry, id, dropOrigin);
	if (newItem == NULL) return newItem;
	if (!autorouteTypePCB()) return newItem;

	ItemBase * board = findBoard();
	if (board == NULL) {
		return newItem;
	}

	dealWithDefaultParts();

	// This is a 2d bin-packing problem. We can use our tile datastructure for this.  
	// Use a simple best-fit approach for now.  No idea how optimal a solution it is.

	CMRouter router(this);
	int keepout = 10;
	router.setKeepout(keepout);
	Plane * plane = router.initPlane(false);
	QList<Tile *> alreadyTiled;	
	router.initBoard(board, plane, alreadyTiled);

	QRectF boardRect = board->sceneBoundingRect();
	foreach (QGraphicsItem * item, scene()->items()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;
		if (itemBase->layerKinChief() == board) continue;
		if (itemBase->layerKinChief() == newItem) continue;

		QRectF r = itemBase->sceneBoundingRect();
		r.adjust(-keepout, -keepout, keepout, keepout);
		QRectF s = r.intersected(boardRect);
		if (s.width() <= 0) continue;
		if (s.height() <= 0) continue;	

		router.insertTile(plane, s, alreadyTiled, NULL, Tile::OBSTACLE, CMRouter::IgnoreAllOverlaps);
	}

	TileRect tileBoardRect = router.boardRect();
	BestPlace bestPlace;
	bestPlace.bestTile = NULL;
	bestPlace.width = CMRouter::realToTile(newItem->boundingRect().width());
	bestPlace.height = CMRouter::realToTile(newItem->boundingRect().height());
	TiSrArea(NULL, plane, &tileBoardRect, placeBestFit, &bestPlace);
	if (bestPlace.bestTile != NULL) {
		QRectF r;
		CMRouter::tileToQRect(bestPlace.bestTile, r);
		newItem->setPos(r.topLeft());
		alignOneToGrid(newItem);
	}
	router.drcClean();

	return newItem;
}


void PCBSketchWidget::autorouterSettings() {	
	AutorouterSettingsDialog dialog;
	dialog.exec();
}

void PCBSketchWidget::getViaSize(double & ringThickness, double & holeSize) {
	QString ringThicknessStr, holeSizeStr;
	getDefaultViaSize(ringThicknessStr, holeSizeStr);
	double rt = TextUtils::convertToInches(ringThicknessStr);
	double hs = TextUtils::convertToInches(holeSizeStr);
	ringThickness = rt * FSvgRenderer::printerScale();
	holeSize = hs * FSvgRenderer::printerScale();
}

void PCBSketchWidget::getDefaultViaSize(QString & ringThickness, QString & holeSize) {
	// these settings are initialized in hole.cpp
	QSettings settings;
	ringThickness = settings.value(Hole::AutorouteViaRingThickness, "").toString();
	holeSize = settings.value(Hole::AutorouteViaHoleSize, "").toString();
}

void PCBSketchWidget::changeTrace(Wire * wire, ConnectorItem * from, ConnectorItem * to, QUndoCommand * parentCommand) 
{
	// first figure out whether this is a delete or a reconnect
	QList<ConnectorItem *> ends;
	QList<Wire *> wires;
	wire->collectChained(wires, ends);

	ConnectorItem * anchor = wire->otherConnector(from);

	ConnectorItem * fromDest = NULL;
	bool reconnect = (to != NULL);
	if (reconnect) {
		// check whether wire end was dropped on a legitimate target
		if (ends.contains(to)) {
			reconnect = false;
		}
		else {
			foreach (Wire * w, wires) {
				if (w->connector0() == to) {
					reconnect = false;
					break;
				}
				if (w->connector1() == to) {
					reconnect = false;
					break;
				}
			}
		}
	}

	if (reconnect) {
		foreach (ConnectorItem * toConnectorItem, from->connectedToItems()) {
			if (toConnectorItem->attachedToItemType() == ModelPart::Wire) continue;

			// the part we were originally connected to
			fromDest = toConnectorItem;
			break;
		}
	}

	if (fromDest) {
		ViewGeometry vg = wire->getViewGeometry();
		ViewGeometry newvg;
		QPointF wp = wire->pos();
		QPointF ap = anchor->sceneAdjustedTerminalPoint(NULL);
		if (wp == ap) {
			makeRatsnestViewGeometry(newvg, anchor, to);
		}
		else {
			makeRatsnestViewGeometry(newvg, to, anchor);
		}

		new ChangeWireCommand(this, wire->id(), vg.line(), newvg.line(), vg.loc(), newvg.loc(), true, true, parentCommand);

		extendChangeConnectionCommand(BaseCommand::CrossView, from, fromDest, 
					ViewLayer::specFromID(wire->viewLayerID()),
					false, parentCommand);


		ConnectorItem * toDest = to;
		if (toDest->attachedToItemType() == ModelPart::Wire) {
			toDest = this->findNearestPartConnectorItem(toDest);
		}

		QList<ConnectorItem *> connectorItems;
		connectorItems.append(toDest);
		ConnectorItem::collectEqualPotential(connectorItems, true,getTraceFlag() | ViewGeometry::RatsnestFlag);
		if (!connectorItems.contains(fromDest)) {
			// have to make a permanent connection
			ConnectorItem * permanentFrom = fromDest;
			foreach (ConnectorItem * end, ends) {
				if (end != fromDest) {
					permanentFrom = end;
					break;
				}
			}
			makeModifiedWire(permanentFrom, toDest, BaseCommand::CrossView, ViewGeometry::NormalFlag, parentCommand);	
		}	

		extendChangeConnectionCommand(BaseCommand::CrossView, from, to, 
				ViewLayer::specFromID(wire->viewLayerID()),
				true, parentCommand);

		parentCommand->setText(QObject::tr("change trace %1").arg(wire->title()) );
		new CleanUpWiresCommand(this, CleanUpWiresCommand::RedoOnly, parentCommand);
		m_undoStack->push(parentCommand);
		return;
	}


	QList<Wire *> toDelete;
	ConnectorItem * target = anchor;
	toDelete.append(wire);
	while (true) {
		int count = 0;
		Wire * candidate = NULL;
		ConnectorItem * candidateTarget = NULL;
		foreach (ConnectorItem * toTarget, target->connectedToItems()) {
			Wire * w = qobject_cast<Wire *>(toTarget->attachedTo());
			if (w == NULL) {
				// this is a part, so delete all wires
				break;
			}

			candidate = w;
			candidateTarget = w->otherConnector(toTarget);
			++count;

			// it may be that the other connector is the one with multiple connections
			foreach (ConnectorItem * toTargetTarget, toTarget->connectedToItems()) {
				Wire * wx = qobject_cast<Wire *>(toTargetTarget->attachedTo());
				if (wx == NULL) continue;
				if (toTargetTarget->attachedTo() == target->attachedTo()) continue;

				++count;
				break;
			}

			if (count > 1) {
				break;
			}
		}

		if (candidate == NULL) {
			// connected to a part
			break;
		}

		if (count > 1) {
			// junction for multiple wires; stop here
			break;
		}

		toDelete.append(candidate);
		target = candidateTarget;	
	}

	makeWiresChangeConnectionCommands(toDelete, parentCommand);
	foreach (Wire * w, toDelete) {
		makeDeleteItemCommand(w, BaseCommand::CrossView, parentCommand);
	}

	parentCommand->setText(QObject::tr("delete trace %1").arg(wire->title()) );

	new CleanUpWiresCommand(this, CleanUpWiresCommand::RedoOnly, parentCommand);
	m_undoStack->push(parentCommand);
}

void PCBSketchWidget::checkDeleteTrace(CleanUpWiresCommand* command) 
{
	if (command == NULL) return;
	if (m_ratsnestUpdateDisconnect.count() == 0) return;
	if (command->hasTraces(this)) return;			// already dealt with this

	// this code should now only be relevant to the case in which a wire is deleted in breadboard view
	if (command->sketchWidget()->viewIdentifier() != ViewIdentifierClass::BreadboardView) return;

	QSet<Wire *> deletedWires;
	QList<Wire *> visitedWires;
	foreach (QGraphicsItem * item, scene()->items()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		if (!wire->getTrace()) continue;
		if (visitedWires.contains(wire)) continue;

		if (wire->connector0()->connectedToItems().count() == 0) {
			// trace is already going to be deleted
			continue;
		}

		QList<Wire *> wires;
		QList<ConnectorItem *> ends;
		wire->collectChained(wires, ends);
		visitedWires.append(wires);
		if (ends.count() <= 0) continue;

		//foreach (ConnectorItem * ci, ends) ci->debugInfo("end");

		QList<ConnectorItem *> connectorItems;
		connectorItems.append(ends[0]);
		ConnectorItem::collectEqualPotential(connectorItems, true, ViewGeometry::RatsnestFlag | getTraceFlag());
		//foreach (ConnectorItem * ci, connectorItems) ci->debugInfo("   eq");

		bool doDelete = false;
		foreach (ConnectorItem * end, ends) {
			if (!connectorItems.contains(end)) {
				doDelete = true;
				break;
			}
		}

		if (doDelete) {
			foreach (Wire * w, wires) {
				command->addTrace(this, w);
				deletedWires.insert(w);
			}
		}
	}

	foreach (Wire * w, deletedWires.values()) {
		//wire->markDeleted(true);
		deleteItem(w, true, false, false);
	}
}


void PCBSketchWidget::deleteItem(ItemBase * itemBase, bool deleteModelPart, bool doEmit, bool later)
{
	bool boardDeleted = (itemBase->itemType() == ModelPart::Board || itemBase->itemType() == ModelPart::ResizableBoard);
	SketchWidget::deleteItem(itemBase, deleteModelPart, doEmit, later);
	if (boardDeleted) {
		ItemBase * board = findBoard();
		if (board == NULL) {
			// no board found, so set to single-layer by default
			changeBoardLayers(1, true);
			emit boardDeletedSignal();
		}
	}
}

double PCBSketchWidget::getTraceWidth() {
	return Wire::STANDARD_TRACE_WIDTH;
}

double PCBSketchWidget::getAutorouterTraceWidth() {
	QSettings settings;
	int traceWidthMils = settings.value(AutorouterSettingsDialog::AutorouteTraceWidth, QString::number(GraphicsUtils::pixels2mils(getTraceWidth(), FSvgRenderer::printerScale()))).toInt();
	return FSvgRenderer::printerScale() * traceWidthMils / 1000;
}

void PCBSketchWidget::getBendpointWidths(Wire * wire, double width, double & bendpointWidth, double & bendpoint2Width, bool & negativeOffsetRect) 
{
	Q_UNUSED(wire);
	bendpointWidth = bendpoint2Width = (width / -2);
	negativeOffsetRect = false;
}

double PCBSketchWidget::getSmallerTraceWidth(double minDim) {
	int mils = qMax((int) GraphicsUtils::pixels2mils(minDim, FSvgRenderer::printerScale()) - 1, TraceWire::MinTraceWidthMils);
	return GraphicsUtils::mils2pixels(mils, FSvgRenderer::printerScale());
}

bool PCBSketchWidget::groundFill(QUndoCommand * parentCommand)
{
	ItemBase * board = findBoard();
    // barf an error if there's no board
    if (!board) {
        QMessageBox::critical(NULL, tr("Fritzing"),
                   tr("Your sketch does not have a board yet!  Please add a PCB in order to use copper fill."));
        return false;
    }

	LayerList viewLayerIDs;
	viewLayerIDs << ViewLayer::Board;
	QSizeF boardImageSize;
	bool empty;
	QString boardSvg = renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, true, boardImageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
	if (boardSvg.isEmpty()) {
        QMessageBox::critical(NULL, tr("Fritzing"), tr("Fritzing error: unable to render board svg (1)."));
		return false;
	}

	viewLayerIDs.clear();
	viewLayerIDs << ViewLayer::Copper0 << ViewLayer::Copper0Trace;
	QSizeF copperImageSize;

	// hide ground traces so the ground plane will intersect them
	// TODO: make this a parameter...

	showGroundTraces(false);
	QString svg = renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, true, copperImageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, true, empty);
	showGroundTraces(true);
	if (svg.isEmpty()) {
        QMessageBox::critical(NULL, tr("Fritzing"), tr("Fritzing error: unable to render copper svg (1)."));
		return false;
	}

	QString svg2;
	if (boardLayers() > 1) {
		viewLayerIDs.clear();
		viewLayerIDs << ViewLayer::Copper1 << ViewLayer::Copper1Trace;
		showGroundTraces(false);
		svg2 = renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, true, copperImageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, true, empty);
		showGroundTraces(true);
		if (svg2.isEmpty()) {
			QMessageBox::critical(NULL, tr("Fritzing"), tr("Fritzing error: unable to render copper svg (1)."));
			return false;
		}
	}

	QStringList exceptions;
	exceptions << background().name();    // the color of holes in the board

	GroundPlaneGenerator gpg;
	bool result = gpg.generateGroundPlane(boardSvg, boardImageSize, svg, copperImageSize, exceptions, board, 1000 / 5.0  /* 1 MIL */,
											ViewLayer::Copper0Color, "groundplane");
	if (result == false) {
        QMessageBox::critical(NULL, tr("Fritzing"), tr("Fritzing error: unable to write copper fill (1)."));
		return false;
	}

	GroundPlaneGenerator gpg2;
	if (boardLayers() > 1) {
		bool result = gpg2.generateGroundPlane(boardSvg, boardImageSize, svg2, copperImageSize, exceptions, board, 1000 / 5.0  /* 1 MIL */,
												ViewLayer::Copper1Color, "groundplane1");
		if (result == false) {
			QMessageBox::critical(NULL, tr("Fritzing"), tr("Fritzing error: unable to write copper fill (2)."));
			return false;
		}
	}

	int ix = 0;
	foreach (QString svg, gpg.newSVGs()) {
		ViewGeometry vg;
		vg.setLoc(board->pos() + gpg.newOffsets()[ix++]);
		long newID = ItemBase::getNextID();
		new AddItemCommand(this, BaseCommand::CrossView, ModuleIDNames::GroundPlaneModuleIDName, ViewLayer::GroundPlane_Bottom, vg, newID, false, -1, parentCommand);
		new SetPropCommand(this, newID, "svg", svg, svg, true, parentCommand);
	}

	ix = 0;
	foreach (QString svg, gpg2.newSVGs()) {
		ViewGeometry vg;
		vg.setLoc(board->pos() + gpg2.newOffsets()[ix++]);
		long newID = ItemBase::getNextID();
		new AddItemCommand(this, BaseCommand::CrossView, ModuleIDNames::GroundPlaneModuleIDName, ViewLayer::GroundPlane_Top, vg, newID, false, -1, parentCommand);
		new SetPropCommand(this, newID, "svg", svg, svg, true, parentCommand);
	}

	return true;

}

QString PCBSketchWidget::generateCopperFillUnit(ItemBase * itemBase, QPointF whereToStart)
{
	ItemBase * board = findBoard();
    // barf an error if there's no board
    if (!board) {
        QMessageBox::critical(NULL, tr("Fritzing"),
                   tr("Your sketch does not have a board yet!  Please add a PCB in order to use copper fill."));
        return "";
    }

	QRectF r = board->boundingRect();
	r.moveTo(board->pos());
	if (!r.contains(whereToStart)) {
        QMessageBox::critical(NULL, tr("Fritzing"), tr("Unable to create copper fill--probably the part wasn't dropped onto the PCB."));
		return "";
	}

	LayerList viewLayerIDs;
	viewLayerIDs << ViewLayer::Board;
	QSizeF boardImageSize;
	bool empty;
	QString boardSvg = renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, true, boardImageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, false, empty);
	if (boardSvg.isEmpty()) {
        QMessageBox::critical(NULL, tr("Fritzing"), tr("Fritzing error: unable to render board svg (1)."));
		return "";
	}

	ViewLayer::ViewLayerSpec viewLayerSpec = ViewLayer::Bottom;
	QString color = ViewLayer::Copper0Color;
	QString gpLayerName = "groundplane";
	if (m_boardLayers == 2 && layerIsActive(ViewLayer::Copper1)) {
		gpLayerName += "1";
		color = ViewLayer::Copper1Color;
		viewLayerSpec = ViewLayer::Top;
	}

	viewLayerIDs = ViewLayer::copperLayers(viewLayerSpec);
	QSizeF copperImageSize;

	bool vis = itemBase->isVisible();
	itemBase->setVisible(false);
	QString svg = renderToSVG(FSvgRenderer::printerScale(), viewLayerIDs, viewLayerIDs, true, copperImageSize, board, GraphicsUtils::StandardFritzingDPI, false, false, true, empty);
	itemBase->setVisible(vis);
	if (svg.isEmpty()) {
        QMessageBox::critical(NULL, tr("Fritzing"), tr("Fritzing error: unable to render copper svg (1)."));
		return "";
	}

	QStringList exceptions;
	exceptions << background().name();    // the color of holes in the board

	GroundPlaneGenerator gpg;
	bool result = gpg.generateGroundPlaneUnit(boardSvg, boardImageSize, svg, copperImageSize, exceptions, board, 1000 / 5.0  /* 1 MIL */, 
												color, gpLayerName, whereToStart);

	if (result == false || gpg.newSVGs().count() < 1) {
        QMessageBox::critical(NULL, tr("Fritzing"), tr("Unable to create copper fill--possibly the part was dropped onto another part or wire rather than the actual PCB."));
		return "";
	}

	itemBase->setPos(board->pos() + gpg.newOffsets()[0]);
	itemBase->setViewLayerID(gpLayerName, m_viewLayers);

	return gpg.newSVGs()[0];
}


bool PCBSketchWidget::connectorItemHasSpec(ConnectorItem * connectorItem, ViewLayer::ViewLayerSpec spec) {
	if (ViewLayer::specFromID(connectorItem->attachedToViewLayerID()) == spec)  return true;

	connectorItem = connectorItem->getCrossLayerConnectorItem();
	if (connectorItem == NULL) return false;

	return (ViewLayer::specFromID(connectorItem->attachedToViewLayerID()) == spec);
}

ViewLayer::ViewLayerSpec PCBSketchWidget::createWireViewLayerSpec(ConnectorItem * from, ConnectorItem * to) {
	QList<ViewLayer::ViewLayerSpec> guesses;
	guesses.append(layerIsActive(ViewLayer::Copper0) ? ViewLayer::Bottom : ViewLayer::Top);
	guesses.append(layerIsActive(ViewLayer::Copper0) ? ViewLayer::Top : ViewLayer::Bottom);
	foreach (ViewLayer::ViewLayerSpec guess, guesses) {
		if (connectorItemHasSpec(from, guess) && connectorItemHasSpec(to, guess)) {
			return guess;
		}
	}

	return ViewLayer::UnknownSpec;
}

double PCBSketchWidget::getWireStrokeWidth(Wire * wire, double wireWidth)
{
	double w, h;
	wire->originalConnectorDimensions(w, h);
	if (wireWidth < Wire::THIN_TRACE_WIDTH) {
		wire->setConnectorDimensions(qMin(w, wireWidth + 1), qMin(w, wireWidth + 1));
	}
	if (wireWidth < Wire::STANDARD_TRACE_WIDTH) {
		wire->setConnectorDimensions(qMin(w, wireWidth + 1.5), qMin(w, wireWidth + 1.5));
	}
	else {
		wire->setConnectorDimensions(w, h);
	}

	return wireWidth + 6;
}

Wire * PCBSketchWidget::createTempWireForDragging(Wire * fromWire, ModelPart * wireModel, ConnectorItem * connectorItem, ViewGeometry & viewGeometry, ViewLayer::ViewLayerSpec spec) 
{
	if (spec == ViewLayer::UnknownSpec) {
		spec = wireViewLayerSpec(connectorItem);
	}
	viewGeometry.setTrace(true);
	Wire * wire =  SketchWidget::createTempWireForDragging(fromWire, wireModel, connectorItem, viewGeometry, spec);
	if (fromWire == NULL) {
		wire->setColorString(traceColor(connectorItem), 1.0);
		double traceWidth = getTraceWidth();
		double minDim = connectorItem->minDimension();
		if (minDim < traceWidth) {
			traceWidth = getSmallerTraceWidth(minDim);  
		}
		wire->setWireWidth(traceWidth, this, getWireStrokeWidth(wire, traceWidth));
		wire->setProperty(FakeTraceProperty, true);
	}
	else {
		wire->setColorString(fromWire->colorString(), fromWire->opacity());
		wire->setWireWidth(fromWire->width(), this, fromWire->hoverStrokeWidth());
	}

	return wire;
}

void PCBSketchWidget::prereleaseTempWireForDragging(Wire* wire)
{
	if (wire->property(PCBSketchWidget::FakeTraceProperty).toBool()) {
		// make it not look like a trace, or modifyNewWireConnections will create the wrong kind of wire
		wire->setWireFlags(0);
	}
}

bool PCBSketchWidget::curvyWiresIndicated(Qt::KeyboardModifiers)
{
	return false;
}

void PCBSketchWidget::rotatePartLabels(double degrees, QTransform & transform, QPointF center, QUndoCommand * parentCommand)
{
	ItemBase * board = NULL;
	foreach (ItemBase * itemBase, m_savedItems.values()) {
		if (itemBase->itemType() == ModelPart::ResizableBoard || itemBase->itemType() == ModelPart::Board) {
			board = itemBase->layerKinChief();
			break;
		}
	}

	if (board == NULL) return;

	QRectF bbr = board->sceneBoundingRect();

	foreach (QGraphicsItem * item, scene()->items()) {
		PartLabel * partLabel = dynamic_cast<PartLabel *>(item);
		if (partLabel == NULL) continue;
		if (!partLabel->isVisible()) continue;
		if (!bbr.intersects(partLabel->sceneBoundingRect())) continue;

		QPointF offset = partLabel->pos() - partLabel->owner()->pos();
		new MoveLabelCommand(this, partLabel->owner()->id(), partLabel->pos(), offset, partLabel->pos(), offset, parentCommand);
		new RotateFlipLabelCommand(this, partLabel->owner()->id(), degrees, 0, parentCommand);
		QPointF p = GraphicsUtils::calcRotation(transform, center, partLabel->pos(), partLabel->boundingRect().center());
		ViewGeometry vg;
		partLabel->owner()->calcRotation(transform, center, vg);
		new MoveLabelCommand(this, partLabel->owner()->id(), p, p - vg.loc(), p, p - vg.loc(), parentCommand);
	}
}
