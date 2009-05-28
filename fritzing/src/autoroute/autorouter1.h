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

#ifndef AUTOROUTER1_H
#define AUTOROUTER1_H

#include <QAction>
#include <QHash>
#include <QVector>
#include <QList>
#include <QPointF>
#include <QGraphicsItem>
#include <QLine>
#include <QProgressDialog>
#include <QUndoCommand>

#include "../viewgeometry.h"

class Autorouter1 : public QObject
{
	Q_OBJECT

public:
	Autorouter1(class PCBSketchWidget *);
	~Autorouter1(void);

	void start();
	
public:
	static void dijkstra(QList<class ConnectorItem *> & vertices, QHash<class ConnectorItem *, int> & indexer, QVector< QVector<double> *> adjacency, ViewGeometry::WireFlags alreadyWiredBy);
	static void collectAllNets(class SketchWidget *, QHash<class ConnectorItem *, int> & indexer, QList< QList<class ConnectorItem *>* > & allPartConnectorItems);

protected:
	bool drawTrace(class ConnectorItem * from, class ConnectorItem * to, const QPolygonF & boundingPoly, QList<class Wire *> & wires);
	bool drawTrace(QPointF fromPos, QPointF toPos, class ConnectorItem * from, class ConnectorItem * to, QList<class Wire *> & wires, const QPolygonF & boundingPoly, int level, QPointF endPos, bool recurse, bool & shortcut);
	bool tryLeftAndRight(QPointF fromPos, QPointF toPos, class ConnectorItem * from, class ConnectorItem * to, QPointF right, QPointF left, QList<class Wire *> & wires, const QPolygonF & boundingPoly, int level, QPointF endPos, bool & shortcut);
	bool tryOne(QPointF fromPos, QPointF toPos, class ConnectorItem * from, class ConnectorItem * to, QPointF midPos, QList<class Wire *> & wires, const QPolygonF & boundingPoly, int level, QPointF endPos, bool & shortcut);
	bool tryWithWires(QPointF fromPos, QPointF toPos, class ConnectorItem * from, class ConnectorItem * to, QList<class Wire *> & wires, class ConnectorItem * end, QList<class Wire *> & chainedWires, const QPolygonF & boundingPoly, int level, QPointF endPos, bool & shortcut);
	bool tryWithWire(QPointF fromPos, QPointF toPos, class ConnectorItem * from, class ConnectorItem * to, QList<class Wire *> & wires, QPointF midpoint, QList<class Wire *> & chainedWires, const QPolygonF & boundingPoly, int level, QPointF endPos, bool & shortcut);
	bool prePoly(QGraphicsItem * nearestObstacle, QPointF fromPos, QPointF toPos, QPointF & leftPoint, QPointF & rightPoint, bool adjust);
	void cleanUp();
	void updateRatsnest(bool routed, QUndoCommand * parentCommand);
	void drawJumper(ConnectorItem * from, ConnectorItem * to, class ItemBase * partForBounds);
	void restoreOriginalState(QUndoCommand * parentCommand);
	void addToUndo(Wire * wire, QUndoCommand * parentCommand);
	void addToUndo(QUndoCommand * parentCommand);
	void reduceWires(QList<Wire *> & wires, ConnectorItem * from, ConnectorItem * to, const QPolygonF & boundingPoly);
	Wire * reduceWiresAux(QList<Wire *> & wires, ConnectorItem * from, ConnectorItem * to, QPointF fromPos, QPointF toPos, const QPolygonF & boundingPoly);
	bool clean90(ConnectorItem * from, ConnectorItem * to, QList<Wire *> & oldWires);
	bool clean90(QPointF fromPos, QPointF toPos, QList<Wire *> & newWires, int level);
	QPointF calcPrimePoint(ConnectorItem *);
	void findNearestIntersection(QLineF & l1, QPointF & fromPos, const QPolygonF & boundingPoly, bool & inBounds, QPointF & nearestBoundsIntersection, qreal & nearestBoundsIntersectionDistance); 
	class TraceWire * drawOneTrace(QPointF fromPos, QPointF toPos, int width);
	bool hitsObstacle(ItemBase * traceWire, ItemBase * ignore); 
	bool drawThree(QPointF fromPos, QPointF toPos, QPointF d1, QPointF d2, QList<Wire *> & newWires, int level, bool recurse);
	bool drawTwo(QPointF fromPos, QPointF toPos, QPointF d1, QList<Wire *> & newWires, int level, bool recurse);
	void clearLastDrawTraces();

public:
	static void calcDistance(QGraphicsItem * & nearestObstacle, double & nearestObstacleDistance, QPointF fromPos, QGraphicsItem * item);
	static double calcDistance(QPointF fromPos, QGraphicsItem *);
	static double distanceToLine(QPointF fromPos, QPointF p1, QPointF p2);
	static void clearTraces(PCBSketchWidget * sketchWidget, bool deleteAll, QUndoCommand * parentCommand);
	static void addUndoConnections(PCBSketchWidget * sketchWidget, bool connect, QList<Wire *> & wires, QUndoCommand * parentCommand);

public slots:
	void cancel();
	void cancelTrace();
	void stopTrace();

signals:
	void setMaximumProgress(int);
	void setProgressValue(int);

protected:
	class PCBSketchWidget * m_sketchWidget;
	QList< QLine * > m_lastDrawTraces;
	QList< QList<class ConnectorItem *>* > m_allPartConnectorItems;
	QList<class ConnectorItem *> * m_drawingNet;
	bool m_cancelled;
	bool m_cancelTrace;
	bool m_stopTrace;
	QGraphicsItem * m_nearestObstacle;
};

#endif
