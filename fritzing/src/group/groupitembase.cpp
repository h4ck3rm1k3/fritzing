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

#include "groupitembase.h"
#include "../connectoritem.h"
#include "../debugdialog.h"
#include "../items/wire.h"
#include "../modelpart.h"

#include <QTimer>
#include <QSet>

/////////////////////////////////////////

static QBrush GroupBrush(QColor(255, 0, 0));

/////////////////////////////////////////

GroupItemBase::GroupItemBase( ModelPart* modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu) 
	: ItemBase( modelPart, viewIdentifier, viewGeometry, id, itemMenu)
{
	this->setHandlesChildEvents(true);

	this->setCanFlipHorizontal(true);
	this->setCanFlipVertical(true);

	this->setVisible(true);
	this->setFlag(QGraphicsItem::ItemIsSelectable, true);
	this->setPos(viewGeometry.loc());
}

void GroupItemBase::addToGroup(ItemBase * itemBase) 
{
	m_itemsToAdd.append(itemBase);
}

void GroupItemBase::addToGroup(ItemBase * item, const LayerHash & layerHash) {
	if (m_zUninitialized) {
		this->setViewLayerID(item->viewLayerID(), layerHash);
		setZValue(this->z());
	}

	if (!item->canFlipHorizontal()) {
		setCanFlipHorizontal(false);
	}
	if (!item->canFlipVertical()) {
		setCanFlipVertical(false);
	}

	foreach (QGraphicsItem * item, item->childItems()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
		if (connectorItem == NULL) continue;

		connectorItem->setIgnoreAncestorFlagIfExternal(true);
	}

	item->setSelected(false);				// elements inside group can't be individually selected
	item->setSticky(false);					// elements inside group can't be sticky
	item->setFlag(QGraphicsItem::ItemIsSelectable, false);

    QTransform oldSceneMatrix = item->sceneTransform();
	DebugDialog::debug(QString("before pos %1").arg(item->id()), item->pos());
	
	item->layerKinChief()->blockSyncKinMoved(true);
    item->setPos(mapFromItem(item, m_offset.x(), m_offset.y()));
    item->setParentItem(this);
    item->setTransform(oldSceneMatrix
                       * sceneTransform().inverted()
                       * QTransform().translate(-item->x(), -item->y()));
	item->layerKinChief()->blockSyncKinMoved(false);

	DebugDialog::debug("after pos", item->pos());
	prepareGeometryChange();
	update();
}

void GroupItemBase::findConnectorsUnder() {
	foreach (ConnectorItem * connectorItem, m_externalConnectorItems) {
		if (connectorItem->connectorType() == Connector::Female) {
			continue;
		}

		connectorItem->setOverConnectorItem(
				findConnectorUnder(connectorItem,  connectorItem->overConnectorItem(), true, false));

	}
}

void GroupItemBase::saveGeometry() {
	m_viewGeometry.setLoc(this->pos());
}

bool GroupItemBase::itemMoved() {
	return (this->pos() != m_viewGeometry.loc());
}

void GroupItemBase::saveInstanceLocation(QXmlStreamWriter & streamWriter) {
	saveLocAndTransform(streamWriter);
}

void GroupItemBase::moveItem(ViewGeometry & viewGeometry) {
	this->setPos(viewGeometry.loc());
	updateConnections();
}

void GroupItemBase::syncKinMoved(GroupItemBase * originator, QPointF newPos) {
	Q_UNUSED(newPos);
	Q_UNUSED(originator);
}

const QRectF & GroupItemBase::calcBoundingRect() {
	// centers the parent within the bounding rect of the group
	// fixes rotation, among other things
	foreach(ItemBase * item, m_itemsToAdd) {
		m_itemsBoundingRect |= (item->transform() * QTransform().translate(item->x(), item->y()))
                            .mapRect(item->boundingRect() | item->childrenBoundingRect());

	}
	DebugDialog::debug(QString("bounding rect %1 ").arg(this->z()), m_itemsBoundingRect);
	return m_itemsBoundingRect;
}

const QRectF & GroupItemBase::itemsBoundingRect() {
	return m_itemsBoundingRect;
}

void GroupItemBase::setBoundingRect(const QRectF & boundingRect) {
	m_offset = m_itemsBoundingRect.topLeft() - boundingRect.topLeft();
	m_boundingRect.setRect(0, 0, boundingRect.width(), boundingRect.height());
}

void GroupItemBase::doneAdding(const LayerHash & layerHash) 
{
	// assumes setBoundingRect has been called already

	this->setPos(m_itemsBoundingRect.topLeft() - m_offset);
	saveGeometry();

	foreach(ItemBase * itemBase, m_itemsToAdd) {
		addToGroup(itemBase, layerHash);
	}

	//DebugDialog::debug(QString("\tlayer %1").arg(ViewLayer::viewLayerNameFromID(m_viewLayerID)));

	m_itemsToAdd.clear();
}

void GroupItemBase::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	if (m_hidden) return;

	if (option->state & QStyle::State_Selected) {	
		// draw this first because otherwise it seems to draw a dashed line down the middle
		qt_graphicsItem_highlightSelected(this, painter, option, boundingRect(), QPainterPath(), NULL);
	}

	if (this->parentItem() == NULL && !isLowerLayerVisible(this)) {   

		painter->save();
		painter->setOpacity(0.1);
		painter->fillRect(boundingRect(), GroupBrush);
		painter->restore();
	}
	ItemBase::paint(painter, option, widget);
}

QRectF GroupItemBase::boundingRect() const
{
	return m_boundingRect;
}

void GroupItemBase::collectWireConnectees(QSet<Wire *> & wires) {
	foreach (QGraphicsItem * item, childItems()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;

		itemBase->collectWireConnectees(wires);
	}
}

void GroupItemBase::collectFemaleConnectees(QSet<ItemBase *> & items) {
	foreach (QGraphicsItem * item, childItems()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;

		itemBase->collectFemaleConnectees(items);
	}
}

void GroupItemBase::collectExternalConnectorItems() {

	// TODO: what if connectorItems are in the GroupItemKin rather than the GroupItem
	// this may be a general flaw in all the layerKin code that collects connectors

	QList<ItemBase *> itemBases;
	itemBases.append(this);

	for (int i = 0; i < itemBases.count(); i++) {
		foreach (QGraphicsItem * childItem, itemBases[i]->childItems()) {
			ItemBase * itemBase = dynamic_cast<ItemBase *>(childItem);
			if (itemBase != NULL) itemBases.append(itemBase);
		}
	}

	foreach (ItemBase * itemBase, itemBases) {
		foreach (QGraphicsItem * childItem, itemBase->childItems()) {
			ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
			if (connectorItem == NULL) continue;

			if (connectorItem->isExternal()) {
				m_externalConnectorItems.append(connectorItem);
			}
		}
	}
}

void GroupItemBase::collectConnectors(QList<ConnectorItem *> & connectors) {
	foreach (ConnectorItem * connectorItem, m_externalConnectorItems) {
		connectors.append(connectorItem);
	}
}

bool GroupItemBase::hasExternalConnectorItems() {
	return m_externalConnectorItems.count() > 0;
}

bool GroupItemBase::hasConnectors() {
	return hasExternalConnectorItems();
}
