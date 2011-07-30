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

#include "breadboardsketchwidget.h"
#include "../debugdialog.h"
#include "../items/virtualwire.h"
#include "../connectors/connectoritem.h"
#include "../items/moduleidnames.h"
#include "../waitpushundostack.h"
#include "../help/sketchmainhelp.h"

#include <QScrollBar>

static const double WireHoverStrokeFactor = 4.0;

BreadboardSketchWidget::BreadboardSketchWidget(ViewIdentifierClass::ViewIdentifier viewIdentifier, QWidget *parent)
    : SketchWidget(viewIdentifier, parent)
{
	m_shortName = QObject::tr("bb");
	m_viewName = QObject::tr("Breadboard View");
	m_standardBackgroundColor = QColor(204,204,204);
	initBackgroundColor();
}

void BreadboardSketchWidget::setWireVisible(Wire * wire)
{
	bool visible = !wire->getRatsnest();
	wire->setVisible(visible);
	wire->setEverVisible(visible);
	//wire->setVisible(true);					// for debugging
}

bool BreadboardSketchWidget::collectFemaleConnectees(ItemBase * itemBase, QSet<ItemBase *> & items) {
	return itemBase->collectFemaleConnectees(items);
}

bool BreadboardSketchWidget::checkUnder() {
	return true;
};

void BreadboardSketchWidget::findConnectorsUnder(ItemBase * item) {
	item->findConnectorsUnder();
}

void BreadboardSketchWidget::addViewLayers() {
	addBreadboardViewLayers();
}

bool BreadboardSketchWidget::disconnectFromFemale(ItemBase * item, QHash<long, ItemBase *> & savedItems, ConnectorPairHash & connectorHash, bool doCommand, bool disconnectBendable, QUndoCommand * parentCommand)
{
	// if item is attached to a virtual wire or a female connector in breadboard view
	// then disconnect it
	// at the moment, I think this doesn't apply to other views

	bool result = false;
	foreach (ConnectorItem * fromConnectorItem, item->cachedConnectorItems()) {
		if (!disconnectBendable && fromConnectorItem->hasBendableLeg()) continue;

		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems())  {
			if (!disconnectBendable && toConnectorItem->hasBendableLeg()) continue;

			if (toConnectorItem->connectorType() == Connector::Female) {
				if (savedItems.keys().contains(toConnectorItem->attachedTo()->layerKinChief()->id())) {
					// the thing we're connected to is also moving, so don't disconnect
					continue;
				}

				result = true;
				fromConnectorItem->tempRemove(toConnectorItem, true);
				toConnectorItem->tempRemove(fromConnectorItem, true);
				if (doCommand) {
					extendChangeConnectionCommand(BaseCommand::CrossView, fromConnectorItem, toConnectorItem, ViewLayer::Bottom, false, parentCommand);
				}
				connectorHash.insert(fromConnectorItem, toConnectorItem);

			}
		}
	}

	return result;
}


BaseCommand::CrossViewType BreadboardSketchWidget::wireSplitCrossView()
{
	return BaseCommand::SingleView;
}

bool BreadboardSketchWidget::canDropModelPart(ModelPart * modelPart) {
	if (modelPart->itemType() == ModelPart::Board || modelPart->itemType() == ModelPart::ResizableBoard) {
		return matchesLayer(modelPart);
	}
	
	switch (modelPart->itemType()) {
		case ModelPart::Board:
		case ModelPart::ResizableBoard:
			return matchesLayer(modelPart);
		case ModelPart::Logo:
		case ModelPart::Symbol:
		case ModelPart::Jumper:
		case ModelPart::CopperFill:
		case ModelPart::Hole:
		case ModelPart::Via:
			return false;
		default:
			return !modelPart->moduleID().endsWith(ModuleIDNames::SchematicFrameModuleIDName);
	}
}

void BreadboardSketchWidget::initWire(Wire * wire, int penWidth) {
	wire->setPenWidth(penWidth - 2, this, (penWidth - 2) * WireHoverStrokeFactor);
	wire->setColorString("blue", 1.0);
}

void BreadboardSketchWidget::getLabelFont(QFont & font, QColor & color, ViewLayer::ViewLayerSpec) {
	font.setFamily("Droid Sans");
	font.setPointSize(9);
	color.setAlpha(255);
	color.setRgb(0);
}

void BreadboardSketchWidget::setNewPartVisible(ItemBase * itemBase) {
	switch (itemBase->itemType()) {
		case ModelPart::Symbol:
		case ModelPart::Jumper:
		case ModelPart::CopperFill:
		case ModelPart::Logo:
		case ModelPart::Hole:
		case ModelPart::Via:
			itemBase->setVisible(false);
			itemBase->setEverVisible(false);
			return;
		default:
			break;
	}
}

bool BreadboardSketchWidget::canDisconnectAll() {
	return false;
}

bool BreadboardSketchWidget::ignoreFemale() {
	return false;
}

double BreadboardSketchWidget::defaultGridSizeInches() {
	return 0.1;
}

ViewLayer::ViewLayerID BreadboardSketchWidget::getLabelViewLayerID(ViewLayer::ViewLayerSpec) {
	return ViewLayer::BreadboardLabel;
}

void BreadboardSketchWidget::addDefaultParts() {
	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(QPointF(0, 0));
	m_addedDefaultPart = addItem(paletteModel()->retrieveModelPart(ModuleIDNames::BreadboardModuleIDName), defaultViewLayerSpec(), BaseCommand::CrossView, viewGeometry, newID, -1, NULL, NULL);
	m_addDefaultParts = true;
	// have to put this off until later, because positioning the item doesn't work correctly until the view is visible
}

void BreadboardSketchWidget::showEvent(QShowEvent * event) {
	SketchWidget::showEvent(event);
	if (m_addDefaultParts && (m_addedDefaultPart != NULL)) {
		m_addDefaultParts = false;
		if (m_fixedToCenterItem != NULL) {
			QSizeF helpSize = m_fixedToCenterItem->size();
			QSizeF partSize = m_addedDefaultPart->size();
			QSizeF vpSize = this->viewport()->size();
			//if (vpSize.height() < helpSize.height() + 50 + partSize.height()) {
				//vpSize.setWidth(vpSize.width() - verticalScrollBar()->width());
			//}

			QPointF p;
			p.setX((int) ((vpSize.width() - partSize.width()) / 2.0));
			p.setY((int) helpSize.height());

			// add a board to the empty sketch, and place it below the help area.

			p += QPointF(0, 50);
			QPointF q = mapToScene(p.toPoint());
			m_addedDefaultPart->setPos(q);
			QTimer::singleShot(10, this, SLOT(vScrollToZero()));
		}
	}
}

QPoint BreadboardSketchWidget::calcFixedToCenterItemOffset(const QRect & viewPortRect, const QSizeF & helpSize) {
	QPoint p((int) ((viewPortRect.width() - helpSize.width()) / 2.0),
			 30);
	return p;
}

double BreadboardSketchWidget::getWireStrokeWidth(Wire *, double wireWidth)
{
	return wireWidth * WireHoverStrokeFactor;
}



/*

void BreadboardSketchWidget::schematicDisconnectWireSlot(ConnectorPairHash & foreignMoveItems, QSet<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash *> & deletedConnections, QUndoCommand * parentCommand)
{
	// this slot is obsolete, but some of the code might be useful

	Q_UNUSED(deletedConnections);
	Q_UNUSED(deletedItems);

	QMultiHash<PaletteItemBase *, ConnectorItem *> bases;
	ConnectorPairHash moveItems;
	translateToLocalItems(foreignMoveItems, moveItems, bases);

	QHash<PaletteItemBase *, ItemBase *> detachItems;
	foreach (PaletteItemBase * paletteItemBase, bases.uniqueKeys()) {
		foreach (ConnectorItem * fromConnectorItem, bases.values(paletteItemBase)) {
			if (fromConnectorItem->connectorType() == Connector::Female) {
				// SchematicSketchWidget moveItems may have both A-hashed-to-B connectorItems, 
				// and B-hashed-to-A connectorItems.  We ignore the hash pair starting with the
				// female connector
				continue;
			}
			foreach (ConnectorItem * toConnectorItem, moveItems.values(fromConnectorItem)) {
				detachItems.insert(paletteItemBase, toConnectorItem->attachedTo());
			}
		}
	}
	*/

	/*
	foreach (PaletteItemBase * paletteItemBase, bases.uniqueKeys()) {
		foreach (ConnectorItem * fromConnectorItem, bases.values(paletteItemBase)) {
			int femaleCount = 0;
			int totalCount = 0;
			foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
				if (toConnectorItem->connectorType() == Connector::Female) femaleCount++;
				totalCount++;
			}
			if (femaleCount == 1 && totalCount == 1) {
				detachItems.insert(paletteItemBase, fromConnectorItem->connectedToItems()[0]->attachedTo());
				continue;
			}
			foreach (ConnectorItem * toConnectorItem, moveItems.values(fromConnectorItem)) {
				ItemBase * breadboardItemBase = NULL;
				if (toConnectorItem->connectorType() == Connector::Female) {
					// paletteItemBase directly connected to arduino, for example
					detachItems.insert(paletteItemBase, toConnectorItem->attachedTo());
				}
				else if (shareBreadboard(fromConnectorItem, toConnectorItem, breadboardItemBase)) {
					detachItems.insert(paletteItemBase, breadboardItemBase);
				}
				else {
					// if they they indirectly connected via a female connector, then delete a wire
					QList<ConnectorItem *> connectorItems;
					connectorItems.append(fromConnectorItem);
					connectorItems.append(toConnectorItem);
					ConnectorItem::collectEqualPotential(connectorItems);
					bool foundIt = false;
					foreach (ConnectorItem * candidate, connectorItems) {
						if (candidate->connectorType() != Connector::Female) continue;

						foreach (ConnectorItem * cto, candidate->connectedToItems()) {
							if (cto->attachedToItemType() != ModelPart::Wire) continue;

							QList<Wire *> chained;
							QList<ConnectorItem *> ends;
							Wire * tempWire = dynamic_cast<Wire *>(cto->attachedTo());
							tempWire->collectChained(chained, ends);
							if (ends.contains(fromConnectorItem) || ends.contains(toConnectorItem)) {
								// is this good enough or do we need more confirmation that it's the right wire?
								deletedItems.insert(tempWire);
								ConnectorPairHash * connectorHash = new ConnectorPairHash;
								tempWire->collectConnectors(*connectorHash, this->scene());
								deletedConnections.insert(tempWire, connectorHash);	
								foundIt = true;
								break;
							}
						}
						if (foundIt) break;
					}
				}
			}
		}
	}
	*/

	/*
	foreach (PaletteItemBase * detachee, detachItems.keys()) {
		ItemBase * detachFrom = detachItems.value(detachee);
		QPointF newPos = calcNewLoc(detachee, dynamic_cast<PaletteItemBase *>(detachFrom));

		// delete connections
		// add wires and connections for undisconnected connectors

		detachee->saveGeometry();
		ViewGeometry vg = detachee->getViewGeometry();
		vg.setLoc(newPos);
		new MoveItemCommand(this, detachee->id(), detachee->getViewGeometry(), vg, parentCommand);
		QSet<ItemBase *> tempItems;
		ConnectorPairHash connectorHash;
		disconnectFromFemale(detachee, tempItems, connectorHash, true, false, parentCommand);
		foreach (ConnectorItem * fromConnectorItem, connectorHash.uniqueKeys()) {
			if (moveItems.uniqueKeys().contains(fromConnectorItem)) {
				// don't need to reconnect
				continue;
			}
			if (moveItems.values().contains(fromConnectorItem)) {
				// don't need to reconnect
				continue;
			}

			foreach (ConnectorItem * toConnectorItem, connectorHash.values(fromConnectorItem)) {
				createWire(fromConnectorItem, toConnectorItem, ViewGeometry::NoFlag, BaseCommand::CrossView, parentCommand);
			}
		}
	}
}

*/

/*
void BreadboardSketchWidget::translateToLocalItems(ConnectorPairHash & foreignMoveItems, ConnectorPairHash & moveItems,	QMultiHash<PaletteItemBase *, ConnectorItem *> & bases)
{
	foreach (ConnectorItem * foreignFromConnectorItem, foreignMoveItems.uniqueKeys()) {
		qint64 fromItemID = foreignFromConnectorItem->attachedToID();
		ItemBase * fromItemBase = findItem(fromItemID);
		if (fromItemBase == NULL) continue;

		PaletteItemBase * paletteItemBase = dynamic_cast<PaletteItemBase *>(fromItemBase);
		if (paletteItemBase == NULL) {
			// shouldn't be here: want parts not wires
			continue;
		}

		ConnectorItem * fromConnectorItem = findConnectorItem(fromItemBase, foreignFromConnectorItem->connectorSharedID(), ViewLayer::Bottom);
		if (fromConnectorItem == NULL) continue;

		foreach (ConnectorItem * foreignToConnectorItem, foreignMoveItems.values(foreignFromConnectorItem)) {
			qint64 toItemID = foreignToConnectorItem->attachedToID();
			ItemBase * toItemBase = findItem(toItemID);
			if (toItemBase == NULL) continue;

			ConnectorItem * toConnectorItem = findConnectorItem(toItemBase, foreignToConnectorItem->connectorSharedID(), ViewLayer::Bottom);
			if (toConnectorItem == NULL) continue;

			moveItems.insert(fromConnectorItem, toConnectorItem);
		}
		bases.insert(paletteItemBase, fromConnectorItem);
	}
}

*/

/*
bool BreadboardSketchWidget::shareBreadboard(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, ItemBase * & itemBase)
{
	foreach (ConnectorItem * ftci, fromConnectorItem->connectedToItems()) {
		if (ftci->connectorType() == Connector::Female) {
			foreach (ConnectorItem * ttci, toConnectorItem->connectedToItems()) {
				if (ttci->connectorType() == Connector::Female) {
					if (ftci->bus() == ttci->bus()) {
						itemBase = ftci->attachedTo();
						return true;
					}
				}
			}
		}
	}

	return false;
}
*/
