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

#include "virtualwire.h"
#include "connectoritem.h"

VirtualWire::VirtualWire( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu * itemMenu  ) 
	: Wire(modelPart, viewIdentifier,  viewGeometry,  id, itemMenu)
{
	if (!getRatsnest()) {
		setAcceptedMouseButtons(Qt::NoButton);
		setFlag(QGraphicsItem::ItemIsSelectable, false);
	}
}

void VirtualWire::paint (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) {	
	if (m_hidden) return;
	
	m_hoverCount = m_connectorHoverCount = 0;			// kills any highlighting
	Wire::paint(painter, option, widget);
}

void VirtualWire::connectionChange(ConnectorItem * ) {
}

FSvgRenderer * VirtualWire::setUpConnectors(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier) {
	FSvgRenderer * renderer = Wire::setUpConnectors(modelPart, viewIdentifier);
	hideConnectors();
	return renderer;
}

void VirtualWire::hideConnectors() {
	// m_connector0 and m_connector1 may not yet be initialized
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * item = dynamic_cast<ConnectorItem *>(childItem);
		if (item == NULL) continue;	

		item->setHidden(true);
	}
}

void VirtualWire::setHidden(bool hide) {
	ItemBase::setHidden(hide);
	
	if (!hide) {
		hideConnectors();
	}
}

void VirtualWire::tempRemoveAllConnections() {
	ConnectorItem * connectorItem = connector0();
	for (int j = connectorItem->connectedToItems().count() - 1; j >= 0; j--) {
		connectorItem->connectedToItems()[j]->tempRemove(connectorItem, false);
		connectorItem->tempRemove(connectorItem->connectedToItems()[j], false);
	}
	connectorItem = connector1();
	for (int j = connectorItem->connectedToItems().count() - 1; j >= 0; j--) {
		connectorItem->connectedToItems()[j]->tempRemove(connectorItem, false);
		connectorItem->tempRemove(connectorItem->connectedToItems()[j], false);
	}
}	


