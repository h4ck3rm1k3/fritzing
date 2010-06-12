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

#include "schematicsketchwidget.h"
#include "../autoroute/autorouter1.h"
#include "../debugdialog.h"
#include "../items/virtualwire.h"
#include "../items/symbolpaletteitem.h"
#include "../items/tracewire.h"
#include "../connectors/connectoritem.h"
#include "../waitpushundostack.h"
#include "../items/moduleidnames.h"

#include <limits>

static QString SchematicTraceColor = "blackblack";

SchematicSketchWidget::SchematicSketchWidget(ViewIdentifierClass::ViewIdentifier viewIdentifier, QWidget *parent)
    : PCBSketchWidget(viewIdentifier, parent)
{
	m_shortName = QObject::tr("schem");
	m_viewName = QObject::tr("Schematic View");
	m_standardBackgroundColor = QColor(255,255,255);
	initBackgroundColor();

	m_jumperColor = "blackblack";	
	m_jumperWidth = 2;
	m_cleanType = ninetyClean;

	m_updateDotsTimer.setInterval(20);
	m_updateDotsTimer.setSingleShot(true);
	connect(&m_updateDotsTimer, SIGNAL(timeout()), this, SLOT(updateBigDots()));
}

void SchematicSketchWidget::addViewLayers() {
	addSchematicViewLayers();
}

ViewLayer::ViewLayerID SchematicSketchWidget::getDragWireViewLayerID(ConnectorItem *) {
	return ViewLayer::SchematicTrace;
}

ViewLayer::ViewLayerID SchematicSketchWidget::getWireViewLayerID(const ViewGeometry & viewGeometry, ViewLayer::ViewLayerSpec viewLayerSpec) {
	if (viewGeometry.getTrace()) {
		return ViewLayer::SchematicTrace;
	}

	if (viewGeometry.getRatsnest()) {
		return ViewLayer::SchematicWire;
	}

	return SketchWidget::getWireViewLayerID(viewGeometry, viewLayerSpec);
}

void SchematicSketchWidget::initWire(Wire * wire, int penWidth) {
	Q_UNUSED(penWidth);
	if (wire->getRatsnest()) {
		wire->setPenWidth(1, this);
		wire->setColorString("schematicGrey", 0.7);
	}
	else {
		wire->setPenWidth(2, this);
		wire->setColorString("blackblack", 1.0);
	}
}

bool SchematicSketchWidget::autorouteNeedsBounds() {
	return false;
}

bool SchematicSketchWidget::autorouteCheckWires() {
	return false;
}

bool SchematicSketchWidget::autorouteCheckConnectors() {
	return false;
}

bool SchematicSketchWidget::autorouteCheckParts() {
	return true;
}

void SchematicSketchWidget::tidyWires() {
	QList<Wire *> wires;
	QList<Wire *> visited;
	foreach (QGraphicsItem * item, scene()->selectedItems()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		if (!wire->getTrace()) continue;
		if (visited.contains(wire)) continue;
	}
}

void SchematicSketchWidget::ensureTraceLayersVisible() {
	ensureLayerVisible(ViewLayer::SchematicTrace);
}

void SchematicSketchWidget::ensureTraceLayerVisible() {
	ensureLayerVisible(ViewLayer::SchematicTrace);
}

void SchematicSketchWidget::ensureJumperLayerVisible() {
	ensureLayerVisible(ViewLayer::SchematicTrace);
}

qreal SchematicSketchWidget::getRatsnestOpacity(bool routed) {
	return (routed ? 0.1 : 0.7);
}

void SchematicSketchWidget::setJumperFlags(ViewGeometry & vg) {
	vg.setTrace(true);
}

bool SchematicSketchWidget::usesJumperItem() {
	return false;
}

void SchematicSketchWidget::setClipEnds(ClipableWire * vw, bool) {
	vw->setClipEnds(false);
}

void SchematicSketchWidget::getBendpointWidths(Wire * wire, qreal width, qreal & bendpointWidth, qreal & bendpoint2Width) {
	SketchWidget::getBendpointWidths(wire, width, bendpointWidth, bendpoint2Width);
	bendpoint2Width = width + 3;
}


void SchematicSketchWidget::getLabelFont(QFont & font, QColor & color, ViewLayer::ViewLayerSpec) {
	font.setFamily("Droid Sans");
	font.setPointSize(9);
	color.setAlpha(255);
	color.setRgb(0);
}

void SchematicSketchWidget::setNewPartVisible(ItemBase * itemBase) {
	switch (itemBase->itemType()) {
		case ModelPart::Breadboard:
		case ModelPart::Jumper:
		case ModelPart::CopperFill:
			// don't need to see the breadboard in the other views
			// but it's there so connections can be more easily synched between views
			itemBase->setVisible(false);
			itemBase->setEverVisible(false);
			return;
	}
}

bool SchematicSketchWidget::canDropModelPart(ModelPart * modelPart) {
	switch (modelPart->itemType()) {
		case ModelPart::Jumper:
		case ModelPart::CopperFill:
		case ModelPart::Logo:
		case ModelPart::Board:
		case ModelPart::ResizableBoard:
		case ModelPart::Breadboard:
			return false;
		case ModelPart::Symbol:
			return true;
		default:
			break;
	}

	if (modelPart->moduleID().compare(ModuleIDNames::holeModuleIDName) == 0) return false;
	if (modelPart->moduleID().compare(ModuleIDNames::viaModuleIDName) == 0) return false;

	return PCBSketchWidget::canDropModelPart(modelPart);
}

bool SchematicSketchWidget::includeSymbols() {
	return true;
}

bool SchematicSketchWidget::hasBigDots() {
	return true;
}

void SchematicSketchWidget::updateBigDots() 
{	
	QList<ConnectorItem *> connectorItems;
	foreach (QGraphicsItem * item, scene()->items()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
		if (connectorItem == NULL) continue;
		if (connectorItem->attachedToItemType() != ModelPart::Wire) continue;

		TraceWire * traceWire = dynamic_cast<TraceWire *>(connectorItem->attachedTo());
		if (traceWire == NULL) continue;

		//DebugDialog::debug(QString("update big dot %1 %2").arg(traceWire->id()).arg(connectorItem->connectorSharedID()));

		connectorItem->restoreColor(false, 0, false);
	}
}

void SchematicSketchWidget::changeConnection(long fromID, const QString & fromConnectorID,
									long toID, const QString & toConnectorID,
									ViewLayer::ViewLayerSpec viewLayerSpec,
									bool connect, bool doEmit, bool updateConnections)
{
	m_updateDotsTimer.stop();
	SketchWidget::changeConnection(fromID, fromConnectorID, toID, toConnectorID, viewLayerSpec, connect,  doEmit,  updateConnections);
	m_updateDotsTimer.start();
}

AddItemCommand * SchematicSketchWidget::newAddItemCommand(BaseCommand::CrossViewType crossViewType, QString moduleID, ViewLayer::ViewLayerSpec viewLayerSpec, 
														  ViewGeometry & viewGeometry, qint64 id, bool updateInfoView, 
														  long modelIndex, QUndoCommand *parent)
{
	AddItemCommand* addItemCommand = SketchWidget::newAddItemCommand(crossViewType, moduleID, viewLayerSpec, viewGeometry, id, updateInfoView, modelIndex, parent);
	qreal v = 0;
	bool gotV = false;
	if (moduleID.compare(ModuleIDNames::groundModuleIDName) == 0) {
		gotV = true;
	}
	else if (moduleID.compare(ModuleIDNames::powerModuleIDName) == 0) {
		gotV = true;
		v = SymbolPaletteItem::DefaultVoltage;
	}
	else if (moduleID.compare(ModuleIDNames::justPowerModuleIDName) == 0) {
		gotV = true;
		v = SymbolPaletteItem::DefaultVoltage;
	}

	if (!gotV) {
		return addItemCommand;
	}

	// create the item temporarily, then delete it
	SymbolPaletteItem * newSymbol = dynamic_cast<SymbolPaletteItem *>(addItem(moduleID, viewLayerSpec, BaseCommand::SingleView, viewGeometry, id, modelIndex, NULL));
	
	foreach (QGraphicsItem * item, scene()->items()) {
		SymbolPaletteItem * symbol = dynamic_cast<SymbolPaletteItem *>(item);
		if (symbol == NULL) continue;
		if (symbol == newSymbol) continue;					// don't connect self to self
		if (symbol->connector0() == NULL) continue;			// the drag item

		if (symbol->voltage() == v) {
			makeModifiedWire(newSymbol->connector0(), symbol->connector0(), crossViewType, 0, parent); 
		}

		if (symbol->connector1() != NULL && v == 0) {
			makeModifiedWire(newSymbol->connector0(), symbol->connector1(), crossViewType, 0, parent); 
		}

		if (newSymbol->connector1() != NULL) {
			if (symbol->voltage() == 0) {
				makeModifiedWire(newSymbol->connector1(), symbol->connector0(), crossViewType, 0, parent); 
			}
			if (symbol->connector1() != NULL) {
				makeModifiedWire(newSymbol->connector1(), symbol->connector1(), crossViewType, 0, parent); 
			}
		}
	}

	deleteItem(newSymbol, true, false, false);

	return addItemCommand;
}

void SchematicSketchWidget::setVoltage(qreal v, bool doEmit)
{
	Q_UNUSED(doEmit);

	PaletteItem * item = getSelectedPart();
	if (item == NULL) return;

	if (item->itemType() != ModelPart::Symbol) return;

	SymbolPaletteItem * sitem = qobject_cast<SymbolPaletteItem *>(item);
	if (sitem == NULL) return;

	if (sitem->modelPart()->moduleID().compare("ground symbol", Qt::CaseInsensitive) == 0) return;
	if (v == sitem->voltage()) return;

	QUndoCommand * parentCommand =  new QUndoCommand();
	parentCommand->setText(tr("Change voltage from %1 to %2").arg(sitem->voltage()).arg(v));

	QList<Wire *> done;
	foreach (ConnectorItem * toConnectorItem, sitem->connector0()->connectedToItems()) {
		Wire * w = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
		if (w == NULL) continue;
		if (done.contains(w)) continue;

		QList<ConnectorItem *> ends;
		removeWire(w, ends, done, parentCommand);
	}

	new SetPropCommand(this, item->id(), "voltage", QString::number(sitem->voltage()), QString::number(v), parentCommand);
	
	foreach (QGraphicsItem * item, scene()->items()) {
		SymbolPaletteItem * other = dynamic_cast<SymbolPaletteItem *>(item);
		if (other == NULL) continue;
		if (other == sitem) continue;

		if (other->voltage() == v) {
			this->makeModifiedWire(sitem->connector0(), other->connector0(), BaseCommand::CrossView, 0, parentCommand);
		}
	}

	new CleanUpWiresCommand(this, false, parentCommand);

	m_undoStack->push(parentCommand);
}

double SchematicSketchWidget::defaultGridSizeInches() {
	return 0.3;
}

ViewLayer::ViewLayerID SchematicSketchWidget::getLabelViewLayerID(ViewLayer::ViewLayerSpec) {
	return ViewLayer::SchematicLabel;
}

int SchematicSketchWidget::designRulesCheck() 
{
	return 0;
}

const QString & SchematicSketchWidget::traceColor(ConnectorItem *) {
	return SchematicTraceColor;
}

long SchematicSketchWidget::setUpSwap(ItemBase * itemBase, long newModelIndex, const QString & newModuleID, ViewLayer::ViewLayerSpec viewLayerSpec, bool master, QUndoCommand * parentCommand)
{
	return SketchWidget::setUpSwap(itemBase, newModelIndex, newModuleID, viewLayerSpec, master, parentCommand);
}
