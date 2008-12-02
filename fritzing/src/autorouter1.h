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

#include "viewgeometry.h"

class Autorouter1
{
public:
	Autorouter1(class SketchWidget *);
	~Autorouter1(void);

	void start(QProgressDialog *);
	
public:
	static void dijkstra(QList<class ConnectorItem *> & vertices, QHash<class ConnectorItem *, int> & indexer, QVector< QVector<double> *> adjacency, ViewGeometry::WireFlags alreadyWiredBy);
	static void collectAllNets(class SketchWidget *, QHash<class ConnectorItem *, int> & indexer, QList< QList<class ConnectorItem *>* > & allPartConnectorItems);

protected:
	void drawTrace(class ConnectorItem * from, class ConnectorItem * to, const QPolygonF & boundingPoly, QList<class Wire *> & wires);
	bool drawTrace(QPointF fromPos, QPointF toPos, class ConnectorItem * from, class ConnectorItem * to, QList<class Wire *> & wires, const QPolygonF & boundingPoly);
	bool tryLeftAndRight(QPointF fromPos, QPointF toPos, class ConnectorItem * from, class ConnectorItem * to, QPointF right, QPointF left, QList<class Wire *> & wires, const QPolygonF & boundingPoly);
	bool tryOne(QPointF fromPos, QPointF toPos, class ConnectorItem * from, class ConnectorItem * to, QPointF midPos, QList<class Wire *> & wires, const QPolygonF & boundingPoly);
	bool tryWithWires(QPointF fromPos, QPointF toPos, class ConnectorItem * from, class ConnectorItem * to, QList<class Wire *> & wires, class ConnectorItem * end, QList<class Wire *> & chainedWires, const QPolygonF & boundingPoly);
	bool tryWithWire(QPointF fromPos, QPointF toPos, class ConnectorItem * from, class ConnectorItem * to, QList<class Wire *> & wires, QPointF midpoint, QList<class Wire *> & chainedWires, const QPolygonF & boundingPoly);
	bool prePoly(QGraphicsItem * nearestObstacle, QPointF fromPos, QPointF toPos, QPointF & leftPoint, QPointF & rightPoint);
	void cleanUp();
	void updateRatsnest(bool routed);

public:
	static void calcDistance(QGraphicsItem * & nearestObstacle, double & nearestObstacleDistance, QPointF fromPos, QGraphicsItem * item);
	static double calcDistance(QPointF fromPos, QGraphicsItem *);
	static double distanceToLine(QPointF fromPos, QPointF p1, QPointF p2);
	static void clearTraces(SketchWidget * sketchWidget, bool deleteAll);

protected:
	class SketchWidget * m_sketchWidget;
	QList< QLine * > m_lastDrawTraces;
	QList< QList<class ConnectorItem *>* > m_allPartConnectorItems;
	QList<class ConnectorItem *> * m_drawingNet;
	QProgressDialog * m_progressDialog;


};

#endif
