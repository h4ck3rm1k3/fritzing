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



#ifndef PCBSCHEMATICSKETCHWIDGET_H
#define PCBSCHEMATICSKETCHWIDGET_H

#include "sketchwidget.h"

class PCBSchematicSketchWidget : public SketchWidget
{
	Q_OBJECT

public:
    PCBSchematicSketchWidget(ItemBase::ViewIdentifier, QWidget *parent=0, int size=500, int minSize=100);

	void setNewPartVisible(ItemBase *);
	void updateRatsnestStatus();
	void forwardRoutingStatusSignal(int netCount, int netRoutedCount, int connectorsLeftToRoute, int jumperCount);

protected:
	void redrawRatsnest(QHash<long, ItemBase *> & newItems);
	void dealWithRatsnest(ConnectorItem * from, ConnectorItem * to, bool connect);
	bool canDropModelPart(ModelPart * modelPart);
	virtual void makeWires(QList<ConnectorItem *> & partsConnectorItems, QList <Wire *> & ratsnestWires, Wire * & modelWire) = 0;
	void removeRatsnestWires(QList< QList<ConnectorItem *>* > & allPartConnectorItems);
	void reviewDeletedConnections(QList<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash * > & deletedConnections, QUndoCommand * parentCommand);
	bool alreadyRatsnest(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem);


};

#endif
