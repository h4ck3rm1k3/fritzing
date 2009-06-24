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

// TODO:
//  drag onto breadboard behavior
//  allow arbitrary image to be associated (where to put connectors?)
//	undo group
//  bug: new module not appearing in parts bin
//	pcb view: group kin not synching during rotate


//  * labels
//  * bug: pcb view drawing multiple rectangles
//  * what if module has an internal transform
//  * bug: transform group, save sketch, transform isn't preserved
//  * bug: can't drag wire from external connectors
//	* rotate group bug: connected external wires don't updateConnections
//  * bug: after undo, updateconnections not being called
//	* export with groups
//  * bug: drag out selection so that internal parts are selected, then copy/paste 
//  ** sticky
//  * bug: not connecting across views
//	* delete
//	* undo delete
//  ** ratsnest behavior: what if module contains separated parts (tough)
//  ** autorouting behavior
//	* recursive groups
//		* recursive modelpart ownership
//	* open in new sketch (edit)
//		* need to preserve external connectors
//	* copy/paste
//  * undo copy/paste
//  * allow mouse events to external connections
//		* allow wires to connect within the group
//		* drag doesn't keep wire connections
//		* connect within
//		* ignore submodule external connections
//	** breadboard or arduino in module
//  ** female connectors in modules
//		** connect female connectees
//  * how to hide non-external connectors
//  ** trace wires
//	** ratsnest wires
//	* model parts for grouped items get added as children to the group modelpart
//  * save sketch with group(s)
//  * load sketch with group(s)
//	* bug: no shadow in grouped wire
//  * drop group into sketch
//		* command objects go where?
//	** save as group
//		* store them in the user folder
//		** add to bin
//		* create icon svg
//			* use the bounding rect of the items instead of the scene rect
//		* xml: something that says group; pointer to icon; properties
//		* select external connections
//	* layerkin
//		** pcb view: group part in bb view, items not visually synced in pcb view
//  * scene jumps when creating a new group--triggered by changing the location of the chief item
//  * late updates (paint) after flip/rotate, other?
//	** z-order manipulation
//	* hide/show layer 
//		* still shows group selection box
//  * override QGraphicsItemGroup::paint
//	** rotate/flip
//		* need to center itembase in bounding rect
//		* unable to rotate in pcb view (selection bug?)
//		** is flip always allowed?
//		* undo
//	* figure out which layer the grouped items are on and get the next z id
//	* sort itembases by z
//  * select/unselect bug

#include <QGraphicsScene>
#include <QSet>

#include "groupitem.h"
#include "groupitemkin.h"
#include "../labels/partlabel.h"
#include "../connectoritem.h"
#include "../items/wire.h"
#include "../debugdialog.h"
#include "../modelpart.h"

GroupItem::GroupItem( ModelPart* modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu) 
	: GroupItemBase( modelPart, viewIdentifier, viewGeometry, id, itemMenu)
{
	m_blockSyncKinMoved = false;
	m_partLabel = new PartLabel(this, "", NULL);
}

const QList<ItemBase *> & GroupItem::layerKin() {
	return m_layerKin;
}

void GroupItem::syncKinMoved(GroupItemBase * groupItemBase, QPointF newPos) {
	if (m_blockSyncKinMoved) return;

	if (groupItemBase != this) {
		setPos(newPos);
	}
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->setPos(newPos);
	}
}

QVariant GroupItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (m_layerKin.count() > 0) {
		switch (change) {
			case ItemPositionHasChanged: 
	    		this->syncKinMoved(this, value.toPointF());
				break;
			case ItemSelectedChange:
				//DebugDialog::debug(QString("chief sel change %1 %2").arg(this->id()).arg(value.toBool()));
				break;
			default:
				break;
	   	}
   	}

    return GroupItemBase::itemChange(change, value);
}

void GroupItem::rotateItem(qreal degrees) {
	GroupItemBase::rotateItem(degrees);
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->rotateItem(degrees);
	}
}

void GroupItem::flipItem(Qt::Orientations orientation) {
	GroupItemBase::flipItem(orientation);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->flipItem(orientation);
	}
}

void GroupItem::doneAdding(const LayerHash & layerHash, ViewLayer::ViewLayerID viewLayerID) 
{
	qint64 id = m_id + 1;

	// need to redistribute items based on which layer they belong to
	QList<ItemBase *> tempItemsToAdd(m_itemsToAdd);
	m_itemsToAdd.clear();

	foreach (ItemBase * itemBase, tempItemsToAdd) {
		addToList(itemBase, viewLayerID, layerHash, id);
		foreach (ItemBase * lkpi, itemBase->layerKin()) {
			addToList(lkpi, viewLayerID, layerHash, id);
		}
	}

	blockSyncKinMoved(true);				// prevent recursive moves when layerkin are moved during doneAdding

	// need to set them all to the same bounding rect
	QRectF boundingRect = calcBoundingRect();
	foreach (ItemBase * kin, m_layerKin) {
		boundingRect |= dynamic_cast<GroupItemBase *>(kin)->calcBoundingRect();
	}
	setBoundingRect(boundingRect);

	DebugDialog::debug(QString("total bounding rect %1 ").arg(this->z()), boundingRect);

	foreach (ItemBase * kin, m_layerKin) {
		dynamic_cast<GroupItemBase *>(kin)->setBoundingRect(boundingRect);
	}

	GroupItemBase::doneAdding(layerHash);
	foreach (ItemBase * kin, m_layerKin) {
		dynamic_cast<GroupItemBase *>(kin)->doneAdding(layerHash);
	}
	blockSyncKinMoved(false);

	syncKinMoved(this, this->pos());

	figureHover();
}

void GroupItem::addToList(ItemBase * candidate, ViewLayer::ViewLayerID viewLayerID, const LayerHash & layerHash, qint64 & id) 
{
	if (candidate->viewLayerID() == viewLayerID) {
		addToGroup(candidate);
		return;
	}

	foreach (ItemBase * mylkpi, layerKin()) {
		if (candidate->viewLayerID() == mylkpi->viewLayerID()) {
			dynamic_cast<GroupItemKin *>(mylkpi)->addToGroup(candidate);
			return;
		}
	}

	GroupItemKin * mylkpi = new GroupItemKin(m_modelPart, m_viewIdentifier, m_viewGeometry, id++, NULL);
	mylkpi->setViewLayerID(candidate->viewLayerID(), layerHash);
	mylkpi->setLayerKinChief(this);
	scene()->addItem(mylkpi);
	m_layerKin.append(mylkpi);
	mylkpi->addToGroup(candidate);
	mylkpi->setZValue(mylkpi->z());
}

void GroupItem::collectWireConnectees(QSet<Wire *> & wires) 
{
	QSet<Wire *> tempWires;
	GroupItemBase::collectWireConnectees(tempWires);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->collectWireConnectees(tempWires);
	}

	foreach (Wire * wire, tempWires) {
		QGraphicsItem * parent = wire;
		while (parent->parentItem()) {
			parent = parent->parentItem();
		}
		bool gotOne = false;
		if (parent == this) {
			gotOne = true;
		}
		else {
			foreach (ItemBase * kin, m_layerKin) {
				if (parent == kin) {
					gotOne = true;
					break;
				}
			}
		}

		if (!gotOne) {
			wires.insert(wire);
		}
	}
}

void GroupItem::collectFemaleConnectees(QSet<ItemBase *> & items) {
	QSet<ItemBase *> temp;
	GroupItemBase::collectFemaleConnectees(temp);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->collectFemaleConnectees(temp);
	}

	foreach (ItemBase * itemBase, temp) {
		if (this->isAncestorOf(itemBase)) continue;

		bool common = false;
		foreach (ItemBase * lkpi, m_layerKin) {
			if (lkpi->isAncestorOf(itemBase)) {
				common = true;
				break;
			}
		}
		if (common) continue;

		items.insert(itemBase);
	}
}

void GroupItem::removeLayerKin() {
	// assumes item is still in scene
	for (int i = 0; i < m_layerKin.size(); i++) {
		//DebugDialog::debug(QString("removing group kin %1 %2").arg(m_layerKin[i]->id()).arg(m_layerKin[i]->z()));
		this->scene()->removeItem(m_layerKin[i]);
		delete m_layerKin[i];
	}

	m_layerKin.clear();
}

void GroupItem::clearModelPart() {
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->setModelPart(NULL);
	}
	ItemBase::clearModelPart();
}

void GroupItem::resetID() {
	ItemBase::resetID();
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->resetID();
	}
}

void GroupItem::updateConnections() {
	updateExternalConnections();
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->updateExternalConnections();
	}
}

void GroupItem::moveItem(ViewGeometry & viewGeometry) {
	GroupItemBase::moveItem(viewGeometry);
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->moveItem(viewGeometry);
	}
}

void GroupItem::setTransforms() {
	setTransform(getViewGeometry().transform());
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->setTransform(m_layerKin[i]->getViewGeometry().transform());
	}
}

bool GroupItem::isLowerLayerVisible(GroupItemBase * groupItemBase) {
	if (m_layerKin.count() == 0) return false;

	// DebugDialog::debug(QString("incoming z: %1, chief z: %2").arg(groupItemBase->zValue()).arg(this->zValue()));

	if ((groupItemBase != this) 
		&& this->isVisible() 
		&& (!this->hidden()) && (this->zValue() < groupItemBase->zValue())) 
	{
		return true;
	}

	foreach (ItemBase * lkpi, m_layerKin) {
		if (lkpi == groupItemBase) continue;

		// DebugDialog::debug(QString("lkpi z: %1").arg(lkpi->zValue()));

		if (lkpi->isVisible() 
			&& (!lkpi->hidden()) 
			&& (lkpi->zValue() < groupItemBase->zValue())  ) 
		{
			return true;
		}
	}

	return false;
}

void GroupItem::blockSyncKinMoved(bool block) {
	m_blockSyncKinMoved = block;
}

ItemBase * GroupItem::lowerConnectorLayerVisible(ItemBase * itemBase) {
	if (m_layerKin.count() == 0) return NULL;

	if ((itemBase != this) 
		&& this->isVisible() 
		&& (!this->hidden()) && (this->zValue() < itemBase->zValue())
		&& hasConnectors()) 
	{
		return this;
	}

	foreach (ItemBase * lkpi, m_layerKin) {
		if (lkpi == itemBase) continue;

		if (lkpi->isVisible() 
			&& (!lkpi->hidden()) 
			&& (lkpi->zValue() < itemBase->zValue()) 
			&& lkpi->hasConnectors() ) 
		{
			return lkpi;
		}
	}

	return NULL;
}

void GroupItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (lowerConnectorLayerVisible(this)) {
		DebugDialog::debug("GroupItem::mousePressEvent isn't obsolete");
		event->ignore();
		return;
	}

	GroupItemBase::mousePressEvent(event);
}


void GroupItem::setHidden(bool hide) {
	ItemBase::setHidden(hide);
	figureHover();
}


void GroupItem::figureHover() {
	// if a layer contains connectors, make it the one that accepts hover events
	// if you make all layers accept hover events, then the topmost layer will get the event
	// and lower layers won't

	QList<ItemBase *> allKin;
	allKin.append(this);
	foreach(ItemBase * lkpi, m_layerKin) {
		allKin.append(lkpi);
	}

	qSort(allKin.begin(), allKin.end(), ItemBase::zLessThan);
	foreach (ItemBase * base, allKin) {
		base->setAcceptHoverEvents(false);
		base->setAcceptedMouseButtons(Qt::NoButton);
	}

	int ix = 0;
	foreach (ItemBase * base, allKin) {
		if (!base->hidden() && base->hasConnectors()) {
			base->setAcceptHoverEvents(true);
			base->setAcceptedMouseButtons(ALLMOUSEBUTTONS);
			break;
		}
		ix++;
	}

	for (int i = 0; i < ix; i++) {
		ItemBase * base = allKin[i];
		if (!base->hidden()) {
			base->setAcceptHoverEvents(true);
			base->setAcceptedMouseButtons(ALLMOUSEBUTTONS);
			return;
		}
	}
}
