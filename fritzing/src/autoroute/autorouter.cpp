/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.a

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

#include "autorouter.h"
#include "../sketch/pcbsketchwidget.h"
#include "../debugdialog.h"
#include "../items/virtualwire.h"
#include "../items/tracewire.h"
#include "../items/jumperitem.h"
#include "../utils/graphicsutils.h"
#include "../connectors/connectoritem.h"
#include "../items/moduleidnames.h"
#include "../processeventblocker.h"

#include <qmath.h>
#include <QApplication>

Autorouter::Autorouter(PCBSketchWidget * sketchWidget)
{
	m_sketchWidget = sketchWidget;
	m_stopTracing = m_cancelTrace = m_cancelled = false;
}

Autorouter::~Autorouter(void)
{
}

void Autorouter::cleanUp() {
	foreach (QList<ConnectorItem *> * connectorItems, m_allPartConnectorItems) {
		delete connectorItems;
	}
	m_allPartConnectorItems.clear();
}

void Autorouter::updateRoutingStatus() {
	RoutingStatus routingStatus;
	routingStatus.zero();
	m_sketchWidget->updateRoutingStatus(routingStatus, true);
}

TraceWire * Autorouter::drawOneTrace(QPointF fromPos, QPointF toPos, int width, ViewLayer::ViewLayerSpec viewLayerSpec)
{
	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(fromPos);
	QLineF line(0, 0, toPos.x() - fromPos.x(), toPos.y() - fromPos.y());
	viewGeometry.setLine(line);
	viewGeometry.setTrace(true);
	viewGeometry.setAutoroutable(true);

	ItemBase * trace = m_sketchWidget->
	  addItem(m_sketchWidget->paletteModel()->retrieveModelPart(ModuleIDNames::wireModuleIDName), 
		  viewLayerSpec, BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
	if (trace == NULL) {
		// we're in trouble
		return NULL;
	}

	// addItemAux calls trace->setSelected(true) so unselect it
	// note: modifying selection is dangerous unless you've called SketchWidget::setIgnoreSelectionChangeEvents(true)
	trace->setSelected(false);
	TraceWire * traceWire = dynamic_cast<TraceWire *>(trace);
	m_sketchWidget->setClipEnds(traceWire, false);
	traceWire->setColorString(m_sketchWidget->traceColor(viewLayerSpec), 1.0);
	traceWire->setWireWidth(width, m_sketchWidget);

	return traceWire;
}

void Autorouter::expand(ConnectorItem * originalConnectorItem, QList<ConnectorItem *> & connectorItems, QSet<Wire *> & visited) 
{
	Bus * bus = originalConnectorItem->bus();
	if (bus == NULL) {
		connectorItems.append(originalConnectorItem);
	}
	else {
		// TODO: make sure both sides get bus relatives correctly

		QList<ConnectorItem *> tempConnectorItems;
		originalConnectorItem->attachedTo()->busConnectorItems(bus, tempConnectorItems);
		ViewLayer::ViewLayerID originalViewLayerID = originalConnectorItem->attachedToViewLayerID();
		foreach (ConnectorItem * connectorItem, tempConnectorItems) {
			if (m_sketchWidget->sameElectricalLayer2(connectorItem->attachedToViewLayerID(), originalViewLayerID)) {
				connectorItems.append(connectorItem);
			}
		}
	}

	for (int i = 0; i < connectorItems.count(); i++) { 
		ConnectorItem * fromConnectorItem = connectorItems[i];
		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
			TraceWire * traceWire = dynamic_cast<TraceWire *>(toConnectorItem->attachedTo());
			if (traceWire == NULL) continue;
			if (visited.contains(traceWire)) continue;

			QList<Wire *> wires;
			QList<ConnectorItem *> ends;
			traceWire->collectChained(wires, ends);
			foreach (Wire * wire, wires) {
				ConnectorItem * c0 = wire->connector0();
				if ((c0 != NULL) && c0->chained()) {
					connectorItems.append(c0);
				}
				visited.insert(wire);
			}
			foreach (ConnectorItem * end, ends) {
				if (!connectorItems.contains(end)) {
					connectorItems.append(end);
				}
			}
		}
	}
}

void Autorouter::cancel() {
	m_cancelled = true;
}

void Autorouter::cancelTrace() {
	m_cancelTrace = true;
}

void Autorouter::stopTracing() {
	m_stopTracing = true;
}
