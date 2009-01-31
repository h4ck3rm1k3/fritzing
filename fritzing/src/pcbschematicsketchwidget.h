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
    PCBSchematicSketchWidget(ItemBase::ViewIdentifier, QWidget *parent);

	void setNewPartVisible(ItemBase *);

protected:
	void dealWithRatsnest(long fromID, const QString & fromConnectorID, 
								  long toID, const QString & toConnectorID,
								  bool connect, class RatsnestCommand *, bool doEmit);
	bool dealWithRatsnestAux(ConnectorItem * & from, ConnectorItem * & to,
							long fromID, const QString & fromConnectorID, 
							long toID, const QString & toConnectorID,
							bool connect, class RatsnestCommand *, bool doEmit);
	bool canDropModelPart(ModelPart * modelPart);
	virtual void makeWires(QList<ConnectorItem *> & partsConnectorItems, QList <Wire *> & ratsnestWires, Wire * & modelWire, RatsnestCommand *) = 0;
	virtual void removeRatsnestWires(QList< QList<ConnectorItem *>* > & allPartConnectorItems, CleanUpWiresCommand *);
	void reviewDeletedConnections(QSet<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash * > & deletedConnections, QUndoCommand * parentCommand);
	bool alreadyRatsnest(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem);
	bool canCreateWire(Wire * dragWire, ConnectorItem * from, ConnectorItem * to);
	bool bothEndsConnected(Wire * wire, ViewGeometry::WireFlags, ConnectorItem * oneEnd, QList<Wire *> & wires, QList<ConnectorItem *> & partConnectorItems);
	Wire * makeOneRatsnestWire(ConnectorItem * source, ConnectorItem * dest, RatsnestCommand *);
	bool doRatsnestOnCopy();
	void makeRatsnestViewGeometry(ViewGeometry & viewGeometry, ConnectorItem * source, ConnectorItem * dest); 
	void makeWiresChangeConnectionCommands(const QList<Wire *> & wires, QUndoCommand * parentCommand);
};

#endif
