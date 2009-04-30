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

#include "schematicsketchwidget.h"
#include "autorouter1.h"
#include "debugdialog.h"
#include "items/virtualwire.h"
#include "connectoritem.h"

#include <limits>

static int MAX_INT = std::numeric_limits<int>::max();

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

SchematicSketchWidget::SchematicSketchWidget(ViewIdentifierClass::ViewIdentifier viewIdentifier, QWidget *parent)
    : PCBSchematicSketchWidget(viewIdentifier, parent)
{
	m_viewName = QObject::tr("Schematic View");
}

void SchematicSketchWidget::setWireVisible(Wire * wire)
{
	wire->setVisible(wire->getRatsnest());
	//wire->setVisible(true);
}

void SchematicSketchWidget::addViewLayers() {
	addSchematicViewLayers();
}

void SchematicSketchWidget::updateRatsnestStatus(CleanUpWiresCommand * command, QUndoCommand * undoCommand) {
	Q_UNUSED(undoCommand);
	if (command) {
		QHash<ConnectorItem *, int> indexer;
		QList< QList<ConnectorItem *>* > allPartConnectorItems;
		Autorouter1::collectAllNets(this, indexer, allPartConnectorItems);
		removeRatsnestWires(allPartConnectorItems, command);
		foreach (QList<ConnectorItem *>* list, allPartConnectorItems) {
			delete list;
		}
	}
}

void SchematicSketchWidget::dealWithRatsnest(long fromID, const QString & fromConnectorID, 
								  long toID, const QString & toConnectorID,
								  bool connect, class RatsnestCommand * ratsnestCommand, bool doEmit)
{
	if (!connect) return;

	ConnectorItem * fromConnectorItem = NULL;
	ConnectorItem * toConnectorItem = NULL;
	if (dealWithRatsnestAux(fromConnectorItem, toConnectorItem, fromID, fromConnectorID, 
							toID, toConnectorID,
							connect, ratsnestCommand, doEmit)) 
	{
		return;
	}

	bool useFrom = false;
	bool useTo = false;
	if ((fromConnectorItem->attachedToItemType() == ModelPart::Part) ||
		(fromConnectorItem->attachedToItemType() == ModelPart::Board) ||
		(fromConnectorItem->attachedToItemType() == ModelPart::ResizableBoard))
	{
		useFrom = true;
	}
	if ((toConnectorItem->attachedToItemType() == ModelPart::Part) ||
		(toConnectorItem->attachedToItemType() == ModelPart::Board) ||
		(toConnectorItem->attachedToItemType() == ModelPart::ResizableBoard))
	{
		useTo = true;
	}

	if (useFrom && useTo) {  
		// plugged part into arduino, for example
		makeOneRatsnestWire(fromConnectorItem, toConnectorItem, ratsnestCommand, true);
		return;
	}

	if (useFrom && toConnectorItem->attachedToItemType() == ModelPart::Wire) {
		ConnectorItem * newTo = tryWire(toConnectorItem, fromConnectorItem);
		if (newTo != NULL) {
			// drag a part onto a wire which is already connected to a part at the other end, for example
			makeOneRatsnestWire(fromConnectorItem, newTo, ratsnestCommand, true);
			return;
		}
	}
	else if (useTo && fromConnectorItem->attachedToItemType() == ModelPart::Wire) {
		ConnectorItem * newFrom = tryWire(fromConnectorItem, toConnectorItem);
		if (newFrom != NULL) {
			// drew a wire from one part to another, for example
			makeOneRatsnestWire(toConnectorItem, newFrom, ratsnestCommand, true);
			return;
		}
	}

	QList<ConnectorItem *> connectorItems;
	connectorItems << fromConnectorItem << toConnectorItem;
	ConnectorItem::collectEqualPotential(connectorItems);
	QList<ConnectorItem *> partsConnectorItems;
	ConnectorItem::collectParts(connectorItems, partsConnectorItems);
	if (useFrom) {
		ConnectorItem * newTo = tryParts(fromConnectorItem, toConnectorItem, partsConnectorItems);
		if (newTo != NULL) { 
			// plugged two parts into breadboard bus, for example
			makeOneRatsnestWire(fromConnectorItem, newTo, ratsnestCommand, true);
			return;
		}
	}
	else if (useTo) {
		ConnectorItem * newFrom = tryParts(toConnectorItem, fromConnectorItem, partsConnectorItems);
		if (newFrom != NULL) {
			// drawing a wire from a breadboard to a part, with another part already connected to the same bus, for example
			makeOneRatsnestWire(toConnectorItem, newFrom, ratsnestCommand, true);
			return;
		}
	}
	else {
		for (int i = 0; i < partsConnectorItems.count() - 1; i++) {
			ConnectorItem * ci = partsConnectorItems[i];
			for (int j = i + 1; j < partsConnectorItems.count(); j++) {
				ConnectorItem * cj = partsConnectorItems[j];
				if (ci->bus() != NULL && ci->bus() == cj->bus()) continue;
				if (ci->wiredTo(cj, ViewGeometry::RatsnestFlag)) continue;

				if (alreadyOnBus(ci, cj)) continue;
				if (alreadyOnBus(cj, ci)) continue;
				// for example, wire hooks up two buses on breadboard, with parts attached to each bus
				// or wire connects to wire which connects two parts
				makeOneRatsnestWire(ci, cj, ratsnestCommand, true);
				return;
			}
		}

		return;
	}
}

bool SchematicSketchWidget::alreadyOnBus(ConnectorItem * busCandidate, ConnectorItem * otherCandidate) {
	if (busCandidate->bus() == NULL) return false;

	foreach (ConnectorItem * toConnectorItem, otherCandidate->connectedToItems()) {
		if (toConnectorItem->bus() == busCandidate->bus()) return true;
	}

	return false;
}

ConnectorItem * SchematicSketchWidget::tryParts(ConnectorItem * otherConnectorItem, ConnectorItem * wireConnectorItem, QList<ConnectorItem *> partsConnectorItems)
{
	Q_UNUSED(wireConnectorItem);
	foreach (ConnectorItem * connectorItem, partsConnectorItems) {
		if (connectorItem == otherConnectorItem) continue;
		if (connectorItem->attachedTo() == otherConnectorItem->attachedTo() &&
			connectorItem->bus() != NULL &&
			connectorItem->bus() == otherConnectorItem->bus()) continue;
		if (alreadyOnBus(otherConnectorItem, connectorItem)) continue;
		if (alreadyOnBus(connectorItem, otherConnectorItem)) continue;


		return connectorItem;
	}

	return NULL;

}

ConnectorItem * SchematicSketchWidget::tryWire(ConnectorItem * wireConnectorItem, ConnectorItem * otherConnectorItem)
{
	QList<Wire *> chained;
	QList<ConnectorItem *> ends;
	QList<ConnectorItem *> uniqueEnds;
	dynamic_cast<Wire *>(wireConnectorItem->attachedTo())->collectChained(chained, ends, uniqueEnds);
	foreach (ConnectorItem * end, ends) {
		if (end == wireConnectorItem) continue;
		if (end == otherConnectorItem) continue;
		if (end->attachedToItemType() == ModelPart::Breadboard) continue;
		if (end->attachedTo() == otherConnectorItem->attachedTo() &&
			end->bus() != NULL &&
			end->bus() == otherConnectorItem->bus()) continue;
		if (alreadyOnBus(otherConnectorItem, end)) continue;
		if (alreadyOnBus(end, otherConnectorItem)) continue;


		return end;
	}

	return NULL;
}

void SchematicSketchWidget::makeWires(QList<ConnectorItem *> & partsConnectorItems, QList <Wire *> & ratsnestWires, Wire * & modelWire, RatsnestCommand * ratsnestCommand)
{
	Q_UNUSED(partsConnectorItems);
	Q_UNUSED(modelWire);
	Q_UNUSED(ratsnestWires);
	Q_UNUSED(ratsnestCommand);

	return;
}

bool SchematicSketchWidget::canDeleteItem(QGraphicsItem * item)
{
	VirtualWire * wire = dynamic_cast<VirtualWire *>(item);
	if (wire != NULL) return true;

	return SketchWidget::canDeleteItem(item);
}

bool SchematicSketchWidget::canCopyItem(QGraphicsItem * item)
{
	VirtualWire * wire = dynamic_cast<VirtualWire *>(item);
	if (wire != NULL) return false;

	return SketchWidget::canDeleteItem(item);
}

bool SchematicSketchWidget::reviewDeletedConnections(QSet<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash *> & deletedConnections, QUndoCommand * parentCommand)
{
	// delete any wires connected to non-wire items
	foreach (ItemBase * item, deletedItems) {
		if (item->itemType() == ModelPart::Wire) continue;

		foreach (QGraphicsItem * childItem, item->childItems()) {
			ConnectorItem * fci = dynamic_cast<ConnectorItem *>(childItem);
			if (fci == NULL) continue;

			foreach (ConnectorItem * tci, fci->connectedToItems()) {
				if (!tci->attachedTo()->getVirtual()) continue;
				if (deletedItems.contains(tci->attachedTo())) continue;

				Wire * wire = dynamic_cast<Wire *>(tci->attachedTo());
				deletedItems.insert(wire);
				ConnectorPairHash * cph = new ConnectorPairHash;
				wire->collectConnectors(*cph, wire->scene());
				deletedConnections.insert(wire, cph);
			}
		}
	}

	QList<ConnectorItem *> affectedEnds;
	QSet<Wire *> insertAfter;
	// if there are schematic ratsnest wires that directly correspond to breadboard (normal) wires
	// delete the normal wires
	foreach (ItemBase * item, deletedItems) {
		if (!item->getVirtual()) {
			continue;
		}

		Wire * wire = dynamic_cast<Wire *>(item);
		QList<ConnectorItem *> ends;
		QList<ConnectorItem *> uniqueEnds;
		QList<Wire *> wires;
		wire->collectChained(wires, ends, uniqueEnds);
		if (ends.count() != 2) {
			foreach (ConnectorItem * ci, ends) { affectedEnds.append(ci); }
			foreach (Wire * w, wires) { insertAfter.insert(w); }
			continue;
		}
		
		Wire * normalWire = ends[0]->wiredTo(ends[1], ViewGeometry::NormalFlag);
		if (normalWire == NULL) {
			foreach (ConnectorItem * ci, ends) { affectedEnds.append(ci); }
			foreach (Wire * w, wires) { insertAfter.insert(w); }
			continue;
		}

		QList<ConnectorItem *> ends2;
		QList<ConnectorItem *> uniqueEnds2;
		QList<Wire *> wires2;
		normalWire->collectChained(wires2, ends2, uniqueEnds2);
		if (ends2.count() != 2) {
			foreach (ConnectorItem * ci, ends) { affectedEnds.append(ci); }
			foreach (Wire * w, wires) { insertAfter.insert(w); }
			continue;
		}

		foreach (Wire * w, wires2) {
			insertAfter.insert(w);
		}

		foreach (Wire * w, wires) {
			deletedConnections.remove(w);
			deletedItems.remove(w);						
			insertAfter.insert(w);
		}
	}

	if (deletedConnections.count() > 0) {
		// note: will need to do this when deleting a connection
		reviewDeletedConnectionsAux(deletedItems, deletedConnections, affectedEnds, parentCommand);
	}

	foreach (Wire * w, insertAfter) {
		// add these "normal" wires to the deleted list; don't forget the connections
		deletedItems.insert(w);
		ConnectorPairHash * connectorHash = new ConnectorPairHash;
		w->collectConnectors(*connectorHash, this->scene());
		deletedConnections.insert(w, connectorHash);
	}

	return true;
}

void SchematicSketchWidget::reviewDeletedConnectionsAux(QSet<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash *> & deletedConnections, 
														QList<ConnectorItem *> & affectedEnds, QUndoCommand * parentCommand)
{
	// disconnect all the rats affected by the delete
	foreach (ConnectorPairHash * connectorHash, deletedConnections.values()) {
		foreach (ConnectorItem * fromConnectorItem,  connectorHash->uniqueKeys()) {
			bool removeAll = fromConnectorItem->attachedTo()->getVirtual();
			foreach (ConnectorItem * toConnectorItem, connectorHash->values(fromConnectorItem)) {
				if (toConnectorItem->attachedTo()->getVirtual() || removeAll) {
					fromConnectorItem->tempRemove(toConnectorItem, false);
					toConnectorItem->tempRemove(fromConnectorItem, false);
				}
			}
		}
	}

	// make a list of the new nets
	// this is not an efficient way to do it, but it will do for now

	// first find all the parts connected by ratsnest wires
	QList< QList<ConnectorItem *> *> allEnds;
	QList<Wire *> visitedWires;
	foreach (QGraphicsItem * item, scene()->items()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		if (!wire->getVirtual()) continue;
		if (visitedWires.contains(wire)) continue;

		QList<Wire *> wires;
		QList<ConnectorItem *> * ends = new QList<ConnectorItem *>;
		QList<ConnectorItem *> uniqueEnds;
		wire->collectChained(wires, *ends, uniqueEnds);
		if (ends->count() > 0) {
			allEnds.append(ends);
		}
		foreach (Wire * w, wires) {
			visitedWires.append(w);
		}
	}

	// now combine them into nets
	for (int i = allEnds.count() - 1; i >= 1; i--) {
		QList<ConnectorItem *> * endsi = allEnds[i];
		for (int j = i - 1; j >= 0; j--) {
			QList<ConnectorItem *> * endsj = allEnds[j];
			foreach (ConnectorItem * ci, *endsi) {
				if (endsj->contains(ci)) {
					foreach (ConnectorItem * cj, *endsj) {
						endsi->append(cj);
					}
					endsj->clear();
					break;
				}
			}
		}
	}

	// now remove the combined-out subnets
	for (int i = allEnds.count() - 1; i >= 0; i--) {
		if (allEnds[i]->count() == 0) {
			allEnds.removeAt(i);
		}
	}

	// now for each of the nets, we only need the ones that have been affected by the deletions
	QList< QList<ConnectorItem *> * > newNets;
	foreach (ConnectorPairHash * connectorHash, deletedConnections.values()) {
		foreach (ConnectorItem * fromConnectorItem,  connectorHash->uniqueKeys()) {
			if (!fromConnectorItem->attachedTo()->getVirtual()) {
				checkInNet(fromConnectorItem, allEnds, newNets);
			}

			foreach (ConnectorItem * toConnectorItem, connectorHash->values(fromConnectorItem)) {
				if (!toConnectorItem->attachedTo()->getVirtual()) {
					checkInNet(toConnectorItem, allEnds, newNets);
				}
			}
		}
	}

	foreach (ConnectorItem * ci, affectedEnds) {
		checkInNet(ci, allEnds, newNets);
	}

	foreach (QList<ConnectorItem *> * net, newNets) {
		DebugDialog::debug("collected net");
		foreach (ConnectorItem * ci, *net) {
			DebugDialog::debug(QString("\t%1 %2").arg(ci->attachedToTitle()).arg(ci->connectorSharedID()));
		}
	}

	// order by size:
	qSort(newNets.begin(), newNets.end(), bySize);

	// again, not the most efficient approach, but it will do for now
	QHash<ConnectorItem *, int> indexer;
	QList< QList<ConnectorItem *>* > oldNets;
	Autorouter1::collectAllNets(this, indexer, oldNets);

	QList< QList<ConnectorItem *> * > visitedOldNets;
	QList< QList<ConnectorItem *> * > netsToChange;
	QList< QList<ConnectorItem *> * > netsToLeave;

	// figure out which old net the new net used to be part of
	// if the newnet is the first (biggest) net, then we don't have move its parts
	foreach (QList<ConnectorItem *> * newNet, newNets) {
		foreach(QList<ConnectorItem *> * oldNet, oldNets) {
			if (oldNet->contains(newNet->at(0))) {
				if (visitedOldNets.contains(oldNet)) {
					netsToChange.append(newNet);
				}
				else {
					visitedOldNets.append(oldNet);
					netsToLeave.append(newNet);
				}
				break;
			}
		}
	}

	ConnectorPairHash tempDisconnectItems;
	ConnectorPairHash moveItems;
	QSet<Wire *> possibleOrphans;
	foreach (QList<ConnectorItem *> * net, netsToChange) {
		reviewNet(*net, false, tempDisconnectItems, moveItems, possibleOrphans, deletedItems, parentCommand);
	}
	foreach (QList<ConnectorItem *> * net, netsToLeave) {
		reviewNet(*net, true, tempDisconnectItems, moveItems, possibleOrphans, deletedItems, parentCommand);
	}

	// get ready to restore any nets we've broken
	foreach (ConnectorItem * fromConnectorItem, tempDisconnectItems.uniqueKeys()) {
		foreach (ConnectorItem * toConnectorItem, tempDisconnectItems.values(fromConnectorItem)) {
			fromConnectorItem->tempRemove(toConnectorItem, false);
			toConnectorItem->tempRemove(fromConnectorItem, false);
		}
	}

	foreach (QList<ConnectorItem *> * net, netsToChange) {
		restoreNet(*net, parentCommand);
	}
	foreach (QList<ConnectorItem *> * net, netsToLeave) {
		restoreNet(*net, parentCommand);
	}
	

	// go through each net to change
	// for each connector
	//		for each connectedto
	//			if it's attached to a bus, collect the other bus connectors
	//			if it's a bus, disconnect part and reconnect other connectors, if necessary

	// clean up
	foreach (QList<ConnectorItem *> * net, allEnds) {
		delete net;
	}
	foreach (QList<ConnectorItem *> * net, oldNets) {
		delete net;
	}

	// check which wires are no longer connected to parts, and delete them
	foreach (Wire * wire, possibleOrphans) {
		QList<ConnectorItem *> ends;
		QList<ConnectorItem *> uniqueEnds;
		QList<Wire *> wires;
		wire->collectChained(wires, ends, uniqueEnds);
		if (ends.count() <= 0) {
			foreach (Wire * w, wires) {
				deletedItems.insert(w);
			}
		}
	}

	if (moveItems.count() > 0) {
		emit schematicDisconnectWireSignal(moveItems, deletedItems, deletedConnections, parentCommand);
	}
}


void SchematicSketchWidget::checkInNet(ConnectorItem * connectorItem, QList< QList<ConnectorItem *> * >  & allEnds, QList< QList<ConnectorItem *> * >  & newNets)
{
	foreach (QList<ConnectorItem *> * net, allEnds) {
		if (net->contains(connectorItem)) {
			if (!newNets.contains(net)) {
				newNets.append(net);
			}
			return;
		}
	}

	QList<ConnectorItem *> * net = new QList<ConnectorItem *>;
	net->append(connectorItem);
	allEnds.append(net);
	newNets.append(net);
}

void SchematicSketchWidget::restoreNet(QList<ConnectorItem *> & net, QUndoCommand * parentCommand) {
	QSet<ConnectorItem *> connected;
	for (int i = 0; i < net.count(); i++) {
		ConnectorItem * fromConnectorItem = net[i];
		if (connected.contains(fromConnectorItem)) continue;

		QList<ConnectorItem *> peers;
		peers.append(fromConnectorItem);
		ConnectorItem::collectEqualPotential(peers);
		foreach (ConnectorItem * peer, peers) {
			connected.insert(peer);
		}

		// connect this sub-net to the next subnet
		for (int j = i + 1; j < net.count(); j++) {
			ConnectorItem * toConnectorItem = net[j];
			if (!connected.contains(toConnectorItem)) {
				createWire(fromConnectorItem, toConnectorItem, ViewGeometry::NoFlag, false, false, BaseCommand::CrossView, parentCommand);
				break;
			}
		}
	}
}

void SchematicSketchWidget::reviewNet(QList<ConnectorItem *> & net, bool leaveAlone, ConnectorPairHash & tempDisconnectItems, 
					ConnectorPairHash & moveItems, QSet<Wire *> & possibleOrphans, QSet<ItemBase *> & deletedItems,
					QUndoCommand * parentCommand)
{
	foreach (ConnectorItem * fromConnectorItem, net) {
		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
			if (net.contains(toConnectorItem)) continue;
			if (deletedItems.contains(toConnectorItem->attachedTo())) continue;

			if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
				Wire * wire = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
				QList<ConnectorItem *> ends;
				QList<ConnectorItem *> uniqueEnds;
				QList<Wire *> wires;
				wire->collectChained(wires, ends, uniqueEnds);
				bool outOfNet = false;
				foreach (ConnectorItem * end, ends) {
					if (!net.contains(end)) {
						outOfNet = true;
						break;
					}
				}
				if (!outOfNet) continue;

				new ChangeConnectionCommand(this, BaseCommand::CrossView,
											fromConnectorItem->attachedToID(), fromConnectorItem->connectorSharedID(),
											toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
											false, true, parentCommand);
				
				possibleOrphans.insert(wire);
				tempDisconnectItems.insert(fromConnectorItem, toConnectorItem);
			}
			else {
				DebugDialog::debug(QString("leave alone %1 %2 %3 %4")
					.arg(fromConnectorItem->attachedToTitle())
					.arg(fromConnectorItem->connectorSharedID())
					.arg(toConnectorItem->attachedToTitle())
					.arg(toConnectorItem->connectorSharedID())
					);

				if (leaveAlone) {
					if (toConnectorItem->attachedToItemType() == ModelPart::Breadboard) {
						continue;
					}
				}

				tempDisconnectItems.insert(fromConnectorItem, toConnectorItem);
				moveItems.insert(fromConnectorItem, toConnectorItem);
			}

			// ratsnests are already disconnected at this point

		}
	}
}

void SchematicSketchWidget::obsolete(QSet<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash *> & deletedConnections, QUndoCommand * parentCommand) 
{
	ConnectorPairHash moveItems;
	QSet<Wire *> deleteWires;
	QSet<Wire *> undeleteWires;
	//ConnectorPairHash moveItems;
	foreach (ItemBase * itemBase, deletedItems) {
		Wire * wire = dynamic_cast<Wire *>(itemBase);
		if (wire == NULL) continue;
		if (!wire->getRatsnest()) continue;
		if (undeleteWires.contains(wire)) continue;

		QList <Wire *> directWires;
		wire->collectDirectWires(directWires);
		foreach (Wire * directWire, directWires) {
			undeleteWires.insert(directWire);
		}

		QList<ConnectorItem *> ends;
		calcDistances(wire, ends);
		if (ends.count() < 2) {
			continue;
		}

		ConnectorItem * end0 = ends[0];
		ConnectorItem * end1 = NULL;
		bool fromConnector0_0 = distances.value(end0)->fromConnector0;
		for (int i = 1; i < ends.count(); i++) {
			if (distances.value(ends[i])->fromConnector0 != fromConnector0_0) {
				end1 = ends[i];
				break;
			}
		}
		clearDistances();

		if (end1 == NULL) {
			// not sure what to do if there's no end1
			continue;
		}

		Wire * extraWire = end0->wiredTo(end1, ViewGeometry::NormalFlag);
		if (extraWire != NULL) {
			// need to delete the real wire from breadboard view
			deleteWires.insert(extraWire);
			continue;
		}

		/*
		if (end1) {
			// try to find a real wire between end0 and end1 to delete
			Wire * extraWire = NULL;
			for (int i = 0; i < ends.count() - 1; i++) {
				for (int j = i + 1; j < ends.count(); j++) {
					extraWire = ;
					if (extraWire != NULL) break;
				}
				if (extraWire != NULL) break;
			}
			if (extraWire != NULL) {
				// need to delete the real wire from breadboard view
				deleteWires.insert(extraWire);
				continue;
			}

		}


		end0 = end1 = NULL;

		// if in earlier runs through this loop we've already chosen to move a particular part,
		// then choose to disconnect other connectors on the same part
		foreach (ConnectorItem * move, moveItems.uniqueKeys()) {
			foreach (ConnectorItem * end, ends) {
				if (end->attachedTo() == move->attachedTo()) {
					end0 = end;
					break;
				}
			}
			if (end0 != NULL) break;
		}

		if (end0 == NULL) {
			end0 = ends[0];
			end1 = ends[1];
		}
		else {
			foreach (ConnectorItem * end, ends) {
				if (end != end0) {
					end1 = end;
					break;
				}
			}
		}

		*/


		moveItems.insert(end0, end1);
	}

	if (moveItems.count() > 0) {
		emit schematicDisconnectWireSignal(moveItems, deletedItems, deletedConnections, parentCommand);
	}

	makeWiresChangeConnectionCommands(undeleteWires.values(), parentCommand);

	foreach (Wire * wire, undeleteWires) {
		makeDeleteItemCommand(wire, BaseCommand::SingleView, parentCommand);
		deletedItems.remove(wire);
	}

	foreach (Wire * wire, deleteWires) {
		if (deletedItems.contains(wire)) continue;

		deletedItems.insert(wire);
		ConnectorPairHash * connectorHash = new ConnectorPairHash;
		wire->collectConnectors(*connectorHash, this->scene());
		deletedConnections.insert(wire, connectorHash);
	}
}

bool SchematicSketchWidget::canChainMultiple() {
	return true;
}

bool SchematicSketchWidget::modifyNewWireConnections(Wire * dragWire, ConnectorItem * fromDragWire, ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, QUndoCommand * parentCommand)
{
	// if needed, find or create a new breadboard to make the connection

	if (fromConnectorItem->attachedToItemType() == ModelPart::Wire && 
		toConnectorItem->attachedToItemType() == ModelPart::Wire)
	{
		ConnectorItem * originalFromConnectorItem = fromConnectorItem;
		ConnectorItem * originalToConnectorItem = toConnectorItem;
		fromConnectorItem = lookForBreadboardConnection(fromConnectorItem);
		toConnectorItem = lookForBreadboardConnection(toConnectorItem);
		if (fromConnectorItem->attachedToItemType() == ModelPart::Breadboard &&
			toConnectorItem->attachedToItemType() == ModelPart::Breadboard)
		{
			// connection can be made with one wire
			makeModifiedWire(dragWire, fromDragWire, originalFromConnectorItem, fromConnectorItem, originalToConnectorItem, toConnectorItem, parentCommand);
		}
		else if (toConnectorItem->attachedToItemType() == ModelPart::Breadboard) {
			makeTwoWires(dragWire, fromDragWire, originalToConnectorItem, toConnectorItem, originalFromConnectorItem, fromConnectorItem, parentCommand);
		}
		else {
			makeTwoWires(dragWire, fromDragWire, originalFromConnectorItem, fromConnectorItem, originalToConnectorItem, toConnectorItem, parentCommand);
		}
		return true;
	}
	else if (fromConnectorItem->attachedToItemType() == ModelPart::Wire) {
		modifyNewWireConnectionsAux(dragWire, fromDragWire, fromConnectorItem, toConnectorItem, parentCommand);
		return true;
	}
	else if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
		modifyNewWireConnectionsAux(dragWire, dragWire->otherConnector(fromDragWire), toConnectorItem, fromConnectorItem, parentCommand);
		return true;
	}

	return false;
}

void SchematicSketchWidget::modifyNewWireConnectionsAux(Wire * dragWire, ConnectorItem * fromDragWire, 
														ConnectorItem * fromConnectorItem,
														ConnectorItem * toConnectorItem, 
														QUndoCommand * parentCommand)
{
	ConnectorItem * originalFromConnectorItem = fromConnectorItem;
	fromConnectorItem = lookForBreadboardConnection(fromConnectorItem);		
	if (fromConnectorItem->attachedToItemType() == ModelPart::Breadboard) {
		makeModifiedWire(dragWire, fromDragWire, originalFromConnectorItem, fromConnectorItem, toConnectorItem, toConnectorItem, parentCommand);
		return;
	}

	makeTwoWires(dragWire, fromDragWire, originalFromConnectorItem, fromConnectorItem, toConnectorItem, toConnectorItem, parentCommand);
}

void SchematicSketchWidget::makeTwoWires(Wire * dragWire, ConnectorItem * fromDragWire, 
										 ConnectorItem * originalFromConnectorItem, ConnectorItem * fromConnectorItem,
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
			new AddItemCommand(this, BaseCommand::CrossView, newBreadboard->modelPart()->moduleID(), newBreadboard->getViewGeometry(), newBreadboard->id(), true, -1, -1, parentCommand);
			m_temporaries.append(newBreadboard);			// puts it on a list to be deleted
		}
	}

	ConnectorItem * nearestPartConnectorItem = findNearestPartConnectorItem(originalFromConnectorItem);
	if (nearestPartConnectorItem == NULL) return;

	// make a wire, from the part nearest to fromConnectorItem, to the breadboard
	toConnectorItem = nearestPartConnectorItem;
	makeModifiedWire(dragWire, fromDragWire, originalFromConnectorItem, fromConnectorItem, originalToConnectorItem, toConnectorItem, parentCommand);

	if (originalToConnectorItem->attachedToItemType() == ModelPart::Wire) {
		originalToConnectorItem = findNearestPartConnectorItem(originalToConnectorItem);
		if (originalToConnectorItem == NULL) return;
	}

	// draw a wire from that bus on the breadboard to the other part (toConnectorItem)
	ConnectorItem * otherPartBusConnectorItem = findEmptyBusConnectorItem(fromConnectorItem);
	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	QPointF fromPos = otherPartBusConnectorItem->sceneAdjustedTerminalPoint();
	viewGeometry.setLoc(fromPos);
	QPointF toPos = originalToConnectorItem->sceneAdjustedTerminalPoint();
	QLineF line(0, 0, toPos.x() - fromPos.x(), toPos.y() - fromPos.y());
	viewGeometry.setLine(line);

	DebugDialog::debug(QString("new second wire %1").arg(newID));

	new AddItemCommand(this, BaseCommand::CrossView, Wire::moduleIDName, viewGeometry, newID, true, -1, -1, parentCommand);
	new CheckStickyCommand(this, BaseCommand::CrossView, newID, false, parentCommand);
	new ChangeConnectionCommand(this, BaseCommand::CrossView,
								newID, "connector0",
								otherPartBusConnectorItem->attachedToID(), otherPartBusConnectorItem->connectorSharedID(),
								true, true, parentCommand);
	new ChangeConnectionCommand(this, BaseCommand::CrossView,
								newID, "connector1",
								originalToConnectorItem->attachedToID(), originalToConnectorItem->connectorSharedID(),
								true, true, parentCommand);

	foreach (SketchWidget * target, m_ratsnestTargets) {
		new RatsnestCommand(target, BaseCommand::SingleView,
									newID, "connector0",
									otherPartBusConnectorItem->attachedToID(), otherPartBusConnectorItem->connectorSharedID(),
									true, true, parentCommand);
		new RatsnestCommand(target, BaseCommand::SingleView,
									newID, "connector1",
									originalToConnectorItem->attachedToID(), originalToConnectorItem->connectorSharedID(),
									true, true, parentCommand);

	}

}

ConnectorItem * SchematicSketchWidget::lookForBreadboardConnection(ConnectorItem * connectorItem) {
	Wire * wire = dynamic_cast<Wire *>(connectorItem->attachedTo());

	QList<ConnectorItem *> ends;
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

	ends.clear();
	ends.append(connectorItem);
	ConnectorItem::collectEqualPotential(ends);
	foreach (ConnectorItem * end, ends) {
		if (end->attachedToItemType() == ModelPart::Breadboard) {
			return findEmptyBusConnectorItem(end);
		}
	}

	return connectorItem;
}

ConnectorItem * SchematicSketchWidget::findEmptyBusConnectorItem(ConnectorItem * busConnectorItem) {
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

int SchematicSketchWidget::calcDistance(Wire * wire, ConnectorItem * end, int distance, QList<Wire *> & distanceWires, bool & fromConnector0) {
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

int SchematicSketchWidget::calcDistanceAux(ConnectorItem * from, ConnectorItem * to, int distance, QList<Wire *> & distanceWires) {
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

void SchematicSketchWidget::removeRatsnestWires(QList< QList<ConnectorItem *>* > & allPartConnectorItems, CleanUpWiresCommand * command)
{
	/*
	if (m_deleteStash.count() > 0) {
		foreach(Wire * wire, m_deleteStash) {
			command->addWire(this, wire);
			deleteItem(wire, true, false, false);
		}
		m_deleteStash.clear();
		return;
	}
	*/

	PCBSchematicSketchWidget::removeRatsnestWires(allPartConnectorItems, command);
}

void SchematicSketchWidget::chainVisible(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, bool connect) {
	if (fromConnectorItem->attachedToItemType() != ModelPart::Wire) return;
	if (toConnectorItem->attachedToItemType() != ModelPart::Wire) return;

	fromConnectorItem->setHidden(!connect);
	toConnectorItem->setHidden(!connect);
}

ConnectorItem * SchematicSketchWidget::lookForNewBreadboardConnection(ConnectorItem * connectorItem,  ItemBase * & newBreadboard) {
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
	newBreadboard = this->addItem(ItemBase::tinyBreadboardModuleIDName, BaseCommand::SingleView, vg, id, -1, 0, NULL);
	busConnectorItem = findEmptyBus(newBreadboard);
	return busConnectorItem;
}

ConnectorItem * SchematicSketchWidget::findEmptyBus(ItemBase * breadboard) {
	foreach (Bus * bus, breadboard->buses()) {
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

void SchematicSketchWidget::makeModifiedWire(Wire * wire, ConnectorItem * fromDragWire, 
									  ConnectorItem * originalFromConnectorItem, ConnectorItem * newFromConnectorItem, 
									  ConnectorItem * originalToConnectorItem, ConnectorItem * newToConnectorItem,
									  QUndoCommand * parentCommand) 
{
	DebugDialog::debug(QString("new real wire %1").arg(m_connectorDragWire->id()));
	// create a new "real" wire with the same id as the temporary wire
	new AddItemCommand(this, BaseCommand::CrossView, Wire::moduleIDName, m_connectorDragWire->getViewGeometry(), m_connectorDragWire->id(), true, -1, -1, parentCommand);
	new CheckStickyCommand(this, BaseCommand::CrossView, m_connectorDragWire->id(), false, parentCommand);

	ConnectorItem * anchor = wire->otherConnector(fromDragWire);
	new ChangeConnectionCommand(this, BaseCommand::CrossView,
								anchor->attachedToID(), anchor->connectorSharedID(),
								newFromConnectorItem->attachedToID(), newFromConnectorItem->connectorSharedID(),
								true, true, parentCommand);
	new ChangeConnectionCommand(this, BaseCommand::CrossView,
								fromDragWire->attachedToID(), fromDragWire->connectorSharedID(),
								newToConnectorItem->attachedToID(), newToConnectorItem->connectorSharedID(),
								true, true, parentCommand);

	foreach (SketchWidget * target, m_ratsnestTargets) {
		new RatsnestCommand(target, BaseCommand::SingleView,
									anchor->attachedToID(), anchor->connectorSharedID(),
									newFromConnectorItem->attachedToID(), newFromConnectorItem->connectorSharedID(),
									true, true, parentCommand);
		new RatsnestCommand(target, BaseCommand::SingleView,
									fromDragWire->attachedToID(), fromDragWire->connectorSharedID(),
									newToConnectorItem->attachedToID(), newToConnectorItem->connectorSharedID(),
									true, true, parentCommand);

	}

	long newID = ItemBase::getNextID();

	ViewGeometry vg;
	this->makeRatsnestViewGeometry(vg, originalFromConnectorItem, originalToConnectorItem);

	DebugDialog::debug(QString("new ratsnest wire %1").arg(newID));

	new AddItemCommand(this, BaseCommand::SingleView, Wire::moduleIDName, vg, newID, true, -1, -1, parentCommand);
	new ChangeConnectionCommand(this, BaseCommand::SingleView,
								newID, "connector0",
								originalFromConnectorItem->attachedToID(), originalFromConnectorItem->connectorSharedID(),
								true, true, parentCommand);
	new ChangeConnectionCommand(this, BaseCommand::SingleView,
								newID, "connector1",
								originalToConnectorItem->attachedToID(), originalToConnectorItem->connectorSharedID(),
								true, true, parentCommand);
}


ConnectorItem * SchematicSketchWidget::findNearestPartConnectorItem(ConnectorItem * fromConnectorItem) {
	// find the nearest part to fromConnectorItem
	Wire * wire = dynamic_cast<Wire *>(fromConnectorItem->attachedTo());
	if (wire == NULL) return NULL;

	QList<ConnectorItem *> ends;
	calcDistances(wire, ends);
	clearDistances();
	if (ends.count() < 1) return NULL;

	return ends[0];
}

void SchematicSketchWidget::addRatnestTarget(SketchWidget * target) {
	m_ratsnestTargets.append(target);
}

void SchematicSketchWidget::calcDistances(Wire * wire, QList<ConnectorItem *> & ends) {
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

void SchematicSketchWidget::clearDistances() {
	foreach (ConnectorItem * c, distances.keys()) {
		DistanceThing * dt = distances.value(c, NULL);
		if (dt) delete dt;
	}
	distances.clear();
}

const QString & SchematicSketchWidget::hoverEnterWireConnectorMessage(QGraphicsSceneHoverEvent * event, ConnectorItem * item)
{
	Q_UNUSED(event);
	Q_UNUSED(item);

#ifdef Q_WS_MAC
	static QString message = tr("Shift-click to delete this bend point; Cmd-click to drag out a new wire.");
#else
	static QString message = tr("Shift-click to delete this bend point; Ctrl-click to drag out a new wire.");
#endif

	return message;
}
