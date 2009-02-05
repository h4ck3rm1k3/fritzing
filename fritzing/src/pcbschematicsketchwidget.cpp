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


#include "pcbschematicsketchwidget.h"
#include "debugdialog.h"
#include "autorouter1.h"

PCBSchematicSketchWidget::PCBSchematicSketchWidget(ItemBase::ViewIdentifier viewIdentifier, QWidget *parent)
    : SketchWidget(viewIdentifier, parent)
{
}

void PCBSchematicSketchWidget::setNewPartVisible(ItemBase * itemBase) {
	if (itemBase->itemType() == ModelPart::Breadboard) {
		// don't need to see the breadboard in the other views
		// but it's there so connections can be more easily synched between views
		itemBase->setVisible(false);
	}
}

/*
void PCBSchematicSketchWidget::redrawRatsnest(QHash<long, ItemBase *> & newItems) {
	ConnectorPairHash allConnectors;
	foreach (ItemBase * newItem, newItems.values()) {
		foreach (QGraphicsItem * childItem, newItem->childItems()) {
			ConnectorItem * fromConnectorItem = dynamic_cast<ConnectorItem *>(childItem);
			if (fromConnectorItem == NULL) continue;

			foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
				
				DebugDialog::debug(QString("restoring ratsnest: %1 %2, %3 %4")
					.arg(fromConnectorItem->attachedToTitle())
					.arg(fromConnectorItem->connectorStuffID())
					.arg(toConnectorItem->attachedToTitle())
					.arg(toConnectorItem->connectorStuffID())
					);
				
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
*/

bool PCBSchematicSketchWidget::canDropModelPart(ModelPart * modelPart) {
	if (modelPart->itemType() == ModelPart::Wire || modelPart->itemType() == ModelPart::Breadboard) {
		// can't drag and drop these parts in these views
		return false;
	}

	if (modelPart->itemType() == ModelPart::Board) {
		return matchesLayer(modelPart);
	}

	return true;
}

bool PCBSchematicSketchWidget::alreadyRatsnest(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem) {
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

void PCBSchematicSketchWidget::dealWithRatsnest(long fromID, const QString & fromConnectorID, 
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

	DebugDialog::debug(QString("deal with ratsnest %1 %2 %3, %4 %5 %6")
		.arg(fromConnectorItem->attachedToTitle())
		.arg(fromConnectorItem->attachedToID())
		.arg(fromConnectorItem->connectorStuffID())
		.arg(toConnectorItem->attachedToTitle())
		.arg(toConnectorItem->attachedToID())
		.arg(toConnectorItem->connectorStuffID())
	);

	QList<ConnectorItem *> connectorItems;
	QList<ConnectorItem *> partsConnectorItems;
	connectorItems.append(fromConnectorItem);
	ConnectorItem::collectEqualPotential(connectorItems);
	ConnectorItem::collectParts(connectorItems, partsConnectorItems);

	QList <Wire *> ratsnestWires;
	Wire * modelWire = NULL;

	makeWires(partsConnectorItems, ratsnestWires, modelWire, ratsnestCommand);

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
			wire->setColor(colorAsQColor, wire->getRouted() ? ROUTED_OPACITY : UNROUTED_OPACITY);
		}
	}

	return;
}

void PCBSchematicSketchWidget::removeRatsnestWires(QList< QList<ConnectorItem *>* > & allPartConnectorItems, CleanUpWiresCommand * command)
{
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
		deleteItem(wire, true, false);
	}
}

bool PCBSchematicSketchWidget::bothEndsConnected(Wire * wire, ViewGeometry::WireFlags flag, ConnectorItem * oneEnd, QList<Wire *> & wires, QList<ConnectorItem *> & partConnectorItems)
{
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

		result = bothEndsConnected(w, flag, toConnectorItem, wires, partConnectorItems) || result;   // let it recurse
	}

	if (result) {
		wires.removeOne(wire);
	}

	return result;
}

void PCBSchematicSketchWidget::reviewDeletedConnections(QSet<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash *> & deletedConnections, QUndoCommand * parentCommand)
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
}

bool PCBSchematicSketchWidget::canCreateWire(Wire * dragWire, ConnectorItem * from, ConnectorItem * to)
{
	Q_UNUSED(dragWire);
	return ((from != NULL) && (to != NULL));
}

Wire * PCBSchematicSketchWidget::makeOneRatsnestWire(ConnectorItem * source, ConnectorItem * dest, RatsnestCommand * ratsnestCommand, bool select) {
	long newID = ItemBase::getNextID();

	ViewGeometry viewGeometry;
	makeRatsnestViewGeometry(viewGeometry, source, dest);

	/*
	 DebugDialog::debug(QString("creating ratsnest %10: %1, from %6 %7, to %8 %9, frompos: %2 %3, topos: %4 %5")
	 .arg(newID)
	 .arg(fromPos.x()).arg(fromPos.y())
	 .arg(toPos.x()).arg(toPos.y())
	 .arg(source->attachedToTitle()).arg(source->connectorStuffID())
	 .arg(dest->attachedToTitle()).arg(dest->connectorStuffID())
	 .arg(m_viewIdentifier)
	 );
	 */

	ItemBase * newItemBase = addItem(m_paletteModel->retrieveModelPart(Wire::moduleIDName), BaseCommand::SingleView, viewGeometry, newID, -1, NULL);		
	Wire * wire = dynamic_cast<Wire *>(newItemBase);
	tempConnectWire(wire, source, dest);
	if (!select) {
		wire->setSelected(false);
	}

	Wire * tempWire = source->wiredTo(dest, ViewGeometry::TraceFlag);
	if (tempWire) {
		wire->setOpacity(ROUTED_OPACITY);
	}

	ratsnestCommand->addWire(this, wire, source, dest, select);
	return wire ;
}

void PCBSchematicSketchWidget::makeRatsnestViewGeometry(ViewGeometry & viewGeometry, ConnectorItem * source, ConnectorItem * dest) 
{
	QPointF fromPos = source->sceneAdjustedTerminalPoint();
	viewGeometry.setLoc(fromPos);
	QPointF toPos = dest->sceneAdjustedTerminalPoint();
	QLineF line(0, 0, toPos.x() - fromPos.x(), toPos.y() - fromPos.y());
	viewGeometry.setLine(line);
	viewGeometry.setWireFlags(ViewGeometry::RatsnestFlag | ViewGeometry::VirtualFlag);
}


bool PCBSchematicSketchWidget::dealWithRatsnestAux(ConnectorItem * & fromConnectorItem, ConnectorItem * & toConnectorItem, 
						long fromID, const QString & fromConnectorID, 
						long toID, const QString & toConnectorID,
						bool connect, class RatsnestCommand * ratsnestCommand, bool doEmit) 
{
	SketchWidget::dealWithRatsnest(fromID, fromConnectorID, toID, toConnectorID, connect, ratsnestCommand, doEmit);

	ItemBase * from = findItem(fromID);
	if (from == NULL) return true;

	fromConnectorItem = findConnectorItem(from, fromConnectorID, true);
	if (fromConnectorItem == NULL) return true;

	ItemBase * to = findItem(toID);
	if (to == NULL) return true;

	toConnectorItem = findConnectorItem(to, toConnectorID, true);
	if (toConnectorItem == NULL) return true;

	return alreadyRatsnest(fromConnectorItem, toConnectorItem);
}

bool PCBSchematicSketchWidget::doRatsnestOnCopy() 
{
	return true;
}

void PCBSchematicSketchWidget::makeWiresChangeConnectionCommands(const QList<Wire *> & wires, QUndoCommand * parentCommand)
{
	QList<QString> alreadyList;
	foreach (Wire * wire, wires) {
		QList<ConnectorItem *> wireConnectorItems;
		wireConnectorItems << wire->connector0() << wire->connector1();
		foreach (ConnectorItem * fromConnectorItem, wireConnectorItems) {
			foreach(ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
				QString already = ((fromConnectorItem->attachedToID() <= toConnectorItem->attachedToID()) ? QString("%1.%2.%3.%4") : QString("%3.%4.%1.%2"))
					.arg(fromConnectorItem->attachedToID()).arg(fromConnectorItem->connectorStuffID())
					.arg(toConnectorItem->attachedToID()).arg(toConnectorItem->connectorStuffID());
				if (alreadyList.contains(already)) continue;

				alreadyList.append(already);
				new ChangeConnectionCommand(this, BaseCommand::SingleView,
											fromConnectorItem->attachedToID(), fromConnectorItem->connectorStuffID(),
											toConnectorItem->attachedToID(), toConnectorItem->connectorStuffID(),
											false, true, parentCommand);
			}
		}
	}
}