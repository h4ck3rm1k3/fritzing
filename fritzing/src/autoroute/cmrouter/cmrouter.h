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
#include <QPointer>
#include <QGraphicsLineItem>

#include <limits>

#include "../../viewgeometry.h"
#include "../../viewlayer.h"
#include "../../commands.h"
#include "../autorouter.h"
#include "priorityqueue.h"
#include "tile.h"

struct JEdge {
	class ConnectorItem * from;
	class ConnectorItem * to;
	double distance;
	QList<class ConnectorItem *> fromConnectorItems;
	QSet<class Wire *> fromTraces;
	QList<class ConnectorItem *> toConnectorItems;
	QSet<class Wire *> toTraces;
	bool routed;
	bool withJumper;
	QLineF line;
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
	Plane * plane;

	PathUnit(PriorityQueue<PathUnit *> * pq) {
		priorityQueue = pq;
		sourceCost = destCost = 0;
		wire = NULL;
		tile = NULL;
		connectorItem = NULL;
		edge = NULL;
		parent = NULL;
		plane = NULL;
	}
};

struct CompletePath {
	int sourceCost;
	PathUnit * source;
	PathUnit * dest;
	bool goodEnough;
};

struct TilePointRect
{
	TilePoint tilePoint;
	TileRect tileRect;
};

struct Ordering {
	QList<int> edgeIDs;
	int unroutedCount;
	int jumperCount;
	int viaCount;
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
	bool drawn();
	void setDrawn(bool);

protected:
	bool m_drawn;
};

class CMRouter : public Autorouter
{
	Q_OBJECT

public:
	CMRouter(class PCBSketchWidget *);
	~CMRouter(void);

	void start();
	bool drc(); 
	void drcClean(); 

	
public slots:
	void setMaxCycles(int);

signals:
	void setCycleMessage(const QString &);
	void setCycleCount(int);

protected:
	enum OverlapType {
		IgnoreAllOverlaps = 0,
		ClipAllOverlaps,
		ReportAllOverlaps,
		AllowEquipotentialOverlaps
	};

protected:
	void restoreOriginalState(QUndoCommand * parentCommand);
	void addToUndo(Wire * wire, QUndoCommand * parentCommand);
	void addToUndo(QUndoCommand * parentCommand);
	void collectEdges(QList<JEdge *> & edges);
	//bool findShortcut(TileRect & tileRect, bool useX, bool targetGreater, JSubedge * subedge, QList<QPointF> & allPoints, int ix);
	//void shortenUs(QList<QPointF> & allPoints, JSubedge *);
	void removeCorners(QList<QPointF> & allPoints, Plane *);
	bool checkProposed(const QPointF & proposed, const QPointF & p1, const QPointF & p3, Plane *, bool atStartOrEnd); 
	bool runEdges(QList<JEdge *> &, QVector<int> & netCounters, 
					struct RoutingStatus &, qreal keepout, bool makeJumper);
	void clearEdges(QList<JEdge *> & edges);
	void doCancel(QUndoCommand * parentCommand);
	void updateProgress(int num, int denom);
	GridEntry * drawGridItem(Tile * tile);
	void seedNext(PathUnit *, QList<Tile *> &, QMultiHash<Tile *, PathUnit *> & tilePathUnits);
	Plane * tilePlane(ViewLayer::ViewLayerID viewLayerID, ViewLayer::ViewLayerSpec, QList<Tile *> & alreadyTiled, qreal keepout, CMRouter::OverlapType, CMRouter::OverlapType wireOverlapType, bool eliminateThin);
	void tileWires(QList<Wire *> & wires, QList<Tile *> & alreadyTiled, Tile::TileType, 
					CMRouter::OverlapType overlapType, qreal keepout, bool eliminateThin);
	void tileWire(Wire *, QList<Wire *> & beenThere, QList<Tile *> & alreadyTiled, Tile::TileType, 
					CMRouter::OverlapType overlapType, qreal keepout, bool eliminateThin);
	void hideTiles();
	void displayBadTiles(QList<Tile *> & alreadyTiled);
	void displayBadTileRect(TileRect & tileRect);
	Tile * addTile(class NonConnectorItem * nci, Tile::TileType type, Plane *, QList<Tile *> & alreadyTiled, CMRouter::OverlapType, qreal keepout);
	Tile * insertTile(Plane* thePlane, TileRect &tileRect, QList<Tile *> &alreadyTiled, QGraphicsItem *, Tile::TileType type, CMRouter::OverlapType);
	void clipInsertTile(Plane * thePlane, TileRect &, QList<Tile *> & alreadyTiled, QGraphicsItem * item, Tile::TileType type);
	void clearGridEntries();
	void appendIf(PathUnit * pathUnit, Tile * next, QList<Tile *> &, QMultiHash<Tile *, PathUnit *> & tilePathUnits, PathUnit::Direction, int tWidthNeeded);
	void hookUpWires(QList<PathUnit *> & fullPath, QList<Wire *> & wires, qreal keepout);
	ConnectorItem * splitTrace(Wire * wire, QPointF point);
	void clearEdge(JEdge * edge);
	bool reorderEdges(QList<JEdge *> & edges, QGraphicsLineItem *);
	bool initBoard(ItemBase * board, Plane *, QList<Tile *> & alreadyTiled, qreal keepout);
	void initPathUnit(JEdge * edge, Tile *, PriorityQueue<PathUnit *> & pq, QMultiHash<Tile *, PathUnit *> &);
	bool propagate(PriorityQueue<PathUnit *> & p1, PriorityQueue<PathUnit *> & p2, QMultiHash<Tile *, PathUnit *> &, qreal keepout);
	bool addJumperItem(PriorityQueue<PathUnit *> & p1, PriorityQueue<PathUnit *> & p2, JEdge *, 
						QMultiHash<Tile *, PathUnit *> &, qreal keepout);
	bool propagateUnit(PathUnit * pathUnit, PriorityQueue<PathUnit *> & sourceQueue, PriorityQueue<PathUnit *> & destQueue, QList<PathUnit *> & destPathUnits, QMultiHash<Tile *, PathUnit *> &, CompletePath &);
	TileRect calcMinCostRect(PathUnit * pathUnit, Tile * next);
	TileRect calcMinCostRect(PathUnit * pathUnit, TileRect & next);
	bool isRedundantPath(PathUnit * pathUnit, TileRect & minCostRect, int sourceCost);
	bool goodEnough(CompletePath & completePath);
	void tracePath(CompletePath & completePath, qreal keepout);
	void cleanPoints(QList<QPointF> & allPoints, Plane *); 
	void traceSegments(QList<Segment *> & segments);
	void initConnectorSegments(int ix0, QList<PathUnit *> & fullPath, QList<Segment *> & hSegments, QList<Segment *> & vSegments);
	bool insideV(const QPointF & check, const QPointF & vertex);
	void makeAlignTiles(QMultiHash<Tile *, TileRect *> &, Plane * thePlane);
	bool overlapsOnly(QGraphicsItem * item, QList<Tile *> & alreadyTiled);
	void eliminateThinTiles(QList<TileRect> & tileRects, Plane * thePlane);
	void eliminateThinTiles2(QList<TileRect> & tileRects, Plane * thePlane);
	bool drc(qreal keepout, CMRouter::OverlapType, CMRouter::OverlapType wiresOverlap, bool eliminateThin, bool combinePlanes); 
	void clearPlane(Plane * thePlane, bool rotate90);
	bool allowEquipotentialOverlaps(QGraphicsItem * item, QList<Tile *> & alreadyTiled);
	PathUnit * findNearestSpace(PriorityQueue<PathUnit *> & priorityQueue, QMultiHash<Tile *, PathUnit *> & tilePathUnits, int tWidthNeeded, int tHeightNeeded, TileRect & nearestSpace);
	QPointF calcJumperLocation(PathUnit * pathUnit, TileRect & nearestSpace, int tWidthNeeded, int tHeightNeeded);
	bool addJumperItemHalf(ConnectorItem * jumperConnectorItem, PathUnit * nearest, PathUnit * parent, int searchx, int searchy, 
							JEdge * edge, QMultiHash<Tile *, PathUnit *> & tilePathUnits, qreal keepout);
	JEdge * makeEdge(ConnectorItem * from, ConnectorItem * to, class VirtualWire *);
	void expand(ConnectorItem * originalConnectorItem, QList<ConnectorItem *> & connectorItems, QSet<Wire *> & traceWires);
	void clipParts();
	Plane * initPlane(bool rotate90);
	void insertUnion(TileRect & tileRect, QGraphicsItem *, Tile::TileType tileType);
	bool blockDirection(PathUnit * pathUnit, PathUnit::Direction direction, Tile * next, int tWidthNeeded);
	void clearTracesAndJumpers();
	void saveTracesAndJumpers(QByteArray & byteArray);
	void initUndo(QUndoCommand * parentCommand);
	void addUndoConnection(bool connect, class JumperItem *, QUndoCommand * parentCommand);
	void addUndoConnection(bool connect, TraceWire *, QUndoCommand * parentCommand);
	void addUndoConnection(bool connect, ConnectorItem *, BaseCommand::CrossViewType, QUndoCommand * parentCommand);
	bool reorder(QList<Ordering> & orderings, QList<JEdge *> & edges, int & bestOrdering, QByteArray & bestResult, QGraphicsLineItem * lineItem);
	ConnectorItem * findPartForJumper(ConnectorItem * jumperConnectorItem);

protected:
	QRectF m_maxRect;
	TileRect m_tileMaxRect;
	QRectF m_maxRect90;
	TileRect m_tileMaxRect90;
	QMatrix m_matrix90;
	TileRect m_overlappingTileRect;
	QList<PathUnit *> m_pathUnits;
	LayerList m_viewLayerIDs;
	QHash<ViewLayer::ViewLayerID, Plane *> m_planeHash;
	QHash<Plane*, ViewLayer::ViewLayerSpec> m_specHash;
	QList<Plane *> m_planes;
	Plane * m_unionPlane;
	Plane * m_union90Plane;
	QHash<Wire *, JEdge *> m_tracesToEdges;
	ItemBase * m_board;
	int m_maxCycles;
	QByteArray m_startState;
};

#endif
