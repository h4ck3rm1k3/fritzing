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
#include "tracewire.h"
#include "virtualwire.h"
#include "waitpushundostack.h"
#include "autorouter1.h"

PCBSketchWidget::PCBSketchWidget(ItemBase::ViewIdentifier viewIdentifier, QWidget *parent)
    : PCBSchematicSketchWidget(viewIdentifier, parent)
{
	m_viewName = QObject::tr("PCB View");
	m_netCount = m_netRoutedCount = m_connectorsLeftToRoute = m_jumperCount = 0;
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
	viewLayer = m_viewLayers.value(ViewLayer::Keepout);
	viewLayer->action()->setEnabled(false);
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
				Wire * newWire = makeOneRatsnestWire(source, dest, ratsnestCommand);
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
/*
void PCBSketchWidget::checkAutorouted()
{
	// TODO: the code below is mostly redundant to the code in updateRatsnestStatus

	bool autorouted = true;
	QList<ConnectorItem *> allConnectorItems;
	foreach (QGraphicsItem * item, scene()->items()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
		if (connectorItem == NULL) continue;

		if (connectorItem->attachedToItemType() != ModelPart::Part && connectorItem->attachedToItemType() != ModelPart::Board) continue;
		allConnectorItems.append(connectorItem);
	}

	while (allConnectorItems.count() > 0) {
		ConnectorItem * connectorItem = allConnectorItems.takeFirst();
		QList<ConnectorItem *> ratPartsConnectorItems;
		ratPartsConnectorItems.append(connectorItem);
		ConnectorItem::collectEqualPotentialParts(ratPartsConnectorItems, ViewGeometry::RatsnestFlag);

		QList<ConnectorItem *> tracePartsConnectorItems;
		tracePartsConnectorItems.append(connectorItem);
		ConnectorItem::collectEqualPotentialParts(tracePartsConnectorItems, ViewGeometry::JumperFlag | ViewGeometry::TraceFlag);
		if (tracePartsConnectorItems.count() != ratPartsConnectorItems.count()) {
			autorouted = false;
			allConnectorItems.clear();
			break;
		}

		foreach (ConnectorItem * ci, ratPartsConnectorItems) {
			// don't check these parts again
			allConnectorItems.removeOne(ci);
			DebugDialog::debug(QString("allparts count %1").arg(allConnectorItems.count()) );
		}
	}


	if (autorouted) {
		// TODO need to figure out which net each wire belongs to
		// or save the ratsnest wires so they can simply be reloaded
		DebugDialog::debug("autorouted");
		foreach (QGraphicsItem * item, scene()->items()) {
			Wire * wire = dynamic_cast<Wire *>(item);
			if (wire == NULL) continue;

			if (wire->getRatsnest()) {
				wire->setRouted(true);
				wire->setOpacity(ROUTED_OPACITY);
			}
		}
	}
}
*/

ViewLayer::ViewLayerID PCBSketchWidget::multiLayerGetViewLayerID(ModelPart * modelPart, QString & layerName) {
	Q_UNUSED(layerName);
	Q_UNUSED(modelPart);
	return ViewLayer::Copper0;
}

QString PCBSketchWidget::renderToSVG(qreal printerScale) {

	int width = scene()->width();
	int height = scene()->height();
	qreal trueWidth = width / printerScale;
	qreal trueHeight = height / printerScale;
	static qreal dpi = 1000;

	QString outputSVG;
	QString header = QString("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?> "
							 "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns=\"http://www.w3.org/2000/svg\" "
							 "version=\"1.2\" baseProfile=\"tiny\" "
							 "x=\"0in\" y=\"0in\" width=\"%1in\" height=\"%2in\" "
							 "viewBox=\"0 0 %3 %4\" >")
						.arg(trueWidth)
						.arg(trueHeight)
						.arg(trueWidth * dpi)
						.arg(trueHeight * dpi);
	outputSVG += header;

	QHash<QString, SvgFileSplitter *> svgHash;

	foreach (QGraphicsItem * item, scene()->items()) {
		PaletteItem * paletteItem = dynamic_cast<PaletteItem *>(item);
		if (paletteItem != NULL) {
			QString path = paletteItem->filename();
			DebugDialog::debug(QString("path: %1").arg(path));
			SvgFileSplitter * splitter = svgHash.value(path, NULL);
			if (splitter == NULL) {
				splitter = new SvgFileSplitter();
				bool result = splitter->split(path, "copper0");
				if (!result) {
					delete splitter;
					continue;
				}
				result = splitter->normalize(dpi, "copper0");
				if (!result) {
					delete splitter;
					continue;
				}
				svgHash.insert(path, splitter);
			}
			QPointF loc = paletteItem->scenePos();
			loc.setX(loc.x() * dpi / printerScale);
			loc.setY(loc.y() * dpi / printerScale);

			QString itemSvg = splitter->elementString("copper0");

			if (!paletteItem->transform().isIdentity()) {
				QTransform transform = paletteItem->transform();
				itemSvg = QString("<g transform=\"matrix(%1,%2,%3,%4,%5,%6)\" >")
					.arg(transform.m11())
					.arg(transform.m12())
					.arg(transform.m21())
					.arg(transform.m22())
					.arg(transform.dx())
					.arg(transform.dy())
					.append(itemSvg)
					.append("</g>");
			}
			if (loc.x() != 0 || loc.y() != 0) {
				itemSvg = QString("<g transform=\"translate(%1,%2)\" >")
					.arg(loc.x())
					.arg(loc.y())
					.append(itemSvg)
					.append("</g>");
			}

			outputSVG.append(itemSvg);

			/*
			// TODO:  deal with rotations and flips
			QString shifted = splitter->shift(loc.x(), loc.y(), "copper0");
			outputSVG.append(shifted);
			splitter->shift(-loc.x(), -loc.y(), "copper0");
			*/
		}
		else {
			TraceWire * wire = dynamic_cast<TraceWire *>(item);
			if (wire == NULL) continue;

			QLineF line = wire->getPaintLine();
			QPointF p1 = wire->pos() + line.p1();
			QPointF p2 = wire->pos() + line.p2();
			p1.setX(p1.x() * dpi / printerScale);
			p1.setY(p1.y() * dpi / printerScale);
			p2.setX(p2.x() * dpi / printerScale);
			p2.setY(p2.y() * dpi / printerScale);
			QString lineString = QString("<line style=\"stroke-linecap: round\" stroke=\"black\" x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke-width=\"%5\" />")
							.arg(p1.x())
							.arg(p1.y())
							.arg(p2.x())
							.arg(p2.y())
							.arg(wire->width() * dpi / printerScale);
			outputSVG.append(lineString);
		}
	}

	outputSVG += "</svg>";


	foreach (SvgFileSplitter * splitter, svgHash.values()) {
		delete splitter;
	}

	return outputSVG;

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
	QString commandString = tr("Create Jumper from this Ratsnest Wire");
	QString colorString = tr("jumper");
	createJumperOrTrace(commandString, ViewGeometry::JumperFlag, colorString);
	ensureLayerVisible(ViewLayer::Jumperwires);
}

void PCBSketchWidget::createTrace() {
	QString commandString = tr("Create Trace from this Ratsnest Wire");
	QString colorString = tr("trace");
	createJumperOrTrace(commandString, ViewGeometry::TraceFlag, colorString);
	ensureLayerVisible(ViewLayer::Copper0);
}

void PCBSketchWidget::createJumperOrTrace(const QString & commandString, ViewGeometry::WireFlag flag, const QString & colorString)
{
	QList<QGraphicsItem *> items = scene()->selectedItems();
	if (items.count() != 1) return;

	Wire * wire = dynamic_cast<Wire *>(items[0]);
	if (wire == NULL) return;

	QList<ConnectorItem *> ends;
	Wire * jumperOrTrace = NULL;
	if (wire->getRatsnest()) {
		jumperOrTrace = wire->findJumperOrTraced(ViewGeometry::JumperFlag | ViewGeometry::TraceFlag, ends);
	}
	else {
		jumperOrTrace = wire;
	}

	if (jumperOrTrace != NULL) {
		if (jumperOrTrace->hasFlag(flag)) {
			return;
		}

		QList<Wire *> chained;
		QList<ConnectorItem *> uniqueEnds;
		jumperOrTrace->collectChained(chained, ends, uniqueEnds);
	}

	QUndoCommand * parentCommand = new QUndoCommand(commandString);

	if (jumperOrTrace != NULL) {
		makeDeleteItemCommand(jumperOrTrace, parentCommand);
	}

	long newID = createWire(ends[0], ends[1], flag, false, false, BaseCommand::SingleView, parentCommand);
	new WireColorChangeCommand(this, newID, colorString, colorString, UNROUTED_OPACITY, UNROUTED_OPACITY, parentCommand);
	new WireWidthChangeCommand(this, newID, 3, 3, parentCommand);
	makeChangeRoutedCommand(wire, true, ROUTED_OPACITY, parentCommand);

	new CleanUpWiresCommand(this, parentCommand);
	m_undoStack->push(parentCommand);
}

void PCBSketchWidget::excludeFromAutoroute()
{
	foreach (QGraphicsItem * item, scene()->selectedItems()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;

		if (wire->getTrace() || wire->getJumper()) {
			wire->setAutoroutable(!wire->getAutoroutable());
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
	Autorouter1::clearTraces(this, true, parentCommand);
	updateRatsnestStatus(NULL, parentCommand);
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

		emit routingStatusSignal(netCount, netRoutedCount, connectorsLeftToRoute, jumperCount);

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

