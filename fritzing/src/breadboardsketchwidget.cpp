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


#include "breadboardsketchwidget.h"
#include "debugdialog.h"
#include "virtualwire.h"

BreadboardSketchWidget::BreadboardSketchWidget(ItemBase::ViewIdentifier viewIdentifier, QWidget *parent, int size, int minSize)
    : SketchWidget(viewIdentifier, parent, size, minSize)
{
}

void BreadboardSketchWidget::cleanUpWire(Wire * wire, QList<Wire *> & wires)
{
	Q_UNUSED(wires);
	wire->setVisible(!wire->getVirtual());
	//wire->setVisible(true);					// for debugging
}

void BreadboardSketchWidget::collectFemaleConnectees(PaletteItem * paletteItem) {
	paletteItem->collectFemaleConnectees(m_savedItems);
}

void BreadboardSketchWidget::findConnectorsUnder(ItemBase * item) {
	item->findConnectorsUnder();
}

void BreadboardSketchWidget::addViewLayers() {
	addBreadboardViewLayers();
}

void BreadboardSketchWidget::disconnectFromFemale(ItemBase * item, QSet<ItemBase *> & savedItems, QSet <VirtualWire *> & virtualWires, QUndoCommand * parentCommand)
{
	// if item is attached to a virtual wire or a female connector in breadboard view
	// then disconnect it
	// at the moment, I think this doesn't apply to other views

	foreach (QGraphicsItem * childItem, item->childItems()) {
		ConnectorItem * fromConnectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (fromConnectorItem == NULL) continue;

		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems())  {
			if (toConnectorItem->connectorType() == Connector::Female) {
				if (savedItems.contains(toConnectorItem->attachedTo())) {
					// the thing we're connected to is also moving, so don't disconnect
					continue;
				}

				extendChangeConnectionCommand(fromConnectorItem, toConnectorItem, false, true, parentCommand);
				fromConnectorItem->tempRemove(toConnectorItem);
				toConnectorItem->tempRemove(fromConnectorItem);

				// if the female connector has any virtual wires pointing back to me get rid of them
				testForReturningVirtuals(toConnectorItem, item, virtualWires);
			}
			else if (toConnectorItem->attachedTo()->getVirtual()) {
				VirtualWire * virtualWire = dynamic_cast<VirtualWire *>(toConnectorItem->attachedTo());
				ConnectorItem * realci = virtualWire->otherConnector(toConnectorItem)->firstConnectedToIsh();
				if (realci != NULL) {
					ItemBase * otherEnd = realci->attachedTo();
					if (savedItems.contains(otherEnd)) {
						// the thing we're connected to is also moving, so don't disconnect
						continue;
					}
				}
				else {
					DebugDialog::debug("why is this only connected to a virtual item?");
				}
				virtualWires.insert(virtualWire);
			}
		}
	}
}

void BreadboardSketchWidget::testForReturningVirtuals(ConnectorItem * fromConnectorItem, ItemBase * target, QSet <VirtualWire *> & virtualWires) {
	if (target->buses().count() <= 0) return;

	foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems())  {
		if (toConnectorItem->attachedTo()->getVirtual()) {
			VirtualWire * virtualWire = dynamic_cast<VirtualWire *>(toConnectorItem->attachedTo());
			ConnectorItem * other = virtualWire->otherConnector(toConnectorItem);
			foreach (ConnectorItem * otherToConnectorItem, other->connectedToItems()) {
				if (otherToConnectorItem->attachedTo() == target) {
					virtualWires.insert(virtualWire);
					return;
				}
			}
		}
	}

}
