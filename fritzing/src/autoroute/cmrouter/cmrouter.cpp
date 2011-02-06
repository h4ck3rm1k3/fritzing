/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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

This router is  based on the one described in Contour: A Tile-based Gridless Router
http://www.hpl.hp.com/techreports/Compaq-DEC/WRL-95-3.pdf

Plus additional ideas from An Efficient Tile-Based ECO Router with Routing Graph Reduction
http://www.cis.nctu.edu.tw/~ylli/paper/f69-li.pdf

The corner stitching code is a modified version of code from the Magic VLSI Layout Tool
http://opencircuitdesign.com/magic/

********************************************************************

$Revision$:
$Author$:
$Date$

********************************************************************/

// TODO:
//
//  run separate beginning overlap check with half keepout width
//
//	if wire is split during run, what happens to wire on next run
//
//	would be nice to eliminate ratsnests as we go
//
//	if current cycle unrouted count >= best so far, bail out
//
//	think of orderings like simulated annealing or genetic algorithms
//
//	placing vias
//		if double-sided, tile both sides
//		during the normal course of routing, whenever a space large enough for a via is found
//		somehow include the cost of additional real estate
//			see if there's an intersecting space on the other side
//				if so, add that space to the path
//
//	option to turn off propagation feedback?
//	remove debugging output and extra calls to processEvents
//
//	still seeing a few thin tiles going across the board: 
//		this is because the thick tiles above and below are wider than the thin tile
//
//	slide corner: if dogleg is too close to other connectors, slide it more towards the middle
//
//	new jumper item isn't properly undoable or deleteable
//
//	bugs: 
//		why does the same routing task give different results (qSort?)
//			especially annoying in schematic view when sometimes wires flow along wires and sometimes don't, for the same routing task
//		border seems asymmetric
//		still some funny shaped routes (thin tile problem?)
//		jumper item: sometimes one end doesn't route
//		parking assistant second cycle has weird line
//		split original wire shouldn't have two jumpers
//
//  longer route than expected:  
//		It is possible that the shortest tile route is actually longer than the shortest crow-fly route.  
//		For example, in the fóllowing case, route ABC will reach goal before ABDEF:
//                                   -------
//                                   |  A  |  
//      ------------------------------------
//      |               B                  |
//		------------------------------------
//      |       |                 |    D   |
//      |       |               --------------
//      |   C   |               |      E     |
//      |       |              -------------------
//      |       |              |       F         |
//      ------------------------------------------
//      |              GOAL                      |
//      ------------------------------------------
//		There is a way to deal with this in the following paper: http://eie507.eie.polyu.edu.hk/projects/sp-tiles-00980255.pdf
//
//

#include "cmrouter.h"
#include "../../sketch/pcbsketchwidget.h"
#include "../../debugdialog.h"
#include "../../items/virtualwire.h"
#include "../../items/tracewire.h"
#include "../../items/jumperitem.h"
#include "../../items/resizableboard.h"
#include "../../utils/graphicsutils.h"
#include "../../utils/textutils.h"
#include "../../connectors/connectoritem.h"
#include "../../items/moduleidnames.h"
#include "../../processeventblocker.h"
#include "../../svg/groundplanegenerator.h"
#include "../../fsvgrenderer.h"

#include "tile.h"

#include <qmath.h>
#include <QApplication>
#include <QMessageBox> 
#include <QElapsedTimer>
#include <QSettings>

static const int MaximumProgress = 1000;
static const int TILEFACTOR = 1000;
static int TileStandardWireWidth = 0;
static int TileHalfStandardWireWidth = 0;
static qreal HalfStandardWireWidth = 0;
static const qreal CloseEnough = 0.5;
static const int GridEntryAlpha = 128;

static qint64 seedNextTime = 0;
static qint64 propagateUnitTime = 0;

static const int DefaultMaxCycles = 10;

const int Segment::NotSet = std::numeric_limits<int>::min();

static inline qreal dot(const QPointF & p1, const QPointF & p2)
{
	return (p1.x() * p2.x()) + (p1.y() * p2.y());
}

bool edgeLessThan(JEdge * e1, JEdge * e2)
{
	return e1->distance <= e2->distance;
}

bool tilePointRectXLessThan(TilePointRect * tpr1, TilePointRect * tpr2)
{
	return tpr1->tilePoint.xi <= tpr2->tilePoint.xi;
}

bool tilePointRectYGreaterThan(TilePointRect * tpr1, TilePointRect * tpr2)
{
	return tpr1->tilePoint.yi >= tpr2->tilePoint.yi;
}

///////////////////////////////////////////////
//
// tile functions

static inline void infoTile(const QString & message, Tile * tile)
{
	DebugDialog::debug(QString("%1 tile:%2 l:%3 t:%4 w:%5 h:%6 type:%7 body:%8")
		.arg(message)
		.arg((long) tile, 0, 16)
		.arg(LEFT(tile))
		.arg(YMIN(tile))
		.arg(WIDTH(tile))
		.arg(HEIGHT(tile))
		.arg(TiGetType(tile))
		.arg((long) TiGetBody(tile), 0, 16)
	);
}

static inline void infoTileRect(const QString & message, const TileRect & tileRect)
{
	DebugDialog::debug(QString("%1 l:%2 t:%3 w:%4 h:%5")
		.arg(message)
		.arg(tileRect.xmini)
		.arg(tileRect.ymini)
		.arg(tileRect.xmaxi - tileRect.xmini)
		.arg(tileRect.ymaxi - tileRect.ymini)
	);
}

void tileRotate90(TileRect & tileRect, TileRect & tileRect90)
{
	// x' = x*cos - y*sin
	// y' = x*sin + y*cos
	// where cos90 = 0 and sin90 = 1 (effectively clockwise)

	// rotate top right corner of rect
	tileRect90.xmini = -tileRect.ymaxi;
	tileRect90.ymini = tileRect.xmini;

	// swap width and height
	tileRect90.xmaxi = tileRect90.xmini + (tileRect.ymaxi - tileRect.ymini);
	tileRect90.ymaxi = tileRect90.ymini + (tileRect.xmaxi - tileRect.xmini);
}

void tileUnrotate90(TileRect & tileRect90, TileRect & tileRect)
{
	tileRect.xmini = tileRect90.ymini;
	tileRect.ymaxi = -tileRect90.xmini;

	// swap width and height
	tileRect.xmaxi = tileRect.xmini + (tileRect90.ymaxi - tileRect90.ymini);
	tileRect.ymini = tileRect.ymaxi - (tileRect90.xmaxi - tileRect90.xmini);
}

static inline int manhattan(TileRect & tr1, TileRect & tr2) {
	int dx =  qAbs(tr1.xmaxi - tr2.xmaxi);
	dx = qMin(qAbs(tr1.xmaxi - tr2.xmini), dx);
	dx = qMin(qAbs(tr1.xmini - tr2.xmaxi), dx);
	dx = qMin(qAbs(tr1.xmini - tr2.xmini), dx);
	int dy =  qAbs(tr1.ymaxi - tr2.ymaxi);
	dy = qMin(qAbs(tr1.ymaxi - tr2.ymini), dy);
	dy = qMin(qAbs(tr1.ymini - tr2.ymaxi), dy);
	dy = qMin(qAbs(tr1.ymini - tr2.ymini), dy);
	return dx + dy;
}

static inline GridEntry * TiGetGridEntry(Tile * tile) { return dynamic_cast<GridEntry *>(TiGetClient(tile)); }

inline int realToTile(qreal x) {
	return qRound(x * TILEFACTOR);
}

void realsToTile(TileRect & tileRect, qreal l, qreal t, qreal r, qreal b) {
	tileRect.xmini = realToTile(l);
	tileRect.ymini = realToTile(t);
	tileRect.xmaxi = realToTile(r);
	tileRect.ymaxi = realToTile(b);
}

inline qreal tileToReal(int x) {
	return x / ((qreal) TILEFACTOR);
}

void tileRectToQRect(TileRect & tileRect, QRectF & rect) {
	rect.setCoords(tileToReal(tileRect.xmini), tileToReal(tileRect.ymini), tileToReal(tileRect.xmaxi), tileToReal(tileRect.ymaxi));
}

void tileToQRect(Tile * tile, QRectF & rect) {
	TileRect tileRect;
	TiToRect(tile, &tileRect);
	tileRectToQRect(tileRect, rect);
}

void extendToBounds(TileRect & from, TileRect & to) {
	// bail if it already extends to or past the bounds
	if (from.xmini <= to.xmini) return;
	if (from.xmaxi >= to.xmaxi) return;
	if (from.ymini <= to.ymini) return;
	if (from.ymaxi >= to.ymaxi) return;

	int which = 0;
	int dmin = from.xmini - to.xmini;
	if (to.xmaxi - from.xmaxi < dmin) {
		which = 1;
		dmin = to.xmaxi - from.xmaxi;
	}
	if (from.ymini - to.ymini < dmin) {
		which = 2;
		dmin = from.ymini - to.ymini;
	}
	if (to.ymaxi - from.ymaxi < dmin) {
		which = 3;
		dmin = to.ymaxi - from.ymaxi;
	}
	switch(which) {
		case 0:
			from.xmini = to.xmini;
			return;
		case 1:
			from.xmaxi = to.xmaxi;
			return;
		case 2:
			from.ymini = to.ymini;
			return;
		case 3:
			from.ymaxi = to.ymaxi;
			return;
		default:
			break;
	}
}

bool tileRectsIntersect(TileRect * tile1, TileRect * tile2)
{
    int l1 = tile1->xmini;
    int r1 = tile1->xmaxi;
    if (l1 == r1) // null rect
        return false;

    int l2 = tile2->xmini;
    int r2 = tile2->xmaxi;
    if (l2 == r2) // null rect
        return false;

    if (l1 >= r2 || l2 >= r1)
        return false;

    int t1 = tile1->ymini;
    int b1 = tile1->ymaxi;
    if (t1 == b1) // null rect
        return false;

    int t2 = tile2->ymini;
    int b2 = tile2->ymaxi;
    if (t2 == b2) // null rect
        return false;

    if (t1 >= b2 || t2 >= b1)
        return false;

    return true;
}

////////////////////////////////////////////////////////////////////
//
// tile crawling functions

int simpleList(Tile * tile, UserData userData) {
	QList<Tile *> * tiled = (QList<Tile *> *) userData;
	tiled->append(tile);
	return 0;
}

struct SourceAndDestinationStruct {
	QList<Tile *> tiles;
	JEdge * edge;
};

int findSpaces(Tile * tile, UserData userData) {
	switch (TiGetType(tile)) {
		case Tile::SPACE:
		case Tile::SPACE2:
			break;
		default:
			return 0;
	}

	QList<Tile*> * tiles = (QList<Tile*> *) userData;
	tiles->append(tile);
	return 0;
}

int findSourceAndDestination(Tile * tile, UserData userData) {
	SourceAndDestinationStruct * sourceAndDestinationStruct = (SourceAndDestinationStruct *) userData;

	ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(TiGetBody(tile));
	if (connectorItem) {
		if (sourceAndDestinationStruct->edge->fromConnectorItems.contains(connectorItem)) {
			TiSetType(tile, Tile::SOURCE);
			sourceAndDestinationStruct->tiles.append(tile);
		}
		else if (sourceAndDestinationStruct->edge->toConnectorItems.contains(connectorItem)) {
			TiSetType(tile, Tile::DESTINATION);
			sourceAndDestinationStruct->tiles.append(tile);
		}

		return 0;
	}

	Wire * wire = dynamic_cast<Wire *>(TiGetBody(tile));
	if (wire) {
		TileRect tileRect;
		TiToRect(tile, &tileRect);
		int minDim = qMin(tileRect.xmaxi - tileRect.xmini, tileRect.ymaxi - tileRect.ymini);
		if (minDim < TileStandardWireWidth) {
			return 0;
		}

		int maxDim = qMax(tileRect.xmaxi - tileRect.xmini, tileRect.ymaxi - tileRect.ymini);
		if (maxDim < TileStandardWireWidth * 3) {
			return 0;
		}

		if (sourceAndDestinationStruct->edge->fromTraces.contains(wire)) {
			TiSetType(tile, Tile::SOURCE);
			sourceAndDestinationStruct->tiles.append(tile);
		}
		else if (sourceAndDestinationStruct->edge->toTraces.contains(wire)) {
			TiSetType(tile, Tile::DESTINATION);
			sourceAndDestinationStruct->tiles.append(tile);
		}
	
		return 0;
	}

	return 0;
}

int clearSourceAndDestination(Tile * tile, UserData) {
	if (TiGetType(tile) == Tile::SOURCE || TiGetType(tile) == Tile::DESTINATION) {
		TiSetType(tile, Tile::OBSTACLE);
	}
	return 0;
}


int clearSourceAndDestination2(Tile * tile, UserData) {
	if (TiGetType(tile) == Tile::SOURCE || TiGetType(tile) == Tile::DESTINATION) {
		TraceWire * traceWire = dynamic_cast<TraceWire *>(TiGetBody(tile));
		TiSetType(tile, traceWire == NULL ? Tile::OBSTACLE : Tile::SCHEMATICWIRESPACE);
	}
	return 0;
}

int checkAlready(Tile * tile, UserData userData) {
	switch (TiGetType(tile)) {
		case Tile::SPACE:		
		case Tile::SPACE2:		
		case Tile::SCHEMATICWIRESPACE:		
		case Tile::BUFFER:
			return 0;
		default:
			break;
	}

	QList<Tile *> * tiles = (QList<Tile *> *) userData;
	tiles->append(tile);
	return 0;
}

int collectOneNotEmpty(Tile * tile, UserData) {
	switch (TiGetType(tile)) {
		case Tile::SPACE:
		case Tile::SPACE2:
		case Tile::SCHEMATICWIRESPACE:
			return 0;
		default:
			return 1;  // not empty; will stop the search
	}
}

int prepDeleteTile(Tile * tile, UserData userData) {
	switch(TiGetType(tile)) {
		case Tile::DUMMYLEFT:
		case Tile::DUMMYRIGHT:
		case Tile::DUMMYTOP:
		case Tile::DUMMYBOTTOM:
			return 0;
		default:
			break;
	}

	//infoTile("prep delete", tile);
	QSet<Tile *> * tiles = (QSet<Tile *> *) userData;
	tiles->insert(tile);

	return 0;
}

int collectThinTiles(Tile * tile, UserData userData) {
	if (TiGetType(tile) != Tile::SPACE) return 0;
	if (YMAX(tile) - YMIN(tile) >= TileStandardWireWidth) return 0;

	QList<TileRect> * tiles = (QList<TileRect> *) userData;
	TileRect tileRect;
	TiToRect(tile, &tileRect);
	tiles->append(tileRect);
	return 0;
}

int collectOneThinTile(Tile * tile, UserData userData) {
	if (TiGetType(tile) != Tile::SPACE) return 0;
	if (YMAX(tile) - YMIN(tile) >= TileStandardWireWidth) return 0;

	*((Tile **) userData) = tile;

	return 1;		// stop the search
}

////////////////////////////////////////////////////////////////////

GridEntry::GridEntry(QRectF & r, QGraphicsItem * parent) : QGraphicsRectItem(r, parent)
{
	m_drawn = false;
	setAcceptedMouseButtons(Qt::NoButton);
	setAcceptsHoverEvents(false);
}

bool GridEntry::drawn() {
	return m_drawn;
}

void GridEntry::setDrawn(bool d) {
	m_drawn = d;
}

////////////////////////////////////////////////////////////////////

CMRouter::CMRouter(PCBSketchWidget * sketchWidget) : Autorouter(sketchWidget)
{
	QSettings settings;
	m_maxCycles = settings.value("cmrouter/maxcycles", DefaultMaxCycles).toInt();
		
	m_bothSidesNow = sketchWidget->routeBothSides();
	m_unionPlane = m_union90Plane = NULL;
	m_board = NULL;

	m_matrix90.rotate(90);

	if (sketchWidget->autorouteTypePCB()) {
		m_board = sketchWidget->findBoard();
	}

	if (m_board) {
		m_maxRect = m_board->boundingRect();
		m_maxRect.translate(m_board->pos());
	}
	else {
		m_maxRect = m_sketchWidget->scene()->itemsBoundingRect();
		m_maxRect.adjust(-m_maxRect.width() / 2, -m_maxRect.height() / 2, m_maxRect.width() / 2, m_maxRect.height() / 2);
	}

	m_maxRect90 = m_matrix90.mapRect(m_maxRect);

	realsToTile(m_tileMaxRect, m_maxRect.left(), m_maxRect.top(), m_maxRect.right(), m_maxRect.bottom()); 
	realsToTile(m_tileMaxRect90, m_maxRect90.left(), m_maxRect90.top(), m_maxRect90.right(), m_maxRect90.bottom()); 

	TileStandardWireWidth = realToTile(Wire::STANDARD_TRACE_WIDTH);
	HalfStandardWireWidth = Wire::STANDARD_TRACE_WIDTH / 2;
	TileHalfStandardWireWidth = realToTile(HalfStandardWireWidth);

	ViewGeometry vg;
	vg.setTrace(true);
	ViewLayer::ViewLayerID copper0 = sketchWidget->getWireViewLayerID(vg, ViewLayer::Bottom);
	m_viewLayerIDs << copper0;
	if  (m_bothSidesNow) {
		ViewLayer::ViewLayerID copper1 = sketchWidget->getWireViewLayerID(vg, ViewLayer::Top);
		m_viewLayerIDs.append(copper1);
	}
}

CMRouter::~CMRouter()
{
}

void CMRouter::start()
{	
	m_maximumProgressPart = 1;
	m_currentProgressPart = 0;
	qreal keepout = m_sketchWidget->getKeepout();			// 15 mils space

	emit setMaximumProgress(MaximumProgress);
	emit setProgressMessage("");
	emit setCycleMessage("round 1 of:");
	emit setCycleCount(m_maxCycles);

	RoutingStatus routingStatus;
	routingStatus.zero();

	m_sketchWidget->ensureTraceLayersVisible();

	clearGridEntries();

	QUndoCommand * parentCommand = new QUndoCommand("Autoroute");
	new CleanUpWiresCommand(m_sketchWidget, CleanUpWiresCommand::UndoOnly, parentCommand);

	if (m_bothSidesNow) {
		emit wantBothVisible();
		ProcessEventBlocker::processEvents();
	}

	initUndo(parentCommand);
	updateRoutingStatus();

	QHash<ConnectorItem *, int> indexer;
	m_sketchWidget->collectAllNets(indexer, m_allPartConnectorItems, false, m_bothSidesNow);

	if (m_allPartConnectorItems.count() == 0) {
		return;
	}

	// will list connectors on both sides separately
	routingStatus.m_netCount = m_allPartConnectorItems.count();

	QVector<int> netCounters(m_allPartConnectorItems.count());
	for (int i = 0; i < m_allPartConnectorItems.count(); i++) {
		netCounters[i] = (m_allPartConnectorItems[i]->count() - 1) * 2;			// since we use two connectors at a time on a net
	}

	if (m_cancelled || m_stopTracing) {
		restoreOriginalState(parentCommand);
		cleanUp();
		return;
	}

	ProcessEventBlocker::processEvents(); // to keep the app  from freezing

	//	rip up and reroute:
	//		keep a hash table from traces to edges
	//		when edges are generated give each an integer ID
	//		when first pass at routing is over and there are unrouted edges
	//		save the traces along with some score (number of open edges)
	//		save the list of IDs in order
	//		foreach ratsnest wire remaining (i.e. edge remaining) 
	//			find all intersections with traces and map to edges
	//			move this edge above that edge in the list of edges
	//			just do one edge or all open edges?  
	//		keep the traces from edges above those that got moved, clear the rest
	//		save the new ordering (list of IDs)
	//		now route again with the new edge order from the point of change
	//		if no open edges we are done
	//			compare edge score with previous and keep the best set of traces
	//		how to stop--keep going until user stops or some repeat condition occurs.
	//			check the reordering of edges and if they match a previous set quit 
	//

	QList<JEdge *> edges;
	bool allDone = false;
	QList< Ordering > orderings;
	int bestOrdering = 0;
	QByteArray bestResult;
	collectEdges(edges);
	qSort(edges.begin(), edges.end(), edgeLessThan);	// sort the edges by distance and layer

	QPen pen(QColor(0,0,0,0));
	pen.setWidthF(Wire::STANDARD_TRACE_WIDTH);
	QGraphicsLineItem * lineItem = new QGraphicsLineItem();
	lineItem->setPen(pen);
	m_sketchWidget->scene()->addItem(lineItem);

	for (int run = 0; run < m_maxCycles; run++) {
		QString score;
		if (run > 0) {
			score = tr("best so far: %1 unrouted/%n jumpers", "", orderings.at(bestOrdering).jumperCount)
				.arg(orderings.at(bestOrdering).unroutedCount);
			emit setProgressMessage(score);
		}
		emit setCycleMessage(tr("round %1 of:").arg(run + 1));
		allDone = runEdges(edges, netCounters, routingStatus, keepout, m_sketchWidget->usesJumperItem());
		if (m_cancelled || allDone || m_stopTracing) break;

		bool reordered = reorder(orderings, edges, bestOrdering, bestResult, lineItem);

		// TODO: only delete the edges that have been reordered
		clearTracesAndJumpers();
		drcClean();
		ProcessEventBlocker::processEvents();

		if (!reordered) break;

		if (!m_startState.isEmpty()) {
			m_sketchWidget->pasteHeart(m_startState, true);
			ProcessEventBlocker::processEvents();
		}

	}

	delete lineItem;

	if (m_cancelled) {
		clearEdges(edges);
		drcClean();
		clearTracesAndJumpers();
		doCancel(parentCommand);
		return;
	}

	if (!allDone) {
		if (orderings.count() == 0) {
			// stop where we are
		}
		else {
			m_sketchWidget->pasteHeart(bestResult, true);
			ProcessEventBlocker::processEvents();
		}
	}

	cleanUp();

	addToUndo(parentCommand);

	clearEdges(edges);
	drcClean();
	new CleanUpWiresCommand(m_sketchWidget, CleanUpWiresCommand::RedoOnly, parentCommand);

	// note that the undo command which creates traces and jumpers is set to turnOffFirstRedo()
	// in other words, the traces and jumpers created in the autorouting process are not recreated the first time the undo stack is invoked

	m_sketchWidget->pushCommand(parentCommand);
	m_sketchWidget->repaint();
	DebugDialog::debug("\n\n\nautorouting complete\n\n\n");
}


bool CMRouter::reorder(QList<Ordering> & orderings, QList<JEdge *> & edges, int & bestOrdering, QByteArray & bestResult, QGraphicsLineItem * lineItem) {
	Ordering ordering;
	ordering.jumperCount = ordering.viaCount = ordering.unroutedCount = 0;
	foreach (JEdge * edge, edges) {
		ordering.edgeIDs.append(edge->id);
		if (!edge->routed) ordering.unroutedCount++;
		if (edge->withJumper) ordering.jumperCount++;
	}
	orderings.append(ordering);	
	if (orderings.count() > 1) {
		if (ordering.unroutedCount + ordering.jumperCount < orderings.at(bestOrdering).unroutedCount + orderings.at(bestOrdering).jumperCount) {
			bestOrdering = orderings.count() - 1;
			saveTracesAndJumpers(bestResult);
		}
		else if ((ordering.unroutedCount + ordering.jumperCount == orderings.at(bestOrdering).unroutedCount + orderings.at(bestOrdering).jumperCount)
				&& (ordering.jumperCount > orderings.at(bestOrdering).jumperCount))
		{
			bestOrdering = orderings.count() - 1;
			saveTracesAndJumpers(bestResult);
		}
	}
	else {
		saveTracesAndJumpers(bestResult);
	}

	bool reordered = reorderEdges(edges, lineItem);
	if (reordered) {
		foreach (Ordering ordering, orderings) {
			bool allSame = true;
			for (int i = 0; i < edges.count(); i++) {
				if (ordering.edgeIDs.at(i) != edges.at(i)->id) {
					allSame = false;
					break;
				}
			}
			if (allSame) {
				reordered = false;
				break;
			}
		}
	}
	return reordered;
}

bool CMRouter::drc() 
{
	// TODO: 
	//	what about ground plane?

	qreal keepout = m_sketchWidget->getKeepout() / 2;			// 15 mils space
	m_board = NULL;
	if (m_sketchWidget->autorouteTypePCB()) {
		m_board = m_sketchWidget->findBoard();
	}

	return drc(keepout, CMRouter::ReportAllOverlaps, CMRouter::AllowEquipotentialOverlaps, false, false);
}

void CMRouter::drcClean() 
{
	clearGridEntries();
	foreach (Plane * plane, m_planes) clearPlane(plane, false);
	m_planeHash.clear();
	m_specHash.clear();
	m_planes.clear();
	if (m_unionPlane) {
		clearPlane(m_unionPlane, false);
		m_unionPlane = NULL;
	}
	if (m_union90Plane) {
		clearPlane(m_union90Plane, true);
		m_union90Plane = NULL;
	}
}

bool CMRouter::drc(qreal keepout, CMRouter::OverlapType overlapType, CMRouter::OverlapType wireOverlapType, bool eliminateThin, bool combinePlanes) 
{
	if (combinePlanes) {
		m_unionPlane = initPlane(false);
		m_union90Plane = initPlane(true);
		clipParts();
	}
	else {
		m_union90Plane = m_unionPlane = NULL;
	}

	QList<Tile *> alreadyTiled;
	Plane * plane = tilePlane(m_viewLayerIDs.at(0), ViewLayer::Bottom, alreadyTiled, keepout, overlapType, wireOverlapType, eliminateThin);
	if (alreadyTiled.count() > 0) {
		displayBadTiles(alreadyTiled);
		return false;
	}
	else {
		hideTiles();
	}

	if (m_bothSidesNow) {
		plane = tilePlane(m_viewLayerIDs.at(1), ViewLayer::Top, alreadyTiled, keepout, overlapType, wireOverlapType, eliminateThin);
		if (alreadyTiled.count() > 0) {
			displayBadTiles(alreadyTiled);
			return false;
		}
	}

	if (eliminateThin && m_unionPlane != NULL) {
		QList<TileRect> tileRects;
		TiSrArea(NULL, m_unionPlane, &m_tileMaxRect, collectThinTiles, &tileRects);
		eliminateThinTiles(tileRects, m_unionPlane);
		tileRects.clear();
		TiSrArea(NULL, m_union90Plane, &m_tileMaxRect90, collectThinTiles, &tileRects);
		eliminateThinTiles(tileRects, m_union90Plane);
	}

	hideTiles();
	return true;
}

bool CMRouter::runEdges(QList<JEdge *> & edges, QVector<int> & netCounters, RoutingStatus & routingStatus, qreal keepout, bool makeJumper)
{	
	bool allRouted = true;

	bool result = drc(keepout, CMRouter::ClipAllOverlaps, CMRouter::ClipAllOverlaps, true, m_sketchWidget->autorouteTypePCB());
	if (!result) {
		m_cancelled = true;
		QMessageBox::warning(NULL, QObject::tr("Fritzing"), QObject::tr("Cannot autoroute: parts or traces are overlapping"));
		return false;
	}

	foreach (JEdge * edge, edges) {
		clearEdge(edge);
	}

	int edgesDone = 0;
	foreach (JEdge * edge, edges) {	

		if (edge->routed) {
			// got it one one side; no need to route it on the other
			continue;
		}

		expand(edge->from, edge->fromConnectorItems, edge->fromTraces);
		expand(edge->to, edge->toConnectorItems, edge->toTraces);

		PriorityQueue<PathUnit *> queue1;
		PriorityQueue<PathUnit *> queue2;

		QMultiHash<Tile *, PathUnit *> tilePathUnits;

		SourceAndDestinationStruct sourceAndDestinationStruct;
		sourceAndDestinationStruct.edge = edge;
		foreach (Plane * plane, m_planes) {
			TiSrArea(NULL, plane, &m_tileMaxRect, findSourceAndDestination, &sourceAndDestinationStruct);
		}

		//DebugDialog::debug("begin ipu");
		foreach (Tile * tile, sourceAndDestinationStruct.tiles) {
			initPathUnit(edge, tile, (TiGetType(tile) == Tile::SOURCE ? queue1 : queue2), tilePathUnits);
		}
		//DebugDialog::debug("end ipu");

		foreach (QGraphicsItem * item, m_sketchWidget->items()) {
			GridEntry * gridEntry = dynamic_cast<GridEntry *>(item);
			if (gridEntry) gridEntry->setDrawn(false);
		}

		edge->routed = propagate(queue1, queue2, tilePathUnits, keepout);

		if (!edge->routed) {
			allRouted = false;
			if (makeJumper) {
				if (addJumperItem(queue1, queue2, edge, tilePathUnits, keepout)) {
					edge->withJumper = edge->routed = true;
				}
			}
		}

		foreach (PathUnit * pathUnit, m_pathUnits) {
			delete pathUnit;
		}
		m_pathUnits.clear();

		updateProgress(++edgesDone, edges.count());

		for (int i = 0; i < m_allPartConnectorItems.count(); i++) {
			if (m_allPartConnectorItems[i]->contains(edge->from)) {
				netCounters[i] -= 2;
				break;
			}
		}

		routingStatus.m_netRoutedCount = 0;
		routingStatus.m_connectorsLeftToRoute = edges.count() + 1 - edgesDone;
		foreach (int c, netCounters) {
			if (c <= 0) {
				routingStatus.m_netRoutedCount++;
			}
		}
		m_sketchWidget->forwardRoutingStatus(routingStatus);

		ProcessEventBlocker::processEvents();

		if (m_cancelled) {
			break;
		}

		if (m_stopTracing) {
			break;
		}
	}

	return allRouted;
}

Plane * CMRouter::initPlane(bool rotate90) {
	Tile * bufferTile = TiAlloc();
	TiSetType(bufferTile, Tile::BUFFER);
	TiSetBody(bufferTile, NULL);

	QRectF bufferRect(rotate90 ? m_maxRect90 : m_maxRect);
	bufferRect.adjust(-bufferRect.width(), -bufferRect.height(), bufferRect.width(), bufferRect.height());

	SETLEFT(bufferTile, realToTile(bufferRect.left()));
	SETYMIN(bufferTile, realToTile(bufferRect.top()));		// TILE is Math Y-axis not computer-graphic Y-axis

	Plane * thePlane = TiNewPlane(bufferTile);

	SETRIGHT(bufferTile, realToTile(bufferRect.right()));
	SETYMAX(bufferTile, realToTile(bufferRect.bottom()));		// TILE is Math Y-axis not computer-graphic Y-axis

	// do not use InsertTile here
	TiInsertTile(thePlane, rotate90 ? &m_tileMaxRect90 : &m_tileMaxRect, NULL, Tile::SPACE); 

	return thePlane;
}

Plane * CMRouter::tilePlane(ViewLayer::ViewLayerID viewLayerID, ViewLayer::ViewLayerSpec viewLayerSpec, QList<Tile *> & alreadyTiled, qreal keepout, CMRouter::OverlapType overlapType, CMRouter::OverlapType wireOverlapType, bool eliminateThin) 
{
	Plane * thePlane = initPlane(false);
	m_planeHash.insert(viewLayerID, thePlane);
	m_planes.append(thePlane);
	m_specHash.insert(thePlane, viewLayerSpec);

	// if board is not rectangular, add tiles for the outside edges;
	if (!initBoard(m_board, thePlane, alreadyTiled, keepout)) return thePlane;

	if (m_sketchWidget->autorouteTypePCB()) {
		// deal with "rectangular" elements first
		foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
			ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
			if (connectorItem == NULL) continue;

			if (!connectorItem->attachedTo()->isVisible()) continue;
			if (connectorItem->attachedTo()->hidden()) continue;
			if (connectorItem->attachedToItemType() == ModelPart::Wire) continue;
			if (!m_sketchWidget->sameElectricalLayer2(connectorItem->attachedToViewLayerID(), viewLayerID)) continue;

			addTile(connectorItem, Tile::OBSTACLE, thePlane, alreadyTiled, overlapType, keepout);
			if (alreadyTiled.count() > 0) {
				return thePlane;
			}
		}

		// now insert the wires
		QList<Wire *> beenThere;
		foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
			Wire * wire = dynamic_cast<Wire *>(item);
			if (wire == NULL) continue;
			if (!wire->isVisible()) continue;
			if (wire->hidden()) continue;
			if (!wire->getTrace()) continue;
			if (!m_sketchWidget->sameElectricalLayer2(wire->viewLayerID(), viewLayerID)) continue;
			if (beenThere.contains(wire)) continue;

			tileWire(wire, beenThere, alreadyTiled, m_sketchWidget->autorouteTypePCB() ? Tile::OBSTACLE : Tile::SCHEMATICWIRESPACE, wireOverlapType, keepout, eliminateThin);
			if (alreadyTiled.count() > 0) {
				return thePlane;
			}	
		}

		// now nonconnectors
		foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
			ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
			if (connectorItem != NULL) {
				continue;
			}

			NonConnectorItem * nonConnectorItem = dynamic_cast<NonConnectorItem *>(item);
			if (nonConnectorItem == NULL) continue;

			if (!nonConnectorItem->attachedTo()->isVisible()) continue;
			if (nonConnectorItem->attachedTo()->hidden()) continue;
			if (!m_sketchWidget->sameElectricalLayer2(connectorItem->attachedToViewLayerID(), viewLayerID)) continue;

			DebugDialog::debug(QString("coords nonconnectoritem %1 %2")
									.arg(nonConnectorItem->attachedToTitle())
									.arg(nonConnectorItem->attachedToID())
									);

			addTile(nonConnectorItem, Tile::OBSTACLE, thePlane, alreadyTiled, overlapType, keepout);
			if (alreadyTiled.count() > 0) {
				return thePlane;
			}
		}
	}
	else {
		foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
			ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
			if (itemBase == NULL) continue;

			if (!itemBase->isVisible()) continue;
			if (itemBase->hidden()) continue;
			if (itemBase->itemType() == ModelPart::Wire) continue;

			QRectF r = itemBase->boundingRect();
			r.moveTo(itemBase->pos());
			TileRect partTileRect;
			realsToTile(partTileRect, r.left() - keepout, r.top() - keepout, r.right() + keepout, r.bottom() + keepout);
			insertTile(thePlane, partTileRect, alreadyTiled, itemBase, Tile::OBSTACLE, overlapType);
			if (alreadyTiled.count() > 0) {
				return thePlane;
			}
			
			// TODO: only bother with connectors that might be electrically relevant, since parts are all we need for obstacles

			foreach (QGraphicsItem * childItem, itemBase->childItems()) {
				ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
				if (connectorItem == NULL) continue;

				QRectF r = itemBase->mapRectToScene(connectorItem->rect());
				TileRect tileRect;
				realsToTile(tileRect, r.left() - keepout, r.top() - keepout, r.right() + keepout, r.bottom() + keepout);
				extendToBounds(tileRect, partTileRect);
				insertTile(thePlane, tileRect, alreadyTiled, connectorItem, Tile::OBSTACLE, CMRouter::IgnoreAllOverlaps);
				//drawGridItem(newTile);
			}
		}
	}

	if (eliminateThin) {
		QList<TileRect> tileRects;
		TiSrArea(NULL, thePlane, &m_tileMaxRect, collectThinTiles, &tileRects);
		eliminateThinTiles(tileRects, thePlane);
	}

	return thePlane;
}

void CMRouter::eliminateThinTiles(QList<TileRect> & originalTileRects, Plane * thePlane) {

	QList<TileRect> remainingTileRects;
	foreach (TileRect originalTileRect, originalTileRects) {
		QList<TileRect> tileRects;
		tileRects.append(originalTileRect);

		while (tileRects.count() > 0) {
			TileRect originalTileRect = tileRects.takeFirst();
			if (originalTileRect.xmaxi - originalTileRect.xmini <= 0) {
				continue;
			}
			
			Tile * tile = NULL;
			TiSrArea(NULL, thePlane, &originalTileRect, collectOneThinTile, &tile);
			if (tile == NULL) continue;

			TileRect tileRect;
			TiToRect(tile, &tileRect);
			if (tileRect.xmaxi - tileRect.xmini < TileStandardWireWidth) continue;

			TileRect newRect;
			Tile * extendTile = NULL;
			// look along the top
			for (Tile * tp = RT(tile); LEFT(tp) >= tileRect.xmini; tp = BL(tp)) {
				Tile::TileType tileType = TiGetType(tp);
				if ((tileType == Tile::OBSTACLE || tileType == Tile::SPACE2) && RIGHT(tp) <= tileRect.xmaxi) {
					// obstacle island above (remember y axis is flipped)
					TiToRect(tp, &newRect);
					newRect.ymini = tileRect.ymini;
					newRect.ymaxi = tileRect.ymaxi;
					extendTile = tp;
					break;
				}
			}

			if (extendTile == NULL) {
				// look along the bottom
				for (Tile * tp = LB(tile); RIGHT(tp) <= tileRect.xmaxi; tp = TR(tp)) {
					Tile::TileType tileType = TiGetType(tp);
					if ((tileType == Tile::OBSTACLE || tileType == Tile::SPACE2) && LEFT(tp) >= tileRect.xmini) {
						// obstacle island below (remember y axis is flipped)
						TiToRect(tp, &newRect);
						newRect.ymaxi = tileRect.ymaxi;
						newRect.ymini = tileRect.ymini;
						extendTile = tp;
						break;
					}
				}
			}

			if (extendTile) {
				QList<Tile *> alreadyTiled;
				insertTile(thePlane, newRect, alreadyTiled, NULL, Tile::SPACE2, CMRouter::IgnoreAllOverlaps);
				//drawGridItem(newTile);
				TileRect leftRect = originalTileRect;
				leftRect.xmaxi = newRect.xmini;
				if (leftRect.xmaxi -leftRect.xmini > 0) tileRects.append(leftRect);
				TileRect rightRect = originalTileRect;
				rightRect.xmini = newRect.xmaxi;
				if (rightRect.xmaxi - rightRect.xmini > 0) tileRects.append(rightRect);
			}
			else {
				remainingTileRects.append(originalTileRect);
			}
		}
	}
	eliminateThinTiles2(remainingTileRects, thePlane);
}

void CMRouter::eliminateThinTiles2(QList<TileRect> & tileRects, Plane * thePlane) 
{
	QList<TileRect> remainingTileRects;
	while (tileRects.count() > 0) {
		TileRect originalTileRect = tileRects.takeFirst();
		if (originalTileRect.xmaxi - originalTileRect.xmini <= 0) {
			continue;
		}
			
		Tile * tile = NULL;
		TiSrArea(NULL, thePlane, &originalTileRect, collectOneThinTile, &tile);
		if (tile == NULL) continue;

		// handle four cases from ECO router http://www.cis.nctu.edu.tw/~ylli/paper/f69-li.pdf

		TileRect tileRect;
		TiToRect(tile, &tileRect);
		if (tileRect.xmaxi - tileRect.xmini < TileStandardWireWidth) continue;

		TileRect newRect;
		bool doInsert = false;

		Tile * rt = RT(tile);
		TileRect rtRect;
		TiToRect(rt, &rtRect);

		// case 1
		if (TiGetType(rt) == Tile::SPACE && 
			rtRect.xmaxi > tileRect.xmaxi && 
			rtRect.ymaxi - rtRect.ymini < TileStandardWireWidth * 5 &&
			rtRect.xmaxi - tileRect.xmaxi <= TileStandardWireWidth * 5) 
		{
			bool shrink = true;
			// go along top edge
			for (Tile * tp = RT(rt); LEFT(tp) > tileRect.xmaxi; tp = BL(tp)) {
				if (TiGetType(tp) == Tile::SPACE && LEFT(tp) >= tileRect.xmaxi && LEFT(tp) < rtRect.xmaxi) {
					shrink = false;
					break;
				}
			}
			if (shrink) {
				// go along bottom edge
				for (Tile * tp = TR(tile); LEFT(tp) < rtRect.xmaxi; tp = TR(tp)) {
					if (TiGetType(tp) == Tile::SPACE && LEFT(tp) >= tileRect.xmaxi && LEFT(tp) < rtRect.xmaxi) {
						shrink = false;
						break;
					}
				}
			}

			if (shrink) {
				doInsert = true;
				newRect = rtRect;
				newRect.xmini = tileRect.xmaxi;
			}
		}

		if (!doInsert) {
			// case 2
			if (TiGetType(rt) == Tile::OBSTACLE && 
				rtRect.xmaxi > tileRect.xmaxi && 
				rtRect.xmini > tileRect.xmini &&
				tileRect.ymaxi - tileRect.ymini < TileStandardWireWidth * 5 &&
				rtRect.xmini - tileRect.xmini <= TileStandardWireWidth * 5) 
			{
				bool shrink = true;
				for (Tile * tp = LB(tile); LEFT(tp) < tileRect.xmaxi; tp = TR(tp)) {
					if (TiGetType(tp) == Tile::SPACE && LEFT(tp) >= rtRect.xmini && LEFT(tp) < tileRect.xmaxi) {
						shrink = false;
						break;
					}
				}
				if (shrink) {
					doInsert = true;
					newRect = tileRect;
					newRect.xmaxi = rtRect.xmini;
				}
			}
		}

		if (!doInsert) {
			Tile * lb = LB(tile);
			TileRect lbRect;
			TiToRect(lb, &lbRect);

			// case 3
			if (TiGetType(lb) == Tile::SPACE && 
				lbRect.xmini < tileRect.xmini && 
				lbRect.ymaxi - lbRect.ymini < TileStandardWireWidth * 5 &&
				lbRect.xmaxi - tileRect.xmini <= TileStandardWireWidth * 5) 
			{
				bool shrink = true;
				// go along top edge
				for (Tile * tp = TR(tile); RIGHT(tp) > lbRect.xmini; tp = BL(tp)) {
					if (TiGetType(tp) == Tile::SPACE && RIGHT(tp) > lbRect.xmini && RIGHT(tp) < tileRect.xmini) {
						shrink = false;
						break;
					}
				}
				if (shrink) {
					// go along bottom edge
					for (Tile * tp = LB(lb); RIGHT(tp) < tileRect.xmini; tp = TR(tp)) {
						if (TiGetType(tp) == Tile::SPACE && RIGHT(tp) > lbRect.xmini && RIGHT(tp) < tileRect.xmini) {
							shrink = false;
							break;
						}
					}
				}

				if (shrink) {
					doInsert = true;
					newRect = lbRect;
					newRect.xmini = tileRect.xmini;
				}
			}

			if (!doInsert) {
				// case 4
				if (TiGetType(lb) == Tile::OBSTACLE && 
					lbRect.xmini < tileRect.xmini && 
					lbRect.xmaxi < tileRect.xmaxi &&
					tileRect.ymaxi - tileRect.ymini < TileStandardWireWidth * 5 &&
					tileRect.xmaxi - lbRect.xmaxi <= TileStandardWireWidth * 5) 
				{
					bool shrink = true;
					for (Tile * tp = RT(tile); RIGHT(tp) > tileRect.xmini; tp = BL(tp)) {
						if (TiGetType(tp) == Tile::SPACE && RIGHT(tp) >= tileRect.xmini && RIGHT(tp) < lbRect.xmaxi) {
							shrink = false;
							break;
						}
					}
					if (shrink) {
						doInsert = true;
						newRect = tileRect;
						newRect.xmini = lbRect.xmaxi;
					}
				}
			}
		}

		if (doInsert) {
			QList<Tile *> alreadyTiled;
			insertTile(thePlane, newRect, alreadyTiled, NULL, Tile::SPACE2, CMRouter::IgnoreAllOverlaps);
			//drawGridItem(newTile);
			TileRect leftRect = originalTileRect;
			leftRect.xmaxi = newRect.xmini;
			if (leftRect.xmaxi - leftRect.xmini > 0) tileRects.append(leftRect);
			TileRect rightRect = originalTileRect;
			rightRect.xmini = newRect.xmaxi;
			if (rightRect.xmaxi - rightRect.xmini > 0) tileRects.append(leftRect);
		}
		else {
			remainingTileRects.append(originalTileRect);
		}
	}

	foreach (TileRect tileRect, remainingTileRects) {
		Tile * tile = NULL;
		TiSrArea(NULL, thePlane, &tileRect, collectOneThinTile, &tile);
		if (tile == NULL) continue;

		infoTile("remaining", tile);
		//drawGridItem(tile);
	}

}

bool CMRouter::initBoard(ItemBase * board, Plane * thePlane, QList<Tile *> & alreadyTiled, qreal keepout)
{
	if (board == NULL) return true;

	QHash<QString, SvgFileSplitter *> svgHash;
	QSizeF boardSize = board->size();
	ResizableBoard * resizableBoard = qobject_cast<ResizableBoard *>(board->layerKinChief());
	if (resizableBoard != NULL) {
		boardSize = resizableBoard->getSizeMM() * FSvgRenderer::printerScale() / 25.4;
	}
	QString svg = TextUtils::makeSVGHeader(FSvgRenderer::printerScale(), 
										   FSvgRenderer::printerScale(), 
										   boardSize.width(), 
										   boardSize.height());
	svg += board->retrieveSvg(ViewLayer::Board, svgHash, true, FSvgRenderer::printerScale());
	svg += board->retrieveSvg(ViewLayer::Silkscreen1, svgHash, true, FSvgRenderer::printerScale());
	svg += board->retrieveSvg(ViewLayer::Silkscreen0, svgHash, true, FSvgRenderer::printerScale());
	svg += "</svg>";
	GroundPlaneGenerator gpg;
	QList<QRect> rects;
	gpg.getBoardRects(svg, board, FSvgRenderer::printerScale(), keepout, rects);
	QPointF boardPos = board->pos();
	foreach (QRect r, rects) {
		TileRect tileRect;
		realsToTile(tileRect, r.left() + boardPos.x(), r.top() + boardPos.y(), r.right() + boardPos.x(), r.bottom() + 1 + boardPos.y());  // note off-by-one weirdness
		insertTile(thePlane, tileRect, alreadyTiled, NULL, Tile::OBSTACLE, CMRouter::IgnoreAllOverlaps);
		//drawGridItem(tile);
	}

	return true;
}

bool clipRect(TileRect * r, TileRect * clip, QList<TileRect> & rects) {
	if (!tileRectsIntersect(r, clip)) return false;

	if (r->ymini < clip->ymini)
	{
		TileRect s;
		s.xmini = r->xmini;
		s.ymini = r->ymini; 
		s.xmaxi = r->xmaxi;
		s.ymaxi = clip->ymini;
		rects.append(s);
		r->ymini = clip->ymini;
	}
	if (r->ymaxi > clip->ymaxi)
	{
		TileRect s;
		s.xmini = r->xmini;
		s.ymini = clip->ymaxi;
		s.xmaxi = r->xmaxi;
		s.ymaxi = r->ymaxi;
		rects.append(s);
		r->ymaxi = clip->ymaxi;
	}
	if (r->xmini < clip->xmini)
	{
		TileRect s;
		s.xmini = r->xmini;
		s.ymini = r->ymini;
		s.xmaxi = clip->xmini;
		s.ymaxi = r->ymaxi;
		rects.append(s);
		r->xmini = clip->xmini;
	}
	if (r->xmaxi > clip->xmaxi)
	{
		TileRect s;
		s.xmini = clip->xmaxi;
		s.ymini = r->ymini;
		s.xmaxi = r->xmaxi;
		s.ymaxi = r->ymaxi;
		rects.append(s);
		r->xmaxi = clip->xmaxi;
	}

	return true;
}

void CMRouter::tileWire(Wire * wire, QList<Wire *> & beenThere, QList<Tile *> & alreadyTiled, Tile::TileType tileType, 
						CMRouter::OverlapType overlapType, qreal keepout, bool eliminateThin) 
{
	DebugDialog::debug(QString("coords wire %1, x1:%2 y1:%3, x2:%4 y2:%5")
		.arg(wire->id())
		.arg(wire->pos().x())
		.arg(wire->pos().y())
		.arg(wire->line().p2().x())
		.arg(wire->line().p2().y()) );			


	QList<ConnectorItem *> ends;
	QList<Wire *> wires;
	wire->collectChained(wires, ends);
	beenThere.append(wires);
	if (ends.count() < 1) {
		// something is very wrong
		return;
	}

	tileWires(wires, alreadyTiled, tileType, overlapType, keepout, eliminateThin);
}

void CMRouter::tileWires(QList<Wire *> & wires, QList<Tile *> & alreadyTiled, Tile::TileType tileType, 
						 CMRouter::OverlapType overlapType, qreal keepout, bool eliminateThin) 
{
	QMultiHash<Wire *, QRectF> wireRects;
	foreach (Wire * wire, wires) {
		QPointF p1 = wire->pos();
		QPointF p2 = wire->line().p2() + p1;
		qreal dx = qAbs(p1.x() - p2.x());
		qreal dy = qAbs(p1.y() - p2.y());
		if (dx < CloseEnough) {
			// vertical line
			qreal x = qMin(p1.x(), p2.x()) - (wire->width() / 2) - keepout;
			qreal y = qMin(p1.y(), p2.y()) - keepout;			
			wireRects.insert(wire, QRectF(x, y, wire->width() + dx + keepout + keepout, dy + keepout + keepout));
			qobject_cast<TraceWire *>(wire)->setWireDirection(TraceWire::Vertical);
		}
		else if (dy < CloseEnough) {
			// horizontal line
			qreal y = qMin(p1.y(), p2.y()) - (wire->width() / 2) - keepout;
			qreal x = qMin(p1.x(), p2.x()) - keepout;
			wireRects.insert(wire, QRectF(x, y, qMax(p1.x(), p2.x()) - x + keepout + keepout, wire->width() + dy + keepout + keepout));
			qobject_cast<TraceWire *>(wire)->setWireDirection(TraceWire::Horizontal);
		}
		else {
			qreal factor = (eliminateThin ? Wire::STANDARD_TRACE_WIDTH : 1);
			qreal w = (dx + keepout + keepout) / factor;
			qreal h = (dy + keepout + keepout) / factor;
			QImage image(w, h, QImage::Format_RGB32);
			image.fill(0xff000000);
			QPainter painter(&image);
			painter.setRenderHint(QPainter::Antialiasing);
			painter.scale(1 / factor, 1 / factor);
			qreal tx = keepout;
			qreal ty = keepout;
			if (p2.x() < p1.x()) tx += dx;
			if (p2.y() < p1.y()) ty += dy;
			painter.translate(tx, ty);
			QPen pen = painter.pen();
			pen.setColor(QColor(255,255,255,255));
			pen.setWidth(Wire::STANDARD_TRACE_WIDTH + keepout + keepout);
			painter.setPen(pen);
			painter.drawLine(wire->line());
			painter.end();

			QList<QRect> rects;
			GroundPlaneGenerator::scanLines(image, w, h, rects, 1, 1);
			foreach (QRect rect, rects) {
				QRectF r(p1.x() - tx + (rect.left() * factor),
						 p1.y() - ty + (rect.top() * factor),
						 rect.width() * factor, 
						 rect.height() * factor);
				wireRects.insert(wire, r);
			}

			qobject_cast<TraceWire *>(wire)->setWireDirection(TraceWire::Diagonal);
		}
	}

	foreach (Wire * w, wireRects.uniqueKeys()) {
		foreach (QRectF r, wireRects.values(w)) {
			TileRect tileRect;
			realsToTile(tileRect, r.left(), r.top(), r.right(), r.bottom());
			insertTile(m_planeHash.value(w->viewLayerID()), tileRect, alreadyTiled, w, tileType, overlapType);
			//drawGridItem(tile);
			if (alreadyTiled.count() > 0) {
				return;
			}
		}
	}

}

ConnectorItem * CMRouter::splitTrace(Wire * wire, QPointF point) 
{
	// split the trace at point
	QLineF originalLine = wire->line();
	QLineF newLine(QPointF(0,0), point - wire->pos());
	wire->setLine(newLine);
	TraceWire * splitWire = drawOneTrace(point, originalLine.p2() + wire->pos(), wire->width(), wire->viewLayerSpec());
	splitWire->setAutoroutable(wire->getAutoroutable());
	ConnectorItem * connector1 = wire->connector1();
	ConnectorItem * newConnector1 = splitWire->connector1();
	foreach (ConnectorItem * toConnectorItem, connector1->connectedToItems()) {
		connector1->tempRemove(toConnectorItem, false);
		toConnectorItem->tempRemove(connector1, false);
		newConnector1->tempConnectTo(toConnectorItem, false);
		toConnectorItem->tempConnectTo(newConnector1, false);
	}

	connector1->tempConnectTo(splitWire->connector0(), false);
	splitWire->connector0()->tempConnectTo(connector1, false);

	return splitWire->connector0();
}

void CMRouter::hookUpWires(QList<PathUnit *> & fullPath, QList<Wire *> & wires, qreal keepout) {
	if (wires.count() <= 0) return;
		
	QSet<ViewLayer::ViewLayerID> viewLayerIDs;

	PathUnit * fromPathUnit = fullPath.first();
	PathUnit * toPathUnit = fullPath.last();
	if (fromPathUnit->connectorItem) {
		fromPathUnit->connectorItem->tempConnectTo(wires[0]->connector0(), true);
		wires[0]->connector0()->tempConnectTo(fromPathUnit->connectorItem, true);
	}
	else {
	}
	int last = wires.count() - 1;
	if (toPathUnit->connectorItem) {
		toPathUnit->connectorItem->tempConnectTo(wires[last]->connector1(), true);
		wires[last]->connector1()->tempConnectTo(toPathUnit->connectorItem, true);
	}
	else {
	}
	for (int i = 0; i < last; i++) {
		ConnectorItem * c1 = wires[i]->connector1();
		ConnectorItem * c0 = wires[i + 1]->connector0();
		c1->tempConnectTo(c0, true);
		c0->tempConnectTo(c1, true);
	}

	QList<Tile *> alreadyTiled;
	tileWires(wires, alreadyTiled, m_sketchWidget->autorouteTypePCB() ? Tile::OBSTACLE : Tile::SCHEMATICWIRESPACE, CMRouter::ClipAllOverlaps, keepout, true);
	qreal l = std::numeric_limits<int>::max();
	qreal t = std::numeric_limits<int>::max();
	qreal r = std::numeric_limits<int>::min();
	qreal b = std::numeric_limits<int>::min();
	foreach (Wire * w, wires) {
		viewLayerIDs.insert(w->viewLayerID());
		QPointF p1 = w->pos();
		QPointF p2 = w->line().p2() + p1;
		l = qMin(p1.x(), l);
		r = qMax(p1.x(), r);
		l = qMin(p2.x(), l);
		r = qMax(p2.x(), r);
		t = qMin(p1.y(), t);
		b = qMax(p1.y(), b);
		t = qMin(p2.y(), t);
		b = qMax(p2.y(), b);
	}

	TileRect searchRect;
	realsToTile(searchRect, l - Wire::STANDARD_TRACE_WIDTH, t - Wire::STANDARD_TRACE_WIDTH, r + Wire::STANDARD_TRACE_WIDTH, b + Wire::STANDARD_TRACE_WIDTH); 
	
	foreach (ViewLayer::ViewLayerID viewLayerID, viewLayerIDs.values()) {
		QList<TileRect> tileRects;
		Plane * plane = m_planeHash.value(viewLayerID);
		TiSrArea(NULL, plane, &searchRect, collectThinTiles, &tileRects);
		eliminateThinTiles(tileRects, plane);
	}

	if (m_unionPlane) {
		QList<TileRect> tileRects;
		TiSrArea(NULL, m_unionPlane, &searchRect, collectThinTiles, &tileRects);
		eliminateThinTiles(tileRects, m_unionPlane);

		TileRect searchRect90;
		tileRotate90(searchRect, searchRect90);
		tileRects.clear();
		TiSrArea(NULL, m_union90Plane, &searchRect90, collectThinTiles, &tileRects);
		eliminateThinTiles(tileRects, m_union90Plane);
	}
}

struct Range 
{
	int rmin;
	int rmax;

	Range(int mi, int ma) {
		rmin = mi;
		rmax = ma;
	}
	Range() {}
};
/*
bool CMRouter::findShortcut(TileRect & tileRect, bool useX, bool targetGreater, JSubedge * subedge, QList<QPointF> & allPoints, int ix)
{
	QList<Range> spaces;
	Range space;
	if (useX) {
		space.rmin = tileRect.xmini;
		space.rmax = tileRect.xmaxi;
	}
	else {
		space.rmin = tileRect.ymini;
		space.rmax = tileRect.ymaxi;
	}
	if (tileToReal(space.rmax - space.rmin) < m_wireWidthNeeded) {
		return false;
	}

	spaces.append(space);
	Range cacheSpace(space.rmin, space.rmax);

	QList<Tile *> alreadyTiled;
	TiSrArea(NULL, subedge->edge->plane, &tileRect, simpleList, &alreadyTiled);
	foreach (Tile * tile, alreadyTiled) {
		short result = checkCandidate(subedge, tile, false);
		if (result < GridEntry::BLOCK) continue;

		Range range;
		if (useX) {
			range.rmin = LEFT(tile);
			range.rmax = RIGHT(tile);
		}
		else {
			range.rmin = YMIN(tile);
			range.rmax = YMAX(tile);
		}
		for (int sp = 0; sp < spaces.count(); sp++) {
			Range nextSpace = spaces.at(sp);
			if (nextSpace.rmax <= range.rmin) continue;
			if (range.rmax <= nextSpace.rmin) continue;

			if (range.rmin <= nextSpace.rmin && range.rmax >= nextSpace.rmax) {
				spaces.replace(sp, Range(0,0));
			}
			else if (range.rmin >= nextSpace.rmin && range.rmax <= nextSpace.rmax) {
				spaces.replace(sp, Range(nextSpace.rmin, range.rmin));
				spaces.append(Range(range.rmax, nextSpace.rmax));
			}
			else if (range.rmax > nextSpace.rmin) {
				spaces.replace(sp, Range(range.rmax, nextSpace.rmax));
			}
			else {
				spaces.replace(sp, Range(nextSpace.rmin, range.rmin));
			}
		}
	}

	bool success = false;
	qreal result;
	if (targetGreater) {
		result = std::numeric_limits<int>::min();
	}
	else {
		result = std::numeric_limits<int>::max();
	}
	foreach (Range range, spaces) {
		if (tileToReal(range.rmax - range.rmin) >= m_wireWidthNeeded) {
			if (targetGreater) {
				if (tileToReal(range.rmax) - m_halfWireWidthNeeded > result) {
					result = tileToReal(range.rmax) - m_halfWireWidthNeeded;
					success = true;
				}
			}
			else {
				if (tileToReal(range.rmin) + m_halfWireWidthNeeded < result) {
					result = tileToReal(range.rmin) + m_halfWireWidthNeeded;
					success = true;
				}
			}
		}
	}

	if (!success) return false;

	QPointF newP1 = allPoints.at(ix + 1);
	QPointF newP2 = allPoints.at(ix + 2);
	if (useX) {
		newP1.setX(result);
		newP2.setX(result);
	}
	else {
		newP1.setY(result);
		newP2.setY(result);
	}
	allPoints.replace(ix + 1, newP1);
	allPoints.replace(ix + 2, newP2);
	QPointF p0 = allPoints.at(ix + 0);
	QPointF p3 = allPoints.at(ix + 3);
	if (p0 == newP1) {
		allPoints.removeAt(ix + 1);
	}
	if (p3 == newP2) {
		allPoints.removeAt(ix + 2);
	}

	return true;
}
*/

/*
void CMRouter::shortenUs(QList<QPointF> & allPoints, JSubedge * subedge)
{
	// TODO: this could be implemented recursively as a child tile space 
	//		with the goals being the sides of the U-shape and the obstacles copied in from the parent tile space
	// for now just look for a straight line
	int ix = 0;
	while (ix < allPoints.count() - 3) {
		QPointF p0 = allPoints.at(ix);
		QPointF p1 = allPoints.at(ix + 1);
		QPointF p2 = allPoints.at(ix + 2);
		QPointF p3 = allPoints.at(ix + 3);
		ix += 1;

		TileRect tileRect;
		if (p1.x() == p2.x()) {
			if ((p0.x() > p1.x() && p3.x() > p2.x()) || (p0.x() < p1.x() && p3.x() < p2.x())) {
				// opening to left or right
				bool targetGreater;
				if (p0.x() < p1.x()) {
					// opening left
					targetGreater = false;
					realsToTile(tileRect, qMax(p0.x(), p3.x()), qMin(p0.y(), p3.y()), p2.x(), qMax(p0.y(), p3.y()));
				}
				else {
					// opening right
					targetGreater = true;
					realsToTile(tileRect, p2.x(), qMin(p0.y(), p3.y()), qMin(p0.x(), p3.x()), qMax(p0.y(), p3.y()));
				}

				if (findShortcut(tileRect, true, targetGreater, subedge, allPoints, ix - 1)) {
					ix--;
				}
			}
			else {
				// not a U-shape
				continue;
			}
		}
		else if (p1.y() == p2.y()) {
			if ((p0.y() > p1.y() && p3.y() > p2.y()) || (p0.y() < p1.y() && p3.y() < p2.y())) {
				// opening to top or bottom
				bool targetGreater;
				if (p0.y() < p1.y()) {
					// opening top
					targetGreater = false;
					realsToTile(tileRect, qMin(p0.x(), p3.x()), qMax(p0.y(), p3.y()), qMax(p0.x(), p3.x()), p2.y());
				}
				else {
					// opening bottom
					targetGreater = true;
					realsToTile(tileRect, qMin(p0.x(), p3.x()), p2.y(), qMax(p0.x(), p3.x()), qMin(p0.y(), p3.y()));
				}

				if (findShortcut(tileRect, false, targetGreater, subedge, allPoints, ix - 1)) {
					ix--;
				}
			}
			else {
				// not a U-shape
				continue;
			}
		}

	}
}
*/

void CMRouter::removeCorners(QList<QPointF> & allPoints, Plane * thePlane)
{
	int ix = 0;
	while (ix < allPoints.count() - 3) {
		QPointF p0 = allPoints.at(ix);
		QPointF p1 = allPoints.at(ix + 1);
		QPointF p2 = allPoints.at(ix + 2);
		QPointF p3 = allPoints.at(ix + 3);
		ix += 1;

		bool removeCorner = false;
		QPointF proposed;
		if (p0.y() == p1.y()) {
			if (p2.y() == p3.y() && p1.x() == p2.x()) {
				if ((p0.x() < p1.x() && p2.x() < p3.x()) || (p0.x() > p1.x() && p2.x() > p3.x())) {
					// x must be monotonically increasing or decreasing				
					// dogleg horizontal, vertical, horizontal
				}
				else continue;
			}
			else {
				continue;
			}

			proposed.setX(p3.x());
			proposed.setY(p1.y());
			removeCorner = checkProposed(proposed, p1, p3, thePlane, ix == 1 || ix + 3 == allPoints.count());
			if (!removeCorner) {
				proposed.setX(p0.x());
				proposed.setY(p2.y());
				removeCorner = checkProposed(proposed, p0, p2, thePlane, ix == 1 || ix + 3 == allPoints.count());
			}
		}
		else if (p0.x() == p1.x()) {
			if (p2.x() == p3.x() && p1.y() == p2.y()) {
				if ((p0.y() < p1.y() && p2.y() < p3.y()) || (p0.y() > p1.y() && p2.y() > p3.y())) {
					// y must be monotonically increasing or decreasing
					// dogleg vertical, horizontal, vertical
				}
				else continue;
			}
			else {
				continue;
			}

			proposed.setY(p3.y());
			proposed.setX(p1.x());
			removeCorner = checkProposed(proposed, p1, p3, thePlane, ix == 1 || ix + 3 == allPoints.count());
			if (!removeCorner) {
				proposed.setY(p0.y());
				proposed.setX(p2.x());
				removeCorner = checkProposed(proposed, p0, p2, thePlane, ix == 1 || ix + 3 == allPoints.count());
			}
		}
		if (!removeCorner) continue;

		ix--;
		allPoints.replace(ix + 1, proposed);
		allPoints.removeAt(ix + 2);
		if (ix + 3 < allPoints.count()) {
			if (proposed.x() == p3.x()) {
				if (p3.x() == allPoints.at(ix + 3).x()) {
					allPoints.removeAt(ix + 2);
				}
			}
			else if (proposed.y() == p3.y()) {
				if (p3.y() == allPoints.at(ix + 3).y()) {
					allPoints.removeAt(ix + 2);
				}
			}
		}
		if (ix > 0) {
			if (proposed.x() == p0.x()) {
				if (p0.x() == allPoints.at(ix - 1).x()) {
					allPoints.removeAt(ix);
				}
			}
			else if (proposed.y() == p0.y()) {
				if (p0.y() == allPoints.at(ix - 1).y()) {
					allPoints.removeAt(ix);
				}
			}
		}
		foreach (QPointF p, allPoints) {
			DebugDialog::debug("allpoint during:", p);
		}
	}
}

bool CMRouter::checkProposed(const QPointF & proposed, const QPointF & p1, const QPointF & p3, Plane * thePlane, bool atStartOrEnd) 
{
	if (atStartOrEnd) {
		Tile * tile = TiSrPoint(NULL, thePlane, realToTile(proposed.x()), realToTile(proposed.y()));
		switch (TiGetType(tile)) {
			case Tile::SPACE:
			case Tile::SPACE2:
			case Tile::SCHEMATICWIRESPACE:
				break;
			default:
				return false;
		}
	}

	QList<Tile *> alreadyTiled;
	TileRect tileRect;
	qreal x = proposed.x() - HalfStandardWireWidth;
	realsToTile(tileRect, x, qMin(p1.y(), p3.y()), x + Wire::STANDARD_TRACE_WIDTH, qMax(p1.y(), p3.y()));
	TiSrArea(NULL, thePlane, &tileRect, simpleList, &alreadyTiled);
	qreal y = proposed.y() - HalfStandardWireWidth;
	realsToTile(tileRect, qMin(p1.x(), p3.x()), y, qMax(p1.x(), p3.x()), y + Wire::STANDARD_TRACE_WIDTH);
	TiSrArea(NULL, thePlane, &tileRect, simpleList, &alreadyTiled);
	foreach (Tile * tile, alreadyTiled) {
		switch (TiGetType(tile)) {
			case Tile::SPACE:
			case Tile::SPACE2:
			case Tile::SCHEMATICWIRESPACE:
				break;
			default:
				return false;
		}
	}

	return true;
}

void CMRouter::appendIf(PathUnit * pathUnit, Tile * next, QList<Tile *> & tiles, QMultiHash<Tile *, PathUnit *> & tilePathUnits, PathUnit::Direction direction, int tWidthNeeded) 
{
	if (pathUnit->tile == next) {
		// just came from here
		return;
	}

	foreach (PathUnit * pu, tilePathUnits.values(next)) {
		if (pu->parent == pathUnit) {
			// we've already explored this path
			return;
		}
	}

	if (TiGetType(next) == Tile::BUFFER) {
		return;
	}

	//infoTile("    append if", next);

	bool horizontal = (direction == PathUnit::Left || direction == PathUnit::Right);

	Tile::TileType tileType = TiGetType(pathUnit->tile);

	bool bail = true;
	switch (TiGetType(next)) {  
		// next tile check
		case Tile::OBSTACLE:
			break;
		case Tile::SCHEMATICWIRESPACE:
			// can go through other traces, but not along them
			{
				TraceWire * traceWire = dynamic_cast<TraceWire *>(TiGetBody(next));
				bail = (horizontal && traceWire->wireDirection()  == TraceWire::Horizontal) || (!horizontal && traceWire->wireDirection() == TraceWire::Vertical);
			}
			break;
		case Tile::SOURCE:
			bail = (tileType != Tile::DESTINATION); 
			break;
		case Tile::DESTINATION:
			bail = (tileType != Tile::SOURCE); 
			break;
		case Tile::SPACE:
		case Tile::SPACE2:
			bail = false;
			break;
		default:	
			DebugDialog::debug("default tile type: shouldn't happen");
			return;
	}

	if (bail) return;

	switch(tileType) {	
		// this tile check
		case Tile::SCHEMATICWIRESPACE:
			{
				TraceWire * traceWire = dynamic_cast<TraceWire *>(TiGetBody(pathUnit->tile));
				bail = (horizontal && traceWire->wireDirection()  == TraceWire::Horizontal) || (!horizontal && traceWire->wireDirection() == TraceWire::Vertical);
				if (!bail) {
					bail = blockDirection(pathUnit, direction, next, tWidthNeeded);
				}

			}
			break;
		case Tile::SPACE:
		case Tile::SPACE2:
			if (WIDTH(pathUnit->tile) < tWidthNeeded || HEIGHT(pathUnit->tile) < tWidthNeeded) {
				bail = blockDirection(pathUnit, direction, next, tWidthNeeded);
			}
			break;

	}

	if (bail) return;

	if (pathUnit->parent != NULL || pathUnit->connectorItem == NULL) {
		// wires and space tiles: make sure there is room to draw a wire from pathUnit to next
		if (horizontal) {
			if (qMin(YMAX(pathUnit->tile), YMAX(next)) - qMax(YMIN(pathUnit->tile), YMIN(next)) < tWidthNeeded) bail = true;
		}
		else {
			if (qMin(RIGHT(pathUnit->tile), RIGHT(next)) - qMax(LEFT(pathUnit->tile), LEFT(next)) < tWidthNeeded) bail = true;
		}
	}
	else {
		if (horizontal) {
			if (qMin(pathUnit->minCostRect.ymaxi, YMAX(next)) - qMax(pathUnit->minCostRect.ymini, YMIN(next)) < tWidthNeeded) bail = true;
		}
		else {
			if (qMin(pathUnit->minCostRect.xmaxi, RIGHT(next)) - qMax(pathUnit->minCostRect.xmini, LEFT(next)) < tWidthNeeded) bail = true;
		}
	}

	if (bail) return;

	drawGridItem(next);

	tiles.append(next);
}

bool CMRouter::blockDirection(PathUnit * pathUnit, PathUnit::Direction direction, Tile * next, int tWidthNeeded) 
{
	// if pathUnit is restricted, make sure you can draw a wire from pathUnit->parent to next
	bool bail = false;
	PathUnit * parent = pathUnit->parent;
	if (parent) {
		// can only move through this pathUnit in one direction 
		if (LEFT(parent->tile) == RIGHT(pathUnit->tile)) {
			if (direction == PathUnit::Left) {
				bail = qMin(YMAX(next), YMAX(parent->tile)) - qMax(YMIN(next), YMIN(parent->tile)) < tWidthNeeded;
			}
			else bail = true;
		}
		else if (RIGHT(parent->tile) == LEFT(pathUnit->tile)) {
			if (direction == PathUnit::Right) {
				bail = qMin(YMAX(next), YMAX(parent->tile)) - qMax(YMIN(next), YMIN(parent->tile)) < tWidthNeeded;
			}
			else bail = true;
		}
		else if (YMIN(parent->tile) == YMAX(pathUnit->tile)) {
			if (direction == PathUnit::Up) {
				bail = qMin(RIGHT(next), RIGHT(parent->tile)) - qMax(LEFT(next), LEFT(parent->tile)) < tWidthNeeded;
			}
			else bail = true;
		}
		else if (YMAX(parent->tile) == YMIN(pathUnit->tile)) {
			if (direction == PathUnit::Down) {
				bail = qMin(RIGHT(next), RIGHT(parent->tile)) - qMax(LEFT(next), LEFT(parent->tile)) < tWidthNeeded;
			}
			else bail = true;
		}
	}

	return bail;
}

void CMRouter::seedNext(PathUnit * pathUnit, QList<Tile *> & tiles, QMultiHash<Tile *, PathUnit *> & tilePathUnits) {
	//infoTile("seed next", pathUnit->tile);
	int tWidthNeeded = TileStandardWireWidth;
	if ((RIGHT(pathUnit->tile) < m_tileMaxRect.xmaxi) && (HEIGHT(pathUnit->tile) >= tWidthNeeded)) {
		Tile * next = TR(pathUnit->tile);
		appendIf(pathUnit, next, tiles, tilePathUnits, PathUnit::Right, tWidthNeeded);
		while (true) {
			next = LB(next);
			if (YMAX(next) <= YMIN(pathUnit->tile)) {
				break;
			}

			appendIf(pathUnit, next, tiles, tilePathUnits, PathUnit::Right, tWidthNeeded);
		}
	}

	if ((LEFT(pathUnit->tile) > m_tileMaxRect.xmini) && (HEIGHT(pathUnit->tile) >= tWidthNeeded)) {
		Tile * next = BL(pathUnit->tile);
		appendIf(pathUnit, next, tiles, tilePathUnits, PathUnit::Left, tWidthNeeded);
		while (true) {
			next = RT(next);
			if (YMIN(next) >= YMAX(pathUnit->tile)) {
				break;
			}

			appendIf(pathUnit, next, tiles, tilePathUnits, PathUnit::Left, tWidthNeeded);
		}
	}

	if ((YMAX(pathUnit->tile) < m_tileMaxRect.ymaxi) && (WIDTH(pathUnit->tile) >= tWidthNeeded)) {	
		Tile * next = RT(pathUnit->tile);
		appendIf(pathUnit, next, tiles, tilePathUnits, PathUnit::Down, tWidthNeeded);
		while (true) {
			next = BL(next);
			if (RIGHT(next) <= LEFT(pathUnit->tile)) {
				break;
			}

			appendIf(pathUnit, next, tiles, tilePathUnits, PathUnit::Down, tWidthNeeded);
		}
	}

	if ((YMIN(pathUnit->tile) > m_tileMaxRect.ymini) && (WIDTH(pathUnit->tile) >= tWidthNeeded)) {		
		Tile * next = LB(pathUnit->tile);
		appendIf(pathUnit, next, tiles, tilePathUnits, PathUnit::Up, tWidthNeeded);
		while (true) {
			next = TR(next);
			if (LEFT(next) >= RIGHT(pathUnit->tile)) {
				break;
			}

			appendIf(pathUnit, next, tiles, tilePathUnits, PathUnit::Up, tWidthNeeded);
		}
	}
}

GridEntry * CMRouter::drawGridItem(Tile * tile)
{
	if (tile == NULL) return NULL;

	QRectF r;
	tileToQRect(tile, r);

	GridEntry * gridEntry = TiGetGridEntry(tile);
	if (gridEntry == NULL) {
		gridEntry = new GridEntry(r, NULL);
		gridEntry->setZValue(m_sketchWidget->getTopZ());
		TiSetClient(tile, gridEntry);
	}
	else {
		QRectF br = gridEntry->boundingRect();
		if (br != r) {
			gridEntry->setRect(r);
			gridEntry->setDrawn(false);
		}
	}

	if (gridEntry->drawn()) return gridEntry;

	QColor c;
	switch (TiGetType(tile)) {
		case Tile::SPACE:
			c = QColor(255, 255, 0, GridEntryAlpha);
			break;
		case Tile::SPACE2:
			c = QColor(200, 200, 0, GridEntryAlpha);
			break;
		case Tile::SOURCE:
			c = QColor(0, 255, 0, GridEntryAlpha);
			break;
		case Tile::DESTINATION:
			c = QColor(0, 0, 255, GridEntryAlpha);
			break;
		case Tile::SCHEMATICWIRESPACE:
			c = QColor(255, 192, 203, GridEntryAlpha);
			break;
		case Tile::OBSTACLE:
			c = QColor(60, 60, 60, GridEntryAlpha);
			break;
		default:
			c = QColor(255, 0, 0, GridEntryAlpha);
			break;
	}

	gridEntry->setPen(c);
	gridEntry->setBrush(QBrush(c));
	if (gridEntry->scene() == NULL) {
		m_sketchWidget->scene()->addItem(gridEntry);
	}
	gridEntry->show();
	gridEntry->setDrawn(true);
	ProcessEventBlocker::processEvents();
	return gridEntry;
}


void CMRouter::collectEdges(QList<JEdge *> & edges)
{
	foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
		VirtualWire * vw = dynamic_cast<VirtualWire *>(item);
		if (vw == NULL) continue;

		ConnectorItem * from = vw->connector0()->firstConnectedToIsh();
		if (!from) {
			continue;
		}

		ConnectorItem * to = vw->connector1()->firstConnectedToIsh();
		if (!to) {
			continue;
		}

		bool fromOK = false;
		foreach (ViewLayer::ViewLayerID viewLayerID, m_viewLayerIDs) {
			if (m_sketchWidget->sameElectricalLayer2(viewLayerID, from->attachedToViewLayerID())) {
				fromOK = true;
				break;
			}
		}

		if (!fromOK) {
			from = from->getCrossLayerConnectorItem();
		}

		bool toOK = false;
		foreach (ViewLayer::ViewLayerID viewLayerID, m_viewLayerIDs) {
			if (m_sketchWidget->sameElectricalLayer2(viewLayerID, to->attachedToViewLayerID())) {
				toOK = true;
				break;
			}
		}

		if (!toOK) {
			to = to->getCrossLayerConnectorItem();
		}

		from->debugInfo("from");
		to->debugInfo("to");

		JEdge * edge0 = makeEdge(from, to, vw);
		edge0->id = edges.count();
		edges.append(edge0);
	}
}

void CMRouter::clearEdge(JEdge * edge) {
	edge->withJumper = edge->routed = false;
	edge->fromConnectorItems.clear();
	edge->toConnectorItems.clear();
	edge->fromTraces.clear();
	edge->toTraces.clear();
}

JEdge * CMRouter::makeEdge(ConnectorItem * from, ConnectorItem * to,  VirtualWire * vw) {
	JEdge * edge = new JEdge;
	edge->from = from;
	edge->to = to;
	edge->routed = false;
	edge->line = QLineF(vw->pos(), vw->pos() + vw->line().p2());
	QPointF pi = from->sceneAdjustedTerminalPoint(NULL);
	QPointF pj = to->sceneAdjustedTerminalPoint(NULL);
	double px = pi.x() - pj.x();
	double py = pi.y() - pj.y();
	edge->distance = (px * px) + (py * py);
	return edge;
}


void CMRouter::initUndo(QUndoCommand * parentCommand) {
	QList<JumperItem *> jumperItems;
	QList<TraceWire *> traceWires;
	QList<ItemBase *> doNotAutoroute;
	if (m_sketchWidget->usesJumperItem()) {
		foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
			JumperItem * jumperItem = dynamic_cast<JumperItem *>(item);
			if (jumperItem == NULL) continue;

			jumperItems.append(jumperItem);
			addUndoConnection(false, jumperItem, parentCommand);
			if (jumperItem->getAutoroutable()) continue;

			doNotAutoroute.append(jumperItem);
			// now deal with the traces connecting the jumperitem to the part
			QList<ConnectorItem *> both;
			foreach (ConnectorItem * ci, jumperItem->connector0()->connectedToItems()) both.append(ci);
			foreach (ConnectorItem * ci, jumperItem->connector1()->connectedToItems()) both.append(ci);
			foreach (ConnectorItem * connectorItem, both) {
				TraceWire * w = dynamic_cast<TraceWire *>(connectorItem->attachedTo());
				if (w == NULL) continue;

				QList<Wire *> wires;
				QList<ConnectorItem *> ends;
				w->collectChained(wires, ends);
				foreach (Wire * wire, wires) {
					wire->setAutoroutable(false);
				}
			}
		}
	}

	foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
		TraceWire * traceWire = dynamic_cast<TraceWire *>(item);
		if (traceWire == NULL) continue;

		traceWires.append(traceWire);
		addUndoConnection(false, traceWire, parentCommand);
		if (!traceWire->getAutoroutable()) {
			doNotAutoroute.append(traceWire);
		}
	}

	m_startState.clear();
	if (doNotAutoroute.count() > 0) {
		QList<long> modelIndexes;
		m_sketchWidget->copyHeart(doNotAutoroute, true, m_startState, modelIndexes);
	}

	foreach (TraceWire * traceWire, traceWires) {
		m_sketchWidget->makeDeleteItemCommand(traceWire, BaseCommand::SingleView, parentCommand);
	}
	foreach (JumperItem * jumperItem, jumperItems) {
		m_sketchWidget->makeDeleteItemCommand(jumperItem, BaseCommand::CrossView, parentCommand);
	}
	
	foreach (TraceWire * traceWire, traceWires) {
		if (traceWire->getAutoroutable()) {
			m_sketchWidget->deleteItem(traceWire, true, false, false);
		}
	}
	foreach (JumperItem * jumperItem, jumperItems) {
		if (jumperItem->getAutoroutable()) {
			m_sketchWidget->deleteItem(jumperItem, true, true, false);
		}
	}
}

void CMRouter::restoreOriginalState(QUndoCommand * parentCommand) {
	QUndoStack undoStack;
	undoStack.push(parentCommand);
	undoStack.undo();
}

void CMRouter::addToUndo(Wire * wire, QUndoCommand * parentCommand) 
{
	AddItemCommand * addItemCommand = new AddItemCommand(m_sketchWidget, BaseCommand::SingleView, ModuleIDNames::wireModuleIDName, wire->viewLayerSpec(), wire->getViewGeometry(), wire->id(), false, -1, parentCommand);
	new CheckStickyCommand(m_sketchWidget, BaseCommand::SingleView, wire->id(), false, CheckStickyCommand::RemoveOnly, parentCommand);
	
	new WireWidthChangeCommand(m_sketchWidget, wire->id(), wire->width(), wire->width(), parentCommand);
	new WireColorChangeCommand(m_sketchWidget, wire->id(), wire->colorString(), wire->colorString(), wire->opacity(), wire->opacity(), parentCommand);
	addItemCommand->turnOffFirstRedo();
}

void CMRouter::addToUndo(QUndoCommand * parentCommand) 
{
	QList<TraceWire *> wires;
	QList<JumperItem *> jumperItems;
	foreach (QGraphicsItem * item, m_sketchWidget->items()) {
		TraceWire * wire = dynamic_cast<TraceWire *>(item);
		if (wire != NULL) {
			m_sketchWidget->setClipEnds(wire, true);
			wire->update();
			if (wire->getAutoroutable()) {
				wire->setWireWidth(Wire::STANDARD_TRACE_WIDTH, m_sketchWidget);
			}
			addToUndo(wire, parentCommand);
			wires.append(wire);
			continue;
		}
		else {
			JumperItem * jumperItem = dynamic_cast<JumperItem *>(item);
			if (jumperItem == NULL) continue;

			ConnectorItem * connector0 = NULL;
			ConnectorItem * connector1 = NULL;
			QList<ConnectorItem *> connectorItems;
			connectorItems.append(jumperItem->connector0());
			ConnectorItem::collectEqualPotential(connectorItems, true, ViewGeometry::TraceFlag | ViewGeometry::RatsnestFlag);
			QList<ConnectorItem *> partsConnectors;
			ConnectorItem::collectParts(connectorItems, partsConnectors, false, ViewLayer::TopAndBottom);
			if (partsConnectors.count() <= 4) {
				// a jumperItem has 4 connectors (2 per 2 layers), so this jumper is not connected to anything else except by a trace
				// so add a "normal" connection
				connector0 = findPartForJumper(jumperItem->connector0());
				connector1 = findPartForJumper(jumperItem->connector1());
				if (connector0 == NULL || connector1 == NULL) {
					DebugDialog::debug("unable to find part for jumper");   // something is really fucked up...
					continue;
				}
			}

			jumperItems.append(jumperItem);
			jumperItem->saveParams();
			QPointF pos, c0, c1;
			jumperItem->getParams(pos, c0, c1);

			AddItemCommand * addItemCommand = new AddItemCommand(m_sketchWidget, BaseCommand::CrossView, ModuleIDNames::jumperModuleIDName, jumperItem->viewLayerSpec(), jumperItem->getViewGeometry(), jumperItem->id(), false, -1, parentCommand);
			addItemCommand->turnOffFirstRedo();
			new ResizeJumperItemCommand(m_sketchWidget, jumperItem->id(), pos, c0, c1, pos, c0, c1, parentCommand);
			new CheckStickyCommand(m_sketchWidget, BaseCommand::SingleView, jumperItem->id(), false, CheckStickyCommand::RemoveOnly, parentCommand);

			if (connector0 != NULL && connector1 != NULL) {
				m_sketchWidget->createWire(jumperItem->connector0(), connector0, ViewGeometry::NoFlag, false, true, BaseCommand::CrossView, parentCommand);
				m_sketchWidget->createWire(jumperItem->connector1(), connector1, ViewGeometry::NoFlag, false, true, BaseCommand::CrossView, parentCommand);
			}

		}
	}

	foreach (TraceWire * traceWire, wires) {
		addUndoConnection(true, traceWire, parentCommand);
	}
	foreach (JumperItem * jumperItem, jumperItems) {
		addUndoConnection(true, jumperItem, parentCommand);
	}
}

ConnectorItem * CMRouter::findPartForJumper(ConnectorItem * jumperConnectorItem) {
	TraceWire * traceWire = NULL;
	foreach (ConnectorItem * connectorItem, jumperConnectorItem->connectedToItems()) {
		traceWire = qobject_cast<TraceWire *>(connectorItem->attachedTo());
		if (traceWire) break;
	}
	if (traceWire == NULL) return NULL;

	QList<Wire *> wires;
	QList<ConnectorItem *> ends;
	traceWire->collectChained(wires, ends);
	foreach (ConnectorItem * end, ends) {
		if (end->attachedTo()->layerKinChief() != jumperConnectorItem->attachedTo()->layerKinChief()) {
			return end;
		}
	}

	return NULL;
}

void CMRouter::addUndoConnection(bool connect, JumperItem * jumperItem, QUndoCommand * parentCommand) {
	addUndoConnection(connect, jumperItem->connector0(), BaseCommand::CrossView, parentCommand);
	addUndoConnection(connect, jumperItem->connector1(), BaseCommand::CrossView, parentCommand);
}

void CMRouter::addUndoConnection(bool connect, TraceWire * traceWire, QUndoCommand * parentCommand) {
	addUndoConnection(connect, traceWire->connector0(), BaseCommand::SingleView, parentCommand);
	addUndoConnection(connect, traceWire->connector1(), BaseCommand::SingleView, parentCommand);
}

void CMRouter::addUndoConnection(bool connect, ConnectorItem * connectorItem, BaseCommand::CrossViewType crossView, QUndoCommand * parentCommand) 
{
	foreach (ConnectorItem * toConnectorItem, connectorItem->connectedToItems()) {
		VirtualWire * vw = qobject_cast<VirtualWire *>(toConnectorItem->attachedTo());
		if (vw != NULL) continue;

		ChangeConnectionCommand * ccc = new ChangeConnectionCommand(m_sketchWidget, crossView, 
												toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
												connectorItem->attachedToID(), connectorItem->connectorSharedID(),
												ViewLayer::specFromID(toConnectorItem->attachedToViewLayerID()),
												connect, parentCommand);
		ccc->setUpdateConnections(false);
	}
}

void CMRouter::clearEdges(QList<JEdge *> & edges) {
	foreach (JEdge * edge, edges) {
		delete edge;
	}
	edges.clear();
}

void CMRouter::doCancel(QUndoCommand * parentCommand) {
	restoreOriginalState(parentCommand);
	cleanUp();
}

void CMRouter::updateProgress(int num, int denom) 
{
	emit setProgressValue((int) MaximumProgress * (m_currentProgressPart + (num / (qreal) denom)) / (qreal) m_maximumProgressPart);
}

Tile * CMRouter::addTile(NonConnectorItem * nci, Tile::TileType type, Plane * thePlane, QList<Tile *> & alreadyTiled, CMRouter::OverlapType overlapType, qreal keepout) 
{
	QRectF r = nci->attachedTo()->mapRectToScene(nci->rect());
	TileRect tileRect;
	realsToTile(tileRect, r.left() - keepout, r.top() - keepout, r.right() + keepout, r.bottom() + keepout);
	Tile * tile = insertTile(thePlane, tileRect, alreadyTiled, nci, type, overlapType);
	//drawGridItem(tile);
	return tile;
}

void CMRouter::hideTiles() 
{
	foreach (QGraphicsItem * item, m_sketchWidget->items()) {
		GridEntry * gridEntry = dynamic_cast<GridEntry *>(item);
		if (gridEntry) gridEntry->setVisible(false);
	}
}

void CMRouter::clearPlane(Plane * thePlane, bool rotate90) 
{
	if (thePlane == NULL) return;

	QSet<Tile *> tiles;
	TiSrArea(NULL, thePlane, rotate90 ? &m_tileMaxRect90 : &m_tileMaxRect, prepDeleteTile, &tiles);
	foreach (Tile * tile, tiles) {
		TiFree(tile);
	}

	TiFreePlane(thePlane);
}

void CMRouter::displayBadTiles(QList<Tile *> & alreadyTiled) {
	hideTiles();
	foreach (Tile * tile, alreadyTiled) {
		TileRect tileRect;
		TiToRect(tile, &tileRect);
		displayBadTileRect(tileRect);
	}
	displayBadTileRect(m_overlappingTileRect);
}

void CMRouter::displayBadTileRect(TileRect & tileRect) {
	QRectF r;
	tileRectToQRect(tileRect, r);
	GridEntry * gridEntry = new GridEntry(r, NULL);
	gridEntry->setZValue(m_sketchWidget->getTopZ());
	QColor c(255, 0, 0, GridEntryAlpha);
	gridEntry->setPen(c);
	gridEntry->setBrush(QBrush(c));
	m_sketchWidget->scene()->addItem(gridEntry);
	gridEntry->show();
	ProcessEventBlocker::processEvents();
}

Tile * CMRouter::insertTile(Plane * thePlane, TileRect & tileRect, QList<Tile *> & alreadyTiled, QGraphicsItem * item, Tile::TileType tileType, CMRouter::OverlapType overlapType) 
{
	//infoTileRect("insert tile", tileRect);
	if (tileRect.xmaxi - tileRect.xmini <= 0) {
		DebugDialog::debug("zero width tile");
		return NULL;
	}

	bool gotOverlap = false;
	bool doClip = false;
	if (overlapType != CMRouter::IgnoreAllOverlaps) {
		TiSrArea(NULL, thePlane, &tileRect, checkAlready, &alreadyTiled);
		if (alreadyTiled.count() > 0) {	
			switch(overlapType) {
				case CMRouter::ReportAllOverlaps:
					gotOverlap = true;
					break;
				case CMRouter::ClipAllOverlaps:
					doClip = overlapsOnly(item, alreadyTiled);
					break;
				case CMRouter::AllowEquipotentialOverlaps:
					gotOverlap = !allowEquipotentialOverlaps(item, alreadyTiled);
					doClip = alreadyTiled.count() > 0;
					break;
			}
		}
	}

	if (gotOverlap) {
		m_overlappingTileRect = tileRect;
		DebugDialog::debug("!!!!!!!!!!!!!!!!!!!!!!! overlaps not allowed !!!!!!!!!!!!!!!!!!!!!!");
		return NULL;
	}
	Tile * newTile = NULL;
	if (doClip) {
		clipInsertTile(thePlane, tileRect, alreadyTiled, item, tileType);
	}
	else {
		newTile = TiInsertTile(thePlane, &tileRect, item, tileType);
		insertUnion(tileRect, item, tileType);
	}

	//drawGridItem(newTile);
	return newTile;
}

bool CMRouter::overlapsOnly(QGraphicsItem *, QList<Tile *> & alreadyTiled)
{
	bool doClip = false;
	for (int i = alreadyTiled.count() - 1;  i >= 0; i--) {
		Tile * intersectingTile = alreadyTiled.at(i);
		if (dynamic_cast<Wire *>(TiGetBody(intersectingTile)) != NULL || dynamic_cast<ConnectorItem *>(TiGetBody(intersectingTile)) != NULL) {
			doClip = true;
			continue;
		}

		alreadyTiled.removeAt(i);
	}

	return doClip;
}

bool CMRouter::allowEquipotentialOverlaps(QGraphicsItem * item, QList<Tile *> & alreadyTiled)
{
	bool collected = false;
	QList<ConnectorItem *> equipotential;
	Wire * w = dynamic_cast<Wire *>(item);
	if (w) {
		equipotential.append(w->connector0());
	}
	else {
		ConnectorItem * ci = dynamic_cast<ConnectorItem *>(item);
		equipotential.append(ci);
	}
		
	foreach (Tile * intersectingTile, alreadyTiled) {
		QGraphicsItem * bodyItem = TiGetBody(intersectingTile);
		ConnectorItem * ci = dynamic_cast<ConnectorItem *>(bodyItem);
		if (ci != NULL) {
			if (!collected) {
				ConnectorItem::collectEqualPotential(equipotential, false, ViewGeometry::NoFlag);
				collected = true;
			}
			if (!equipotential.contains(ci)) {
				// overlap not allowed
				infoTile("intersecting", intersectingTile);
				return false;
			}
		}
		else {
			Wire * w = dynamic_cast<Wire *>(bodyItem);
			if (w == NULL) return false;

			if (!collected) {
				ConnectorItem::collectEqualPotential(equipotential, false, ViewGeometry::NoFlag);
				collected = true;
			}

			if (!equipotential.contains(w->connector0())) {
				// overlap not allowed
				infoTile("intersecting", intersectingTile);
				return false;
			}
		}
	}

	return true;

}

void CMRouter::clipInsertTile(Plane * thePlane, TileRect & tileRect, QList<Tile *> & alreadyTiled, QGraphicsItem * item, Tile::TileType type) 
{
	//infoTileRect("clip insert", tileRect);

	QList<TileRect> tileRects;
	tileRects.append(tileRect);
	int ix = 0;
	while (ix < tileRects.count()) {
		bool clipped = false;
		TileRect * r = &tileRects[ix++];
		foreach (Tile * intersectingTile, alreadyTiled) {
			TileRect intersectingRect;
			TiToRect(intersectingTile, &intersectingRect);

			if (clipRect(r, &intersectingRect, tileRects)) {
				clipped = true;
				break;
			}
		}
		if (clipped) continue;


		TiInsertTile(thePlane, r, item, type);
		insertUnion(*r, item, type);
	}

	alreadyTiled.clear();
}

void CMRouter::clearGridEntries() {
	foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
		GridEntry * gridEntry = dynamic_cast<GridEntry *>(item);
		if (gridEntry == NULL) continue;

		delete gridEntry;
	}
}

bool CMRouter::reorderEdges(QList<JEdge *> & edges, QGraphicsLineItem * lineItem) {
	bool result = false;
	int ix = 0;
	while (ix < edges.count()) {
		JEdge * edge = edges.at(ix++);
		if (edge->routed && !edge->withJumper) continue;

		lineItem->setLine(edge->line);
		int minIndex = edges.count();
		foreach (QGraphicsItem * item, m_sketchWidget->scene()->collidingItems(lineItem)) {
			TraceWire * traceWire = dynamic_cast<TraceWire *>(item);
			if (traceWire == NULL) {
				ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
				if (connectorItem) {
					traceWire = TraceWire::getTrace(connectorItem);
					if (traceWire == NULL) {
						foreach (ConnectorItem * toConnectorItem, connectorItem->connectedToItems()) {
							traceWire = TraceWire::getTrace(toConnectorItem);
							if (traceWire) break;
						}
					}
				}
			}

			if (traceWire == NULL) continue;

			JEdge * tedge = m_tracesToEdges.value(traceWire, NULL);
			if (tedge != NULL) {
				int tix = edges.indexOf(tedge);
				minIndex = qMin(tix, minIndex);
			}
		}

		if (minIndex < ix) {
			result = true;
			edges.removeOne(edge);
			edges.insert(minIndex, edge);
		}
	}

	return result;
}

void CMRouter::initPathUnit(JEdge * edge, Tile * tile, PriorityQueue<PathUnit *> & pq, QMultiHash<Tile *, PathUnit *> & tilePathUnits)
{	
	PathUnit * pathUnit = new PathUnit(&pq);

	ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(TiGetBody(tile));
	if (connectorItem) {
		//connectorItem->debugInfo("init path unit");
		pathUnit->connectorItem = connectorItem;
		foreach (ViewLayer::ViewLayerID viewLayerID, m_viewLayerIDs) {
			if (m_sketchWidget->sameElectricalLayer2(viewLayerID, connectorItem->attachedToViewLayerID())) {
				pathUnit->plane = m_planeHash.value(viewLayerID);
				break;
			}
		}
		QPointF p = connectorItem->sceneAdjustedTerminalPoint(NULL);
		realsToTile(pathUnit->minCostRect, p.x() - HalfStandardWireWidth, p.y() - HalfStandardWireWidth, p.x() + HalfStandardWireWidth, p.y() + HalfStandardWireWidth);
	}
	else {
		TraceWire * traceWire = dynamic_cast<TraceWire *>(TiGetBody(tile));
		if (traceWire == NULL) {
			// shouldn't be here
			delete pathUnit;
			return;
		}

		pathUnit->plane = m_planeHash.value(traceWire->viewLayerID());
		pathUnit->wire = traceWire;
		QLineF line = traceWire->line();
		QPointF p1 = traceWire->pos() + line.p1();
		QPointF p2 = traceWire->pos() + line.p2();
		TileRect tileRect;
		TiToRect(tile, &tileRect);
		switch (traceWire->wireDirection()) {
			case TraceWire::Vertical:
				pathUnit->minCostRect.ymini = tileRect.ymini;
				pathUnit->minCostRect.ymaxi = tileRect.ymaxi;
				pathUnit->minCostRect.xmini = realToTile(qMin(p1.x(), p2.x()) - HalfStandardWireWidth);
				pathUnit->minCostRect.xmaxi = realToTile(qMax(p1.x(), p2.x()) + HalfStandardWireWidth);
				break;

			case TraceWire::Horizontal:
				pathUnit->minCostRect.xmini = tileRect.xmini;
				pathUnit->minCostRect.xmaxi = tileRect.xmaxi;
				pathUnit->minCostRect.ymini = realToTile(qMin(p1.y(), p2.y()) - HalfStandardWireWidth);
				pathUnit->minCostRect.ymaxi = realToTile(qMax(p1.y(), p2.y()) + HalfStandardWireWidth);
				break;

			case TraceWire::Diagonal:
				pathUnit->minCostRect = tileRect;
				break;

			case TraceWire::NoDirection:
				DebugDialog::debug("Wire direction not set; shouldn't be here");
				delete pathUnit;
				return;
		}
	}

	m_pathUnits.append(pathUnit);
	pathUnit->edge = edge;
	pathUnit->tile = tile;
	drawGridItem(tile);
	pq.append(pathUnit);
	tilePathUnits.insert(tile, pathUnit);
}

PathUnit * CMRouter::findNearestSpace(PriorityQueue<PathUnit *> & priorityQueue, QMultiHash<Tile *, PathUnit *> & tilePathUnits, int tWidthNeeded, int tHeightNeeded, TileRect & nearestSpace) 
{
	PathUnit * nearest = NULL;
	int bestCost = std::numeric_limits<int>::max();
	foreach (PathUnit * pathUnit, tilePathUnits.values()) {
		Tile::TileType tileType = TiGetType(pathUnit->tile);
		if (tileType != Tile::SPACE && tileType != Tile::SPACE2) continue;
		if (pathUnit->priorityQueue != &priorityQueue) continue;

		TileRect tileRect;
		TiToRect(pathUnit->tile, &tileRect);
		if (tileRect.xmaxi - tileRect.xmini < tWidthNeeded) {
			// tile not wide enough; bail
			continue;
		}

		if (pathUnit->sourceCost >= bestCost) {
			// the current solution is closer to the connector; bail
			continue;
		}

		// look at adjacent space tiles to see if the via or jumperItem connector can fit
		// this also checks that the space isn't beneath a part

		TileRect searchRect = tileRect;
		searchRect.ymini = qMax(m_tileMaxRect.ymini, tileRect.ymini - tHeightNeeded + TileStandardWireWidth);
		searchRect.ymaxi = qMin(m_tileMaxRect.ymaxi, tileRect.ymaxi + tHeightNeeded - TileStandardWireWidth);
		if (searchRect.ymaxi - searchRect.ymini < tHeightNeeded) continue;

		//infoTileRect("search rect", searchRect);

		QList<Tile *> spaces;
		TiSrArea(NULL, m_unionPlane, &searchRect, findSpaces, &spaces);
		foreach (Tile * space, spaces) {
			if (WIDTH(space) < tWidthNeeded || HEIGHT(space) < tHeightNeeded) continue;

			TileRect minCostRect = calcMinCostRect(pathUnit, space);
			int sourceCost = pathUnit->sourceCost + manhattan(pathUnit->minCostRect, minCostRect);	
			if (sourceCost < bestCost) {
				bestCost = sourceCost;
				TiToRect(space, &nearestSpace);
				nearest = pathUnit;
			}
		}

		TileRect searchRect90;
		tileRotate90(searchRect, searchRect90);
		spaces.clear();
		TiSrArea(NULL, m_union90Plane, &searchRect90, findSpaces, &spaces);
		foreach (Tile * space, spaces) {
			if (WIDTH(space) < tWidthNeeded || HEIGHT(space) < tHeightNeeded) continue;

			TileRect spaceTileRect, spaceTileRect90;
			TiToRect(space, &spaceTileRect90);
			tileUnrotate90(spaceTileRect90, spaceTileRect);
			TileRect minCostRect = calcMinCostRect(pathUnit, spaceTileRect);
			int sourceCost = pathUnit->sourceCost + manhattan(pathUnit->minCostRect, minCostRect);	
			if (sourceCost < bestCost) {
				bestCost = sourceCost;
				nearestSpace = spaceTileRect;
				nearest = pathUnit;
			}
		}
	}

	return nearest;
}




void CMRouter::clipParts() 
{
	foreach (QGraphicsItem * item,  m_sketchWidget->scene()->items()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;

		switch (itemBase->itemType()) {
			case ModelPart::Wire: 
			case ModelPart::Jumper: 
			case ModelPart::Board: 
			case ModelPart::Breadboard: 
			case ModelPart::ResizableBoard: 
			case ModelPart::Note: 
			case ModelPart::Ruler: 
				continue;
			default:
				break;
		}

		if (itemBase->hidden()) continue;
		if (!itemBase->isVisible()) continue;
		if (itemBase == m_board) continue;
		if (itemBase->layerKinChief() == m_board) continue;
		if (!itemBase->layerKinChief()->hasConnectors()) continue;

		switch (itemBase->viewLayerID()) {
			case ViewLayer::Silkscreen0:
			case ViewLayer::Silkscreen1:
				break;
			default:
				continue;
		}

		QRectF partRect = itemBase->boundingRect();
		partRect.moveTo(itemBase->pos());
		TileRect partTileRect;
		realsToTile(partTileRect, partRect.left(), partRect.top(), partRect.right(), partRect.bottom());
		if (partTileRect.xmini < m_tileMaxRect.xmini) partTileRect.xmini = m_tileMaxRect.xmini;
		if (partTileRect.xmaxi > m_tileMaxRect.xmaxi) partTileRect.xmaxi = m_tileMaxRect.xmaxi;
		if (partTileRect.ymini < m_tileMaxRect.ymini) partTileRect.ymini = m_tileMaxRect.ymini;
		if (partTileRect.ymaxi > m_tileMaxRect.ymaxi) partTileRect.ymaxi = m_tileMaxRect.ymaxi;
		insertUnion(partTileRect, NULL, Tile::OBSTACLE);
	}
}

bool CMRouter::addJumperItem(PriorityQueue<PathUnit *> & p1, PriorityQueue<PathUnit *> & p2, JEdge * edge, 
							QMultiHash<Tile *, PathUnit *> & tilePathUnits, qreal keepout)
{
	QSizeF sizeNeeded(m_sketchWidget->jumperItemSize().width(), m_sketchWidget->jumperItemSize().height());
	int tWidthNeeded = realToTile(sizeNeeded.width());
	int tHeightNeeded = realToTile(sizeNeeded.height());

	TileRect nearestSpace1;
	PathUnit * nearest1 = findNearestSpace(p1, tilePathUnits, tWidthNeeded, tHeightNeeded, nearestSpace1);
	if (nearest1 == NULL) return false;

	TileRect nearestSpace2;
	PathUnit * nearest2 = findNearestSpace(p2, tilePathUnits, tWidthNeeded, tHeightNeeded, nearestSpace2);
	if (nearest2 == NULL) return false;

	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	ItemBase * itemBase = m_sketchWidget->addItem(m_sketchWidget->paletteModel()->retrieveModelPart(ModuleIDNames::jumperModuleIDName), 
												  m_specHash.value(nearest1->plane), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
	if (itemBase == NULL) {
		return NULL;
	}

	JumperItem * jumperItem = dynamic_cast<JumperItem *>(itemBase);
	m_sketchWidget->scene()->addItem(jumperItem);
	QPointF destPoint1 = calcJumperLocation(nearest1, nearestSpace1, tWidthNeeded, tHeightNeeded);
	QPointF destPoint2 = calcJumperLocation(nearest2, nearestSpace2, tWidthNeeded, tHeightNeeded);
	jumperItem->resize(destPoint1, destPoint2);

	ProcessEventBlocker::processEvents();

	PathUnit * parent1 = nearest1;
	while (parent1->parent) {
		parent1 = parent1->parent;
	}
	TileRect tileRect1;
	TiToRect(parent1->tile, &tileRect1);
	PathUnit * parent2 = nearest2;
	while (parent2->parent) {
		parent2 = parent2->parent;
	}
	TileRect tileRect2;
	TiToRect(parent2->tile, &tileRect2);

	bool result = addJumperItemHalf(jumperItem->connector0(), nearest1, parent1, 
									(tileRect1.xmaxi + tileRect1.xmini) / 2, (tileRect1.ymaxi + tileRect1.ymini) / 2, 
									edge, tilePathUnits, keepout);
	result = addJumperItemHalf(jumperItem->connector1(), nearest2, parent2, 
							   (tileRect2.xmaxi + tileRect2.xmini) / 2, (tileRect2.ymaxi + tileRect2.ymini) / 2, 
							   edge, tilePathUnits, keepout);

	return result;
}

bool CMRouter::addJumperItemHalf(ConnectorItem * jumperConnectorItem, PathUnit * nearest, PathUnit * parent, int searchx, int searchy, JEdge * edge,
								 QMultiHash<Tile *, PathUnit *> & tilePathUnits, qreal keepout)
{
	QList<Tile *> alreadyTiled;
	Tile * destTile = addTile(jumperConnectorItem, Tile::DESTINATION, nearest->plane, alreadyTiled, CMRouter::IgnoreAllOverlaps, keepout);

	clearEdge(edge);
	if (parent->connectorItem) {
		edge->fromConnectorItems.append(parent->connectorItem);
	}
	else {
		edge->fromTraces.insert(parent->wire);
	}
	edge->toConnectorItems.append(jumperConnectorItem);

	// have to search; can't rely on parent->tile to be valid once the jumper tile is inserted
	Tile * sourceTile = TiSrPoint(NULL, nearest->plane, searchx, searchy);
	TiSetType(sourceTile, Tile::SOURCE);

	PriorityQueue<PathUnit *> q1, q2;
	initPathUnit(edge, sourceTile, q1, tilePathUnits);
	initPathUnit(edge, destTile, q2, tilePathUnits);

	return propagate(q1, q2, tilePathUnits, keepout);
}

QPointF CMRouter::calcJumperLocation(PathUnit * pathUnit, TileRect & nearestSpace, int tWidthNeeded, int tHeightNeeded) 
{
	int cx = (pathUnit->minCostRect.xmaxi + pathUnit->minCostRect.xmini) / 2;
	int cy = (pathUnit->minCostRect.ymaxi + pathUnit->minCostRect.ymini) / 2;
	TileRect wantRect;
	wantRect.xmini = cx - (tWidthNeeded / 2);
	wantRect.xmaxi = wantRect.xmini + tWidthNeeded;
	wantRect.ymini = cy - (tHeightNeeded / 2);
	wantRect.ymaxi = wantRect.ymini + tHeightNeeded;
	if (wantRect.xmini < nearestSpace.xmini) {
		wantRect.xmini = nearestSpace.xmini;
		wantRect.xmaxi = nearestSpace.xmini + tWidthNeeded;
	}
	else if (wantRect.xmaxi > nearestSpace.xmaxi) {
		wantRect.xmaxi = nearestSpace.xmaxi;
		wantRect.xmini = nearestSpace.xmaxi - tWidthNeeded;
	}
	if (wantRect.ymini < nearestSpace.ymini) {
		wantRect.ymini = nearestSpace.ymini;
		wantRect.ymaxi = nearestSpace.ymini + tHeightNeeded;
	}
	else if (wantRect.ymaxi > nearestSpace.ymaxi) {
		wantRect.ymaxi = nearestSpace.ymaxi;
		wantRect.ymini = nearestSpace.ymaxi - tHeightNeeded;
	}

	return QPointF(tileToReal(wantRect.xmini + wantRect.xmaxi) / 2, tileToReal(wantRect.ymini + wantRect.ymaxi) / 2);
}

bool CMRouter::propagate(PriorityQueue<PathUnit *> & p1, PriorityQueue<PathUnit *> & p2, 
						 QMultiHash<Tile *, PathUnit *> & tilePathUnits, qreal keepout)
{

	if (p1.count() == 0) return false;
	if (p2.count() == 0) return false;

	QList<Wire *> wires;

	QList<PathUnit *> p1Terminals;
	QList<PathUnit *> p2Terminals;

	bool firstTime = true;
	for (int i = p1.count() - 1; i >= 0; i--) {
		PathUnit * p1PathUnit = p1.at(i);
		p1Terminals.append(p1PathUnit);
		p1PathUnit->destCost = std::numeric_limits<int>::max();
		int keepj = -1;
		for (int j = p2.count() - 1; j >= 0; j--) {
			PathUnit * p2PathUnit = p2.at(j);
			if (firstTime) p2Terminals.append(p2PathUnit);
			p2PathUnit->destCost = std::numeric_limits<int>::max();
			int d = manhattan(p1PathUnit->minCostRect, p2PathUnit->minCostRect);
			if (d < p1PathUnit->destCost) {
				p1PathUnit->destCost = d;
				keepj = j;
			}
		}
		firstTime = false;
		p2.at(keepj)->destCost = p1PathUnit->destCost;
		p2.resetPriority(keepj, p1PathUnit->destCost);
		p1.resetPriority(i, p1PathUnit->destCost);
	}
	p1.sort();
	p2.sort();


	//QElapsedTimer propagateUnitTimer;

	CompletePath completePath;
	completePath.source = completePath.dest = NULL;
	bool success = false;
	while (p1.count() > 0 && p2.count() > 0) {
		PathUnit * pathUnit1 = p1.dequeue();
		PathUnit * pathUnit2 = p2.dequeue();

		if (success && completePath.sourceCost < pathUnit1->sourceCost + pathUnit2->sourceCost) {
			// we won't find anything better than the current solution, so bail
			break;
		}

		//propagateUnitTimer.start();
		bool ok = propagateUnit(pathUnit1, p1, p2, p2Terminals, tilePathUnits, completePath);
		//propagateUnitTime += propagateUnitTimer.elapsed();
		if (ok) {
			success = true;
			if (completePath.goodEnough) break;
		}

		//propagateUnitTimer.start();
		ok = propagateUnit(pathUnit2, p2, p1, p1Terminals, tilePathUnits, completePath);
		//propagateUnitTime += propagateUnitTimer.elapsed();
		if (ok) {
			success = true;
			if (completePath.goodEnough) break;
		}

		ProcessEventBlocker::processEvents();
		if (m_cancelTrace || m_stopTracing || m_cancelled) {
			return false;
		}
	}


	if (success) {
		tracePath(completePath, keepout);
	}

	foreach (Plane * plane, m_planes) {
		if (m_sketchWidget->autorouteTypePCB()) {
			TiSrArea(NULL, plane, &m_tileMaxRect, clearSourceAndDestination, NULL);
		}
		else {
			TiSrArea(NULL, plane, &m_tileMaxRect, clearSourceAndDestination2, NULL);
		}
	}

	hideTiles();

	return success;
}

bool CMRouter::propagateUnit(PathUnit * pathUnit, PriorityQueue<PathUnit *> & sourceQueue, 
							PriorityQueue<PathUnit *> & destQueue, QList<PathUnit *> & destTerminals, 
							QMultiHash<Tile *, PathUnit *> & tilePathUnits, CompletePath & completePath)
{
	bool result = false;
	QList<Tile *> tiles;
	//QElapsedTimer seedNextTimer;
	//seedNextTimer.start();
	seedNext(pathUnit, tiles, tilePathUnits);
	//seedNextTime += seedNextTimer.elapsed();
	foreach (Tile * tile, tiles) {
		int destCost = std::numeric_limits<int>::max();
		TileRect minCostRect = calcMinCostRect(pathUnit, tile);
		int sourceCost = pathUnit->sourceCost + manhattan(pathUnit->minCostRect, minCostRect);	
		bool redundantTile = false;
		QList<PathUnit *> redundantPathUnits;
		foreach (PathUnit * tpu, tilePathUnits.values(tile)) {
			if (tpu->priorityQueue == &sourceQueue) {
				if (!isRedundantPath(tpu, minCostRect, sourceCost)) continue;
				
				if (sourceCost >= tpu->sourceCost) {
					redundantTile = true;
					break;
				}
				else {
					redundantPathUnits.append(tpu);
				}
			}
		}

		foreach(PathUnit * rpu, redundantPathUnits) {
			tilePathUnits.remove(tile, rpu);
			sourceQueue.removeOne(rpu);
		}

		if (redundantTile) {
			continue;
		}
		
		QList<PathUnit *> goals;
		foreach (PathUnit * tpu, tilePathUnits.values(tile)) {
			if (tpu->priorityQueue == &destQueue) {
				goals.append(tpu);
			}
		}
		foreach (PathUnit * destTerminal, destTerminals) {
			destCost = qMin(manhattan(destTerminal->minCostRect, minCostRect), destCost);
		}

		PathUnit * nextPathUnit = new PathUnit(&sourceQueue);
		m_pathUnits.append(nextPathUnit);
		nextPathUnit->sourceCost = sourceCost;
		nextPathUnit->destCost = destCost;
		nextPathUnit->minCostRect = minCostRect;
		nextPathUnit->parent = pathUnit;
		nextPathUnit->edge = pathUnit->edge;
		nextPathUnit->plane = pathUnit->plane;
		nextPathUnit->tile = tile;
		tilePathUnits.insert(tile, nextPathUnit);
		//infoTile("tilepathunits insert", tile);
		sourceQueue.enqueue(sourceCost + destCost, nextPathUnit);
		if (goals.count() > 0) {
			PathUnit * bestGoal = NULL;
			int bestCost;
			foreach (PathUnit * goalUnit, goals) {
				if (bestGoal == NULL) {
					bestGoal = goalUnit;
					bestCost = manhattan(goalUnit->minCostRect, nextPathUnit->minCostRect) + goalUnit->sourceCost + nextPathUnit->sourceCost;
				}
				else {
					int cost = manhattan(goalUnit->minCostRect, nextPathUnit->minCostRect)  + goalUnit->sourceCost + nextPathUnit->sourceCost;
					if (cost < bestCost) {
						bestCost = cost;
						bestGoal = goalUnit;
					}
				}
			}
			if (completePath.source == NULL || completePath.sourceCost > bestCost) {
				result = true;
				completePath.source = nextPathUnit;
				completePath.dest = bestGoal;
				completePath.sourceCost = bestCost;
				completePath.goodEnough = goodEnough(completePath);
				if (completePath.goodEnough) break;
			}
		}
	}

	return result;
}


TileRect CMRouter::calcMinCostRect(PathUnit * pathUnit, Tile * next)
{
	//infoTile("pathunit", pathUnit->tile);
	//infoTileRect("    pmcr", pathUnit->minCostRect);
	//infoTile("    next", next);

	TileRect nextRect;
	TiToRect(next, &nextRect);
	return calcMinCostRect(pathUnit, nextRect);
}

TileRect CMRouter::calcMinCostRect(PathUnit * pathUnit, TileRect & next)
{
	TileRect nextMinCostRect;
	if (pathUnit->minCostRect.xmaxi <= next.xmini) {
		nextMinCostRect.xmini = next.xmini;
		nextMinCostRect.xmaxi = qMin(next.xmini + TileStandardWireWidth, next.xmaxi);
	}
	else if (pathUnit->minCostRect.xmini >= next.xmaxi) {
		nextMinCostRect.xmaxi = next.xmaxi;
		nextMinCostRect.xmini = qMax(next.xmaxi - TileStandardWireWidth, next.xmini);
	}
	else {
		nextMinCostRect.xmini = qMax(next.xmini, pathUnit->minCostRect.xmini);
		nextMinCostRect.xmaxi = qMin(next.xmaxi, pathUnit->minCostRect.xmaxi);
	}

	if (pathUnit->minCostRect.ymaxi <= next.ymini) {
		nextMinCostRect.ymini = next.ymini;
		nextMinCostRect.ymaxi = qMin(next.ymini + TileStandardWireWidth, next.ymaxi);
	}
	else if (pathUnit->minCostRect.ymini >= next.ymaxi) {
		nextMinCostRect.ymaxi = next.ymaxi;
		nextMinCostRect.ymini = qMax(next.ymaxi - TileStandardWireWidth, next.ymini);
	}
	else {
		nextMinCostRect.ymini = qMax(next.ymini, pathUnit->minCostRect.ymini);
		nextMinCostRect.ymaxi = qMin(next.ymaxi, pathUnit->minCostRect.ymaxi);
	}

	//infoTileRect("     mcr", nextMinCostRect);

	return nextMinCostRect;
}

bool CMRouter::isRedundantPath(PathUnit * pathUnit, TileRect & minCostRect, int sourceCost)
{
	// eliminate redundant (not min cost) paths using "cost cones"

	// TODO: slopes are now assuming equal horizontal and vertical cost

	// first find the point of each cone
	QLineF minx1(minCostRect.xmini, sourceCost, minCostRect.xmini - 1000, sourceCost + 1000);
	QLineF maxx1(minCostRect.xmaxi, sourceCost, minCostRect.xmaxi + 1000, sourceCost + 1000);
	QPointF x1;
	minx1.intersect(maxx1, &x1);

	QLineF minx2(pathUnit->minCostRect.xmini, pathUnit->sourceCost, pathUnit->minCostRect.xmini - 1000, pathUnit->sourceCost + 1000);
	QLineF maxx2(pathUnit->minCostRect.xmaxi, pathUnit->sourceCost, pathUnit->minCostRect.xmaxi + 1000, pathUnit->sourceCost + 1000);
	QPointF x2;
	minx2.intersect(maxx2, &x2);

	if (x1.y() < x2.y()) {
		if (!insideV(x2, x1)) return false;
	}
	else if (x1.y() > x2.y()) {
		if (!insideV(x1, x2)) return false;
	}
	// else equal vertices is redundant

	QLineF miny1(minCostRect.ymini, sourceCost, minCostRect.ymini - 1000, sourceCost + 1000);
	QLineF maxy1(minCostRect.ymaxi, sourceCost, minCostRect.ymaxi + 1000, sourceCost + 1000);
	QPointF y1;
	miny1.intersect(maxy1, &y1);

	QLineF miny2(pathUnit->minCostRect.ymini, pathUnit->sourceCost, pathUnit->minCostRect.ymini - 1000, pathUnit->sourceCost + 1000);
	QLineF maxy2(pathUnit->minCostRect.ymaxi, pathUnit->sourceCost, pathUnit->minCostRect.ymaxi + 1000, pathUnit->sourceCost + 1000);
	QPointF y2;
	miny2.intersect(maxy2, &y2);

	if (y1.y() < y2.y()) {
		if (!insideV(y2, y1)) return false;
	}
	else if (y1.y() > y2.y()) {
		if (!insideV(y1, y2)) return false;
	}
	// else equal vertices is redundant

	return true;
}

bool CMRouter::insideV(const QPointF & check, const QPointF & vertex)
{
	// form the V from p2

	QPointF lv(vertex.x() - 10 + vertex.y() - check.y(), check.y() + 10);
	QPointF rv(vertex.x() + 10 - vertex.y() + check.y(), check.y() + 10);

	// the rest of this from: http://www.blackpawn.com/texts/pointinpoly/default.html

	QPointF v0 = rv - vertex;
	QPointF v1 = lv - vertex;
	QPointF v2 = check - vertex;

	// Compute dot products
	qreal dot00 = dot(v0, v0);
	qreal dot01 = dot(v0, v1);
	qreal dot02 = dot(v0, v2);
	qreal dot11 = dot(v1, v1);
	qreal dot12 = dot(v1, v2);

	// Compute barycentric coordinates
	qreal invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
	qreal u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	qreal v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	// Check if point is in or on triangle
	return (u >= 0) && (v >= 0) && (u + v <= 1);
}

bool CMRouter::goodEnough(CompletePath & completePath) {
	PathUnit * sourcePathUnit = completePath.source;
	PathUnit * destPathUnit = completePath.dest;
	for ( ; sourcePathUnit->parent; sourcePathUnit = sourcePathUnit->parent);
	for ( ; destPathUnit->parent; destPathUnit = destPathUnit->parent);
	int bestCost = manhattan(sourcePathUnit->minCostRect, destPathUnit->minCostRect);
	return completePath.sourceCost < ((qreal) bestCost) * 1.05;
}

void CMRouter::tracePath(CompletePath & completePath, qreal keepout)
{
	QList<PathUnit *> fullPath;
	for (PathUnit * spu = completePath.source; spu; spu = spu->parent) {
		fullPath.push_front(spu);
	}
	fullPath.takeLast();			// this tile is redundant so pop it from the list
	for (PathUnit * dpu = completePath.dest; dpu; dpu = dpu->parent) {
		fullPath.append(dpu);
	}

	//foreach (PathUnit * pathUnit, fullPath) {
		//infoTile("tracepath", pathUnit->tile);
	//}

	QList<Segment *> hSegments;
	QList<Segment *> vSegments;
	foreach (PathUnit * pathUnit, fullPath) {
		Segment * h = new Segment;
		h->sEntry = h->sExit = Segment::NotSet;
		h->sMin = LEFT(pathUnit->tile);
		h->sMax = RIGHT(pathUnit->tile);
		Segment * v = new Segment;
		v->sEntry = v->sExit = Segment::NotSet;
		v->sMin = YMIN(pathUnit->tile);
		v->sMax = YMAX(pathUnit->tile);
		hSegments.append(h);
		vSegments.append(v);
	}

	initConnectorSegments(0, fullPath, hSegments, vSegments);
	initConnectorSegments(fullPath.count() - 1, fullPath, hSegments, vSegments);

	traceSegments(hSegments);
	traceSegments(vSegments);

	QList<QPointF> allPoints;
	for (int i = 0; i < hSegments.count(); i++) {
		Segment * h = hSegments.at(i);
		Segment * v = vSegments.at(i);
		allPoints.append(QPointF(tileToReal(h->sEntry), tileToReal(v->sEntry)));
		allPoints.append(QPointF(tileToReal(h->sExit), tileToReal(v->sExit)));
	}
	allPoints.takeFirst();
	allPoints.takeLast();

	foreach (Segment * segment, hSegments) {
		delete segment;
	}
	foreach (Segment * segment, vSegments) {
		delete segment;
	}

	PathUnit * first = fullPath.first();
	PathUnit * last = fullPath.last();

	if (first->connectorItem == NULL) {
		bool atEndpoint;
		double distance, dx, dy;
		QPointF p = first->wire->pos();
		QPointF pp = first->wire->line().p2() + p;
		QPointF ppp = allPoints.first();
		GraphicsUtils::distanceFromLine(ppp.x(), ppp.y(), p.x(), p.y(), pp.x(), pp.y(), dx, dy, distance, atEndpoint);
		allPoints.push_front(QPointF(dx, dy));
	}
	if (last->connectorItem == NULL) {
		bool atEndpoint;
		double distance, dx, dy;
		QPointF p = last->wire->pos();
		QPointF pp = last->wire->line().p2() + p;
		QPointF ppp = allPoints.last();
		GraphicsUtils::distanceFromLine(ppp.x(), ppp.y(), p.x(), p.y(), pp.x(), pp.y(), dx, dy, distance, atEndpoint);
		allPoints.append(QPointF(dx, dy));
	}

	cleanPoints(allPoints, first->plane);

	QList<Wire *> wires;
	for (int i = 0; i < allPoints.count() - 1; i++) {
		QPointF p1 = allPoints.at(i);
		QPointF p2 = allPoints.at(i + 1);
		Wire * trace = drawOneTrace(p1, p2, Wire::STANDARD_TRACE_WIDTH, m_specHash.value(first->plane));
		ProcessEventBlocker::processEvents();
		wires.append(trace);
	}

	// TODO: make sure that splitTrace succeeds if trace was not autoroutable

	if (first->connectorItem == NULL) {
		first->connectorItem = splitTrace(first->wire, allPoints.first());
		Wire * split = qobject_cast<Wire *>(first->connectorItem->attachedTo());
		m_tracesToEdges.insert(split, first->edge);
	}
	if (last->connectorItem == NULL) {
		last->connectorItem = splitTrace(last->wire, allPoints.last());
		Wire * split = qobject_cast<Wire *>(last->connectorItem->attachedTo());
		m_tracesToEdges.insert(split, first->edge);
	}

	ProcessEventBlocker::processEvents();

	// hook everyone up
	hookUpWires(fullPath, wires, keepout);
	foreach (Wire * wire, wires) {
		m_tracesToEdges.insert(wire, first->edge);
	}
}

void CMRouter::cleanPoints(QList<QPointF> & allPoints, Plane * thePlane) 
{
	foreach (QPointF p, allPoints) {
		DebugDialog::debug("allpoint before:", p);
	}

	// remove redundant pairs
	int ix = allPoints.count() - 1;
	while (ix > 0) {
		if (allPoints[ix] == allPoints[ix - 1]) {
			allPoints.removeAt(ix);
		}
		ix--;
	}

	// make sure all pairs have 90 degree turns
	ix = 0;
	while (ix < allPoints.count() - 1) {
		QPointF p1 = allPoints[ix];
		QPointF p2 = allPoints[ix + 1];
		if (qAbs(p1.x() - p2.x()) >= CloseEnough && qAbs(p1.y() - p2.y()) >= CloseEnough) {
			// insert another point
			QPointF p3(p2.x(), p1.y());   // the other corner may be better
			allPoints.insert(ix + 1, p3);
		}
		ix++;
	}

	// eliminate redundant colinear points
	ix = 0;
	while (ix < allPoints.count() - 2) {
		QPointF p1 = allPoints[ix];
		QPointF p2 = allPoints[ix + 1];
		QPointF p3 = allPoints[ix + 2];
		if (p1.x() == p2.x() && p2.x() == p3.x()) {
			allPoints.removeAt(ix + 1);
			ix--;
		}
		else if (p1.y() == p2.y() && p2.y() == p3.y()) {
			allPoints.removeAt(ix + 1);
			ix--;
		}
		ix++;
	}


	foreach (QPointF p, allPoints) {
		DebugDialog::debug("allpoint before rc:", p);
	}

	removeCorners(allPoints, thePlane);
	//shortenUs(allPoints, subedge);

	foreach (QPointF p, allPoints) {
		DebugDialog::debug("allpoint after:", p);
	}
}

void CMRouter::initConnectorSegments(int ix0, QList<PathUnit *> & fullPath, QList<Segment *> & hSegments, QList<Segment *> & vSegments) 
{
	PathUnit * pathUnit = fullPath.at(ix0);
	if (pathUnit->connectorItem == NULL) return;

	Segment * h = hSegments.at(ix0);
	Segment * v = vSegments.at(ix0);
	h->sMin = pathUnit->minCostRect.xmini;
	h->sEntry = h->sMin + TileHalfStandardWireWidth;
	h->sMax = pathUnit->minCostRect.xmaxi;
	h->sExit = h->sMax - TileHalfStandardWireWidth;

	v->sMin = pathUnit->minCostRect.ymini;
	v->sEntry = v->sMin + TileHalfStandardWireWidth;
	v->sMax = pathUnit->minCostRect.ymaxi;
	v->sExit = v->sMax - TileHalfStandardWireWidth;
}

void CMRouter::traceSegments(QList<Segment *> & segments) {
	//foreach(Segment * segment, segments) {
		//DebugDialog::debug(QString("segment %1 %2 %3 %4").arg(segment->sMin).arg(segment->sMax).arg(segment->sEntry).arg(segment->sExit));
	//}

	// use non-overlaps to set entry and exit
	for (int ix = 0; ix < segments.count(); ix++) {
		Segment * from = segments.at(ix);
		if (ix > 0 && from->sEntry == Segment::NotSet) {
			// from->sEntry not set
			Segment * prev = segments.at(ix - 1);
			if (prev->sExit > from->sMax - TileHalfStandardWireWidth) {
				from->setEntry(from->sMax - TileHalfStandardWireWidth);
			}
			else if (prev->sExit < from->sMin + TileHalfStandardWireWidth) {
				from->setEntry(from->sMin + TileHalfStandardWireWidth);
			}
			else {
				from->setEntry(prev->sExit);
			}
		}

		if (from->sEntry != Segment::NotSet) {
			if (ix < segments.count() - 1) {
				Segment * to = segments.at(ix + 1);
				if (to->sMin <= from->sEntry - TileHalfStandardWireWidth && to->sMax >= from->sEntry + TileHalfStandardWireWidth) {
					// can enter and exit at the same place in the current segment and put the problem off to the next segment
					to->setEntry(from->sEntry);
					from->setExit(from->sEntry);
					continue;
				}
			}
		}

		// if no non-overlaps, use limitMin and limitMax
		bool gotNonOverlap = false;
		int limitMin = from->sMin;			
		int limitMax = from->sMax;
		for (int jx = ix + 1; jx < segments.count(); jx++) {
			Segment * to = segments.at(jx);
			int entryMin = qMax(from->sMin, to->sMin);
			int entryMax = qMin(from->sMax, to->sMax);
			if (entryMax - entryMin >= TileStandardWireWidth) {
				limitMin = qMin(qMax(limitMin, to->sMin), limitMax - TileStandardWireWidth);
				limitMax = qMax(qMin(limitMax, to->sMax), limitMin + TileStandardWireWidth);
				continue;
			}

			gotNonOverlap = true;
			//int clipMin;
			//int clipMax;
			if (from->sMax < to->sMax) {
				from->setExit(from->sMax - TileHalfStandardWireWidth);
				to->setEntry(to->sMin + TileHalfStandardWireWidth);
				//clipMin = from->sMax - TileStandardWireWidth;
				//clipMax = to->sMin + TileStandardWireWidth;
			}
			else {
				from->setExit(from->sMin + TileHalfStandardWireWidth);
				to->setEntry(to->sMax - TileHalfStandardWireWidth);
				//clipMax = from->sMin + TileStandardWireWidth;
				//clipMin = to->sMax - TileStandardWireWidth;
			}
			/*
			for (int kx = ix + 1; kx < jx; kx++) {
				Segment * btween = segments.at(kx);
				if (clipMax < btween->sMax) {
					btween->sMax = clipMax;
				}
				if (clipMin > btween->sMin) {
					btween->sMin = clipMin;
				}
				if (btween->sMin >= btween->sMax) {
					DebugDialog::debug("clipping failure");
				}
			}
			*/

			break;
		}

		if (!gotNonOverlap) {
			if (from->sEntry == Segment::NotSet) {
				from->setEntry(limitMin + TileHalfStandardWireWidth);
			}
			if (qAbs(from->sEntry - limitMin) <= qAbs(from->sEntry - limitMax)) {
				from->setExit(limitMin + TileHalfStandardWireWidth);
			}
			else {
				from->setExit(limitMax - TileHalfStandardWireWidth);
			}
		}
	}


#ifndef QT_NO_DEBUG
	for (int ix = 0; ix < segments.count(); ix++) {
		Segment * segment = segments.at(ix);
		//DebugDialog::debug(QString("final segment %1 %2 %3 %4").arg(segment->sMin).arg(segment->sMax).arg(segment->sEntry).arg(segment->sExit));
		if (ix > 0 && segment->sEntry == Segment::NotSet) {
			DebugDialog::debug("segment failure");
		}
		if (ix < segments.count() - 1 && segment->sExit == Segment::NotSet) {
			DebugDialog::debug("segment failure");
		}
	}
#endif QT_NO_DEBUG

}

void CMRouter::expand(ConnectorItem * originalConnectorItem, QList<ConnectorItem *> & connectorItems, QSet<Wire *> & visited) 
{
	connectorItems.append(originalConnectorItem);
	ConnectorItem::collectEqualPotential(connectorItems, m_bothSidesNow, ViewGeometry::RatsnestFlag | ViewGeometry::NormalFlag);
	foreach (ConnectorItem * connectorItem, connectorItems) {
		TraceWire * traceWire = TraceWire::getTrace(connectorItem);
		if (traceWire) visited.insert(traceWire);
	}
}

void CMRouter::insertUnion(TileRect & tileRect, QGraphicsItem *, Tile::TileType tileType) {
	if (m_unionPlane == NULL) return;
	if (tileType == Tile::SPACE) return;
	if (tileType == Tile::SPACE2) return;
			
	TiInsertTile(m_unionPlane, &tileRect, NULL, Tile::OBSTACLE);
	//infoTileRect("union", tileRect);

	TileRect tileRect90;
	tileRotate90(tileRect, tileRect90);
	TiInsertTile(m_union90Plane, &tileRect90, NULL, Tile::OBSTACLE);
}

void CMRouter::clearTracesAndJumpers() {
	QList<JumperItem *> jumperItems;
	QList<TraceWire *> traceWires;

	foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
		JumperItem * jumperItem = dynamic_cast<JumperItem *>(item);
		if (jumperItem != NULL) {
			jumperItems.append(jumperItem);
			continue;
		}
		TraceWire * traceWire = dynamic_cast<TraceWire *>(item);
		if (traceWire != NULL) {
			traceWires.append(traceWire);
			continue;
		}
	}

	foreach (Wire * traceWire, traceWires) {
		m_sketchWidget->deleteItem(traceWire, true, false, false);
	}
	foreach (JumperItem * jumperItem, jumperItems) {
		m_sketchWidget->deleteItem(jumperItem, true, true, false);
	}
}

void CMRouter::saveTracesAndJumpers(QByteArray & byteArray) {
	QList<ItemBase *> itemBases;
	foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
		JumperItem * jumperItem = dynamic_cast<JumperItem *>(item);
		if (jumperItem != NULL) {
			itemBases.append(jumperItem);
			continue;
		}
		TraceWire * traceWire = dynamic_cast<TraceWire *>(item);
		if (traceWire != NULL) {
			itemBases.append(traceWire);
			continue;
		}
	}

	QList<long> modelIndexes;
	m_sketchWidget->copyHeart(itemBases, true, byteArray, modelIndexes);
}

void CMRouter::setMaxCycles(int maxCycles) 
{
	m_maxCycles = maxCycles;
	QSettings settings;
	settings.setValue("cmrouter/maxcycles", maxCycles);
}
