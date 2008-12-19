/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 1617 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-22 20:32:44 +0100 (Sat, 22 Nov 2008) $

********************************************************************/



#ifndef SCHEMATICSKETCHWIDGET_H
#define SCHEMATICSKETCHWIDGET_H

#include "pcbschematicsketchwidget.h"

class SchematicSketchWidget : public PCBSchematicSketchWidget
{
	Q_OBJECT

public:
    SchematicSketchWidget(ItemBase::ViewIdentifier, QWidget *parent=0);

	void addViewLayers();
	bool canDeleteItem(QGraphicsItem * item);
	bool canCopyItem(QGraphicsItem * item);
	const QString & viewName();

signals:
	void schematicDisconnectWireSignal(	ConnectorPairHash & moveItems,  QList<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash *> & deletedConnections, QUndoCommand * parentCommand);

protected:
	void setWireVisible(Wire * wire);
	void makeWires(QList<ConnectorItem *> & partsConnectorItems, QList <Wire *> & ratsnestWires, Wire * & modelWire, RatsnestCommand *);
	void updateRatsnestStatus(CleanUpWiresCommand*, QUndoCommand *);
	//void dealWithRatsnest(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, bool connect);
	void dealWithRatsnest(long fromID, const QString & fromConnectorID, 
								  long toID, const QString & toConnectorID,
								  bool connect, class RatsnestCommand *, bool doEmit);
	ConnectorItem * tryWire(ConnectorItem * wireConnectorItem, ConnectorItem * otherConnectorItem);
	ConnectorItem * tryParts(ConnectorItem * otherConnectorItem, ConnectorItem * wireConnectorItem, QList<ConnectorItem *> partsConnectorItems);
	void reviewDeletedConnections(QList<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash * > & deletedConnections, QUndoCommand * parentCommand);
	bool canChainMultiple();
	bool alreadyOnBus(ConnectorItem * busCandidate, ConnectorItem * otherCandidate);
	void modifyNewWireConnections(qint64 wireID, ConnectorItem * & from, ConnectorItem * & to);
	ConnectorItem * lookForBreadboardConnection(ConnectorItem * & connectorItem);
	int calcDistance(Wire * wire, ConnectorItem * end, int distance, QList<Wire *> & distanceWires);
	int calcDistanceAux(ConnectorItem * from, ConnectorItem * to, int distance, QList<Wire *> & distanceWires);
	void removeRatsnestWires(QList< QList<ConnectorItem *>* > & allPartConnectorItems, CleanUpWiresCommand *);
	ConnectorItem * findEmptyBusConnectorItem(ConnectorItem * busConnectorItem);
	void chainVisible(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, bool connect);

protected:
	QHash<int, ConnectorItem *> m_wireHash;
	QList<Wire *> m_deleteStash;

};

#endif
