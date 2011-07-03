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

#include "schematicsketchwidget.h"
#include "../debugdialog.h"
#include "../items/virtualwire.h"
#include "../items/symbolpaletteitem.h"
#include "../items/tracewire.h"
#include "../connectors/connectoritem.h"
#include "../waitpushundostack.h"
#include "../items/moduleidnames.h"
#include "../fsvgrenderer.h"

#include <limits>

static QString SchematicTraceColor = "schematic black";

static const qreal TraceWidthMils = 33.3333;

bool sameGround(ConnectorItem * c1, ConnectorItem * c2) 
{
	bool c1Grounded = c1->isGrounded();
	bool c2Grounded = c2->isGrounded();
			
	return (c1Grounded == c2Grounded);
}

SchematicSketchWidget::SchematicSketchWidget(ViewIdentifierClass::ViewIdentifier viewIdentifier, QWidget *parent)
    : PCBSketchWidget(viewIdentifier, parent)
{
	m_shortName = QObject::tr("schem");
	m_viewName = QObject::tr("Schematic View");
	m_standardBackgroundColor = QColor(255,255,255);
	initBackgroundColor();

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
	if (viewGeometry.getAnyTrace()) {
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
		wire->setPenWidth(0.5, this);
		wire->setColorString("schematicGrey", 0.7);
	}
	else {
		wire->setPenWidth(getTraceWidth(), this);
		wire->setColorString("schematic black", 1.0);
	}
}

bool SchematicSketchWidget::autorouteTypePCB() {
	return false;
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

qreal SchematicSketchWidget::getRatsnestOpacity(bool routed) {
	return (routed ? 0.1 : 0.7);
}

bool SchematicSketchWidget::usesJumperItem() {
	return false;
}

void SchematicSketchWidget::setClipEnds(ClipableWire * vw, bool) {
	vw->setClipEnds(false);
}

void SchematicSketchWidget::getBendpointWidths(Wire * wire, qreal width, qreal & bendpointWidth, qreal & bendpoint2Width, bool & negativeOffsetRect) 
{
	Q_UNUSED(wire);
	bendpointWidth = -width - 1;
	bendpoint2Width = width + 3;
	negativeOffsetRect = true;
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
		case ModelPart::Logo:
		case ModelPart::Via:
		case ModelPart::Hole:
			// don't need to see the breadboard in the other views
			// but it's there so connections can be more easily synched between views
			itemBase->setVisible(false);
			itemBase->setEverVisible(false);
			return;
		default:
			break;
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
		case ModelPart::Via:
		case ModelPart::Hole:
			return false;
		case ModelPart::Symbol:
			return true;
		default:
			break;
	}

	if (modelPart->moduleID().endsWith(ModuleIDNames::SchematicFrameModuleIDName)) return true;

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

		connectorItems.append(connectorItem);
		connectorItem->setMarked(false);
	}

	foreach (ConnectorItem * connectorItem, connectorItems) {
		if (!connectorItem->marked()) {
			connectorItem->restoreColor(false, 0, false);
		}
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
	if (moduleID.compare(ModuleIDNames::GroundModuleIDName) == 0) {
		gotV = true;
	}
	else if (moduleID.compare(ModuleIDNames::PowerModuleIDName) == 0) {
		gotV = true;
		v = SymbolPaletteItem::DefaultVoltage;
	}
	else if (moduleID.compare(ModuleIDNames::JustPowerModuleIDName) == 0) {
		gotV = true;
		v = SymbolPaletteItem::DefaultVoltage;
	}

	if (!gotV) {
		return addItemCommand;
	}

	// create the item temporarily, only in order to call makeModifiedWire, then delete it
	SymbolPaletteItem * newSymbol = dynamic_cast<SymbolPaletteItem *>(addItem(moduleID, viewLayerSpec, BaseCommand::SingleView, viewGeometry, id, modelIndex, NULL));
	
	foreach (QGraphicsItem * item, scene()->items()) {
		SymbolPaletteItem * symbol = dynamic_cast<SymbolPaletteItem *>(item);
		if (symbol == NULL) continue;
		if (symbol == newSymbol) continue;					// don't connect self to self
		if (symbol == m_droppingItem) continue;				// the drag item

		if (symbol->voltage() == v && sameGround(newSymbol->connector0(), symbol->connector0())) {
			makeModifiedWire(newSymbol->connector0(), symbol->connector0(), crossViewType, ViewGeometry::NormalFlag, parent); 
		}

		// connector1 is always ground
		// connector0 may be ground or power

		if (symbol->connector1() != NULL && v == 0 && sameGround(newSymbol->connector0(), symbol->connector1())) {
			makeModifiedWire(newSymbol->connector0(), symbol->connector1(), crossViewType, ViewGeometry::NormalFlag, parent); 
		}

		if (newSymbol->connector1() != NULL) {
			if (symbol->voltage() == 0 && sameGround(newSymbol->connector1(), symbol->connector0())) {
				makeModifiedWire(newSymbol->connector1(), symbol->connector0(), crossViewType, ViewGeometry::NormalFlag, parent); 
			}
			if (symbol->connector1() != NULL) {
				makeModifiedWire(newSymbol->connector1(), symbol->connector1(), crossViewType, ViewGeometry::NormalFlag, parent); 
			}
		}
	}

	deleteItem(newSymbol, false, false, false);				// WARNING: don't delete the model part here...

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

	if (sitem->moduleID().compare("ground symbol", Qt::CaseInsensitive) == 0) return;
	if (v == sitem->voltage()) return;

	QUndoCommand * parentCommand =  new QUndoCommand();
	parentCommand->setText(tr("Change voltage from %1 to %2").arg(sitem->voltage()).arg(v));

	new CleanUpWiresCommand(this, CleanUpWiresCommand::UndoOnly, parentCommand);

	QList<Wire *> done;
	foreach (ConnectorItem * toConnectorItem, sitem->connector0()->connectedToItems()) {
		Wire * w = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
		if (w == NULL) continue;
		if (done.contains(w)) continue;

		QList<ConnectorItem *> ends;
		removeWire(w, ends, done, parentCommand);
	}

	new SetPropCommand(this, item->id(), "voltage", QString::number(sitem->voltage()), QString::number(v), true, parentCommand);
	
	foreach (QGraphicsItem * item, scene()->items()) {
		SymbolPaletteItem * other = dynamic_cast<SymbolPaletteItem *>(item);
		if (other == NULL) continue;
		if (other == sitem) continue;

		if (other->voltage() == v && sameGround(sitem->connector0(), other->connector0())) {
			this->makeModifiedWire(sitem->connector0(), other->connector0(), BaseCommand::CrossView, ViewGeometry::NormalFlag, parentCommand);
		}
	}

	new CleanUpWiresCommand(this, CleanUpWiresCommand::RedoOnly, parentCommand);

	m_undoStack->push(parentCommand);
}

double SchematicSketchWidget::defaultGridSizeInches() {
	return 0.3;
}

ViewLayer::ViewLayerID SchematicSketchWidget::getLabelViewLayerID(ViewLayer::ViewLayerSpec) {
	return ViewLayer::SchematicLabel;
}

const QString & SchematicSketchWidget::traceColor(ConnectorItem *) {
	if (m_lastColorSelected.isEmpty()) return SchematicTraceColor;

	else return m_lastColorSelected;
}

const QString & SchematicSketchWidget::traceColor(ViewLayer::ViewLayerSpec) {
	return SchematicTraceColor;
}

long SchematicSketchWidget::setUpSwap(ItemBase * itemBase, long newModelIndex, const QString & newModuleID, ViewLayer::ViewLayerSpec viewLayerSpec, bool master, bool noFinalChangeWiresCommand, QUndoCommand * parentCommand)
{
	return SketchWidget::setUpSwap(itemBase, newModelIndex, newModuleID, viewLayerSpec, master, noFinalChangeWiresCommand, parentCommand);
}


bool SchematicSketchWidget::isInLayers(ConnectorItem * connectorItem, ViewLayer::ViewLayerSpec viewLayerSpec) {
	Q_UNUSED(connectorItem);
	Q_UNUSED(viewLayerSpec);
	return true;
}

bool SchematicSketchWidget::routeBothSides() {
	return false;
}

QPoint SchematicSketchWidget::calcFixedToCenterItemOffset(const QRect & viewPortRect, const QSizeF & helpSize) {
	return SketchWidget::calcFixedToCenterItemOffset(viewPortRect, helpSize);
}

void SchematicSketchWidget::addDefaultParts() {
	SketchWidget::addDefaultParts();
}

bool SchematicSketchWidget::sameElectricalLayer2(ViewLayer::ViewLayerID, ViewLayer::ViewLayerID) {
	// schematic is always one layer
	return true;
}

qreal SchematicSketchWidget::getKeepout() {
	return 0.1 * FSvgRenderer::printerScale();
}

bool SchematicSketchWidget::acceptsTrace(const ViewGeometry & viewGeometry) {
	return viewGeometry.getSchematicTrace();
}

ViewGeometry::WireFlag SchematicSketchWidget::getTraceFlag() {
	return ViewGeometry::SchematicTraceFlag;
}

qreal SchematicSketchWidget::getTraceWidth() {
	return FSvgRenderer::printerScale() * TraceWidthMils / 1000;
}

qreal SchematicSketchWidget::getAutorouterTraceWidth() {
	return getTraceWidth();
}


void SchematicSketchWidget::extraRenderSvgStep(ItemBase * itemBase, QPointF offset, qreal dpi, qreal printerScale, QString & outputSvg)
{
	TraceWire * traceWire = dynamic_cast<TraceWire *>(itemBase);
	if (traceWire == NULL) return;

	if (traceWire->connector0()->isBigDot()) {
		qreal r = traceWire->connector0()->rect().width();
		outputSvg += makeCircleSVG(traceWire->connector0()->sceneAdjustedTerminalPoint(NULL), r, offset, dpi, printerScale);
	}
	if (traceWire->connector1()->isBigDot()) {
		qreal r = traceWire->connector0()->rect().width();
		outputSvg += makeCircleSVG(traceWire->connector1()->sceneAdjustedTerminalPoint(NULL), r, offset, dpi, printerScale);
	}

}

QString SchematicSketchWidget::makeCircleSVG(QPointF p, qreal r, QPointF offset, qreal dpi, qreal printerScale)
{
	qreal cx = (p.x() - offset.x()) * dpi / printerScale;
	qreal cy = (p.y() - offset.y()) * dpi / printerScale;
	qreal rr = r * dpi / printerScale;

	QString stroke = "black";
	return QString("<circle  fill=\"black\" cx=\"%1\" cy=\"%2\" r=\"%3\" stroke-width=\"0\" stroke=\"none\" />")
			.arg(cx)
			.arg(cy)
			.arg(rr);
}

QString SchematicSketchWidget::generateCopperFillUnit(ItemBase * itemBase, QPointF whereToStart)
{
	Q_UNUSED(itemBase);
	Q_UNUSED(whereToStart);
	return "";
}

