/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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

$Revision: 4469 $:
$Author: cohen@irascible.com $:
$Date: 2010-09-28 14:37:11 +0200 (Tue, 28 Sep 2010) $

********************************************************************/

#ifndef JROUTER_H
#define JROUTER_H

#include <QAction>
#include <QHash>
#include <QVector>
#include <QList>
#include <QSet>
#include <QPointF>
#include <QGraphicsItem>
#include <QLine>
#include <QProgressDialog>
#include <QUndoCommand>

#include "../viewgeometry.h"
#include "../viewlayer.h"


struct FourInts {
	int minX;
	int minY;
	int maxX;
	int maxY;
};

struct JPoint {
	JPoint(qreal _x, qreal _y, int _d) {
		x = _x;
		y = _y;
		d = _d;
	}

	qreal x;
	qreal y;
	int d;
};

struct JEdge {
	class ConnectorItem * from;
	class ConnectorItem * to;
	double distance;
	bool ground;
	QList<class ConnectorItem *> fromConnectorItems;
	QSet<class Wire *> fromTraces;
	QList<class ConnectorItem *> toConnectorItems;
	QSet<class Wire *> toTraces;
};

struct JSubedge {
	struct JEdge * edge;
	ConnectorItem * from;
	ConnectorItem * to;
	QPointF fromPoint;
	QPointF toPoint;
	class Wire * fromWire;
	class Wire * toWire;
	QPointF point;
	double distance;
};

struct GridEntry {
	enum {
		EMPTY = 1,
		IGNORE = 2,
		SAFE = 4,
		SELF = 8,
		GOAL = 16,
		OWNSIDE = 32,
		BLOCKER = 64
	};

	int distance;
	short flags;
	QGraphicsRectItem* item;
};

class JRouter : public QObject
{
	Q_OBJECT

public:
	JRouter(class PCBSketchWidget *);
	~JRouter(void);

	void start();
	
protected:
	bool drawTrace(struct JSubedge *, struct Plane *, ViewLayer::ViewLayerID viewLayerID);
	bool drawTrace(QPointF fromPos, QPointF toPos, class ConnectorItem * from, class ConnectorItem * to, QList<class Wire *> & wires, const QPolygonF & boundingPoly, int level, QPointF endPos, bool recurse, bool & shortcut);
	void cleanUp();
	void updateRoutingStatus();
	class JumperItem * drawJumperItem(struct JumperItemStruct *);
	void restoreOriginalState(QUndoCommand * parentCommand);
	void addToUndo(Wire * wire, QUndoCommand * parentCommand);
	void addToUndo(QUndoCommand * parentCommand, QList<struct JumperItemStruct *> &);
	void reduceWires(QList<Wire *> & wires, ConnectorItem * from, ConnectorItem * to, const QPolygonF & boundingPoly);
	Wire * reduceWiresAux(QList<Wire *> & wires, ConnectorItem * from, ConnectorItem * to, QPointF fromPos, QPointF toPos, const QPolygonF & boundingPoly);
	void findNearestIntersection(QLineF & l1, QPointF & fromPos, const QPolygonF & boundingPoly, bool & inBounds, QPointF & nearestBoundsIntersection, qreal & nearestBoundsIntersectionDistance); 
	class TraceWire * drawOneTrace(QPointF fromPos, QPointF toPos, int width, ViewLayer::ViewLayerSpec);
	void reduceColinearWires(QList<Wire *> &);
	bool sameY(const QPointF & fromPos0, const QPointF & fromPos1, const QPointF & toPos0, const QPointF & toPos1);
	bool sameX(const QPointF & fromPos0, const QPointF & fromPos1, const QPointF & toPos0, const QPointF & toPos1);
	void expand(ConnectorItem * connectorItem, QList<ConnectorItem *> & connectorItems, QSet<Wire *> & visited);
	bool findSpaceFor(ConnectorItem * & from, class JumperItem *, struct JumperItemStruct *, QPointF & candidate); 
	void collectEdges(QVector<int> & netCounters, QList<struct JEdge *> & edges, ViewLayer::ViewLayerID);
	bool traceSubedge(struct JSubedge* subedge, struct Plane *, class ItemBase * partForBounds, QGraphicsLineItem *, ViewLayer::ViewLayerID);
	ItemBase * getPartForBounds(struct JEdge *);
	void fixupJumperItems(QList<struct JumperItemStruct *> &);
	void runEdges(QList<JEdge *> & edges, QGraphicsLineItem * lineItem, 	
				  QList<struct JumperItemStruct *> & jumperItemStructs,
				  QVector<int> & netCounters, struct RoutingStatus &);
	void clearEdges(QList<JEdge *> & edges);
	void doCancel(QUndoCommand * parentCommand);
	bool alreadyJumper(QList<struct JumperItemStruct *> & jumperItemStructs, ConnectorItem * from, ConnectorItem * to);
	bool hasCollisions(JumperItem *, ViewLayer::ViewLayerID, QGraphicsItem *, ConnectorItem * from); 
	void updateProgress(int num, int denom);
	GridEntry * drawGridItem(qreal x1, qreal y1, qreal x2, qreal y2, int distance, short flag);
	bool propagate(JSubedge * subedge, QList<JPoint> seeds, struct Plane *, ViewLayer::ViewLayerID);
	bool backPropagate(JSubedge * subedge, QList<JPoint> seeds, struct Plane *, ViewLayer::ViewLayerID viewLayerID);
	short checkCandidate(JSubedge * subedge, QGraphicsItem * item, ViewLayer::ViewLayerID);
	JSubedge * makeSubedge(JEdge * edge, QPointF p1, ConnectorItem * from, QPointF p2, ConnectorItem * to);
	struct Tile * addTile(class NonConnectorItem * nci, struct Plane *);

protected:
	static void clearTraces(PCBSketchWidget * sketchWidget, bool deleteAll, QUndoCommand * parentCommand);
	static void addUndoConnections(PCBSketchWidget * sketchWidget, bool connect, QList<Wire *> & wires, QUndoCommand * parentCommand);
	static int deleteGridEntry(struct Tile * tile, void *);
	static int alreadyCallback(struct Tile * tile, void *);

public slots:
	void cancel();
	void cancelTrace();
	void stopTrace();

signals:
	void setMaximumProgress(int);
	void setProgressValue(int);
	void wantTopVisible();
	void wantBottomVisible();

protected:
	class PCBSketchWidget * m_sketchWidget;
	QList< QList<class ConnectorItem *>* > m_allPartConnectorItems;
	bool m_cancelled;
	bool m_cancelTrace;
	bool m_stopTrace;
	bool m_bothSidesNow;
	QGraphicsItem * m_nearestObstacle;
	QList<Wire *> m_cleanWires;
	ViewLayer::ViewLayerSpec m_viewLayerSpec;
	int m_maximumProgressPart;
	int m_currentProgressPart;
	QRectF m_maxRect;
};

#endif