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
//  ** allow mouse events to external connections
//		** don't allow wires to connect within the group
//		** drag doesn't keep wire connections
//		connect within
//  female connectors in modules
//		what happens to ratsnest wires
//		connect female connectees
//  traces in modules
//  save and load sketch with group(s)
//  drop group into sketch
//		command objects go where?
//	export with groups
//	recursive groups
//		recursive model part ownership
//	open in new sketch (edit)
//	copy/paste
//	undo group
//	delete
//	undo delete
//	rotate group: connected external wires don't updateConnections
//	do model parts for grouped items get added as children to the group modelpart?

//	** save as group
//		** store them in the user folder
//		** add to bin
//		** create icon svg
//			** use the bounding rect of the items instead of the scene rect
//		** xml: something that says group; pointer to icon; properties
//		** select external connections
//	** layerkin
//		** pcb view: group part in bb view, items not visually synced in pcb view
//  ** scene jumps when creating a new group--triggered by changing the location of the chief item
//  ** late updates (paint) after flip/rotate, other?
//	** z-order manipulation
//	** hide/show layer 
//		** still shows group selection box
//  ** override QGraphicsItemGroup::paint
//	** rotate/flip
//		** need to center itembase in bounding rect
//		** unable to rotate in pcb view (selection bug?)
//		** is flip always allowed?
//		** undo
//	** figure out which layer the grouped items are on and get the next z id
//	** sort itembases by z
//  ** select/unselect bug

#include <QGraphicsScene>

#include "groupitem.h"
#include "groupitemkin.h"
#include "../debugdialog.h"

QString GroupItem::moduleIDName = "GroupModuleID";

GroupItem::GroupItem( ModelPart* modelPart, ItemBase::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu) 
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
	GroupItemBase::collectFemaleConnectees(items);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->collectFemaleConnectees(items);
	}
}
