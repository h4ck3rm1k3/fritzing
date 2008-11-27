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


#include "pcbschematicsketchwidget.h"
#include "debugdialog.h"
#include "autorouter1.h"

PCBSchematicSketchWidget::PCBSchematicSketchWidget(ItemBase::ViewIdentifier viewIdentifier, QWidget *parent, int size, int minSize)
    : SketchWidget(viewIdentifier, parent, size, minSize)
{
}

void PCBSchematicSketchWidget::setNewPartVisible(ItemBase * itemBase) {
	if (itemBase->itemType() == ModelPart::Breadboard) {
		// don't need to see the breadboard in the other views
		// but it's there so connections can be more easily synched between views
		itemBase->setVisible(false);
	}
}

void PCBSchematicSketchWidget::redrawRatsnest(QHash<long, ItemBase *> & newItems) {
	QMultiHash<ConnectorItem *, ConnectorItem *> allConnectors;
	foreach (ItemBase * newItem, newItems.values()) {
		foreach (QGraphicsItem * childItem, newItem->childItems()) {
			ConnectorItem * fromConnectorItem = dynamic_cast<ConnectorItem *>(childItem);
			if (fromConnectorItem == NULL) continue;

			foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
				/*
				DebugDialog::debug(QString("restoring ratsnest: %1 %2, %3 %4")
					.arg(fromConnectorItem->attachedToTitle())
					.arg(fromConnectorItem->connectorStuffID())
					.arg(toConnectorItem->attachedToTitle())
					.arg(toConnectorItem->connectorStuffID())
					);
				*/
				allConnectors.insert(fromConnectorItem, toConnectorItem);
			}
		}
	}

	// have to store these all up in a table and deal with separarately
	// if you deal with them in the loop above, then connectors are being added/destroyed
	// while looping and that causes crashes.
	foreach (ConnectorItem * fromConnectorItem, allConnectors.uniqueKeys()) {
		foreach (ConnectorItem * toConnectorItem, allConnectors.values(fromConnectorItem)) {
			dealWithRatsnest(fromConnectorItem, toConnectorItem, true);
		}
	}
}

bool PCBSchematicSketchWidget::canDropModelPart(ModelPart * modelPart) {
	if (modelPart->itemType() == ModelPart::Wire || modelPart->itemType() == ModelPart::Breadboard) {
		// can't drag and drop these parts in these views
		return false;
	}

	return true;
}

void PCBSchematicSketchWidget::dealWithRatsnest(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, bool connect) {

	if (fromConnectorItem->attachedToItemType() == ModelPart::Wire) {
		Wire * wire = dynamic_cast<Wire *>(fromConnectorItem->attachedTo());
		if (wire->getRatsnest() || wire->getJumper() || wire->getTrace()) {
			// don't make further ratsnest's from ratsnest
			return;
		}
	}

	if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
		Wire * wire = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
		if (wire->getRatsnest() || wire->getJumper() || wire->getTrace()) {
			// don't make further ratsnest's from ratsnest
			return;
		}
	}

	DebugDialog::debug(QString("deal with ratsnest %1 %2 %3, %4 %5 %6")
		.arg(fromConnectorItem->attachedToTitle())
		.arg(fromConnectorItem->attachedToID())
		.arg(fromConnectorItem->connectorStuffID())
		.arg(toConnectorItem->attachedToTitle())
		.arg(toConnectorItem->attachedToID())
		.arg(toConnectorItem->connectorStuffID())
	);

	if (connect) {
		QList<ConnectorItem *> connectorItems;
		QList<ConnectorItem *> partsConnectorItems;
		connectorItems.append(fromConnectorItem);
		ConnectorItem::collectEqualPotential(connectorItems);
		ConnectorItem::collectParts(connectorItems, partsConnectorItems);

		QList <Wire *> ratsnestWires;
		Wire * modelWire = NULL;

		makeWires(partsConnectorItems, ratsnestWires, modelWire);

		if (ratsnestWires.count() > 0) {
			const QColor * color = NULL;
			if (modelWire) {
				color = modelWire->color();
			}
			else {
				color = Wire::netColor(m_viewIdentifier);
			}
			foreach (Wire * wire, ratsnestWires) {
				QColor colorAsQColor = (QColor) *color;
				wire->setColor(colorAsQColor, wire->getRouted() ? 0.35 : 1.0);
			}
		}

		return;
	}

}

void PCBSchematicSketchWidget::updateRatsnestStatus() 
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

	QSet<Wire *> deleteWires;
	foreach (QGraphicsItem * item, scene()->items()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		if (!wire->getRatsnest()) continue;

		// if a ratsnest is connecting two items that aren't connected
		// delete the ratsnest

		// TODO: the code below incorrectly deletes chained ratsnest wires

		ConnectorItem * c0 = wire->connector0();
		ConnectorItem * c1 = wire->connector1();
		ConnectorItem * c0to = c0->firstConnectedToIsh();
		ConnectorItem * c1to = c1->firstConnectedToIsh();
		foreach (QList<ConnectorItem *>* list, allPartConnectorItems) {
			if ((list->contains(c0to) && !list->contains(c1to)) || (list->contains(c1to) && !list->contains(c0to))) {
				deleteWires.insert(wire);
			}
		}
	}

	foreach (Wire * wire, deleteWires) {
		deleteItem(wire, false, false);
	}


	foreach (QList<ConnectorItem *>* list, allPartConnectorItems) {
		delete list;
	}

	// divide jumpercount by two since we counted both ends of each wire
	emit routingStatusSignal(netCount, netRoutedCount, connectorsLeftToRoute, jumperCount / 2);
}
