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


#include "groupitemkin.h"


GroupItemKin::GroupItemKin( ModelPart* modelPart, ItemBase::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu) 
	: GroupItemBase( modelPart, viewIdentifier, viewGeometry, id, itemMenu)
{
}

ItemBase * GroupItemKin::layerKinChief() {
	return m_layerKinChief;
}

void GroupItemKin::setLayerKinChief(GroupItemBase * lkc) {
	m_layerKinChief = lkc;
}

void GroupItemKin::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	m_layerKinChief->mousePressEvent(event);
}

QVariant GroupItemKin::itemChange(GraphicsItemChange change, const QVariant &value)
{
	//DebugDialog::debug(QString("lk item change %1 %2").arg(this->id()).arg(change));
	if (m_layerKinChief != NULL) {
	    if (change == ItemPositionHasChanged) {
	    	m_layerKinChief->syncKinMoved(this, value.toPointF());
	   	}
   	}
    return GroupItemBase::itemChange(change, value);
}
