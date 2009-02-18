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

$Revision: 2085 $:
$Author: cohen@irascible.com $:
$Date: 2009-01-06 12:15:02 +0100 (Tue, 06 Jan 2009) $

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



#include "groupitemkin.h"


GroupItemKin::GroupItemKin( ModelPart* modelPart, ItemBase::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, bool topLevel, QMenu * itemMenu) 
	: GroupItemBase( modelPart, viewIdentifier, viewGeometry, id, topLevel, itemMenu)
{
}

ItemBase * GroupItemKin::layerKinChief() {
	return m_layerKinChief;
}

void GroupItemKin::setLayerKinChief(GroupItemBase * lkc) {
	m_layerKinChief = lkc;
}


