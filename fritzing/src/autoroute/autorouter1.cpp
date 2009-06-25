/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2009 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "autorouter1.h"
#include "../pcbsketchwidget.h"
#include "../debugdialog.h"
#include "../items/virtualwire.h"
#include "../items/tracewire.h"
#include "../connectoritem.h"

#include <math.h>
#include <QApplication>

static int kExtraLength = 1000000;

struct Edge {
	ConnectorItem * from;
	ConnectorItem * to;
	double distance;
};

bool edgeLessThan(Edge * e1, Edge * e2)
{
	return e1->distance < e2->distance;
}

bool edgeGreaterThan(Edge * e1, Edge * e2)
{
	return e1->distance > e2->distance;
}

static int keepOut = 4;
static int boundingKeepOut = 4;

static const int StandardTraceWidth = 3;

////////////////////////////////////////////////////////////////////

// tangent to polygon code adapted from http://www.geometryalgorithms.com/Archive/algorithm_0201/algorithm_0201.htm
//
// Copyright 2002, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

// isLeft(): test if a point is Left|On|Right of an infinite line.
//    Input:  three points P0, P1, and P2
//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2 on the line
//            <0 for P2 right of the line
float isLeft( QPointF P0, QPointF P1, QPointF P2 )
{
    return (P1.x() - P0.x())*(P2.y() - P0.y()) - (P2.x() - P0.x())*(P1.y() - P0.y());
}

// tests for polygon vertex ordering relative to a fixed point P
bool isAbove (QPointF p, QPointF vi, QPointF vj) {
	return isLeft(p, vi, vj) > 0;
}

// tests for polygon vertex ordering relative to a fixed point P
bool isBelow (QPointF p, QPointF vi, QPointF vj) {
	return isLeft(p, vi, vj) < 0;
}

// tangent_PointPoly(): find any polygon's exterior tangents
//    Input:  P = a 2D point (exterior to the polygon)
//            n = number of polygon vertices
//            V = array of vertices for any 2D polygon with V[n]=V[0]
//    Output: *rtan = index of rightmost tangent point V[*rtan]
//            *ltan = index of leftmost tangent point V[*ltan]
void tangent_PointPoly( QPointF P, QPolygonF & poly, int & rightTangent, int & leftTangent )
{
    float  eprev, enext;       // V[i] previous and next edge turn direction

    rightTangent = leftTangent = 0;         // initially assume V[0] = both tangents
    eprev = isLeft(poly.at(0), poly.at(1), P);
	int count = poly.count();
    for (int i = 1; i < count; i++) {
        enext = isLeft(poly.at(i), poly.at((i + 1) % count), P);
        if ((eprev <= 0) && (enext > 0)) {
            if (!isBelow(P, poly.at(i), poly.at(rightTangent)))
                rightTangent = i;
        }
        else if ((eprev > 0) && (enext <= 0)) {
            if (!isAbove(P, poly.at(i), poly.at(leftTangent)))
                leftTangent = i;
        }
        eprev = enext;
    }
}

////////////////////////////////////////////////////////////////////////

Autorouter1::Autorouter1(PCBSketchWidget * sketchWidget)
{
	m_sketchWidget = sketchWidget;
	m_stopTrace = m_cancelTrace = m_cancelled = false;
}

Autorouter1::~Autorouter1()
{
}

void Autorouter1::cancel() {
	m_cancelled = true;
}

void Autorouter1::cancelTrace() {
	m_cancelTrace = true;
}

void Autorouter1::stopTrace() {
	m_stopTrace = true;
}

void Autorouter1::start()
{
	// TODO: tighten path between connectors once trace has succeeded
	// TODO: for a given net, after each trace, recalculate subsequent path based on distance to existing equipotential traces
	
	m_sketchWidget->ensureTraceLayersVisible();

	QUndoCommand * parentCommand = new QUndoCommand("Autoroute");

	clearTraces(m_sketchWidget, false, parentCommand);
	updateRatsnest(false, parentCommand);
	// associate ConnectorItem with index
	QHash<ConnectorItem *, int> indexer;
	collectAllNets(m_sketchWidget, indexer, m_allPartConnectorItems);

	if (m_allPartConnectorItems.count() == 0) {
		return;
	}

	// want adjacency[count][count] but some C++ compilers don't like it
	int count = indexer.count();
	QVector< QVector<double> *> adjacency(count);
	for (int i = 0; i < count; i++) {
		QVector<double> * row = new QVector<double>(count);
		adjacency[i] = row;
	}

	QList<Edge *> edges;

	// run dykstra over each net

	QVector<int> netCounters(m_allPartConnectorItems.count());
	for (int i = 0; i < m_allPartConnectorItems.count(); i++) {
		netCounters[i] = (m_allPartConnectorItems[i]->count() - 1) * 2;			// since we use two connectors at a time on a net
	}
	foreach (QList<ConnectorItem *>* partConnectorItems, m_allPartConnectorItems) {
		// dijkstra will reorder *partConnectorItems
		dijkstra(*partConnectorItems, indexer, adjacency, ViewGeometry::JumperFlag | ViewGeometry::TraceFlag);
		for (int i = 0; i < partConnectorItems->count() - 1; i++) {
			Edge * edge = new Edge;
			edge->from = partConnectorItems->at(i);
			edge->to = partConnectorItems->at(i + 1);
			edge->distance = (*adjacency[indexer.value(edge->from)])[indexer.value(edge->to)];
			if (edge->distance == 0) {
				// do not autoroute this edge
				delete edge;
			}
			else {
				edges.append(edge);
			}
		}
	}

	foreach (QVector<double> * row, adjacency) {
		delete row;
	}
	adjacency.clear();

	if (m_cancelled || m_stopTrace) {
		restoreOriginalState(parentCommand);
		cleanUp();
		return;
	}

	emit setMaximumProgress(edges.count());
	QApplication::processEvents(); // to keep the app  from freezing

	// sort the edges by distance
	// TODO: for each edge, determine a measure of pin density, and use that, weighted with length, as the sort order
	qSort(edges.begin(), edges.end(), edgeLessThan);

	QPolygonF boundingPoly(m_sketchWidget->scene()->itemsBoundingRect().adjusted(-200, -200, 200, 200));

	int edgesDone = 0;
	int jumperCount = 0;
	foreach (Edge * edge, edges) {
		QList<ConnectorItem *> fromConnectorItems;
		expand(edge->from, fromConnectorItems, false);
		QList<ConnectorItem *> toConnectorItems;
		expand(edge->to, toConnectorItems, true);

		QList<Edge *> subedges;
		foreach (ConnectorItem * from, fromConnectorItems) {
			QPointF p1 = from->sceneAdjustedTerminalPoint(NULL);
			foreach (ConnectorItem * to, toConnectorItems) {
				QPointF p2 = to->sceneAdjustedTerminalPoint(NULL);
				Edge * subedge = new Edge;
				subedge->from = from;
				subedge->to = to;
				subedge->distance = (p1.x() - p2.x()) * (p1.x() - p2.x()) + (p1.y() - p2.y()) * (p1.y() - p2.y());		
				subedges.append(subedge);
			}
		}
		
		qSort(subedges.begin(), subedges.end(), edgeLessThan);

		DebugDialog::debug(QString("\n\nedge from %1 %2 %3 to %4 %5 %6, %7")
			.arg(edge->from->attachedToTitle())
			.arg(edge->from->attachedToID())
			.arg(edge->from->connectorSharedID())
			.arg(edge->to->attachedToTitle())
			.arg(edge->to->attachedToID())
			.arg(edge->to->connectorSharedID())
			.arg(edge->distance) );

		// if both connections are stuck to or attached to the same part
		// then use that part's boundary to constrain the path
		ItemBase * partForBounds = NULL;
		if (m_sketchWidget->autorouteNeedsBounds()) {
			if (edge->from->attachedTo()->stuckTo() != NULL && edge->from->attachedTo()->stuckTo() == edge->to->attachedTo()->stuckTo()) {
				partForBounds = edge->from->attachedTo()->stuckTo();
			}
			else if (edge->from->attachedTo()->sticky() && edge->from->attachedTo() == edge->to->attachedTo()->stuckTo()) {
				partForBounds = edge->from->attachedTo();
			}
			else if (edge->to->attachedTo()->sticky() && edge->to->attachedTo() == edge->from->attachedTo()->stuckTo()) {
				partForBounds = edge->to->attachedTo();
			}
			else if (edge->to->attachedTo() == edge->from->attachedTo()) {
				partForBounds = edge->from->attachedTo();
			}
			else {
				// TODO:  if we're stuck on two boards, use the union as the constraint?
			}
		}

		bool routedFlag = false;
		QList<Wire *> wires;
		foreach (Edge * subedge, subedges) {
			if (m_cancelled || m_stopTrace) break;
			if (routedFlag) break;

			ConnectorItem * from = subedge->from;
			ConnectorItem * to = subedge->to;

			// find the ratsnest connecting the two connectors
			Wire * ratsnest = NULL;
			foreach (ConnectorItem * fromToConnectorItem, from->connectedToItems()) {
				Wire * wire = dynamic_cast<Wire *>(fromToConnectorItem->attachedTo());
				if (wire == NULL) continue;
				if (!wire->getRatsnest()) continue;

				ConnectorItem * otherConnectorItem = wire->otherConnector(fromToConnectorItem);
				if (otherConnectorItem->connectedToItems().contains(to)) {
					ratsnest = wire;
					break;
				}
			}

			int ratsnestWidth = 0;
			if (ratsnest) {
				ratsnestWidth = ratsnest->width();
				ratsnest->setWidth(5);
			};

			wires.clear();
			if (!m_sketchWidget->autorouteNeedsBounds()) {
				routedFlag = drawTrace(from, to, boundingPoly, wires);
			}
			else if (partForBounds) {
				QRectF boundingRect = partForBounds->boundingRect();
				boundingRect.adjust(boundingKeepOut, boundingKeepOut, -boundingKeepOut, -boundingKeepOut);
				routedFlag = drawTrace(from, to, partForBounds->mapToScene(boundingRect), wires);
				if (routedFlag) {
					foreach (Wire * wire, wires) {
						wire->addSticky(partForBounds, true);
						partForBounds->addSticky(wire, true);
						//DebugDialog::debug(QString("added wire %1").arg(wire->id()));
					}
				}
			}
			
			if (ratsnest) {
				ratsnest->setWidth(ratsnestWidth);
			}
		}

		foreach (Edge * subedge, subedges) {
			delete subedge;
		}
		subedges.clear();

		if (!routedFlag && !m_stopTrace) {
			drawJumper(edge->from, edge->to, partForBounds);
			jumperCount++;
		}

		emit setProgressValue(++edgesDone);

		for (int i = 0; i < m_allPartConnectorItems.count(); i++) {
			if (m_allPartConnectorItems[i]->contains(edge->from)) {
				netCounters[i] -= 2;
				break;
			}
		}

		int netsDone = 0;
		foreach (int c, netCounters) {
			if (c <= 0) {
				netsDone++;
			}
		}
		m_sketchWidget->forwardRoutingStatusSignal(m_allPartConnectorItems.count(), netsDone, edges.count() + 1 - edgesDone, jumperCount);

		QApplication::processEvents();

		if (m_cancelled) {
			clearTraces(m_sketchWidget, false, NULL);
			restoreOriginalState(parentCommand);
			cleanUp();
			return;
		}

		if (m_stopTrace) {
			break;
		}
	}

	cleanUp();
	foreach (Edge * edge, edges) {
		delete edge;
	}
	edges.clear();

	addToUndo(parentCommand);

	emit setProgressValue(edgesDone);
	
	updateRatsnest(!m_stopTrace, parentCommand);
	m_sketchWidget->updateRatsnestStatus(NULL, parentCommand);
	m_sketchWidget->pushCommand(parentCommand);
	DebugDialog::debug("\n\n\nautorouting complete\n\n\n");
}

void Autorouter1::expand(ConnectorItem * originalConnectorItem, QList<ConnectorItem *> & connectorItems, bool onlyBus) {
	Bus * bus = originalConnectorItem->bus();
	if (bus == NULL) {
		connectorItems.append(originalConnectorItem);
	}
	else {
		originalConnectorItem->attachedTo()->busConnectorItems(bus, connectorItems);
	}
	if (onlyBus) return;

	QList<Wire *> visited;
	for (int i = 0; i < connectorItems.count(); i++) { 
		ConnectorItem * fromConnectorItem = connectorItems[i];
		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
			TraceWire * traceWire = dynamic_cast<TraceWire *>(toConnectorItem->attachedTo());
			if (traceWire == NULL) continue;
			if (visited.contains(traceWire)) continue;

			QList<Wire *> wires;
			QList<ConnectorItem *> uniqueEnds;
			QList<ConnectorItem *> ends;
			traceWire->collectChained(wires, ends, uniqueEnds);
			foreach (Wire * wire, wires) {
				visited.append(wire);
			}
			foreach (ConnectorItem * end, ends) {
				if (!connectorItems.contains(end)) {
					connectorItems.append(end);
				}
			}
		}
	}
}

void Autorouter1::cleanUp() {
	foreach (QList<ConnectorItem *> * connectorItems, m_allPartConnectorItems) {
		delete connectorItems;
	}
	m_allPartConnectorItems.clear();
	clearLastDrawTraces();
}

void Autorouter1::clearLastDrawTraces() {
	foreach (QLine * lastDrawTrace, m_lastDrawTraces) {
		delete lastDrawTrace;
	}
	m_lastDrawTraces.clear();
}

void Autorouter1::clearTraces(PCBSketchWidget * sketchWidget, bool deleteAll, QUndoCommand * parentCommand) {
	QList<Wire *> oldTraces;
	foreach (QGraphicsItem * item, sketchWidget->scene()->items()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		
		if (wire->getTrace() || wire->getJumper()) {
			if (deleteAll || wire->getAutoroutable()) {
				oldTraces.append(wire);
			}
		}
		else if (wire->getRatsnest()) {
			if (parentCommand) {
				sketchWidget->makeChangeRoutedCommand(wire, false, Wire::UNROUTED_OPACITY, parentCommand);
			}
			wire->setRouted(false);
			wire->setOpacity(Wire::UNROUTED_OPACITY);	
		}
	}


	if (parentCommand) {
		addUndoConnections(sketchWidget, false, oldTraces, parentCommand);
		foreach (Wire * wire, oldTraces) {
			sketchWidget->makeDeleteItemCommand(wire, BaseCommand::SingleView, parentCommand);
		}
	}

	
	foreach (Wire * wire, oldTraces) {
		sketchWidget->deleteItem(wire, true, false, false);
	}
}

void Autorouter1::updateRatsnest(bool routed, QUndoCommand * parentCommand) {

	foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		if (!wire->getRatsnest()) continue;
		
		QList<ConnectorItem *>  ends;
		if (wire->findJumperOrTraced(ViewGeometry::TraceFlag | ViewGeometry::JumperFlag, ends)) {
			m_sketchWidget->makeChangeRoutedCommand(wire, true, Wire::ROUTED_OPACITY, parentCommand);
			wire->setOpacity(Wire::ROUTED_OPACITY);
			wire->setRouted(true);
		}
		else {	
			m_sketchWidget->makeChangeRoutedCommand(wire, routed, routed ? Wire::ROUTED_OPACITY : Wire::UNROUTED_OPACITY, parentCommand);
			wire->setOpacity(routed ? Wire::ROUTED_OPACITY : Wire::UNROUTED_OPACITY);	
			wire->setRouted(routed);
		}
	}
}


void Autorouter1::dijkstra(QList<ConnectorItem *> & vertices, QHash<ConnectorItem *, int> & indexer, QVector< QVector<double> *> adjacency, ViewGeometry::WireFlags alreadyWiredBy) {
	// TODO: this is the most straightforward dijkstra, but there are more efficient implementations

	int count = vertices.count();
	if (count < 2) return;

	int leastDistanceStartIndex = 0;
	double leastDistance = 0;

	// set up adjacency matrix
	for (int i = 0; i < count; i++) {
		for (int j = i; j < count; j++) {
			if (i == j) {
				int index = indexer[vertices[i]];
				(*adjacency[index])[index] = 0;
			}
			else {
				ConnectorItem * ci = vertices[i];
				ConnectorItem * cj = vertices[j];
				double d = 0;
				Wire * wire = ci->wiredTo(cj, alreadyWiredBy);
				if (wire && !wire->getAutoroutable()) {
					// leave the distance at zero
					// do not autoroute--user says leave it alone
				}
				else if ((ci->attachedTo() == cj->attachedTo()) && ci->bus() && (ci->bus() == cj->bus())) {
					// leave the distance at zero
					// if connections are on the same bus on a given part
				}
				else {
					QPointF pi = ci->sceneAdjustedTerminalPoint(NULL);
					QPointF pj = cj->sceneAdjustedTerminalPoint(NULL);
					double px = pi.x() - pj.x();
					double py = pi.y() - pj.y();
					d = (px * px) + (py * py);
				}
				int indexI = indexer.value(ci);
				int indexJ = indexer.value(cj);
				(*adjacency[indexJ])[indexI] = (*adjacency[indexI])[indexJ] = d;
				if ((i == 0 && j == 1) || (d < leastDistance)) {
					leastDistance = d;
					leastDistanceStartIndex = i;
				}
				//DebugDialog::debug(QString("adj from %1 %2 to %3 %4, %5")
					//.arg(ci->attachedToTitle())
					//.arg(ci->connectorSharedID())
					//.arg(cj->attachedToTitle())
					//.arg(cj->connectorSharedID())
					//.arg((*adjacency[indexJ])[indexI]) );


			}
		}
	}

	QList<ConnectorItem *> path;
	path.append(vertices[leastDistanceStartIndex]);
	int currentIndex = indexer.value(vertices[leastDistanceStartIndex]);
	QList<ConnectorItem *> todo;
	for (int i = 0; i < count; i++) {
		if (i == leastDistanceStartIndex) continue;
		todo.append(vertices[i]);
	};
	while (todo.count() > 0) {
		ConnectorItem * leastConnectorItem = todo[0];
		int leastIndex = indexer.value(todo[0]);
		double leastDistance = (*adjacency[currentIndex])[leastIndex];
		for (int i = 1; i < todo.count(); i++) {
			ConnectorItem * candidateConnectorItem = todo[i];
			int candidateIndex = indexer.value(candidateConnectorItem);
			double candidateDistance = (*adjacency[currentIndex])[candidateIndex];
			if (candidateDistance < leastDistance) {
				leastDistance = candidateDistance;
				leastIndex = candidateIndex;
				leastConnectorItem = candidateConnectorItem;
			}
		}
		path.append(leastConnectorItem);
		todo.removeOne(leastConnectorItem);
		currentIndex = leastIndex;
	}

	// should now have shortest path through vertices, so replace original list
	vertices.clear();
	//DebugDialog::debug("shortest path:");
	foreach (ConnectorItem * connectorItem, path) {
		vertices.append(connectorItem);
		/*
		DebugDialog::debug(QString("\t%1 %2 %3 %4")
				.arg(connectorItem->attachedToTitle())
				.arg(connectorItem->connectorSharedID())
				.arg(connectorItem->sceneAdjustedTerminalPoint(NULL).x())
				.arg(connectorItem->sceneAdjustedTerminalPoint(NULL).y()) );
		*/
	}

}

 bool Autorouter1::drawTrace(ConnectorItem * from, ConnectorItem * to, const QPolygonF & boundingPoly, QList<Wire *> & wires) {

	QPointF fromPos = from->sceneAdjustedTerminalPoint(NULL);
	QPointF toPos = to->sceneAdjustedTerminalPoint(NULL);

	m_drawingNet = NULL;
	foreach (QList<ConnectorItem *> * connectorItems, m_allPartConnectorItems) {
		if (connectorItems->contains(from)) {
			m_drawingNet = connectorItems;
			break;
		}
	}

	bool shortcut = false;
	bool backwards = false;
	m_autobail = 0;
	bool result = drawTrace(fromPos, toPos, from, to, wires, boundingPoly, 0, toPos, true, shortcut);
	if (m_cancelled) {
		return false;
	}

	if (m_cancelTrace || m_stopTrace) {
	}
	else if (!result) {
		//DebugDialog::debug("backwards?");
		m_autobail = 0;
		result = drawTrace(toPos, fromPos, to, from, wires, boundingPoly, 0, fromPos, true, shortcut);
		if (result) {
			backwards = true;
			//DebugDialog::debug("backwards.");
		}
	}
	if (m_cancelled) {
		return false;
	}

	// clear the cancel flag if it's been set so the next trace can proceed
	m_cancelTrace = false;

	if (result) {
		if (backwards) {
			ConnectorItem * temp = from;
			from = to;
			to = temp;
		}

		bool cleaned = false;
		switch (m_sketchWidget->cleanType()) {
			case PCBSketchWidget::noClean:
				break;
			case PCBSketchWidget::ninetyClean:
				cleaned = clean90(from, to, wires);
				break;
		}

		if (cleaned) {
			reduceColinearWires(wires);
		}
		else {
			reduceWires(wires, from, to, boundingPoly);
		}

		// hook everyone up
		from->tempConnectTo(wires[0]->connector0(), false);
		wires[0]->connector0()->tempConnectTo(from, false);
		int last = wires.count() - 1;
		to->tempConnectTo(wires[last]->connector1(), false);
		wires[last]->connector1()->tempConnectTo(to, false);
		for (int i = 0; i < last; i++) {
			ConnectorItem * c1 = wires[i]->connector1();
			ConnectorItem * c0 = wires[i + 1]->connector0();
			c1->tempConnectTo(c0, false);
			c0->tempConnectTo(c1, false);
		}
		return true;
	}

	return false;
}

 void Autorouter1::drawJumper(ConnectorItem * from, ConnectorItem * to, ItemBase * partForBounds) {
	QPointF fromPos = from->sceneAdjustedTerminalPoint(NULL);
	QPointF toPos = to->sceneAdjustedTerminalPoint(NULL);
 	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(fromPos);
	QLineF line(0, 0, toPos.x() - fromPos.x(), toPos.y() - fromPos.y());
	viewGeometry.setLine(line);
	m_sketchWidget->setJumperFlags(viewGeometry);
	viewGeometry.setAutoroutable(true);

	ItemBase * jumper = m_sketchWidget->addItem(m_sketchWidget->paletteModel()->retrieveModelPart(ItemBase::wireModuleIDName), 
												BaseCommand::SingleView, viewGeometry, newID, -1, -1, NULL, NULL);
	if (jumper == NULL) {
		// we're in trouble
		return;
	}

	Wire * jumperWire = dynamic_cast<Wire *>(jumper);
	jumperWire->setColorString(m_sketchWidget->jumperColor(), Wire::UNROUTED_OPACITY);
	jumperWire->setWidth(m_sketchWidget->jumperWidth());
	jumperWire->setSelected(false);

	from->tempConnectTo(jumperWire->connector0(), false);
	jumperWire->connector0()->tempConnectTo(from, false);
	to->tempConnectTo(jumperWire->connector1(), false);
	jumperWire->connector1()->tempConnectTo(to, false);

	if (partForBounds) {
		jumperWire->addSticky(partForBounds, true);
		partForBounds->addSticky(jumperWire, true);
	}
 }

bool Autorouter1::drawTrace(QPointF fromPos, QPointF toPos, ConnectorItem * from, ConnectorItem * to, QList<Wire *> & wires, const QPolygonF & boundingPoly, int level, QPointF endPos, bool recurse, bool & shortcut)
{
	if (++m_autobail > 300) {
		return false;
	}

	QApplication::processEvents();
	DebugDialog::debug(QString("%5 drawtrace from:%1 %2, to:%3 %4")
		.arg(fromPos.x()).arg(fromPos.y()).arg(toPos.x()).arg(toPos.y()).arg(QString(level, ' ')) );
	if (m_cancelled || m_cancelTrace || m_stopTrace) {
		return false;
	}

	// round to int to compare
	QPoint fp((int) fromPos.x(), (int) fromPos.y());
	QPoint tp((int) toPos.x(), (int) toPos.y());
	foreach (QLine * lastDrawTrace, m_lastDrawTraces) {
		if (lastDrawTrace->p1() == fp && lastDrawTrace->p2() == tp) {
			// been there done that
			return false;
		}
	}

	m_lastDrawTraces.prepend(new QLine(fp, tp));   // push most recent


	if (!boundingPoly.isEmpty()) {
		if (!boundingPoly.containsPoint(fromPos, Qt::OddEvenFill)) {
			return false;
		}
	}

	TraceWire * traceWire = drawOneTrace(fromPos, toPos, StandardTraceWidth + 1);
	if (traceWire == NULL) {
		return false;
	}

	QGraphicsItem * nearestObstacle = m_nearestObstacle = NULL;
	double nearestObstacleDistance = -1;

	// TODO: if a trace is chained, make set trace on the chained wire

	foreach (QGraphicsItem * item, m_sketchWidget->scene()->collidingItems(traceWire)) {
		if (item == from) continue;
		if (item == to) continue;

		bool gotOne = false;
		Wire * candidateWire = m_sketchWidget->autorouteCheckWires() ? dynamic_cast<Wire *>(item) : NULL;
		if (candidateWire != NULL) {
			if (candidateWire->getTrace() &&
				candidateWire->connector0()->connectionsCount() == 0 &&
				candidateWire->connector1()->connectionsCount() == 0)
			{
				// this is part of the trace we're trying to draw
				continue;
			}

			if (!candidateWire->getTrace()) {
				continue;
			}

			if (candidateWire->viewLayerID() != traceWire->viewLayerID()) {
				// needs to be on the same layer (shouldn't get here until we have traces on multiple layers)
				continue;
			}

			QList<Wire *> chainedWires;
			QList<ConnectorItem *> ends;
			QList<ConnectorItem *> uniqueEnds;
			candidateWire->collectChained(chainedWires, ends, uniqueEnds);
			if (ends.count() > 0 && m_drawingNet && m_drawingNet->contains(ends[0])) {
				// it's the same potential, so it's safe to cross
				continue;
			}

			gotOne = true;
			/*

			DebugDialog::debug(QString("candidate wire %1, trace:%2, %3 %4, %5 %6")
				.arg(candidateWire->id())
				.arg(candidateWire->getTrace())
				.arg(candidateWire->pos().x())
				.arg(candidateWire->pos().y())
				.arg(candidateWire->line().p2().x())
				.arg(candidateWire->line().p2().y()) );

				*/
		}
		if (!gotOne) {
			ConnectorItem * candidateConnectorItem = m_sketchWidget->autorouteCheckConnectors() ? dynamic_cast<ConnectorItem *>(item) : NULL;
			if (candidateConnectorItem != NULL) {
				candidateWire = dynamic_cast<Wire *>(candidateConnectorItem->attachedTo());
				if (candidateWire != NULL) {
					// handle this from the wire rather than the connector
					continue;
				}

				if (!sameEffectiveLayer(candidateConnectorItem->attachedTo()->viewLayerID(), traceWire->viewLayerID())) {
					// needs to be on the same layer
					continue;
				}

				if (m_drawingNet != NULL && m_drawingNet->contains(candidateConnectorItem)) {
					// it's the same potential, so it's safe to cross
					continue;
				}

				//QPolygonF poly = candidateConnectorItem->mapToScene(candidateConnectorItem->boundingRect());
				//QString temp = "";
				//foreach (QPointF p, poly) {
					//temp += QString("(%1,%2) ").arg(p.x()).arg(p.y());
				//}
				/*
				DebugDialog::debug(QString("candidate connectoritem %1 %2 %3\n\t%4")
									.arg(candidateConnectorItem->connectorSharedID())
									.arg(candidateConnectorItem->attachedToTitle())
									.arg(candidateConnectorItem->attachedToID())
									.arg(temp) );
									*/
				gotOne = true;
			}
		}
		if (!gotOne) {
			ItemBase * candidateItemBase = m_sketchWidget->autorouteCheckParts() ? dynamic_cast<ItemBase *>(item) : NULL;
			if (candidateItemBase != NULL) {
				if (candidateItemBase->itemType() == ModelPart::Wire) {
					Wire * candidateWire = qobject_cast<Wire *>(candidateItemBase);
					if (!m_cleanWires.contains(candidateWire)) continue;

					QPointF fromPos0 = traceWire->connector0()->sceneAdjustedTerminalPoint(NULL);
					QPointF fromPos1 = traceWire->connector1()->sceneAdjustedTerminalPoint(NULL);
					QPointF toPos0 = candidateWire->connector0()->sceneAdjustedTerminalPoint(NULL);
					QPointF toPos1 = candidateWire->connector1()->sceneAdjustedTerminalPoint(NULL);

					if (sameY(fromPos0, fromPos1, toPos0, toPos1)) {
						// if we're going in the same direction it's an obstacle
						// eventually, if it's the same potential, combine it
						if (qMax(fromPos0.x(), fromPos1.x()) <= qMin(toPos0.x(), toPos1.x()) || 
							qMax(toPos0.x(), toPos1.x()) <= qMin(fromPos0.x(), fromPos1.x())) 
						{
							// no overlap
							continue;
						}
						//DebugDialog::debug("got an obstacle in y");
					}
					else if (sameX(fromPos0, fromPos1, toPos0, toPos1)) {
						if (qMax(fromPos0.y(), fromPos1.y()) <= qMin(toPos0.y(), toPos1.y()) || 
							qMax(toPos0.y(), toPos1.y()) <= qMin(fromPos0.y(), fromPos1.y())) 
						{
							// no overlap
							continue;
						}
						//DebugDialog::debug("got an obstacle in x");
					}
					else {
						continue;
					}
				}
				else {
					if (from && (from->attachedTo() == candidateItemBase)) {
						continue;
					}

					if (to && (to->attachedTo() == candidateItemBase)) {
						continue;
					}
				}

				//if (candidateConnectorItem->attachedTo()->viewLayerID() != traceWire->viewLayerID()) {
					// needs to be on the same layer
					//continue;
				//}


				//QPolygonF poly = candidateItemBase->mapToScene(candidateItemBase->boundingRect());
				//QString temp = "";
				//foreach (QPointF p, poly) {
					//temp += QString("(%1,%2) ").arg(p.x()).arg(p.y());
				//}
				/*
				DebugDialog::debug(QString("candidate connectoritem %1 %2 %3\n\t%4")
									.arg(candidateConnectorItem->connectorSharedID())
									.arg(candidateConnectorItem->attachedToTitle())
									.arg(candidateConnectorItem->attachedToID())
									.arg(temp) );
									*/
				gotOne = true;
			}
		}

		if (gotOne) {
			calcDistance(nearestObstacle, nearestObstacleDistance, fromPos, item);
		}
	}

	bool inBounds = true;
	QPointF nearestBoundsIntersection;
	double nearestBoundsIntersectionDistance;
	// now make sure it fits into the bounds
	if (!boundingPoly.isEmpty()) {
		QLineF l1(fromPos, toPos);
		findNearestIntersection(l1, fromPos, boundingPoly, inBounds, nearestBoundsIntersection, nearestBoundsIntersectionDistance);
	}

	if ((nearestObstacle == NULL) && inBounds) {
		wires.append(traceWire);
		return true;
	}

	m_sketchWidget->deleteItem(traceWire, true, false, false);

	m_nearestObstacle = nearestObstacle;

	if (!recurse) {
		return false;
	}

	if (toPos != endPos) {
		// just for grins, try a direct line to the end point
		if (drawTrace(fromPos, endPos, from, to, wires, boundingPoly, level + 1, endPos, false, shortcut)) {
			shortcut = true;
			return true;
		}
	}

	if (!inBounds) {
		if ((nearestObstacle == NULL) || (nearestObstacleDistance > nearestBoundsIntersectionDistance)) {
			return tryOne(fromPos, toPos, from, to, nearestBoundsIntersection, wires, boundingPoly, level, endPos, shortcut);
		}
	}

	// hunt for a tangent from fromPos to the obstacle
	QPointF rightPoint, leftPoint;
	Wire * wireObstacle = dynamic_cast<Wire *>(nearestObstacle);
	bool prePolyResult = false;
	if (wireObstacle == NULL) {
		/*
		ConnectorItem * ci = dynamic_cast<ConnectorItem *>(nearestObstacle);
		DebugDialog::debug(QString("nearest obstacle connectoritem %1 %2 %3")
					.arg(ci->connectorSharedID())
					.arg(ci->attachedToTitle())
					.arg(ci->attachedToID()) );
					*/

		prePolyResult = prePoly(nearestObstacle, fromPos, toPos, leftPoint, rightPoint, true);
		if (!prePolyResult) return false;

		/*
		DebugDialog::debug(QString("tryleft and right from %1 %2, to %3 %4, left %5 %6, right %7 %8")
			.arg(fromPos.x()).arg(fromPos.y())
			.arg(toPos.x()).arg(toPos.y())
			.arg(leftPoint.x()).arg(leftPoint.y())
			.arg(rightPoint.x()).arg(rightPoint.y()) );
			*/

		return tryLeftAndRight(fromPos, toPos, from, to, leftPoint, rightPoint, wires, boundingPoly, level, endPos, shortcut);
	}
	else {
		// if the obstacle is a wire, then it's a trace, so find tangents to the objects the obstacle wire is connected to

		//DebugDialog::debug(QString("nearest obstacle: wire %1").arg(wireObstacle->id()));

		QList<Wire *> chainedWires;
		QList<ConnectorItem *> ends;
		QList<ConnectorItem *> uniqueEnds;
		wireObstacle->collectChained(chainedWires, ends, uniqueEnds);

		foreach (ConnectorItem * end, ends) {
			if (tryWithWires(fromPos, toPos, from, to, wires, end, chainedWires, boundingPoly, level, endPos, shortcut)) {
				return true;
			}
		}

		foreach (ConnectorItem * end, uniqueEnds) {
			if (tryWithWires(fromPos, toPos, from, to, wires, end, chainedWires, boundingPoly, level, endPos, shortcut)) {
				return true;
			}
		}

		return false;

	}

}

void Autorouter1::calcDistance(QGraphicsItem * & nearestObstacle, double & nearestObstacleDistance, QPointF fromPos, QGraphicsItem * item) {
	if (nearestObstacle == NULL) {
		nearestObstacle = item;
	}
	else {
		if (nearestObstacleDistance < 0) {
			nearestObstacleDistance = calcDistance(fromPos, nearestObstacle);
		}
		double candidateDistance = calcDistance(fromPos, item);
		if (candidateDistance < nearestObstacleDistance) {
			nearestObstacle = item;
			nearestObstacleDistance = candidateDistance;
		}
	}
}

bool Autorouter1::tryWithWires(QPointF fromPos, QPointF toPos, ConnectorItem * from, ConnectorItem * to,
							   QList<Wire *> & wires, ConnectorItem * end, QList<Wire *> & chainedWires,
							   const QPolygonF & boundingPoly, int level, QPointF endPos, bool & shortcut) {
	QPointF leftPoint, rightPoint;

	bool prePolyResult = prePoly(end, fromPos, toPos, leftPoint, rightPoint, true);
	if (!prePolyResult) return false;

	bool result = tryWithWire(fromPos, toPos, from, to, wires, leftPoint, chainedWires, boundingPoly, level, endPos, shortcut);
	if (result) return result;

	return tryWithWire(fromPos, toPos, from, to, wires, rightPoint, chainedWires, boundingPoly, level, endPos, shortcut);
}

bool Autorouter1::tryWithWire(QPointF fromPos, QPointF toPos, ConnectorItem * from, ConnectorItem * to,
							  QList<Wire *> & wires, QPointF midPoint, QList<Wire *> & chainedWires, const QPolygonF & boundingPoly,
							  int level, QPointF endPos, bool & shortcut)
{

	QLineF l(fromPos, midPoint);
	l.setLength(l.length() + kExtraLength);

	foreach (Wire * wire, chainedWires) {
		QLineF w = wire->line();
		w.translate(wire->pos());
		if (w.intersect(l, NULL) == QLineF::BoundedIntersection) {
			return false;
		}
	}

	return tryOne(fromPos, toPos, from, to, midPoint, wires, boundingPoly, level, endPos, shortcut);
}


bool Autorouter1::prePoly(QGraphicsItem * nearestObstacle, QPointF fromPos, QPointF toPos,
						  QPointF & leftPoint, QPointF & rightPoint, bool adjust)
{
	QRectF r = nearestObstacle->boundingRect();
	r.adjust(-keepOut, -keepOut, keepOut, keepOut);			// TODO: make this a variable
	QPolygonF poly = nearestObstacle->mapToScene(r);
	int leftIndex, rightIndex;
	tangent_PointPoly(fromPos, poly, leftIndex, rightIndex);
	if (leftIndex == rightIndex) {
		//DebugDialog::debug("degenerate 1");
	}
	QPointF l0 = poly.at(leftIndex);
	QPointF r0 = poly.at(rightIndex);

/*
	tangent_PointPoly(toPos, poly, leftIndex, rightIndex);
	if (leftIndex == rightIndex) {
		DebugDialog::debug("degenerate 2");
	}
	QPointF l1 = poly.at(leftIndex);
	QPointF r1 = poly.at(rightIndex);

	DebugDialog::debug(QString("prepoly from: %1 %2, from: %3 %4, to %5 %6, to %7 %8")
		.arg(l0.x()).arg(l0.y())
		.arg(r0.x()).arg(r0.y())
		.arg(l1.x()).arg(l1.y())
		.arg(r1.x()).arg(r1.y()) );

	// extend the lines from fromPos to its tangents, and from toPos to its tangents
	// where lines intersect are the new positions from which to recurse

	QLineF fl0(fromPos, l0);
	fl0.setLength(fl0.length() + kExtraLength);
	QLineF fr0(fromPos, r0);
	fr0.setLength(fr0.length() + kExtraLength);

	QLineF tl1(toPos, l1);
	tl1.setLength(tl1.length() + kExtraLength);
	QLineF tr1(toPos, r1);
	tr1.setLength(tr1.length() + kExtraLength);

	if (fl0.intersect(tl1, &leftPoint) == QLineF::BoundedIntersection) {
	}
	else if (fl0.intersect(tr1, &leftPoint) == QLineF::BoundedIntersection) {
	}
	else {
		DebugDialog::debug("intersection failed (1)");
		DebugDialog::debug(QString("%1 %2 %3 %4, %5 %6 %7 %8, %9 %10 %11 %12, %13 %14 %15 %16")
			.arg(fl0.x1()).arg(fl0.y1()).arg(fl0.x2()).arg(fl0.y2())
			.arg(fr0.x1()).arg(fr0.y1()).arg(fr0.x2()).arg(fr0.y2())
			.arg(tl1.x1()).arg(tl1.y1()).arg(tl1.x2()).arg(tl1.y2())
			.arg(tr1.x1()).arg(tr1.y1()).arg(tr1.x2()).arg(tr1.y2()) );
		// means we're already at a tangent point
		return false;
	}

	if (fr0.intersect(tl1, &rightPoint) == QLineF::BoundedIntersection) {
	}
	else if (fr0.intersect(tr1, &rightPoint) == QLineF::BoundedIntersection) {
	}
	else {
		DebugDialog::debug("intersection failed (2)");
		DebugDialog::debug(QString("%1 %2 %3 %4, %5 %6 %7 %8, %9 %10 %11 %12, %13 %14 %15 %16")
			.arg(fl0.x1()).arg(fl0.y1()).arg(fl0.x2()).arg(fl0.y2())
			.arg(fr0.x1()).arg(fr0.y1()).arg(fr0.x2()).arg(fr0.y2())
			.arg(tl1.x1()).arg(tl1.y1()).arg(tl1.x2()).arg(tl1.y2())
			.arg(tr1.x1()).arg(tr1.y1()).arg(tr1.x2()).arg(tr1.y2()) );
		// means we're already at a tangent point
		return false;
	}

	*/

	Q_UNUSED(toPos);
	if (adjust) {
		// extend just a little bit past the tangent
		QLineF fl0(fromPos, l0);
		fl0.setLength(fl0.length() + 2);
		QLineF fr0(fromPos, r0);
		fr0.setLength(fr0.length() + 2);

		leftPoint = fl0.p2();
		rightPoint = fr0.p2();
	}
	else {
		leftPoint = l0;
		rightPoint = r0;
	}

	return true;
}

bool Autorouter1::tryLeftAndRight(QPointF fromPos, QPointF toPos, ConnectorItem * from, ConnectorItem * to, QPointF left, QPointF right,
								  QList<Wire *> & wires, const QPolygonF & boundingPoly, int level, QPointF endPos, bool & shortcut)
{
	double dl = ((left.x() - toPos.x()) * (left.x() - toPos.x())) +
				((left.y() - toPos.y()) * (left.y() - toPos.y()));
	double dr = ((right.x() - toPos.x()) * (right.x() - toPos.x())) +
				((right.y() - toPos.y()) * (right.y() - toPos.y()));

	// try the one closer to toPos first
	QPointF first, second;
	if (dr <= dl) {
		first = right;
		second = left;
	}
	else {
		first = left;
		second = right;
	}

	bool result = tryOne(fromPos, toPos, from, to, first, wires, boundingPoly, level, endPos, shortcut);
	if (result) return result;

	if (left == right) return result;

	return tryOne(fromPos, toPos, from, to, second, wires, boundingPoly, level, endPos, shortcut);
}

bool Autorouter1::tryOne(QPointF fromPos, QPointF toPos, ConnectorItem * from, ConnectorItem * to, QPointF midPos,
						 QList<Wire *> & wires, const QPolygonF & boundingPoly, int level, QPointF endPos, bool & shortcut)
{
	if (fromPos == midPos) return false;

	QList<Wire *> localWires;
	bool result = drawTrace(fromPos, midPos, from, to, localWires, boundingPoly, level + 1, endPos, true, shortcut);
	if (result) {
		if (shortcut || drawTrace(midPos, toPos, from, to, localWires, boundingPoly, level + 1, endPos, true, shortcut)) {
			foreach (Wire * wire, localWires) {
				wires.append(wire);
			}
			return true;
		}
	}

	foreach (Wire * wire, localWires) {
		m_sketchWidget->deleteItem(wire, true, false, false);
	}
	return false;
}

double Autorouter1::calcDistance(QPointF fromPos, QGraphicsItem * item) {
	Wire * wire = dynamic_cast<Wire *>(item);
	if (wire != NULL) {
		QPointF p = wire->pos();
		QLineF line = wire->line();
		return distanceToLine(fromPos, p + line.p1(), p + line.p2());
	}

	// calc the distance from each line in the polygon and choose the smallest.  There's probably a better way.

	QRectF r = item->boundingRect();
	QPolygonF poly = item->mapToScene(r);
	double nearestDistance = distanceToLine(fromPos, poly.at(0), poly.at(1));
	for (int i = 1; i < poly.count() - 1; i++) {
		double candidateDistance = distanceToLine(fromPos, poly.at(i), poly.at(i + 1));
		if (candidateDistance < nearestDistance) {
			nearestDistance = candidateDistance;
		}
	}

	return nearestDistance;

}

double Autorouter1::distanceToLine(QPointF p0, QPointF p1, QPointF p2) {
	double x0 = p0.x();
	double y0 = p0.y();
	double x1 = p1.x();
	double y1 = p1.y();
	double x2 = p2.x();
	double y2 = p2.y();
	return qAbs((x2 - x1) * (y1 - y0) - (x1 - x0) * (y2 - y1)) / sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void Autorouter1::collectAllNets(SketchWidget * sketchWidget, QHash<ConnectorItem *, int> & indexer, QList< QList<class ConnectorItem *>* > & allPartConnectorItems) 
{
	// get the set of all connectors in the sketch
	QList<ConnectorItem *> allConnectors;
	foreach (QGraphicsItem * item, sketchWidget->scene()->items()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
		if (connectorItem == NULL) continue;

		allConnectors.append(connectorItem);
	}

	// find all the nets and make a list of nodes (i.e. part ConnectorItems) for each net
	while (allConnectors.count() > 0) {
		
		ConnectorItem * connectorItem = allConnectors.takeFirst();

		QList<ConnectorItem *> connectorItems;
		connectorItems.append(connectorItem);
		ConnectorItem::collectEqualPotential(connectorItems);
		if (connectorItems.count() <= 0) {
			continue;
		}

		foreach (ConnectorItem * ci, connectorItems) {
			allConnectors.removeOne(ci);
		}

		if (connectorItems.count() <= 1) {
			continue;
		}

		QList<ConnectorItem *> * partConnectorItems = new QList<ConnectorItem *>;
		ConnectorItem::collectParts(connectorItems, *partConnectorItems);

		if (partConnectorItems->count() <= 1) {
			delete partConnectorItems;
			continue;
		}

		foreach (ConnectorItem * ci, *partConnectorItems) {
			indexer.insert(ci, indexer.count());
		}
		allPartConnectorItems.append(partConnectorItems);
	}
}

void Autorouter1::restoreOriginalState(QUndoCommand * parentCommand) {
	QUndoStack undoStack;
	addToUndo(parentCommand);
	undoStack.push(parentCommand);
	undoStack.undo();
}

void Autorouter1::addToUndo(Wire * wire, QUndoCommand * parentCommand) {
	if (!wire->getAutoroutable()) {
		// it was here before the autoroute, so don't add it again
		return;
	}

	AddItemCommand * addItemCommand = new AddItemCommand(m_sketchWidget, BaseCommand::SingleView, ItemBase::wireModuleIDName, wire->getViewGeometry(), wire->id(), false, -1, -1, parentCommand);
	new CheckStickyCommand(m_sketchWidget, BaseCommand::SingleView, wire->id(), false, parentCommand);
	
	new WireWidthChangeCommand(m_sketchWidget, wire->id(), wire->width(), wire->width(), parentCommand);
	new WireColorChangeCommand(m_sketchWidget, wire->id(), wire->colorString(), wire->colorString(), wire->opacity(), wire->opacity(), parentCommand);
	addItemCommand->turnOffFirstRedo();
}

void Autorouter1::addToUndo(QUndoCommand * parentCommand) 
{
	QList<Wire *> wires;
	foreach (QGraphicsItem * item, m_sketchWidget->items()) {
		TraceWire * wire = dynamic_cast<TraceWire *>(item);
		if (wire != NULL) {
			m_sketchWidget->setClipEnds(wire, true);
			wire->update();
			if (wire->getAutoroutable()) {
				wire->setWidth(StandardTraceWidth);
			}
			addToUndo(wire, parentCommand);
			wires.append(wire);
		}
		else {
			Wire * w = dynamic_cast<Wire *>(item);
			if (w != NULL && w->getJumper()) {
				addToUndo(w, parentCommand);
				wires.append(w);
			}
		}
	}

	addUndoConnections(m_sketchWidget, true, wires, parentCommand);
}

void Autorouter1::addUndoConnections(PCBSketchWidget * sketchWidget, bool connect, QList<Wire *> & wires, QUndoCommand * parentCommand) 
{
	foreach (Wire * wire, wires) {
		if (!wire->getAutoroutable()) {
			// since the autorouter didn't change this wire, don't add undo connections
			continue;
		}

		ConnectorItem * connector1 = wire->connector1();
		foreach (ConnectorItem * toConnectorItem, connector1->connectedToItems()) {
			ChangeConnectionCommand * ccc = new ChangeConnectionCommand(sketchWidget, BaseCommand::SingleView, toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
												wire->id(), connector1->connectorSharedID(),
												connect, true, parentCommand);
			ccc->setUpdateConnections(false);
		}
		ConnectorItem * connector0 = wire->connector0();
		foreach (ConnectorItem * toConnectorItem, connector0->connectedToItems()) {
			ChangeConnectionCommand * ccc = new ChangeConnectionCommand(sketchWidget, BaseCommand::SingleView, toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
												wire->id(), connector0->connectorSharedID(),
												connect, true, parentCommand);
			ccc->setUpdateConnections(false);
		}
	}
}

void Autorouter1::reduceColinearWires(QList<Wire *> & wires)
{
	if (wires.count() < 2) return;

	for (int i = 0; i < wires.count() - 1; i++) {
		Wire * w0 = wires[i];
		Wire * w1 = wires[i + 1];

		QPointF fromPos = w0->connector0()->sceneAdjustedTerminalPoint(NULL);
		QPointF toPos = w1->connector1()->sceneAdjustedTerminalPoint(NULL);

		if (qAbs(fromPos.y() - toPos.y()) < .001 || qAbs(fromPos.x() - toPos.x()) < .001) {
			TraceWire * traceWire = drawOneTrace(fromPos, toPos, 5);
			if (traceWire == NULL)continue;

			m_sketchWidget->deleteItem(wires[i], true, false, false);
			m_sketchWidget->deleteItem(wires[i + 1], true, false, false);

			wires[i] = traceWire;
			wires.removeAt(i + 1);
			i--;								// don't forget to check the new wire
		}
	}
}

void Autorouter1::reduceWires(QList<Wire *> & wires, ConnectorItem * from, ConnectorItem * to, const QPolygonF & boundingPoly)
{
	if (wires.count() < 2) return;

	for (int i = 0; i < wires.count() - 1; i++) {
		Wire * w0 = wires[i];
		Wire * w1 = wires[i + 1];

		QPointF fromPos = w0->connector0()->sceneAdjustedTerminalPoint(NULL);
		QPointF toPos = w1->connector1()->sceneAdjustedTerminalPoint(NULL);

		Wire * traceWire = reduceWiresAux(wires, from, to, fromPos, toPos, boundingPoly);
		if (traceWire == NULL) continue;

		m_sketchWidget->deleteItem(wires[i], true, false, false);
		m_sketchWidget->deleteItem(wires[i + 1], true, false, false);

		wires[i] = traceWire;
		wires.removeAt(i + 1);
		i--;								// don't forget to check the new wire
	}
}

Wire * Autorouter1::reduceWiresAux(QList<Wire *> & wires, ConnectorItem * from, ConnectorItem * to, QPointF fromPos, QPointF toPos, const QPolygonF & boundingPoly)
{
	QLineF line(0, 0, toPos.x() - fromPos.x(), toPos.y() - fromPos.y());

	bool insidePoly = true;
	if (!boundingPoly.isEmpty()) {
		int count = boundingPoly.count();
		for (int i = 0; i < count; i++) {
			QLineF l2(boundingPoly[i], boundingPoly[(i + 1) % count]);
			QPointF intersectingPoint;
			if (line.intersect(l2, &intersectingPoint) == QLineF::BoundedIntersection) {
				insidePoly = false;
				break;
			}
		}
	}
	if (!insidePoly) return NULL;

	TraceWire * traceWire = drawOneTrace(fromPos, toPos, 5);
	if (traceWire == NULL) return NULL;

	bool intersects = false;
	foreach (QGraphicsItem * item, m_sketchWidget->scene()->collidingItems(traceWire)) {
		if (item == from) continue;
		if (item == to) continue;

		Wire * candidateWire = m_sketchWidget->autorouteCheckWires() ? dynamic_cast<Wire *>(item) : NULL;
		if (candidateWire) {
			if (!candidateWire->getTrace()) {
				continue;
			}

			if (candidateWire->viewLayerID() != traceWire->viewLayerID()) {
				// needs to be on the same layer (shouldn't get here until we have traces on multiple layers)
				continue;
			}

			if (wires.contains(candidateWire)) continue;

			// eventually check if intersecting wire has the same potential

			intersects = true;
			break;
		}

		ConnectorItem * candidateConnectorItem = m_sketchWidget->autorouteCheckWires() ? dynamic_cast<ConnectorItem *>(item) : NULL;
		if (candidateConnectorItem) {
			candidateWire = dynamic_cast<Wire *>(candidateConnectorItem->attachedTo());
			if (candidateWire != NULL) {
				// handle this from the wire rather than the connector
				continue;
			}

			if (!sameEffectiveLayer(candidateConnectorItem->attachedTo()->viewLayerID(), traceWire->viewLayerID())) {
				// needs to be on the same layer
				continue;
			}

			intersects = true;
			break;
		}
	}	
	if (intersects) {
		m_sketchWidget->deleteItem(traceWire, true, false, false);
		return NULL;
	}

	traceWire->setWidth(StandardTraceWidth);									// restore normal width
	return traceWire;
}

QPointF Autorouter1::calcPrimePoint(ConnectorItem * connectorItem) {
	QPointF c = connectorItem->sceneAdjustedTerminalPoint(NULL);
	QLineF lh(QPointF(c.x() - 999999, c.y()), QPointF(c.x() + 99999, c.y()));
	QLineF lv(QPointF(c.x(), c.y() - 999999), QPointF(c.x(), c.y() + 999999));
	QRectF r = connectorItem->attachedTo()->boundingRect();
	r.adjust(-keepOut, -keepOut, keepOut, keepOut);		
	QPolygonF poly = connectorItem->attachedTo()->mapToScene(r);
	bool inBounds;
	QPointF lhIntersection;
	qreal lhDistance;
	findNearestIntersection(lh, c, poly, inBounds, lhIntersection, lhDistance);
	QPointF lvIntersection;
	qreal lvDistance;
	findNearestIntersection(lv, c, poly, inBounds, lvIntersection, lvDistance);
	if (lhDistance <= lvDistance) return lhIntersection;

	return lvIntersection;
}

void Autorouter1::findNearestIntersection(QLineF & l1, QPointF & fromPos, const QPolygonF & boundingPoly, bool & inBounds, QPointF & nearestBoundsIntersection, qreal & nearestBoundsIntersectionDistance) 
{
	inBounds = true;
	nearestBoundsIntersectionDistance = 0;
	int count = boundingPoly.count();
	for (int i = 0; i < count; i++) {
		QLineF l2(boundingPoly[i], boundingPoly[(i + 1) % count]);
		QPointF intersectingPoint;
		if (l1.intersect(l2, &intersectingPoint) == QLineF::BoundedIntersection) {
			if (inBounds == true) {
				nearestBoundsIntersection = intersectingPoint;
				inBounds = false;
				nearestBoundsIntersectionDistance = (intersectingPoint.x() - fromPos.x()) * (intersectingPoint.x() - fromPos.x()) +
													(intersectingPoint.y() - fromPos.y()) * (intersectingPoint.y() - fromPos.y());
			}
			else {
				double d = (intersectingPoint.x() - fromPos.x()) * (intersectingPoint.x() - fromPos.x()) +
						   (intersectingPoint.y() - fromPos.y()) * (intersectingPoint.y() - fromPos.y());
				if (d < nearestBoundsIntersectionDistance) {
					nearestBoundsIntersectionDistance = d;
					nearestBoundsIntersection = intersectingPoint;
				}
			}
		}
	}
}

TraceWire * Autorouter1::drawOneTrace(QPointF fromPos, QPointF toPos, int width)
{
	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(fromPos);
	QLineF line(0, 0, toPos.x() - fromPos.x(), toPos.y() - fromPos.y());
	viewGeometry.setLine(line);
	viewGeometry.setTrace(true);
	viewGeometry.setAutoroutable(true);

	ItemBase * trace = m_sketchWidget->addItem(m_sketchWidget->paletteModel()->retrieveModelPart(ItemBase::wireModuleIDName), 
												BaseCommand::SingleView, viewGeometry, newID, -1, -1, NULL, NULL);
	if (trace == NULL) {
		// we're in trouble
		return NULL;
	}

	// addItemAux calls trace->setSelected(true) so unselect it
	// note: modifying selection is dangerous unless you've called SketchWidget::setIgnoreSelectionChangeEvents(true)
	trace->setSelected(false);
	TraceWire * traceWire = dynamic_cast<TraceWire *>(trace);
	m_sketchWidget->setClipEnds(traceWire, false);
	traceWire->setColorString(m_sketchWidget->traceColor(), Wire::UNROUTED_OPACITY);
	traceWire->setWidth(width);

	return traceWire;
}

bool Autorouter1::hitsObstacle(ItemBase * traceWire, ItemBase * ignore) 
{
	foreach (QGraphicsItem * item, m_sketchWidget->scene()->collidingItems(traceWire)) {

		ItemBase * candidateItemBase = m_sketchWidget->autorouteCheckParts() ? dynamic_cast<ItemBase *>(item) : NULL;
		if (candidateItemBase) {
			if (ignore == candidateItemBase) continue;
			if (candidateItemBase->itemType() == ModelPart::Wire) continue;

			//if (candidateConnectorItem->attachedTo()->viewLayerID() != traceWire->viewLayerID()) {
				// needs to be on the same layer
				//continue;
			//}

			return true;
		}

	}	

	return false;
}

bool Autorouter1::clean90(ConnectorItem * from, ConnectorItem * to, QList<Wire *> & oldWires) {
	QList<Wire *> newWires;
	// 1. draw wire from FROM to just outside its nearest part border
	QPointF fromPrime = calcPrimePoint(from);
	TraceWire * fromWire = drawOneTrace(from->sceneAdjustedTerminalPoint(NULL), fromPrime, StandardTraceWidth + 1);
	if (fromWire == NULL) return false;

	if (hitsObstacle(fromWire, from->attachedTo())) {
		m_sketchWidget->deleteItem(fromWire, true, false, false);
		return false;
	}

	// 2. draw wire from TO to just outside its nearest part border
	QPointF toPrime = calcPrimePoint(to);
	TraceWire * toWire = drawOneTrace(toPrime, to->sceneAdjustedTerminalPoint(NULL), StandardTraceWidth + 1);
	if (toWire == NULL) {
		m_sketchWidget->deleteItem(fromWire, true, false, false);
		return false;
	}

	if (hitsObstacle(toWire, to->attachedTo())) {
		m_sketchWidget->deleteItem(fromWire, true, false, false);
		m_sketchWidget->deleteItem(toWire, true, false, false);
		return false;
	}

	newWires.append(fromWire);
	bool result = clean90(fromPrime, toPrime, newWires, 0);
	newWires.append(toWire);
	if (result) {
		foreach (Wire * w, oldWires) {
			m_sketchWidget->deleteItem(w, true, false, false);
		}
		oldWires.clear();
		foreach (Wire * w, newWires) {
			oldWires.append(w);
			if (w != toWire && w != fromWire) {
				m_cleanWires.append(w);
				//QPointF p0 = w->connector0()->sceneAdjustedTerminalPoint(NULL);
				//QPointF p1 = w->connector1()->sceneAdjustedTerminalPoint(NULL);
				//DebugDialog::debug(QString("adding clean %1 (%2,%3) (%4,%5)")
					//.arg(w->id())
					//.arg(p0.x()).arg(p0.y())
					//.arg(p1.x()).arg(p1.y()) );
			}
		}
	}
	else {
		foreach (Wire * w, newWires) {
			m_sketchWidget->deleteItem(w, true, false, false);
		}
	}
	newWires.clear();

	return result;
}

bool Autorouter1::clean90(QPointF fromPos, QPointF toPos, QList<Wire *> & newWires, int level)
{
	// 4. if step 3 fails, try to draw as two straight lines, recursing if blocked
	QPointF f1(fromPos.x(), toPos.y());
	if (drawTwo(fromPos, toPos, f1, newWires, level, false)) {
		return true;
	}

	QPointF g1(toPos.x(), fromPos.y());
	if (drawTwo(fromPos, toPos, g1, newWires, level, false)) {
		return true;
	}

	// 3. figure the center between fromPos and toPos and try to draw as three straight lines, recursing if blocked
	QPointF center((fromPos.x() + toPos.x()) / 2.0, (fromPos.y() + toPos.y()) / 2.0);
	QPointF d1(center.x(), fromPos.y());
	QPointF d2(d1.x(), toPos.y());
	if (drawThree(fromPos, toPos, d1, d2, newWires, level, false)) {
		return true;
	}

	QPointF e1(fromPos.x(), center.y());
	QPointF e2(toPos.x(), center.y());
	if (drawThree(fromPos, toPos, e1, e2, newWires, level, false)) {
		return true;
	}

	// once more with recursion
	if (drawTwo(fromPos, toPos, f1, newWires, level, true)) {
		return true;
	}

	if (drawTwo(fromPos, toPos, g1, newWires, level, true)) {
		return true;
	}

	if (drawThree(fromPos, toPos, d1, d2, newWires, level, true)) {
		return true;
	}

	if (drawThree(fromPos, toPos, e1, e2, newWires, level, true)) {
		return true;
	}

	return false;
}

#define clearTemp() { foreach (Wire * w, temp) { m_sketchWidget->deleteItem(w, true, false, false); } temp.clear(); }
#define copyTemp() { foreach (Wire * w, temp) { newWires.append(w); } temp.clear(); }

bool Autorouter1::drawThree(QPointF fromPos, QPointF toPos, QPointF d1, QPointF d2, QList<Wire *> & newWires, int level, bool recurse) {
	bool shortcut = false;
	QList<Wire *> temp;
	if (!drawTrace(fromPos, d1, NULL, NULL, temp, QPolygonF(), 0, d1, false, shortcut)) {
		if (!recurse || (m_nearestObstacle == NULL)) {
			clearTemp();
			return false;
		}

		QPointF leftPoint, rightPoint;
		bool prePolyResult = prePoly(m_nearestObstacle, fromPos, toPos, leftPoint, rightPoint, false);
		m_nearestObstacle = NULL;
		if (!prePolyResult) return false;
		
		if (clean90(fromPos, leftPoint, temp, level + 1) && clean90(leftPoint, toPos, temp, level + 1)) {
			copyTemp();
			return true;
		}

		clearTemp();

		if (clean90(fromPos, rightPoint, temp, level + 1) && clean90(rightPoint, toPos, temp, level + 1)) {
			copyTemp();
			return true;
		}

		clearTemp();
		return false;
	}

	if (!drawTrace(d1, d2, NULL, NULL, temp, QPolygonF(), 0, d2, false, shortcut)) {
		if (!recurse || (m_nearestObstacle == NULL)) {
			clearTemp();
			return false;
		}

		QPointF leftPoint, rightPoint;
		bool prePolyResult = prePoly(m_nearestObstacle, d1, toPos, leftPoint, rightPoint, false);
		m_nearestObstacle = NULL;
		if (!prePolyResult) return false;
		
		if (clean90(d1, leftPoint, temp, level + 1) && clean90(leftPoint, toPos, temp, level + 1)) {
			copyTemp();
			return true;
		}

		clearTemp();

		if (clean90(d1, rightPoint, temp, level + 1) && clean90(rightPoint, toPos, temp, level + 1)) {
			copyTemp();
			return true;
		}

		clearTemp();
		return false;
	}

	if (drawTrace(d2, toPos, NULL, NULL, temp, QPolygonF(), 0, toPos, false, shortcut)) {
		copyTemp();
		return true;
	}

	if (!recurse || (m_nearestObstacle == NULL)) {
		clearTemp();
		return false;
	}

	QPointF leftPoint, rightPoint;
	bool prePolyResult = prePoly(m_nearestObstacle, d2, toPos, leftPoint, rightPoint, false);
	m_nearestObstacle = NULL;
	if (!prePolyResult) return false;
	
	if (clean90(d2, leftPoint, temp, level + 1) && clean90(leftPoint, toPos, temp, level + 1)) {
		copyTemp();
		return true;
	}

	clearTemp();

	if (clean90(d2, rightPoint, temp, level + 1) && clean90(rightPoint, toPos, temp, level + 1)) {
		copyTemp();
		return true;
	}

	clearTemp();
	return false;
}

bool Autorouter1::drawTwo(QPointF fromPos, QPointF toPos, QPointF d1, QList<Wire *> & newWires, int level, bool recurse) {
	bool shortcut = false;
	QList<Wire *> temp;
	if (!drawTrace(fromPos, d1, NULL, NULL, temp, QPolygonF(), 0, d1, false, shortcut)) {
		if (!recurse || (m_nearestObstacle == NULL)) {
			clearTemp();
			return false;
		}

		QPointF leftPoint, rightPoint;
		bool prePolyResult = prePoly(m_nearestObstacle, fromPos, toPos, leftPoint, rightPoint, false);
		m_nearestObstacle = NULL;
		if (!prePolyResult) return false;
		
		if (clean90(fromPos, leftPoint, temp, level + 1) && clean90(leftPoint, toPos, temp, level + 1)) {
			copyTemp();
			return true;
		}

		clearTemp();

		if (clean90(fromPos, rightPoint, temp, level + 1) && clean90(rightPoint, toPos, temp, level + 1)) {
			copyTemp();
			return true;
		}

		clearTemp();
		return false;

	}

	if (drawTrace(d1, toPos, NULL, NULL, temp, QPolygonF(), 0, toPos, false, shortcut)) {
		copyTemp();
		return true;
	}

	if (!recurse || (m_nearestObstacle == NULL)) {
		clearTemp();
		return false;
	}

	QPointF leftPoint, rightPoint;
	bool prePolyResult = prePoly(m_nearestObstacle, d1, toPos, leftPoint, rightPoint, false);
	m_nearestObstacle = NULL;
	if (!prePolyResult) return false;
	
	if (clean90(d1, leftPoint, temp, level + 1) && clean90(leftPoint, toPos, temp, level + 1)) {
		copyTemp();
		return true;
	}

	clearTemp();

	if (clean90(d1, rightPoint, temp, level + 1) && clean90(rightPoint, toPos, temp, level + 1)) {
		copyTemp();
		return true;
	}

	clearTemp();
	return false;
}

bool Autorouter1::sameY(const QPointF & fromPos0, const QPointF & fromPos1, const QPointF & toPos0, const QPointF & toPos1) {
	return  qAbs(toPos1.y() - fromPos1.y()) < 1.0 &&
		    qAbs(toPos0.y() - fromPos0.y()) < 1.0 &&
			qAbs(toPos0.y() - toPos1.y()) < 1.0;
}

bool Autorouter1::sameX(const QPointF & fromPos0, const QPointF & fromPos1, const QPointF & toPos0, const QPointF & toPos1) {
	return  qAbs(toPos1.x() - fromPos1.x()) < 1.0 &&
		    qAbs(toPos0.x() - fromPos0.x()) < 1.0 &&
			qAbs(toPos0.x() - toPos1.x()) < 1.0;
}

bool Autorouter1::sameEffectiveLayer(ViewLayer::ViewLayerID viewLayerID1, ViewLayer::ViewLayerID viewLayerID2) {
	Q_UNUSED(viewLayerID2);

	if (viewLayerID1 == ViewLayer::Copper0) return true;

	return false;
}