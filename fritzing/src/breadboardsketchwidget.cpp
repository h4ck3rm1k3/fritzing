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

static const QString ___viewName___ = QObject::tr("Breadboard View");

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

bool BreadboardSketchWidget::disconnectFromFemale(ItemBase * item, QSet<ItemBase *> & savedItems, ConnectorPairHash & connectorHash, QUndoCommand * parentCommand)
{
	// if item is attached to a virtual wire or a female connector in breadboard view
	// then disconnect it
	// at the moment, I think this doesn't apply to other views

	bool result;
	foreach (QGraphicsItem * childItem, item->childItems()) {
		ConnectorItem * fromConnectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (fromConnectorItem == NULL) continue;

		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems())  {
			if (toConnectorItem->connectorType() == Connector::Female) {
				if (savedItems.contains(toConnectorItem->attachedTo())) {
					// the thing we're connected to is also moving, so don't disconnect
					continue;
				}

				result = true;
				extendChangeConnectionCommand(fromConnectorItem, toConnectorItem, false, true, parentCommand);
				fromConnectorItem->tempRemove(toConnectorItem);
				toConnectorItem->tempRemove(fromConnectorItem);
				connectorHash.insert(fromConnectorItem, toConnectorItem);

			}
		}
	}

	return result;
}

BaseCommand::CrossViewType BreadboardSketchWidget::wireSplitCrossView()
{
	return BaseCommand::CrossView;
}

void BreadboardSketchWidget::schematicDisconnectWireSlot(QMultiHash<qint64, QString> & moveItems, QUndoCommand * parentCommand)
{
	foreach (qint64 itemID, moveItems.uniqueKeys()) {
		ItemBase * itemBase = findItem(itemID);
		if (itemBase == NULL) continue;

		PaletteItemBase * paletteItemBase = dynamic_cast<PaletteItemBase *>(itemBase);
		if (paletteItemBase == NULL) continue;

		PaletteItemBase * detachFrom = NULL;
		foreach (QString connectorID, moveItems.values(itemID)) {
			ConnectorItem * fromConnectorItem = findConnectorItem(itemBase, connectorID, true);
			if (fromConnectorItem == NULL) continue;

			foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
				if (toConnectorItem->attachedToItemType() == ModelPart::Breadboard || toConnectorItem->attachedToItemType() == ModelPart::Board ) {
					detachFrom = dynamic_cast<PaletteItemBase *>(toConnectorItem->attachedTo());
					break;
				}
			}
			if (detachFrom != NULL) break;
		}
		if (detachFrom == NULL) continue;

		QPointF newPos = calcNewLoc(paletteItemBase, detachFrom);

		// delete connections
		// add wires and connections for undisconnected connectors

		paletteItemBase->saveGeometry();
		ViewGeometry vg = paletteItemBase->getViewGeometry();
		vg.setLoc(newPos);
		new MoveItemCommand(this, paletteItemBase->id(), paletteItemBase->getViewGeometry(), vg, parentCommand);
		QSet<ItemBase *> tempItems;
		ConnectorPairHash connectorHash;
		disconnectFromFemale(paletteItemBase, tempItems, connectorHash, parentCommand);
		foreach (ConnectorItem * fromConnectorItem, connectorHash.uniqueKeys()) {
			if (moveItems.values(itemID).contains(fromConnectorItem->connectorStuffID())) {
				// don't need to reconnect
				continue;
			}

			foreach (ConnectorItem * toConnectorItem, connectorHash.values(fromConnectorItem)) {
				createWire(fromConnectorItem, toConnectorItem, ViewGeometry::NoFlag, false, BaseCommand::CrossView, parentCommand);
			}
		}
	}
}

QPointF BreadboardSketchWidget::calcNewLoc(PaletteItemBase * moveBase, PaletteItemBase * detachFrom)
{
	QRectF dr = detachFrom->boundingRect();
	dr.moveTopLeft(detachFrom->pos());
		
	QPointF pos = moveBase->pos();
	QRectF r = moveBase->boundingRect();
	pos.setX(pos.x() + (r.width() / 2));
	pos.setY(pos.y() + (r.height() / 2));
	qreal d[4];
	d[0] = qAbs(pos.y() - dr.top());
	d[1] = qAbs(pos.y() - dr.bottom());
	d[2] = qAbs(pos.x() - dr.left());
	d[3] = qAbs(pos.x() - dr.right());
	int ix = 0;
	for (int i = 1; i < 4; i++) {
		if (d[i] < d[ix]) {
			ix = i;
		}
	}
	QPointF newPos = moveBase->pos();
	switch (ix) {
		case 0:
			newPos.setY(dr.top() - r.height());
			break;
		case 1:
			newPos.setY(dr.bottom());
			break;
		case 2:
			newPos.setX(dr.left() - r.width());
			break;
		case 3:
			newPos.setY(dr.right());
			break;
	}
	return newPos;
}

const QString & BreadboardSketchWidget::viewName() {
	return ___viewName___;
}
