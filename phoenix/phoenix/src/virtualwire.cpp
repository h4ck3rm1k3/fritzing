#include "virtualwire.h"
#include "connectoritem.h"

VirtualWire::VirtualWire( ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu * itemMenu  ) 
	: Wire(modelPart, viewIdentifier,  viewGeometry,  id, itemMenu)
{
	setFlag(QGraphicsItem::ItemIsMovable, false);

	if (!getRatsnest()) {
		setAcceptedMouseButtons(Qt::NoButton);
		setFlag(QGraphicsItem::ItemIsSelectable, false);
	}
}

void VirtualWire::paint (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) {	
	if (m_hidden) return;
	
	m_connectorHoverCount = 0;			// kills any highlighting
	Wire::paint(painter, option, widget);
}

void VirtualWire::connectionChange(ConnectorItem * ) {
}

QSvgRenderer * VirtualWire::setUpConnectors(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier) {
	QSvgRenderer * renderer = Wire::setUpConnectors(modelPart, viewIdentifier);
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

void VirtualWire::setHidden(bool hidden) {
	ItemBase::setHidden(hidden);
	
	if (!hidden) {
		hideConnectors();
	}
}

void VirtualWire::tempRemoveAllConnections() {
	ConnectorItem * connectorItem = connector0();
	for (int j = connectorItem->connectedToItems().count() - 1; j >= 0; j--) {
		connectorItem->connectedToItems()[j]->tempRemove(connectorItem);
		connectorItem->tempRemove(connectorItem->connectedToItems()[j]);
	}
	connectorItem = connector1();
	for (int j = connectorItem->connectedToItems().count() - 1; j >= 0; j--) {
		connectorItem->connectedToItems()[j]->tempRemove(connectorItem);
		connectorItem->tempRemove(connectorItem->connectedToItems()[j]);
	}
}	

void VirtualWire::setChained(ConnectorItem * item, bool chained) {
	Wire::setChained(item, chained);
	item->setHidden(!chained);
}


