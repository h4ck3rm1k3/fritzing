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

$Revision$:
$Author$:
$Date$

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

#include <limits>

#include "../../viewgeometry.h"
#include "../../viewlayer.h"
#include "../autorouter.h"
#include "priorityqueue.h"
#include "tile.h"

struct JEdge {
	class ConnectorItem * from;
	class ConnectorItem * to;
	double distance;
	ViewLayer::ViewLayerSpec viewLayerSpec;
	ViewLayer::ViewLayerID viewLayerID;
	QList<class ConnectorItem *> fromConnectorItems;
	QSet<class Wire *> fromTraces;
	QList<class ConnectorItem *> toConnectorItems;
	QSet<class Wire *> toTraces;
	JEdge * linkedEdge;
	Plane * plane;
	bool routed;
	class VirtualWire * vw;
	class JumperItem * jumperItem;
	int id;
};

struct PathUnit {
	enum Direction {
		None = 0,
		Left,
		Up,
		Right,
		Down
	};

	class ConnectorItem * connectorItem;
	class Wire * wire;
	Tile * tile;
	JEdge * edge;
	PathUnit * parent;
	int sourceCost;
	int destCost;
	TileRect minCostRect;
	PriorityQueue<PathUnit *> * priorityQueue;

	PathUnit(PriorityQueue<PathUnit *> * pq) {
		priorityQueue = pq;
		sourceCost = destCost = 0;
		wire = NULL;
		tile = NULL;
		connectorItem = NULL;
		edge = NULL;
		parent = NULL;
	}
};

struct CompletePath {
	int sourceCost;
	PathUnit * source;
	PathUnit * dest;
	bool goodEnough;
};

struct TilePair
{
	Tile * tile1;
	Tile * tile2;
};

struct Segment {
	static const int NotSet;

	int sMin;
	int sMax;
	int sEntry;
	int sExit;
	
	void setEntry(int entry) {
		if (sEntry != NotSet) return;

		sEntry = entry;
	}

	void setExit(int exit) {
		if (sExit != NotSet) return;

		sExit = exit;
	}
};

class GridEntry : public QGraphicsRectItem {
public:

public:
	GridEntry(QRectF &, QGraphicsItem * parent); 
};

class CMRouter : public Autorouter
{
	Q_OBJECT

public:
	CMRouter(class PCBSketchWidget *);
	~CMRouter(void);

	void start();
	
protected:
	enum OverlapType {
		IgnoreAllOverlaps = 0,
		ClipAllOverlaps,
		ReportAllOverlaps
	};

protected:
	void restoreOriginalState(QUndoCommand * parentCommand);
	void addToUndo(Wire * wire, QUndoCommand * parentCommand);
	void addToUndo(QUndoCommand * parentCommand, QList<JEdge *> &);
	void collectEdges(QList<JEdge *> & edges, Plane * plane0, Plane * plane1, ViewLayer::ViewLayerID copper0, ViewLayer::ViewLayerID copper1);
	//void fixupJumperItems(QList<JEdge *> &, ItemBase * board);
	//bool findShortcut(TileRect & tileRect, bool useX, bool targetGreater, JSubedge * subedge, QList<QPointF> & allPoints, int ix);
	//void shortenUs(QList<QPointF> & allPoints, JSubedge *);
	//class JumperItem * drawJumperItem(JEdge *, class ItemBase * board);
	//void removeCorners(QList<QPointF> & allPoints, JEdge *);
	//bool checkProposed(const QPointF & proposed, const QPointF & p1, const QPointF & p3, JEdge *, bool atStartOrEnd); 
	//Tile::TileType checkCandidate(JEdge * edge, Tile * tile);
	bool runEdges(QList<JEdge *> &, QList<Plane *> &, class ItemBase * board, QVector<int> & netCounters, struct RoutingStatus &, bool firstTime, QHash<Wire *, JEdge *> & tracesToEdges);
	void clearEdges(QList<JEdge *> & edges);
	void doCancel(QUndoCommand * parentCommand);
	void updateProgress(int num, int denom);
	GridEntry * drawGridItem(Tile * tile);
	void seedNext(PathUnit *, QList<Tile *> &, QMultiHash<Tile *, PathUnit *> & tilePathUnits);
	Plane * tilePlane(ItemBase * board, ViewLayer::ViewLayerID, QList<Tile *> & alreadyTiled);
	void tileWires(QList<Wire *> & wires, Plane * thePlane, QList<Tile *> & alreadyTiled, Tile::TileType, CMRouter::OverlapType overlapType);
	void tileWire(Wire *, Plane *, QList<Wire *> & beenThere, QList<Tile *> & alreadyTiled, Tile::TileType, CMRouter::OverlapType overlapType);
	void clearTiles(Plane * thePlane);
	void hideTiles();
	void displayBadTiles(QList<Tile *> & alreadyTiled);
	Tile * addTile(class NonConnectorItem * nci, Tile::TileType type, Plane *, QList<Tile *> & alreadyTiled, CMRouter::OverlapType);
	Tile * insertTile(Plane* thePlane, TileRect &tileRect, QList<Tile *> &alreadyTiled, QGraphicsItem *, Tile::TileType type, CMRouter::OverlapType);
	void clipInsertTile(Plane * thePlane, TileRect &, QList<Tile *> & alreadyTiled, QGraphicsItem * item, Tile::TileType type);
	void clearGridEntries();
	void appendIf(PathUnit * pathUnit, Tile * next, QList<Tile *> &, QMultiHash<Tile *, PathUnit *> & tilePathUnits, PathUnit::Direction, int tWidthNeeded);
	void sliceWireHorizontally(Wire * w, qreal angle, QPointF p1, QPointF p2, QList<QRectF> & rects);
	void sliceWireVertically(Wire * w, qreal angle, QPointF p1, QPointF p2, QList<QRectF> & rects);
	void hookUpWires(JEdge *, QList<PathUnit *> & fullPath, QList<Wire *> & wires);
	ConnectorItem * splitTrace(Wire * wire, QPointF point, ItemBase * board);
	JEdge * makeEdge(ConnectorItem * from, ConnectorItem * to, ViewLayer::ViewLayerSpec, ViewLayer::ViewLayerID, Plane *, VirtualWire *);
	void reorderEdges(QList<JEdge *> & edges, QHash<Wire *, JEdge *> & tracesToEdges);
	bool initBoard(ItemBase * board, Plane *, QList<Tile *> & alreadyTiled);
	void initPathUnit(JEdge * edge, Tile *, PriorityQueue<PathUnit *> & pq, QMultiHash<Tile *, PathUnit *> &);
	bool propagate(PriorityQueue<PathUnit *> & p1, PriorityQueue<PathUnit *> & p2, JEdge *, QHash<Wire *, JEdge *> & tracesToEdges, QMultiHash<Tile *, PathUnit *> &, ItemBase * board);
	bool propagateUnit(PathUnit * pathUnit, PriorityQueue<PathUnit *> & sourceQueue, PriorityQueue<PathUnit *> & destQueue, QList<PathUnit *> & destPathUnits, QMultiHash<Tile *, PathUnit *> &, CompletePath &);
	TileRect calcMinCostRect(PathUnit * pathUnit, Tile * next);
	bool isRedundantPath(PathUnit * pathUnit, TileRect & minCostRect, int sourceCost);
	bool goodEnough(CompletePath & completePath);
	void tracePath(JEdge *, CompletePath & completePath, QHash<Wire *, JEdge *> & tracesToEdges, ItemBase * board);
	Tile * insertTiny(Plane * thePlane, TileRect & tinyRect);
	void cleanPoints(QList<QPointF> & allPoints, JEdge *); 
	void traceSegments(QList<Segment *> & segments);
	void initConnectorSegments(int ix0, QList<PathUnit *> & fullPath, QList<Segment *> & hSegments, QList<Segment *> & vSegments);
	bool insideV(const QPointF & check, const QPointF & vertex);
	void makeAlignTiles(QMultiHash<Tile *, TileRect *> &, Plane * thePlane);
	bool overlapsOnly(QGraphicsItem * item, QList<Tile *> & alreadyTiled);

protected:
	static void clearTraces(PCBSketchWidget * sketchWidget, bool deleteAll, QUndoCommand * parentCommand);
	static void addUndoConnections(PCBSketchWidget * sketchWidget, bool connect, QList<Wire *> & wires, QUndoCommand * parentCommand);

protected:
	QRectF m_maxRect;
	TileRect m_tileMaxRect;
	QList<PathUnit *> m_pathUnits;
};

#endif
