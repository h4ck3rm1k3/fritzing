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
//	** figure out which layer the grouped items are on and get the next z id
//	** sort itembases by z
//	** layerkin
//  ** allow mouse events to external connections
//		don't allow wires to connect with the group
//		drag doesn't keep wire connections
//	save as group
//	load into sketch
//	delete
//	undo delete
//	rotate/flip
//	undo rotate/flip
//	add to bin
//	open in new sketch (edit)
//	** z-order manipulation
//	** hide/show layer 
//		still shows group selection box
//  override QGraphicsItemGroup::paint
//	copy/paste
//  select external connections
//	undo group?

#include <QGraphicsScene>

#include "groupitem.h"
#include "groupitemkin.h"

QString GroupItem::moduleIDName = "NoteModuleID";

GroupItem::GroupItem( ModelPart* modelPart, ItemBase::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, bool topLevel, QMenu * itemMenu) 
	: GroupItemBase( modelPart, viewIdentifier, viewGeometry, id, topLevel, itemMenu)
{
}

void GroupItem::addToGroup(ItemBase * itemBase, const LayerHash & layerHash) 
{
	GroupItemBase::addToGroup(itemBase, layerHash);

	qint64 id = m_id + 1;
	foreach (ItemBase * lkpi, itemBase->layerKin()) {
		bool gotOne = false;
		foreach (ItemBase * mylkpi, layerKin()) {
			if (lkpi->viewLayerID() == mylkpi->viewLayerID()) {
				dynamic_cast<GroupItemKin *>(mylkpi)->addToGroup(lkpi, layerHash);
				gotOne = true;
				break;
			}
		}
		if (!gotOne) {
			GroupItemKin * mylkpi = new GroupItemKin(m_modelPart, m_viewIdentifier, m_viewGeometry, id++, false, NULL);
			mylkpi->setLayerKinChief(this);
			scene()->addItem(mylkpi);
			m_layerKin.append(mylkpi);
			mylkpi->addToGroup(lkpi, layerHash);
		}
	}
}

ItemBase * GroupItem::layerKinChief() {
	return this;
}

const QList<ItemBase *> & GroupItem::layerKin() {
	return m_layerKin;
}

void GroupItem::syncKinMoved(GroupItemBase * groupItemBase, QPointF newPos) {
	if (groupItemBase != this) {
		setPos(newPos);
	}
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->setPos(newPos);
	}
}

QVariant GroupItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	//DebugDialog::debug(QString("chief item change %1 %2").arg(this->id()).arg(change));
	if (m_layerKin.count() > 0) {
	    if (change == ItemPositionHasChanged) {
	    	this->syncKinMoved(this, value.toPointF());
	   	}
   	}


    return GroupItemBase::itemChange(change, value);
}
