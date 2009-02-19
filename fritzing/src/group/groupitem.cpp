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
//	** layerkin
//		pcb view: group part in bb view, items not visually synced in pcb view
//  scene jumps when creating a new group--triggered by changing the location of the chief item
//  late updates (paint) after flip/rotate other?
//  ** allow mouse events to external connections
//		** don't allow wires to connect within the group
//		drag doesn't keep wire connections
//	save as group
//		store them in the user folder
//		create icon
//		add to bin
//		select external connections
//  save and load sketch with group(s)
//	recursive groups
//	open in new sketch (edit)
//	copy/paste
//	undo group
//	delete
//	undo delete

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
}

void GroupItem::addToGroup(ItemBase * itemBase, const LayerHash & layerHash) 
{
	Q_UNUSED(layerHash);
	m_itemsToAdd.append(itemBase);
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
	if (m_layerKin.count() > 0) {
		switch (change) {
			case ItemPositionHasChanged: 
	    		this->syncKinMoved(this, value.toPointF());
				break;
			case ItemSelectedChange:
				DebugDialog::debug(QString("chief sel change %1 %2").arg(this->id()).arg(value.toBool()));
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
	// centers the parent within the bounding rect of the group
	// fixes rotation, among other things

	QRectF itemsBoundingRect;
	foreach(ItemBase * item, m_itemsToAdd) {
		itemsBoundingRect |= (item->transform() * QTransform().translate(item->x(), item->y()))
                            .mapRect(item->boundingRect() | item->childrenBoundingRect());

	}

	this->setPos(itemsBoundingRect.center());
	saveGeometry();

	foreach(ItemBase * itemBase, m_itemsToAdd) {
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
				GroupItemKin * mylkpi = new GroupItemKin(m_modelPart, m_viewIdentifier, m_viewGeometry, id++, NULL);
				mylkpi->setLayerKinChief(this);
				scene()->addItem(mylkpi);
				m_layerKin.append(mylkpi);
				mylkpi->addToGroup(lkpi, layerHash);
			}
		}
	}

	m_itemsToAdd.clear();
}
