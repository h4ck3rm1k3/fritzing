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


#include "schematicsketchwidget.h"
#include "autorouter1.h"

SchematicSketchWidget::SchematicSketchWidget(ItemBase::ViewIdentifier viewIdentifier, QWidget *parent, int size, int minSize)
    : PCBSchematicSketchWidget(viewIdentifier, parent, size, minSize)
{
}

void SchematicSketchWidget::cleanUpWire(Wire * wire, QList<Wire *> & wires)
{
	Q_UNUSED(wires);
	wire->setVisible(wire->getRatsnest());
}

void SchematicSketchWidget::addViewLayers() {
	addSchematicViewLayers();
}


void SchematicSketchWidget::makeWires(QList<ConnectorItem *> & partsConnectorItems, QList <Wire *> & ratsnestWires, Wire * & modelWire)
{
	modelWire = NULL;

	// delete all the ratsnest wires before running dijkstra
	int count = partsConnectorItems.count();
	for (int i = 0; i < count - 1; i++) {
		ConnectorItem * source = partsConnectorItems[i];
		for (int j = i + 1; j < count; j++) {
			ConnectorItem * dest = partsConnectorItems[j];
			// if you can't get from i to j via wires, then add a virtual ratsnest wire
			Wire* tempWire = source->wiredTo(dest, ViewGeometry::RatsnestFlag);
			if (tempWire != NULL) {
				deleteItem(tempWire, false, false);
			}
		}
	}

	QHash<ConnectorItem *, int> indexer;
	int ix = 0;
	foreach (ConnectorItem * connectorItem, partsConnectorItems) {
		indexer.insert(connectorItem, ix++);
	}

	QVector< QVector<double> *> adjacency(count);
	for (int i = 0; i < count; i++) {
		QVector<double> * row = new QVector<double>(count);
		adjacency[i] = row;
	}

	Autorouter1::dijkstra(partsConnectorItems, indexer, adjacency);

	foreach (QVector<double> * row, adjacency) {
		delete row;
	}

	count = partsConnectorItems.count();
	for (int i = 0; i < count - 1; i++) {
		ConnectorItem * source = partsConnectorItems[i];
		ConnectorItem * dest = partsConnectorItems[i + 1];
		Wire * newWire = makeOneRatsnestWire(source, dest);
		ratsnestWires.append(newWire);
	}
}
