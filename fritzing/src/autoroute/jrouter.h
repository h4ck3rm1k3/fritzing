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

#include "../viewgeometry.h"
#include "../viewlayer.h"

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
	struct Plane * plane;
	bool routed;
};

struct JSubedge {
	struct JEdge * edge;
	ConnectorItem * fromConnectorItem;
	ConnectorItem * toConnectorItem;
	QPointF fromPoint;
	QPointF toPoint;
	class Wire * fromWire;
	class Wire * toWire;
	double distance;
	bool forward;
};

class GridEntry : public QGraphicsRectItem {
public:
	enum {
		EMPTY = 1,
		TINY,
		IGNORE,
		SAFE,
		SELF,
		GOAL,   // goal followed by blockers must be highest values
		OWNSIDE,
		BLOCK,
		NOTBOARD
	};

public:
	GridEntry(qreal x, qreal y, qreal width, qreal height, int type, QGraphicsItem * parent); 

public:
	int m_type;
};

class JRouter : public QObject
{
	Q_OBJECT

public:
	JRouter(class PCBSketchWidget *);
	~JRouter(void);

	void start();
	
protected:
	struct Tile * drawTrace(struct JSubedge *, QList<Wire *> &, bool forEmpty);
	void cleanUp();
	void updateRoutingStatus();
	class JumperItem * drawJumperItem(struct JumperItemStruct *);
	void restoreOriginalState(QUndoCommand * parentCommand);
	void addToUndo(Wire * wire, QUndoCommand * parentCommand);
	void addToUndo(QUndoCommand * parentCommand, QList<struct JumperItemStruct *> &);
	class TraceWire * drawOneTrace(QPointF fromPos, QPointF toPos, int width, ViewLayer::ViewLayerSpec);
	void reduceColinearWires(QList<Wire *> &);
	void expand(ConnectorItem * connectorItem, QList<ConnectorItem *> & connectorItems, QSet<Wire *> & visited);
	void collectEdges(QList<JEdge *> & edges, Plane * plane0, Plane * plane1, ViewLayer::ViewLayerID copper0, ViewLayer::ViewLayerID copper1);
	bool traceSubedge(struct JSubedge* subedge, class ItemBase * partForBounds, QHash<Wire *, JEdge *> &);
	void fixupJumperItems(QList<struct JumperItemStruct *> &);
	void runEdges(QList<JEdge *> &, QList<struct Plane *> &, QList<struct JumperItemStruct *> &, QVector<int> & netCounters, struct RoutingStatus &);
	void clearEdges(QList<JEdge *> & edges);
	void doCancel(QUndoCommand * parentCommand);
	bool alreadyJumper(QList<struct JumperItemStruct *> & jumperItemStructs, ConnectorItem * from, ConnectorItem * to);
	void updateProgress(int num, int denom);
	GridEntry * drawGridItem(qreal x1, qreal y1, qreal x2, qreal y2, short flag, GridEntry *);
	bool propagate(JSubedge * subedge, QList<struct Tile *> & path, bool forEmpty);
	bool backPropagate(JSubedge * subedge, QList<struct Tile *> & path, QList<Wire *> & wires, bool forEmpty);
	short checkCandidate(JSubedge * subedge, struct Tile *, bool forEmpty);
	JSubedge * makeSubedge(JEdge * edge, QPointF p1, ConnectorItem * from, Wire * fromWire, QPointF p2, bool forward);
	struct Tile * addTile(class NonConnectorItem * nci, int type, struct Plane *, QList<struct Tile *> & alreadyTiled);
	void seedNext(Tile * seed, QList<Tile *> & seeds);
	struct Plane * tilePlane(ItemBase * board, ViewLayer::ViewLayerID, QList<struct Tile *> & alreadyTiled);
	void tileWire(Wire *, struct Plane *, QList<Wire *> & beenThere, QList<struct Tile *> & alreadyTiled);
	short checkConnector(JSubedge * subedge, Tile * tile, ConnectorItem *, bool forEmpty);
	short checkTrace(JSubedge * subedge, Tile * tile, Wire *, bool forEmpty);
	short checkSpace(JSubedge * subedge, Tile * tile, bool forEmpty); 
	void clearTiles(struct Plane * thePlane);
	void hideTiles();
	void displayBadTiles(QList<struct Tile *> & alreadyTiled);
	struct Tile * insertTile(struct Plane* thePlane, struct TileRect &tileRect, QList<struct Tile *> &alreadyTiled, QGraphicsItem *, int type, bool clipWire);
	void clearGridEntries();
	void appendIf(Tile * seed, Tile * next, QList<Tile *> & seeds, bool (*enoughOverlap)(Tile*, Tile*, qreal));
	void sliceWireHorizontally(Wire * w, qreal angle, QPointF p1, QPointF p2, QList<QRectF> & rects);
	void sliceWireVertically(Wire * w, qreal angle, QPointF p1, QPointF p2, QList<QRectF> & rects);
	struct SeedTree * followPath(SeedTree * & root, QList<Tile *> & path);
	void drawDirection(QPointF & startPoint, QPointF & lastTracePoint, QPointF & endPoint, QRectF & fromTileRect, QRectF & toTileRect, 
		QList<Wire *> & wires, ConnectorItem * terminalConnectorItem, Wire * terminalWire, ViewLayer::ViewLayerSpec);
	void drawDirectionHorizontal(QPointF & startPoint, QPointF & lastTracePoint, QPointF & endPoint, QRectF & fromTileRect, QRectF & toTileRect, 
		QList<Wire *> & wires, ConnectorItem * terminalConnectorItem, Wire * terminalWire, ViewLayer::ViewLayerSpec);
	void drawDirectionVertical(QPointF & startPoint, QPointF & lastTracePoint, QPointF & endPoint, QRectF & fromTileRect, QRectF & toTileRect, 
		QList<Wire *> & wires, ConnectorItem * terminalConnectorItem, Wire * terminalWire, ViewLayer::ViewLayerSpec);
	QPointF drawLastNotHorizontal(const QPointF & startPoint, const QPointF & nextPoint, const QPointF & lastTracePoint, QList<Wire *> & wires, ViewLayer::ViewLayerSpec);
	QPointF drawLastNotVertical(const QPointF & startPoint, const QPointF & nextPoint, const QPointF & lastTracePoint, QList<Wire *> & wires, ViewLayer::ViewLayerSpec);
	void clearJumperItemStructs(QList<JumperItemStruct *> jumperItemStructs);
	void hookUpWires(JSubedge * subedge, QList<Wire *> & wires);
	QPointF findNearestSpace(Tile * tile, qreal widthNeeded, qreal heightNeeded, Plane * thePlane, const QPointF & nearPoint);
	ConnectorItem * splitTrace(Wire * wire, QPointF point);
	Tile * clipInsertTile(Plane * thePlane, TileRect &, QList<Tile *> & alreadyTiled, QGraphicsItem * item, int type);
	void handleChangedTiles(Plane * thePlane, TileRect &);
	Tile * tileOneWire(Plane * thePlane, TileRect & tileRect, QList<Tile *> & alreadyTiled, Wire * w);
	JEdge * makeEdge(ConnectorItem * from, ConnectorItem * to, ViewLayer::ViewLayerSpec, ViewLayer::ViewLayerID, Plane *);

protected:
	static void clearTraces(PCBSketchWidget * sketchWidget, bool deleteAll, QUndoCommand * parentCommand);
	static void addUndoConnections(PCBSketchWidget * sketchWidget, bool connect, QList<Wire *> & wires, QUndoCommand * parentCommand);

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
	QList<Wire *> m_cleanWires;
	int m_maximumProgressPart;
	int m_currentProgressPart;
	QRectF m_maxRect;
	qreal m_wireWidthNeeded;
	qreal m_halfWireWidthNeeded;


};

#endif
