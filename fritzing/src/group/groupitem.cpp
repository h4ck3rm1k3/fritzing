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
//	layerkin
//	save and load
//	delete
//	undo delete
//	rotate/flip
//	undo rotate/flip
//	add to bin
//	open in new sketch (edit)
//	** z-order manipulation
//	hide/show layer (still shows group selection box)
//	copy/paste
//  select external connections

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
			GroupItemKin * kin = new GroupItemKin(m_modelPart, m_viewIdentifier, m_viewGeometry, id++, false, NULL);
			m_layerKin.append(kin);
			kin->addToGroup(lkpi, layerHash);
		}
	}
}

ItemBase * GroupItem::layerKinChief() {
	return this;
}

const QList<ItemBase *> & GroupItem::layerKin() {
	return m_layerKin;
}
