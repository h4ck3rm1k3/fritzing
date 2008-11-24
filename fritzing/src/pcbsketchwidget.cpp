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

$Revision: 1617 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-22 20:32:44 +0100 (Sat, 22 Nov 2008) $

********************************************************************/


#include "pcbsketchwidget.h"
#include "debugdialog.h"
#include "busconnectoritem.h"

PCBSketchWidget::PCBSketchWidget(ItemBase::ViewIdentifier viewIdentifier, QWidget *parent, int size, int minSize)
    : PCBSchematicSketchWidget(viewIdentifier, parent, size, minSize)
{
}

void PCBSketchWidget::cleanUpWire(Wire * wire, QList<Wire *> & wires)
{
	Q_UNUSED(wires);
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


void PCBSketchWidget::makeWires(QList<ConnectorItem *> & partsConnectorItems, QList <Wire *> & ratsnestWires, Wire * & modelWire)
{
	int count = partsConnectorItems.count();
	for (int i = 0; i < count - 1; i++) {
		ConnectorItem * source = partsConnectorItems[i];
		for (int j = i + 1; j < count; j++) {
			ConnectorItem * dest = partsConnectorItems[j];
			// if you can't get from i to j via wires, then add a virtual ratsnest wire
			Wire* tempWire = source->wiredTo(dest, ViewGeometry::RatsnestFlag);
			if (tempWire == NULL) {
				Wire * newWire = makeOneRatsnestWire(source, dest);
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
		QList<ConnectorItem *> connectorItems;
		QList<ConnectorItem *> ratPartsConnectorItems;
		QList<ConnectorItem *> tracePartsConnectorItems;
		QList<BusConnectorItem *> busConnectorItems;
		connectorItems.append(allConnectorItems[0]);
		BusConnectorItem::collectEqualPotential(connectorItems, busConnectorItems, true, ViewGeometry::RatsnestFlag);
		BusConnectorItem::collectParts(connectorItems, ratPartsConnectorItems);

		connectorItems.clear();
		busConnectorItems.clear();
		connectorItems.append(allConnectorItems[0]);
		BusConnectorItem::collectEqualPotential(connectorItems, busConnectorItems, true, ViewGeometry::JumperFlag | ViewGeometry::TraceFlag);
		BusConnectorItem::collectParts(connectorItems, tracePartsConnectorItems);
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
				wire->setOpacity(0.35);
			}
		}
	}
}

ViewLayer::ViewLayerID PCBSketchWidget::multiLayerGetViewLayerID(ModelPart * modelPart, QString & layerName) {
	Q_UNUSED(layerName);
	Q_UNUSED(modelPart);
	return ViewLayer::Copper0;
}

