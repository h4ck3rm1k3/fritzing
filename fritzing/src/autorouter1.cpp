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

$Revision$:
$Author$:
$Date$

********************************************************************/

#include "autorouter1.h"
#include "sketchwidget.h"
#include "debugdialog.h"
#include "virtualwire.h"
#include "tracewire.h"

#include <math.h>
#include <QApplication>

static int kExtraLength = 1000000;

struct Edge {
	ConnectorItem * from;
	ConnectorItem * to;
	double distance;
};

bool edgeGreaterThan(Edge * e1, Edge * e2)
{
	return e1->distance > e2->distance;
}

static int keepOut = 4;
static int boundingKeepOut = 4;

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

Autorouter1::Autorouter1(SketchWidget * sketchWidget)
{
	m_progressDialog = NULL;
	m_sketchWidget = sketchWidget;
}

Autorouter1::~Autorouter1()
{
}

void Autorouter1::start(QProgressDialog * progressDialog)
{
	// TODO: put this in a command object
	// TODO: tighten path between connectors once trace has succeeded
	// TODO: for a given net, after each trace, recalculate subsequent path based on distance to existing equipotential traces
	
	m_progressDialog = progressDialog;
	m_sketchWidget->ensureLayerVisible(ViewLayer::Copper0);
	m_sketchWidget->ensureLayerVisible(ViewLayer::Jumperwires);

	clearTraces(m_sketchWidget, false);
	updateRatsnest(false);
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

	if (m_progressDialog && m_progressDialog->wasCanceled()) {
		cleanUp();
		return;
	}

	if (m_progressDialog) {
		m_progressDialog->setMaximum(edges.count());
	}
	QApplication::processEvents(); // to keep the app  from freezing

	// sort the edges by distance (bigger distances first)
	// TODO: for each edge, determine a measure of pin density, and use that, weighted with length, as the sort order
	qSort(edges.begin(), edges.end(), edgeGreaterThan);

	int edgesDone = 0;
	foreach (Edge * edge, edges) {
		/*
		DebugDialog::debug(QString("edge from %1 %2 to %3 %4, %5")
			.arg(edge->from->attachedToTitle())
			.arg(edge->from->connectorStuffID())
			.arg(edge->to->attachedToTitle())
			.arg(edge->to->connectorStuffID())
			.arg(edge->distance) );
	*/
		// if both connections are stuck to or attached to the same part
		// then use that part's boundary to constrain the path
		ItemBase * partForBounds = NULL;
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


		QList<Wire *> wires;
		if (partForBounds == NULL) {
			drawTrace(edge->from, edge->to, QPolygonF(), wires);
		}
		else {
			QRectF boundingRect = partForBounds->boundingRect();
			boundingRect.adjust(boundingKeepOut, boundingKeepOut, -boundingKeepOut, -boundingKeepOut);
			drawTrace(edge->from, edge->to, partForBounds->mapToScene(boundingRect), wires);
			//foreach (Wire * wire, wires) {
				//wire->addSticky(partForBounds, true);
				//partForBounds->addSticky(wire, true);
			//}
		}


		if (m_progressDialog) {
			m_progressDialog->setValue(++edgesDone);
		}
		QApplication::processEvents();

		if (m_progressDialog && m_progressDialog->wasCanceled()) {
			clearTraces(m_sketchWidget, false);
			cleanUp();
			return;
		}
	}

	cleanUp();

	foreach (QGraphicsItem * item, m_sketchWidget->items()) {
		TraceWire * wire = dynamic_cast<TraceWire *>(item);
		if (wire == NULL) continue;

		wire->setClipEnds(true);
		wire->update();
	}

	if (m_progressDialog) {
		m_progressDialog->setValue(edgesDone);
	}
	
	updateRatsnest(true);

	DebugDialog::debug("\n\n\nautorouting complete\n\n\n");
}

void Autorouter1::cleanUp() {
	foreach (QList<ConnectorItem *> * connectorItems, m_allPartConnectorItems) {
		delete connectorItems;
	}
	m_allPartConnectorItems.clear();
	foreach (QLine * lastDrawTrace, m_lastDrawTraces) {
		delete lastDrawTrace;
	}
	m_lastDrawTraces.clear();

}

void Autorouter1::clearTraces(SketchWidget * sketchWidget, bool deleteAll) {
	QList<Wire *> oldTraces;
	foreach (QGraphicsItem * item, sketchWidget->scene()->items()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		
		if (wire->getTrace() || wire->getJumper()) {
			if (deleteAll || wire->getAutoroutable()) {
				oldTraces.append(wire);
			}
		}
	}
	
	foreach (Wire * wire, oldTraces) {
		sketchWidget->deleteItem(wire, true, false);
	}
}

void Autorouter1::updateRatsnest(bool routed) {

	foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		if (!wire->getRatsnest()) continue;
		
		QList<ConnectorItem *>  ends;
		if (wire->findJumperOrTraced(ViewGeometry::TraceFlag | ViewGeometry::JumperFlag, ends)) {
			wire->setOpacity(0.35);
			wire->setRouted(true);
		}
		else {		
			wire->setOpacity(routed ? 0.35 : 1.0);	
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
				else if (ci->attachedTo() == cj->attachedTo() && ci->bus() == cj->bus()) {
					// leave the distance at zero
					// if connections are on the same bus on a given part
				}
				else {
					QPointF pi = ci->sceneAdjustedTerminalPoint();
					QPointF pj = cj->sceneAdjustedTerminalPoint();
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
					//.arg(ci->connectorStuffID())
					//.arg(cj->attachedToTitle())
					//.arg(cj->connectorStuffID())
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
				.arg(connectorItem->connectorStuffID())
				.arg(connectorItem->sceneAdjustedTerminalPoint().x())
				.arg(connectorItem->sceneAdjustedTerminalPoint().y()) );
		*/
	}

}

void Autorouter1::drawTrace(ConnectorItem * from, ConnectorItem * to, const QPolygonF & boundingPoly, QList<Wire *> & wires) {

	QPointF fromPos = from->sceneAdjustedTerminalPoint();
	QPointF toPos = to->sceneAdjustedTerminalPoint();

	m_drawingNet = NULL;
	foreach (QList<ConnectorItem *> * connectorItems, m_allPartConnectorItems) {
		if (connectorItems->contains(from)) {
			m_drawingNet = connectorItems;
			break;
		}
	}

	bool result = drawTrace(fromPos, toPos, from, to, wires, boundingPoly);
	if (m_progressDialog && m_progressDialog->wasCanceled()) {
		return;
	}
	if (!result) {
		result = drawTrace(toPos, fromPos, to, from, wires, boundingPoly);
		if (result) {
			DebugDialog::debug("backwards");
		}
	}
	if (m_progressDialog && m_progressDialog->wasCanceled()) {
		return;
	}
	if (result) {
		// hook everyone up
		from->tempConnectTo(wires[0]->connector0());
		wires[0]->connector0()->tempConnectTo(from);
		int last = wires.count() - 1;
		to->tempConnectTo(wires[last]->connector1());
		wires[last]->connector1()->tempConnectTo(to);
		for (int i = 0; i < last; i++) {
			ConnectorItem * c1 = wires[i]->connector1();
			ConnectorItem * c0 = wires[i + 1]->connector0();
			c1->tempConnectTo(c0);
			c0->tempConnectTo(c1);
			c1->setChained(true);
			c0->setChained(true);
		}
	}
	else {
		long newID = ItemBase::getNextID();
		ViewGeometry viewGeometry;
		viewGeometry.setLoc(fromPos);
		QLineF line(0, 0, toPos.x() - fromPos.x(), toPos.y() - fromPos.y());
		viewGeometry.setLine(line);
		viewGeometry.setJumper(true);
		viewGeometry.setAutoroutable(true);

		ItemBase * jumper = m_sketchWidget->addItem(m_sketchWidget->paletteModel()->retrieveModelPart(Wire::moduleIDName), 
													BaseCommand::SingleView, viewGeometry, newID, NULL);
		if (jumper == NULL) {
			// we're in trouble
			return;
		}

		Wire * jumperWire = dynamic_cast<Wire *>(jumper);
		jumperWire->setColorString("jumper", 1.0);
		jumperWire->setWidth(3);
		jumperWire->setSelected(false);

		from->tempConnectTo(jumperWire->connector0());
		jumperWire->connector0()->tempConnectTo(from);
		to->tempConnectTo(jumperWire->connector1());
		jumperWire->connector1()->tempConnectTo(to);

		wires.append(jumperWire);
	}
}

bool Autorouter1::drawTrace(QPointF fromPos, QPointF toPos, ConnectorItem * from, ConnectorItem * to, QList<Wire *> & wires, const QPolygonF & boundingPoly)
{
	QApplication::processEvents();
	DebugDialog::debug(QString("drawtrace from:%1 %2, to:%3 %4").arg(fromPos.x()).arg(fromPos.y()).arg(toPos.x()).arg(toPos.y()) );
	if (m_progressDialog && m_progressDialog->wasCanceled()) {
		return false;
	}

	QPoint fp(fromPos.x(), fromPos.y());
	QPoint tp(toPos.x(), toPos.y());
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

	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(fromPos);
	QLineF line(0, 0, toPos.x() - fromPos.x(), toPos.y() - fromPos.y());
	viewGeometry.setLine(line);
	viewGeometry.setTrace(true);
	viewGeometry.setAutoroutable(true);

	ItemBase * trace = m_sketchWidget->addItem(m_sketchWidget->paletteModel()->retrieveModelPart(Wire::moduleIDName), 
												BaseCommand::SingleView, viewGeometry, newID, NULL);
	if (trace == NULL) {
		// we're in trouble
		return false;
	}

	// addItemAux calls trace->setSelected(true) so unselect it
	// note: modifying selection is dangerous unless you've called SketchWidget::setIgnoreSelectionChangeEvents(true)
	trace->setSelected(false);

	TraceWire * traceWire = dynamic_cast<TraceWire *>(trace);
	traceWire->setClipEnds(false);
	traceWire->setColorString("trace", 1.0);
	traceWire->setWidth(3);

	QGraphicsItem * nearestObstacle = NULL;
	double nearestObstacleDistance = -1;

	// TODO: if a trace is chained, make set trace on the chained wire

	foreach (QGraphicsItem * item, m_sketchWidget->scene()->collidingItems(trace)) {
		if (item == from) continue;
		if (item == to) continue;

		ConnectorItem * candidateConnectorItem = NULL;
		Wire * candidateWire = dynamic_cast<Wire *>(item);
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
		else {
			candidateConnectorItem = dynamic_cast<ConnectorItem *>(item);
			if (candidateConnectorItem == NULL) {
				// only want wires or connectorItems
				continue;
			}

			candidateWire = dynamic_cast<Wire *>(candidateConnectorItem->attachedTo());
			if (candidateWire != NULL) {
				// handle this from the wire rather than the connector
				continue;
			}

			if (candidateConnectorItem->attachedTo()->viewLayerID() != traceWire->viewLayerID()) {
				// needs to be on the same layer
				continue;
			}

			if (m_drawingNet != NULL && m_drawingNet->contains(candidateConnectorItem)) {
				// it's the same potential, so it's safe to cross
				continue;
			}

			QPolygonF poly = candidateConnectorItem->mapToScene(candidateConnectorItem->boundingRect());
			QString temp = "";
			foreach (QPointF p, poly) {
				temp += QString("(%1,%2) ").arg(p.x()).arg(p.y());
			}
			/*
			DebugDialog::debug(QString("candidate connectoritem %1 %2 %3\n\t%4")
								.arg(candidateConnectorItem->connectorStuffID())
								.arg(candidateConnectorItem->attachedToTitle())
								.arg(candidateConnectorItem->attachedToID())
								.arg(temp) );
								*/
		}


		calcDistance(nearestObstacle, nearestObstacleDistance, fromPos, item);
	}

	bool inBounds = true;
	QPointF nearestBoundsIntersection;
	double nearestBoundsIntersectionDistance = 0;
	// now make sure it fits into the bounds
	if (!boundingPoly.isEmpty()) {
		QLineF l1(fromPos, toPos);
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

	if ((nearestObstacle == NULL) && inBounds) {
		wires.append(traceWire);
		return true;
	}

	m_sketchWidget->deleteItem(trace, true, false);

	if (!inBounds) {
		if ((nearestObstacle == NULL) || (nearestObstacleDistance > nearestBoundsIntersectionDistance)) {
			return tryOne(fromPos, toPos, from, to, nearestBoundsIntersection, wires, boundingPoly);
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
					.arg(ci->connectorStuffID())
					.arg(ci->attachedToTitle())
					.arg(ci->attachedToID()) );
					*/

		prePolyResult = prePoly(nearestObstacle, fromPos, toPos, leftPoint, rightPoint);
		if (!prePolyResult) return false;

		/*
		DebugDialog::debug(QString("tryleft and right from %1 %2, to %3 %4, left %5 %6, right %7 %8")
			.arg(fromPos.x()).arg(fromPos.y())
			.arg(toPos.x()).arg(toPos.y())
			.arg(leftPoint.x()).arg(leftPoint.y())
			.arg(rightPoint.x()).arg(rightPoint.y()) );
			*/

		return tryLeftAndRight(fromPos, toPos, from, to, leftPoint, rightPoint, wires, boundingPoly);
	}
	else {
		// if the obstacle is a wire, then it's a trace, so find tangents to the objects the obstacle wire is connected to

		//DebugDialog::debug(QString("nearest obstacle: wire %1").arg(wireObstacle->id()));

		QList<Wire *> chainedWires;
		QList<ConnectorItem *> ends;
		QList<ConnectorItem *> uniqueEnds;
		wireObstacle->collectChained(chainedWires, ends, uniqueEnds);

		foreach (ConnectorItem * end, ends) {
			if (tryWithWires(fromPos, toPos, from, to, wires, end, chainedWires, boundingPoly)) {
				return true;
			}
		}

		foreach (ConnectorItem * end, uniqueEnds) {
			if (tryWithWires(fromPos, toPos, from, to, wires, end, chainedWires, boundingPoly)) {
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
							   const QPolygonF & boundingPoly) {
	QPointF leftPoint, rightPoint;

	bool prePolyResult = prePoly(end, fromPos, toPos, leftPoint, rightPoint);
	if (!prePolyResult) return false;

	bool result = tryWithWire(fromPos, toPos, from, to, wires, leftPoint, chainedWires, boundingPoly);
	if (result) return result;

	return tryWithWire(fromPos, toPos, from, to, wires, rightPoint, chainedWires, boundingPoly);
}

bool Autorouter1::tryWithWire(QPointF fromPos, QPointF toPos, ConnectorItem * from, ConnectorItem * to,
							  QList<Wire *> & wires, QPointF midPoint, QList<Wire *> & chainedWires, const QPolygonF & boundingPoly)
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

	return tryOne(fromPos, toPos, from, to, midPoint, wires, boundingPoly);
}


bool Autorouter1::prePoly(QGraphicsItem * nearestObstacle, QPointF fromPos, QPointF toPos,
						  QPointF & leftPoint, QPointF & rightPoint)
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


	// extend just a little bit past the tangent
	Q_UNUSED(toPos);
	QLineF fl0(fromPos, l0);
	fl0.setLength(fl0.length() + 2);
	QLineF fr0(fromPos, r0);
	fr0.setLength(fr0.length() + 2);

	leftPoint = fl0.p2();
	rightPoint = fr0.p2();

	return true;
}

bool Autorouter1::tryLeftAndRight(QPointF fromPos, QPointF toPos, ConnectorItem * from, ConnectorItem * to, QPointF left, QPointF right,
								  QList<Wire *> & wires, const QPolygonF & boundingPoly)
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

	bool result = tryOne(fromPos, toPos, from, to, first, wires, boundingPoly);
	if (result) return result;

	if (left == right) return result;

	return tryOne(fromPos, toPos, from, to, second, wires, boundingPoly);
}

bool Autorouter1::tryOne(QPointF fromPos, QPointF toPos, ConnectorItem * from, ConnectorItem * to, QPointF midPos,
						 QList<Wire *> & wires, const QPolygonF & boundingPoly)
{
	if (fromPos == midPos) return false;

	QList<Wire *> localWires;
	bool result = drawTrace(fromPos, midPos, from, to, localWires, boundingPoly);
	if (result) {
		result = drawTrace(midPos, toPos, from, to, localWires, boundingPoly);
		if (result) {
			foreach (Wire * wire, localWires) {
				wires.append(wire);
			}
			return true;
		}
	}

	foreach (Wire * wire, localWires) {
		m_sketchWidget->deleteItem(wire, true, false);
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
