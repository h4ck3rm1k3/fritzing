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
#include "../wire.h"

#include <QTimer>
#include <QSet>

GroupItemBase::GroupItemBase( ModelPart* modelPart, ItemBase::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu) 
	: ItemBase( modelPart, viewIdentifier, viewGeometry, id, itemMenu)
{
	this->setHandlesChildEvents(true);

	this->setCanFlipHorizontal(true);
	this->setCanFlipVertical(true);

	this->setVisible(true);
	this->setFlag(QGraphicsItem::ItemIsSelectable);
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

		connectorItem->setIgnoreAncestorFlag(true);
	}

	item->setSelected(false);

    QTransform oldSceneMatrix = item->sceneTransform();
    item->setPos(mapFromItem(item, 0, 0));
    item->setParentItem(this);
    item->setTransform(oldSceneMatrix
                       * sceneTransform().inverted()
                       * QTransform().translate(-item->x(), -item->y()));

	prepareGeometryChange();
	update();
}

void GroupItemBase::findConnectorsUnder() {
}

void GroupItemBase::saveGeometry() {
	m_viewGeometry.setLoc(this->pos());
}

bool GroupItemBase::itemMoved() {
	return (this->pos() != m_viewGeometry.loc());
}

void GroupItemBase::saveInstanceLocation(QXmlStreamWriter & streamWriter) {
}

void GroupItemBase::moveItem(ViewGeometry & viewGeometry) {
	this->setPos(viewGeometry.loc());
}

void GroupItemBase::syncKinMoved(GroupItemBase * originator, QPointF newPos) {
	Q_UNUSED(newPos);
	Q_UNUSED(originator);
}

void GroupItemBase::doneAdding(const LayerHash & layerHash) {

	// centers the parent within the bounding rect of the group
	// fixes rotation, among other things
	QRectF itemsBoundingRect;
	foreach(ItemBase * item, m_itemsToAdd) {
		itemsBoundingRect |= (item->transform() * QTransform().translate(item->x(), item->y()))
                            .mapRect(item->boundingRect() | item->childrenBoundingRect());

	}

	m_boundingRect.setRect(0, 0, itemsBoundingRect.width(), itemsBoundingRect.height());

	DebugDialog::debug(QString("items bounding rect  "), itemsBoundingRect);

	this->setPos(itemsBoundingRect.topLeft());
	saveGeometry();

	foreach(ItemBase * itemBase, m_itemsToAdd) {
		addToGroup(itemBase, layerHash);
	}

	DebugDialog::debug(QString("\tlayer %1").arg(ViewLayer::viewLayerNameFromID(m_viewLayerID)));

	m_itemsToAdd.clear();
}

void GroupItemBase::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	if (m_hidden) return;

    if (option->state & QStyle::State_Selected) {	
		// draw this first because otherwise it seems to draw a dashed line down the middle
        qt_graphicsItem_highlightSelected(this, painter, option, boundingRect(), QPainterPath(), NULL);
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

		QSet<Wire *> tempWires;
		itemBase->collectWireConnectees(tempWires);
		foreach (Wire * wire, tempWires) {
			if (this->commonAncestorItem(wire) == NULL) {
				wires.insert(wire);
				break;
			}
		}
	}
}

void GroupItemBase::collectFemaleConnectees(QSet<ItemBase *> & items) {
	foreach (QGraphicsItem * item, childItems()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;

		itemBase->collectFemaleConnectees(items);
	}
}

