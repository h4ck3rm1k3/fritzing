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
//  labels?
//  bug: new module not appearing in parts bin

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
//		** pcb view: group kin not synching
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
#include "../connectoritem.h"
#include "../debugdialog.h"

QString GroupItem::moduleIDName = "GroupModuleID";

GroupItem::GroupItem( ModelPart* modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu) 
	: GroupItemBase( modelPart, viewIdentifier, viewGeometry, id, itemMenu)
{
	m_blockSync = false;
}


const QList<ItemBase *> & GroupItem::layerKin() {
	return m_layerKin;
}

void GroupItem::syncKinMoved(GroupItemBase * groupItemBase, QPointF newPos) {
	if (m_blockSync) return;

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

void GroupItem::doneAdding(const LayerHash & layerHash) 
{
	qint64 id = m_id + 1;
	foreach(ItemBase * itemBase, m_itemsToAdd) {
		foreach (ItemBase * lkpi, itemBase->layerKin()) {
			bool gotOne = false;
			foreach (ItemBase * mylkpi, layerKin()) {
				if (lkpi->viewLayerID() == mylkpi->viewLayerID()) {
					dynamic_cast<GroupItemKin *>(mylkpi)->addToGroup(lkpi);
					gotOne = true;
					break;
				}
			}
			if (!gotOne) {
				GroupItemKin * mylkpi = new GroupItemKin(m_modelPart, m_viewIdentifier, m_viewGeometry, id++, NULL);
				mylkpi->setViewLayerID(lkpi->viewLayerID(), layerHash);
				mylkpi->setLayerKinChief(this);
				scene()->addItem(mylkpi);
				m_layerKin.append(mylkpi);
				mylkpi->addToGroup(lkpi);
				mylkpi->setZValue(mylkpi->z());
			}
		}
	}

	syncKinMoved(this, this->pos());

	m_blockSync = true;				// prevent recursive moves when layerkin are moved during doneAdding
	GroupItemBase::doneAdding(layerHash);
	foreach (ItemBase * kin, m_layerKin) {
		dynamic_cast<GroupItemBase *>(kin)->doneAdding(layerHash);
	}
	m_blockSync = false;

	syncKinMoved(this, this->pos());
}

void GroupItem::collectWireConnectees(QSet<Wire *> & wires) {
	GroupItemBase::collectWireConnectees(wires);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->collectWireConnectees(wires);
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

	DebugDialog::debug(QString("incoming z: %1, chief z: %2").arg(groupItemBase->zValue()).arg(this->zValue()));

	if ((groupItemBase != this) 
		&& this->isVisible() 
		&& (!this->hidden()) && (this->zValue() < groupItemBase->zValue())) 
	{
		return true;
	}

	foreach (ItemBase * lkpi, m_layerKin) {
		if (lkpi == groupItemBase) continue;

		DebugDialog::debug(QString("lkpi z: %1").arg(lkpi->zValue()));

		if (lkpi->isVisible() 
			&& (!lkpi->hidden()) 
			&& (lkpi->zValue() < groupItemBase->zValue())  ) 
		{
			return true;
		}
	}

	return false;
}
