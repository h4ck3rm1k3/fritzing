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

static QColor schematicColor;

SchematicSketchWidget::SchematicSketchWidget(ViewIdentifierClass::ViewIdentifier viewIdentifier, QWidget *parent)
    : PCBSketchWidget(viewIdentifier, parent)
{
	m_shortName = QObject::tr("schem");
	m_viewName = QObject::tr("Schematic View");
	m_standardBackgroundColor = QColor(255,255,255);
	initBackgroundColor();

	m_traceColor = "blackblack";
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

ViewLayer::ViewLayerID SchematicSketchWidget::getDragWireViewLayerID() {
	return ViewLayer::SchematicTrace;
}

ViewLayer::ViewLayerID SchematicSketchWidget::getWireViewLayerID(const ViewGeometry & viewGeometry) {
	if (viewGeometry.getTrace()) {
		return ViewLayer::SchematicTrace;
	}

	if (viewGeometry.getRatsnest()) {
		return ViewLayer::SchematicWire;
	}

	return SketchWidget::getWireViewLayerID(viewGeometry);
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


void SchematicSketchWidget::getLabelFont(QFont & font, QColor & color) {
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
	if (modelPart->itemType() == ModelPart::Jumper) return false;
	if (modelPart->itemType() == ModelPart::CopperFill) return false;
	if (modelPart->itemType() == ModelPart::Logo) return false;

	bool result = PCBSketchWidget::canDropModelPart(modelPart);
	if (result) return result;

	if (modelPart->itemType() == ModelPart::Symbol) return true;

	return result;
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

		connectorItem->restoreColor(false, 0);
	}
}

void SchematicSketchWidget::changeConnection(long fromID, const QString & fromConnectorID,
									long toID, const QString & toConnectorID,
									bool connect, bool doEmit, bool seekLayerKin, bool updateConnections)
{
	m_updateDotsTimer.stop();
	SketchWidget::changeConnection(fromID, fromConnectorID, toID, toConnectorID, connect,  doEmit,  seekLayerKin,  updateConnections);
	m_updateDotsTimer.start();
}

AddItemCommand * SchematicSketchWidget::newAddItemCommand(BaseCommand::CrossViewType crossViewType, QString moduleID, ViewGeometry & viewGeometry, qint64 id, bool updateInfoView, long modelIndex, long originalModelIndex, QUndoCommand *parent)
{
	AddItemCommand* addItemCommand = SketchWidget::newAddItemCommand(crossViewType, moduleID, viewGeometry, id, updateInfoView, modelIndex, originalModelIndex, parent);
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
	SymbolPaletteItem * newSymbol = dynamic_cast<SymbolPaletteItem *>(addItem(moduleID, BaseCommand::SingleView, viewGeometry, id, modelIndex, originalModelIndex, NULL));
	
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

	foreach (ConnectorItem * toConnectorItem, sitem->connector0()->connectedToItems()) {
		Wire * w = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
		if (w == NULL) continue;

		QList<Wire *> chained;
		QList<ConnectorItem *> ends;
		QList<ConnectorItem *> uniqueEnds;
		w->collectChained(chained, ends, uniqueEnds);
		makeWiresChangeConnectionCommands(chained, parentCommand);
		foreach (Wire * c, chained) {
			makeDeleteItemCommand(c, BaseCommand::CrossView, parentCommand);
		}
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
