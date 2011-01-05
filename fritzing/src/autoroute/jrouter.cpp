/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de\

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

// TODO:
//
//	wire bendpoint is not a blocker if wire is ownside
//
//	make DRC available from trace menu
//
//	schematic view: blocks parts, not traces
//
//	fix up cancel/stop: 
//		stop either stops you where you are, 
//		or goes back to the best outcome, if you're in ripup-and-reroute phase
//
//	placing jumpers: if double-sided, must make sure there is room on both sides
//	space finder needs to be tweaked?
//
//	use tile to place vias
//		tile both sides
//		for each onside tile
//			do jumper search, but for only one connector
//				when such tile is found, do normal trace search on the other side from via to connector
//
//	option to turn off propagation feedback
//	remove debugging output and extra calls to processEvents
//
//	bugs: 
//		loop: longer route
//		arduino isp: arduino too close to the edge? fails an early easy route in the backtrace
//		arduino no ftdi (barebones arduino): chips too close to the edge? fails easy routes in the backtrace; weird short traces; one off-center trace
//		blink: arduino overlaps?
//
//	need to put a border no-go area around the board
//
//	better approach for starting/ending on trace rather than connector
//
//  longer route than expected: 
//		This is because outward propagation stops when the goal is first reached.  
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
//	redo tiling non-manhattan wires
//
//	still seeing a few thin tiles going across the board: 
//		this is because the thick tiles above and below are wider than the thin tile
//
//	rip up and reroute:
//		keep a hash table from traces to edges
//		when edges are generated give each an integer ID
//		when first pass at routing is over and there are unrouted edges
//		save the traces (how) along with some score (number of open edges)
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


#include "jrouter.h"
#include "../sketch/pcbsketchwidget.h"
#include "../debugdialog.h"
#include "../items/virtualwire.h"
#include "../items/tracewire.h"
#include "../items/jumperitem.h"
#include "../items/resizableboard.h"
#include "../utils/graphicsutils.h"
#include "../utils/textutils.h"
#include "../connectors/connectoritem.h"
#include "../items/moduleidnames.h"
#include "../processeventblocker.h"
#include "../svg/groundplanegenerator.h"
#include "../fsvgrenderer.h"

#include "tile.h"

#include <qmath.h>
#include <QApplication>
#include <limits>
#include <QMessageBox>

static const int MaximumProgress = 1000;
static qreal KeepoutSpace = 0;
static const qreal TINYSPACEMAX = 10;

enum TileType {
	BUFFER = 1,
	NOTBOARD,
	NONCONNECTOR,
	TRACE,
	CONNECTOR,
	PART,
	SPACE,
	TINYSPACE
};

bool edgeLessThan(JEdge * e1, JEdge * e2)
{
	if (e1->viewLayerSpec != e2->viewLayerSpec) {
		// do bottom edges first
		return e1->viewLayerSpec == ViewLayer::Bottom;
	}

	return e1->distance < e2->distance;
}

bool subedgeLessThan(JSubedge * se1, JSubedge * se2)
{
	return se1->distance < se2->distance;
}

inline int realToTile(qreal x) {
	return qRound(x * 100);
}

void realsToTile(TileRect & tileRect, qreal l, qreal t, qreal r, qreal b) {
	tileRect.xmini = realToTile(l);
	tileRect.ymini = realToTile(t);
	tileRect.xmaxi = realToTile(r);
	tileRect.ymaxi = realToTile(b);
}

inline qreal tileToReal(int x) {
	return x / 100.0;
}

void tileToQRect(Tile * tile, QRectF & rect) {
	TileRect tileRect;
	TiToRect(tile, &tileRect);
	rect.setCoords(tileToReal(tileRect.xmini), tileToReal(tileRect.ymini), tileToReal(tileRect.xmaxi), tileToReal(tileRect.ymaxi));
}

bool tileRectIntersects(TileRect * tile1, TileRect * tile2)
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

QSet<Tile *> ChangedTiles;

static void CollectChangedTiles(Tile * tile) {
	ChangedTiles.insert(tile);
}

static void RemoveChangedTiles(Tile * tile) {
	ChangedTiles.remove(tile);
}

////////////////////////////////////////////////////////////////////

GridEntry::GridEntry(qreal x, qreal y, qreal w, qreal h, int type, QGraphicsItem * parent) : QGraphicsRectItem(x, y, w, h, parent)
{
	setAcceptedMouseButtons(Qt::NoButton);
	setAcceptsHoverEvents(false);
	m_type = type;
}

////////////////////////////////////////////////////////////////////

JRouter::JRouter(PCBSketchWidget * sketchWidget)
{
	KeepoutSpace = 0.015 * FSvgRenderer::printerScale();			// 15 mils space
	m_sketchWidget = sketchWidget;
	m_stopTrace = m_cancelTrace = m_cancelled = false;
	m_wireWidthNeeded = KeepoutSpace + KeepoutSpace + Wire::STANDARD_TRACE_WIDTH;
	m_halfWireWidthNeeded = m_wireWidthNeeded / 2;
}

JRouter::~JRouter()
{
}

void JRouter::cancel() {
	m_cancelled = true;
}

void JRouter::cancelTrace() {
	m_cancelTrace = true;
}

void JRouter::stopTrace() {
	m_stopTrace = true;
}

void JRouter::start()
{	
	m_maximumProgressPart = 2;
	m_currentProgressPart = 0;

	emit setMaximumProgress(MaximumProgress);

	RoutingStatus routingStatus;
	routingStatus.zero();

	m_sketchWidget->ensureTraceLayersVisible();

	clearGridEntries();

	QUndoCommand * parentCommand = new QUndoCommand("Autoroute");
	new CleanUpWiresCommand(m_sketchWidget, CleanUpWiresCommand::UndoOnly, parentCommand);

	m_bothSidesNow = m_sketchWidget->routeBothSides();
	if (m_bothSidesNow) {
		m_maximumProgressPart = 3;
		emit wantBottomVisible();
		ProcessEventBlocker::processEvents();
	}

	clearTraces(m_sketchWidget, false, parentCommand);
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

	if (m_cancelled || m_stopTrace) {
		restoreOriginalState(parentCommand);
		cleanUp();
		return;
	}

	ProcessEventBlocker::processEvents(); // to keep the app  from freezing

	ItemBase * board = NULL;
	if (m_sketchWidget->autorouteNeedsBounds()) {
		board = m_sketchWidget->findBoard();
	}

	QList<JEdge *> edges;
	QList<Plane *> planes;
	bool allDone = false;
	QList< QList<int> > orderings;
	QHash<Wire *, JEdge *> tracesToEdges;
	for (int run = 0; run < 10; run++) {
		allDone = runEdges(edges, planes, board, netCounters, routingStatus, run == 0, tracesToEdges);
		if (m_cancelled) break;
		if (allDone) break;

		QList<int> ordering;
		foreach (JEdge * edge, edges) {
			ordering.append(edge->id);
		}
		orderings.append(ordering);	

		reorderEdges(edges, tracesToEdges);
		bool gotOne = false;
		foreach (QList<int> ordering, orderings) {
			bool allGood = true;
			for (int i = 0; i < edges.count(); i++) {
				if (ordering.at(i) != edges.at(i)->id) {
					allGood = false;
					break;
				}
			}
			if (allGood) {
				gotOne = true;
				break;
			}
		}
		if (gotOne) {
			// we cycled back to a previous order
			break;
		}

		// TODO: only delete the ones that have been reordered
		foreach (Wire * trace, tracesToEdges.keys()) {
			delete trace;
		}
		tracesToEdges.clear();
		foreach (Plane * plane, planes) clearTiles(plane);
		planes.clear();
	}

	if (m_cancelled) {
		clearEdges(edges);
		foreach (Plane * plane, planes) clearTiles(plane);
		doCancel(parentCommand);
		return;
	}

	m_currentProgressPart++;
	fixupJumperItems(edges, board);

	cleanUp();

	addToUndo(parentCommand, edges);

	clearEdges(edges);
	foreach (Plane * plane, planes) clearTiles(plane);
	new CleanUpWiresCommand(m_sketchWidget, CleanUpWiresCommand::RedoOnly, parentCommand);

	m_sketchWidget->pushCommand(parentCommand);
	m_sketchWidget->repaint();
	DebugDialog::debug("\n\n\nautorouting complete\n\n\n");
}

bool JRouter::runEdges(QList<JEdge *> & edges, QList<Plane *> & planes, ItemBase * board,
					   QVector<int> & netCounters, RoutingStatus & routingStatus, bool firstTime,
					   QHash<Wire *, JEdge *> & tracesToEdges)
{	
	bool allRouted = true;

	ViewGeometry vg;
	vg.setTrace(true);
	ViewLayer::ViewLayerID copper0 = m_sketchWidget->getWireViewLayerID(vg, ViewLayer::Bottom);
	ViewLayer::ViewLayerID copper1 = m_sketchWidget->getWireViewLayerID(vg, ViewLayer::Top);

	QList<Tile *> alreadyTiled;
	Plane * plane0 = tilePlane(board, copper0, alreadyTiled);
	if (plane0) planes.append(plane0);
	Plane * plane1 = NULL;
	if (alreadyTiled.count() > 0) {
		m_cancelled = true;
		displayBadTiles(alreadyTiled);
		QMessageBox::warning(NULL, QObject::tr("Fritzing"), QObject::tr("Cannot autoroute: parts or traces are overlapping"));
		return false;
	}

	if (m_bothSidesNow) {
		plane1 = tilePlane(board, copper1, alreadyTiled);
		if (plane1) planes.append(plane1);
		if (alreadyTiled.count() > 0) {
			m_cancelled = true;
			displayBadTiles(alreadyTiled);
			QMessageBox::warning(NULL, QObject::tr("Fritzing"), QObject::tr("Cannot autoroute: parts or traces are overlapping"));
			return false;
		}
	}

	if (firstTime) {
		collectEdges(edges, plane0, plane1, copper0, copper1);
		qSort(edges.begin(), edges.end(), edgeLessThan);	// sort the edges by distance and layer
	}
	else {
		foreach (JEdge * edge, edges) {
			edge->plane = (edge->viewLayerID == copper0) ? plane0 : plane1;
			edge->routed = false;
			edge->fromConnectorItems.clear();
			edge->toConnectorItems.clear();
			edge->fromTraces.clear();
			edge->toTraces.clear();
		}
	}

	int edgesDone = 0;
	foreach (JEdge * edge, edges) {	
		if (edge->linkedEdge && edge->linkedEdge->routed) {
			// got it one one side; no need to route it on the other
			continue;
		}

		// TODO: hide the side that's not involved

		expand(edge->from, edge->fromConnectorItems, edge->fromTraces);
		expand(edge->to, edge->toConnectorItems, edge->toTraces);

		QPointF fp = edge->from->sceneAdjustedTerminalPoint(NULL);
		QPointF tp = edge->to->sceneAdjustedTerminalPoint(NULL);

		QList<JSubedge *> subedges;
		foreach (ConnectorItem * from, edge->fromConnectorItems) {
			QPointF p1 = from->sceneAdjustedTerminalPoint(NULL);
			subedges.append(makeSubedge(edge, p1, from, NULL, tp, true));
		}
		foreach (Wire * from, edge->fromTraces) {
			QPointF p1 = from->connector0()->sceneAdjustedTerminalPoint(NULL);
			QPointF p2 = from->connector1()->sceneAdjustedTerminalPoint(NULL);
			subedges.append(makeSubedge(edge, (p1 + p2) / 2, NULL, from, tp, true));
		}
		// reverse direction
		foreach (ConnectorItem * to, edge->toConnectorItems) {
			QPointF p1 = to->sceneAdjustedTerminalPoint(NULL);
			subedges.append(makeSubedge(edge, p1, to, NULL, fp, false));
		}
		foreach (Wire * to, edge->toTraces) {
			QPointF p1 = to->connector0()->sceneAdjustedTerminalPoint(NULL);
			QPointF p2 = to->connector1()->sceneAdjustedTerminalPoint(NULL);
			subedges.append(makeSubedge(edge, (p1 + p2) / 2, NULL, to, fp, false));
		}
		qSort(subedges.begin(), subedges.end(), subedgeLessThan);

		DebugDialog::debug(QString("\n\nedge from %1 %2 %3 to %4 %5 %6, %7")
			.arg(edge->from->attachedToTitle())
			.arg(edge->from->attachedToID())
			.arg(edge->from->connectorSharedID())
			.arg(edge->to->attachedToTitle())
			.arg(edge->to->attachedToID())
			.arg(edge->to->connectorSharedID())
			.arg(edge->distance) );

		bool routedFlag = false;
		foreach (JSubedge * subedge, subedges) {
			if (m_cancelled || m_stopTrace) break;

			routedFlag = traceSubedge(subedge, board, tracesToEdges);
			if (routedFlag) {
				edge->routed = true;
				break;
			}
		}

		foreach (JSubedge * subedge, subedges) {
			delete subedge;
		}
		subedges.clear();

		if (!routedFlag) {
			allRouted = false;
		}

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

		if (m_stopTrace) {
			break;
		}
	}

	return allRouted;
}

int checkThin(Tile * tile, UserData data) {
	if (TiGetType(tile) != SPACE) return 0;

	//DebugDialog::debug(QString("tile %1 %2 %3 %4").arg(LEFT(tile)).arg(YMIN(tile)).arg(RIGHT(tile)).arg(YMAX(tile)));
	QSet<Tile *> * tiles = (QSet<Tile *> *) data;
	tiles->insert(tile);

	return 0;
}

Plane * JRouter::tilePlane(ItemBase * board, ViewLayer::ViewLayerID viewLayerID, QList<Tile *> & alreadyTiled) {
	TiSetChangedFunc(NULL);
	TiSetFreeFunc(NULL);

	Tile * bufferTile = TiAlloc();
	TiSetType(bufferTile, BUFFER);
	TiSetBody(bufferTile, NULL);

	if (board) {
		m_maxRect = board->boundingRect();
		m_maxRect.translate(board->pos());
	}
	else {
		m_maxRect = m_sketchWidget->scene()->itemsBoundingRect();
		m_maxRect.adjust(-m_maxRect.width() / 2, -m_maxRect.height() / 2, m_maxRect.width() / 2, m_maxRect.height() / 2);
	}

	QRectF bufferRect(m_maxRect);
	bufferRect.adjust(-m_maxRect.width(), -m_maxRect.height(), m_maxRect.width(), m_maxRect.height());

	SETLEFT(bufferTile, realToTile(bufferRect.left()));
	SETYMIN(bufferTile, realToTile(bufferRect.top()));		// TILE is Math Y-axis not computer-graphic Y-axis

	Plane * thePlane = TiNewPlane(bufferTile);

	SETRIGHT(bufferTile, realToTile(bufferRect.right()));
	SETYMAX(bufferTile, realToTile(bufferRect.bottom()));		// TILE is Math Y-axis not computer-graphic Y-axis

	TileRect boardRect;
	realsToTile(boardRect, m_maxRect.left(), m_maxRect.top(), m_maxRect.right(), m_maxRect.bottom());
	QList<Tile *> already;
	insertTile(thePlane, boardRect, already, NULL, SPACE, false);
	// if board is not rectangular, add tiles for the outside edges;

	if (!initBoard(board, thePlane, alreadyTiled)) return thePlane;

	TiSetChangedFunc(CollectChangedTiles);
	TiSetFreeFunc(RemoveChangedTiles);

	if (m_sketchWidget->autorouteCheckConnectors()) {
		// deal with "rectangular" elements first
		foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
			// TODO: need to leave expansion area around coords?
			ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
			if (connectorItem != NULL) {
				if (!connectorItem->attachedTo()->isVisible()) continue;
				if (connectorItem->attachedTo()->hidden()) continue;
				if (connectorItem->attachedToItemType() == ModelPart::Wire) continue;
				if (!m_sketchWidget->sameElectricalLayer2(connectorItem->attachedToViewLayerID(), viewLayerID)) continue;

				DebugDialog::debug(QString("coords connectoritem %1 %2 %3 %4 %5")
										.arg(connectorItem->connectorSharedID())
										.arg(connectorItem->connectorSharedName())
										.arg(connectorItem->attachedToTitle())
										.arg(connectorItem->attachedToID())
										.arg(connectorItem->attachedToInstanceTitle())
								);

				addTile(connectorItem, CONNECTOR, thePlane, alreadyTiled);
				if (alreadyTiled.count() > 0) {
					return thePlane;
				}

				continue;
			}
			NonConnectorItem * nonConnectorItem = dynamic_cast<NonConnectorItem *>(item);
			if (nonConnectorItem != NULL) {
				if (!nonConnectorItem->attachedTo()->isVisible()) continue;
				if (nonConnectorItem->attachedTo()->hidden()) continue;
				if (!m_sketchWidget->sameElectricalLayer2(connectorItem->attachedToViewLayerID(), viewLayerID)) continue;

				DebugDialog::debug(QString("coords nonconnectoritem %1 %2")
										.arg(nonConnectorItem->attachedToTitle())
										.arg(nonConnectorItem->attachedToID())
										);

				addTile(nonConnectorItem, NONCONNECTOR, thePlane, alreadyTiled);
				if (alreadyTiled.count() > 0) {
					return thePlane;
				}

				continue;
			}
		}
	}

	if (m_sketchWidget->autorouteCheckWires()) {
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

			tileWire(wire, thePlane, beenThere, alreadyTiled);
			if (alreadyTiled.count() > 0) {
				return thePlane;
			}	
		}
	}


	if (m_sketchWidget->autorouteCheckParts()) {
		QList<Wire *> beenThere;
		foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
			ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
			if (itemBase == NULL) continue;

			if (itemBase->itemType() == ModelPart::Wire) {
				Wire * wire = dynamic_cast<Wire *>(item);
				if (wire == NULL) continue;
				if (!wire->isVisible()) continue;
				if (wire->hidden()) continue;
				if (!wire->getTrace()) continue;
				if (!m_sketchWidget->sameElectricalLayer2(wire->viewLayerID(), viewLayerID)) continue;
				if (beenThere.contains(wire)) continue;

				//tileWire(wire, thePlane, beenThere, alreadyTiledWithWireCrossing);
				if (alreadyTiled.count() > 0) {
					return thePlane;
				}

				continue;
			}

			//tilePart(itemBase, thePlane);
		}
	}

	QSet<Tile *> tiles;
	TileRect tileRect;
	realsToTile(tileRect, m_maxRect.left(), m_maxRect.top(), m_maxRect.right(), m_maxRect.bottom());
	TiSrArea(NULL, thePlane, &tileRect, checkThin, &tiles);
	handleChangedTilesAux(thePlane, tiles);	

	return thePlane;
}

bool JRouter::initBoard(ItemBase * board, Plane * thePlane, QList<Tile *> & alreadyTiled)
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
	gpg.getBoardRects(svg, board, FSvgRenderer::printerScale(), KeepoutSpace, rects);
	QPointF boardPos = board->pos();
	foreach (QRect r, rects) {
		TileRect tileRect;
		realsToTile(tileRect, r.left() + boardPos.x(), r.top() + boardPos.y(), r.right() + boardPos.x(), r.bottom() + 1 + boardPos.y());  // note off-by-one weirdness
		insertTile(thePlane, tileRect, alreadyTiled, NULL, NOTBOARD, false);
		if (alreadyTiled.count() > 0) {
			return false;
		}
	}

	// now insert tinyspace tiles so that thin tiles don't extend across the board
	QVector<bool> done(rects.count(), false);
	for (int ix = 0; ix < rects.count(); ix++) {
		if (done[ix]) continue;

		done[ix] = true;
		QRect r = rects.at(ix);
		if (r.height() > m_wireWidthNeeded) continue;

		QList<int> join;
		join.append(ix);
		int joinHeight = r.height();
		for (int jx = ix + 1; jx < rects.count(); jx++) {
			QRect s = rects.at(jx);
			if (s.bottom() == r.bottom()) {
				// on same row; keep going
				continue;
			}

			if (s.top() > r.bottom() + 1) {
				// skipped row, can't join
				break;
			}

			if (qAbs(s.left() - r.left()) < TINYSPACEMAX && qAbs(s.right() - r.right()) < TINYSPACEMAX) {
				join.append(jx);
				joinHeight += s.height();
				r = s;
				if (joinHeight > m_wireWidthNeeded) break;
			}
		}

		if (join.count() > 1) {
			int minLeft = std::numeric_limits<int>::max();
			int maxRight = std::numeric_limits<int>::min();
			if (join.count() < 3) {
				DebugDialog::debug("short join count");
			}
			foreach (int kx, join) {
				done[kx] = true;
				QRect r = rects.at(kx);
				minLeft = qMin(minLeft, r.left());
				maxRight = qMax(maxRight, r.right());
			}
			foreach (int kx, join) {
				QRect r = rects.at(kx);
				if (r.left() > minLeft) {
					TileRect tinyRect;
					realsToTile(tinyRect, minLeft + boardPos.x(), r.top() + boardPos.y(), r.left() + boardPos.x(), r.bottom() + 1 + boardPos.y());
					TiInsertTile(thePlane, &tinyRect, NULL, TINYSPACE);
				}
				if (r.right() < maxRight) {
					TileRect tinyRect;
					realsToTile(tinyRect, r.right() + boardPos.x(), r.top() + boardPos.y(), maxRight + boardPos.x(), r.bottom() + 1 + boardPos.y());
					TiInsertTile(thePlane, &tinyRect, NULL, TINYSPACE);
				}
			}
		}
	}

	return true;
}

bool clipRect(TileRect * r, TileRect * clip, QList<TileRect> & rects) {
	if (!tileRectIntersects(r, clip)) return false;

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


void JRouter::tileWire(Wire * wire, Plane * thePlane, QList<Wire *> & beenThere, QList<Tile *> & alreadyTiled) 
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

	foreach (ConnectorItem * connectorItem, ends) {
		DebugDialog::debug(QString("tile wire end %1 %2 %3 %4 %5")
								.arg(connectorItem->connectorSharedID())
								.arg(connectorItem->connectorSharedName())
								.arg(connectorItem->attachedToTitle())
								.arg(connectorItem->attachedToID())
								.arg(connectorItem->attachedToInstanceTitle())
								);
	}

	QList<ConnectorItem *> uniqueEnds;
	foreach (Wire * cw, wires) {
		ConnectorItem * c0 = cw->connector0();
		if ((c0 != NULL) && c0->chained()) {
			uniqueEnds.append(c0);
		}
	}
	foreach (ConnectorItem * connectorItem, uniqueEnds) {
		QPointF c = connectorItem->sceneAdjustedTerminalPoint(NULL);
		Wire * w = qobject_cast<Wire *>(connectorItem->attachedTo());

		TileRect tileRect;
		qreal x = c.x() - (w->width() / 2);
		qreal y = c.y() - (w->width() / 2);
		realsToTile(tileRect, x, y, x + w->width(), y + w->width()); 
		insertTile(thePlane, tileRect, alreadyTiled, connectorItem, CONNECTOR, true);
		if (alreadyTiled.count() > 0) {
			return;
		}
	}

	foreach (Wire * w, wires) {
		QList<QRectF> rects;
		QPointF p1 = w->connector0()->sceneAdjustedTerminalPoint(NULL);
		QPointF p2 = w->connector1()->sceneAdjustedTerminalPoint(NULL);
		qreal dx = qAbs(p1.x() - p2.x());
		qreal dy = qAbs(p1.y() - p2.y());
		if (dx < 1.0) {
			// vertical line
			TileRect tileRect;
			qreal x = qMin(p1.x(), p2.x()) - (w->width() / 2);
			qreal y = qMin(p1.y(), p2.y());
			realsToTile(tileRect, x, y, x + w->width() + dx, y + dy);
			tileOneWire(thePlane, tileRect, alreadyTiled, w);
			if (alreadyTiled.count() > 0) break;
		}
		else if (dy < 1.0) {
			// horizontal line
			TileRect tileRect;
			qreal y =  qMin(p1.y(), p2.y()) - (w->width() / 2);
			realsToTile(tileRect, qMin(p1.x(), p2.x()), y, qMax(p1.x(), p2.x()), y + w->width() + dy);
			tileOneWire(thePlane, tileRect, alreadyTiled, w);
			if (alreadyTiled.count() > 0) break;
		}
		else {
			qreal angle = atan2(p2.y() - p1.y(), p2.x() - p1.x());
			if (dy >= dx) {
				//sliceWireHorizontally(w, angle, p1, p2, rects);
			}
			else {
				//sliceWireVertically(w, angle, p1, p2, rects);
			}
		}

		int ix = 0;
		while (ix < rects.count()) {
			QRectF r = rects.at(ix++);
			TileRect tileRect;
			realsToTile(tileRect, r.left(), r.top(), r.right(), r.bottom());
			DebugDialog::debug("tile wire", r);
			insertTile(thePlane, tileRect, alreadyTiled, w, TRACE, true);
			if (alreadyTiled.count() > 0) break;
		}
		if (alreadyTiled.count() > 0) break;
	}
}

Tile * JRouter::tileOneWire(Plane * thePlane, TileRect & tileRect, QList<Tile *> & alreadyTiled, Wire * w) {
	return insertTile(thePlane, tileRect, alreadyTiled, w, TRACE, true);
}


void JRouter::sliceWireVertically(Wire * w, qreal angle, QPointF p1, QPointF p2, QList<QRectF> & rects) {
	// tiler gets confused when horizontally contiguous tiles are the same type, so join rects horizontally
	qreal x, y, xend, yend;
	qreal slantWidth = qAbs((w->width() + KeepoutSpace + KeepoutSpace) / cos(angle));
	if (p1.x() < p2.x()) {
		x = p1.x();
		y = p1.y();
		xend = p2.x();
		yend = p2.y();
	}
	else {
		x = p2.x();
		y = p2.y();
		xend = p1.x();
		yend = p1.y();
	}

	int xunits = qCeil((xend - x) / Wire::STANDARD_TRACE_WIDTH);
	int miny = qFloor((qMin(y, yend) - (slantWidth / 2)) / Wire::STANDARD_TRACE_WIDTH);
	int maxy = qCeil((qMax(y, yend) + (slantWidth / 2)) / Wire::STANDARD_TRACE_WIDTH);
	int yunits = maxy - miny;
	QVector< QVector<char> > tiler(yunits, QVector<char>(xunits + 1, 0));
	
	qreal cx = x;
	for (int ix = 0; cx < xend; ix++) {
		qreal dx = cx - x;
		qreal dy = dx * (yend - y) / (xend - x);
		qreal y1 = y + dy - (slantWidth / 2);
		qreal y2 = y1 + slantWidth;
		int yi1 = qFloor(y1 / Wire::STANDARD_TRACE_WIDTH);
		int yi2 = qCeil(y2 / Wire::STANDARD_TRACE_WIDTH);
		for (int iy = yi1; iy < yi2; iy++) {
			tiler[iy - miny][ix] = 1;
		}
		cx += Wire::STANDARD_TRACE_WIDTH;
	}

	for (int iy = 0; iy < yunits; iy++) {
		bool inRect = false;
		int started;
		for (int ix = 0; ix < xunits + 1; ix++) {
			if (tiler[iy][ix] == 1) {
				if (!inRect) {
					inRect = true;
					started = ix;
				}
			}
			else {
				if (inRect) {
					inRect = false;
					QRectF r(x + (Wire::STANDARD_TRACE_WIDTH * started), 
							 (miny + iy) * Wire::STANDARD_TRACE_WIDTH,
							 (ix - started) * Wire::STANDARD_TRACE_WIDTH,
							 Wire::STANDARD_TRACE_WIDTH);
					rects.append(r);
				}
			}
		}
	}
}

void JRouter::sliceWireHorizontally(Wire * w, qreal angle, QPointF p1, QPointF p2, QList<QRectF> & rects) {
	qreal x, y, xend, yend;
	qreal slantWidth = qAbs((w->width() + KeepoutSpace + KeepoutSpace) / sin(angle));
	if (p1.y() < p2.y()) {
		x = p1.x();
		y = p1.y();
		xend = p2.x();
		yend = p2.y();
	}
	else {
		x = p2.x();
		y = p2.y();
		xend = p1.x();
		yend = p1.y();
	}

	// begin somewhere on the pseudo-grid
	int ystart = qFloor(y / Wire::STANDARD_TRACE_WIDTH);
	for (qreal cy = ystart * Wire::STANDARD_TRACE_WIDTH; cy < yend; ystart++) {
		qreal dy = cy - y;
		qreal dx = dy * (xend - x) / (yend - y);
		QRectF r(x + dx - (slantWidth / 2), cy, slantWidth, Wire::STANDARD_TRACE_WIDTH);
		rects.append(r);
		cy += Wire::STANDARD_TRACE_WIDTH;
	}	
}

void JRouter::fixupJumperItems(QList<JEdge *> & edges, ItemBase * board) {
	if (edges.count() <= 0) return;

	int todo = 0;
	foreach (JEdge * edge, edges) {
		if (!edge->routed && (edge->linkedEdge && !edge->linkedEdge->routed)) {
			todo++;
		}
	}

	int jumpersDone = 0;
	foreach (JEdge * edge, edges) {
		if (edge->routed) continue;
		if (edge->linkedEdge) {
			if (edge->linkedEdge->routed) continue;
			if (edge->linkedEdge->jumperItem) continue;
		}

		drawJumperItem(edge, board);
		jumpersDone += ((edge->linkedEdge) ? 2 : 1);
		updateProgress(jumpersDone, todo);
	}
}

bool JRouter::traceSubedge(JSubedge* subedge, ItemBase * board, QHash<Wire *, JEdge *> & tracesToEdges) 
{
	Tile * tile = NULL;

	QList<Wire *> wires;
	tile = drawTrace(subedge, wires, false);	
	if (tile) {

		// TODO: handle wire stickyness
		// TODO: get rid of clean type
		// TODO: make sure that spliteTrace succeeds if trace was not autoroutable


		/*
		switch (m_sketchWidget->cleanType()) {
			case PCBSketchWidget::noClean:
				break;
			case PCBSketchWidget::ninetyClean:
				break;
		}

		if (cleaned) {
			reduceColinearWires(wires);
		}
		else {
			reduceWires(wires, from, to, boundingPoly);
		}
		*/

		if (subedge->fromConnectorItem == NULL) {
			subedge->fromConnectorItem = splitTrace(subedge->fromWire, subedge->fromPoint, board);
			Wire * split = qobject_cast<Wire *>(subedge->fromConnectorItem->attachedTo());
			JEdge * edge = tracesToEdges.value(subedge->fromWire);
			if (edge) {
				tracesToEdges.insert(split, edge);
			}
		}
		if (subedge->toConnectorItem == NULL) {
			subedge->toConnectorItem = splitTrace(subedge->toWire, subedge->toPoint, board);
			Wire * split = qobject_cast<Wire *>(subedge->toConnectorItem->attachedTo());
			JEdge * edge = tracesToEdges.value(subedge->toWire);
			if (edge) {
				tracesToEdges.insert(split, edge);
			}
		}

		// hook everyone up
		hookUpWires(subedge, wires);
		foreach (Wire * wire, wires) {
			tracesToEdges.insert(wire, subedge->edge);
		}
	}

	hideTiles();

	return (tile != NULL);
}

ConnectorItem * JRouter::splitTrace(Wire * wire, QPointF point, ItemBase * board) 
{
	// split the trace at point
	QLineF originalLine = wire->line();
	QLineF newLine(QPointF(0,0), point - wire->pos());
	wire->setLine(newLine);
	TraceWire * splitWire = drawOneTrace(point, originalLine.p2() + wire->pos(), wire->width(), wire->viewLayerSpec());
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

	if (board) {
		splitWire->addSticky(board, true);
		board->addSticky(splitWire, true);
	}

	if (!wire->getAutoroutable()) {
		// TODO: deal with undo
	}

	return splitWire->connector0();
}


void JRouter::hookUpWires(JSubedge * subedge, QList<Wire *> & wires) {
	if (wires.count() <= 0) return;
		
	if (subedge->fromConnectorItem) {
		subedge->fromConnectorItem->tempConnectTo(wires[0]->connector0(), true);
		wires[0]->connector0()->tempConnectTo(subedge->fromConnectorItem, true);
	}
	else {
	}
	int last = wires.count() - 1;
	if (subedge->toConnectorItem) {
		subedge->toConnectorItem->tempConnectTo(wires[last]->connector1(), true);
		wires[last]->connector1()->tempConnectTo(subedge->toConnectorItem, true);
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
	QList<Wire *> beenThere;
	tileWire(wires[0], subedge->edge->plane, beenThere, alreadyTiled);
}

Tile * JRouter::drawTrace(JSubedge * subedge, QList<Wire *> & wires, bool forEmpty) 
{
	QList<Tile *> path;
	Tile * tile = NULL;
	bool result = propagate(subedge, path, forEmpty);
	if (result) {
		tile = path.last();
		result = backPropagate(subedge, path, wires, forEmpty);
		if (!result) tile = NULL;
	}

	// clear the cancel flag if it's been set so the next trace can proceed
	m_cancelTrace = false;
	return tile;
}

int JRouter::drawOneStep(int ix, QList<SeedTree *> & seedTreeList, QList<QPointF> & allPoints) {

	// TODO: instead of picking maxTop, minBottom, maxLeft, minRight, choose the center
	// TODO: use 3 lines instead of 2 for turns

	for (int i = ix; i < seedTreeList.count() - 1; i++) {
		SeedTree * from = seedTreeList.at(i);
		SeedTree * to = seedTreeList.at(i + 1);
		QRectF fromTileRect, toTileRect;
		tileToQRect(from->seed, fromTileRect);
		tileToQRect(to->seed, toTileRect);
		DebugDialog::debug("one step", fromTileRect);
		DebugDialog::debug("    to", toTileRect);

		QGraphicsItem * item = TiGetClient(from->seed);
		if (item != NULL) item->setVisible(false);
		// TODO: process events just for debugging
		ProcessEventBlocker::processEvents();	

		if (toTileRect.right() == fromTileRect.left()) {
			if (toTileRect.width() < m_wireWidthNeeded) continue;
			drawDirectionHorizontal(allPoints, fromTileRect, toTileRect);
		}
		else if (toTileRect.left() == fromTileRect.right()) {
			if (toTileRect.width() < m_wireWidthNeeded) continue;
			drawDirectionHorizontal(allPoints, fromTileRect, toTileRect);
		}
		else if (toTileRect.bottom() == fromTileRect.top()) {
			if (toTileRect.height() < m_wireWidthNeeded) continue;
			drawDirectionVertical(allPoints, fromTileRect, toTileRect);
		}
		else if (toTileRect.top() == fromTileRect.bottom()) {
			if (toTileRect.height() < m_wireWidthNeeded) continue;
			drawDirectionVertical(allPoints, fromTileRect, toTileRect);
		}
		return i + 1;
	}

	return seedTreeList.count();
}

void JRouter::drawDirectionVertical(QList<QPointF> & allPoints, QRectF & fromTileRect, QRectF & toTileRect) 
{
	qreal maxLeft = qMax(fromTileRect.left(), toTileRect.left()) + m_halfWireWidthNeeded;
	qreal minRight = qMin(fromTileRect.right(), toTileRect.right()) - m_halfWireWidthNeeded;
	qreal leastY = fromTileRect.top() < toTileRect.top() 
			? toTileRect.top() + m_halfWireWidthNeeded
			: toTileRect.bottom() - m_halfWireWidthNeeded;

	QPointF startPoint = allPoints.last();
	if (startPoint.x() >= maxLeft && startPoint.x() <= minRight) {
		// can draw a straight horizontal line from the last point
		allPoints.append(QPointF(startPoint.x(), leastY));
		return;
	}

	if (startPoint.x() <= maxLeft) {
		// turn right
		allPoints.append(QPointF(maxLeft, startPoint.y()));
		allPoints.append(QPointF(maxLeft, leastY));
		return;
	}

	// turn left
	allPoints.append(QPointF(minRight, startPoint.y()));
	allPoints.append(QPointF(minRight, leastY));
}

void JRouter::drawDirectionHorizontal(QList<QPointF> & allPoints, QRectF & fromTileRect, QRectF & toTileRect) 
{
	qreal maxTop = qMax(fromTileRect.top(), toTileRect.top()) + m_halfWireWidthNeeded;
	qreal minBottom = qMin(fromTileRect.bottom(), toTileRect.bottom()) - m_halfWireWidthNeeded;
	qreal leastX = fromTileRect.left() < toTileRect.left() 
			? toTileRect.left() + m_halfWireWidthNeeded
			: toTileRect.right() - m_halfWireWidthNeeded;

	QPointF startPoint = allPoints.last();
	if (startPoint.y() >= maxTop && startPoint.y() <= minBottom) {
		// can draw a straight horizontal line from the last point
		allPoints.append(QPointF(leastX, startPoint.y()));
		return;
	}

	if (startPoint.y() <= maxTop) {
		// turn down
		allPoints.append(QPointF(startPoint.x(), maxTop));
		allPoints.append(QPointF(leastX, maxTop));
		return;
	}

	// turn up
	allPoints.append(QPointF(startPoint.x(), minBottom));
	allPoints.append(QPointF(leastX, minBottom));
}

QPointF JRouter::calcSpaceEndPoint(JSubedge * subedge, QPointF startPoint, QList<SeedTree *> & seedTreeList) {
	int currentDirection = SeedTree::None;
	for (int i = seedTreeList.count() - 2; i >= 0; i--) {
		SeedTree * to = seedTreeList.at(i + 1);
		SeedTree * from = seedTreeList.at(i);
		if (calcOneStep(from, to, currentDirection, startPoint)) break;
	}

	QSizeF sizeNeeded = m_sketchWidget->jumperItemSize();
	qreal widthNeeded = sizeNeeded.width() + KeepoutSpace + KeepoutSpace;
	qreal heightNeeded = sizeNeeded.height() + KeepoutSpace + KeepoutSpace;

	subedge->spacePoint = findNearestSpace(seedTreeList.last()->seed, widthNeeded, heightNeeded, subedge->edge->plane, startPoint);
	return subedge->spacePoint;
}

QPointF JRouter::calcWireEndPoint(Wire * wire, QPointF startPoint, QList<SeedTree *> & seedTreeList) {
	int currentDirection = SeedTree::None;
	for (int i = seedTreeList.count() - 2; i >= 0; i--) {
		SeedTree * to = seedTreeList.at(i + 1);
		SeedTree * from = seedTreeList.at(i);
		if (calcOneStep(from, to, currentDirection, startPoint)) break;
	}

	bool atEndpoint;
	double distance, dx, dy;
	QPointF p = wire->pos();
	QPointF pp = wire->line().p2() + p;
	double cx1, cy1, cx2, cy2;
	QRectF toTileRect;
	tileToQRect(seedTreeList.last()->seed, toTileRect);
	// clip line in case it's in multiple tiles, and to move it in from the borders
	qreal dw = 0;
	if (toTileRect.width() > m_wireWidthNeeded) dw = m_halfWireWidthNeeded;
	qreal dh = 0;
	if (toTileRect.height() > m_wireWidthNeeded) dh = m_halfWireWidthNeeded;
	bool result = GraphicsUtils::liangBarskyLineClip(p.x(), p.y(), pp.x(), pp.y(),
													 toTileRect.left() + dw, toTileRect.right() - dw, toTileRect.top() + dh, toTileRect.bottom() - dh, 
													 cx1, cy1, cx2, cy2);
	if (result == false) {
		DebugDialog::debug("clip line failure");
	}
	GraphicsUtils::distanceFromLine(startPoint.x(), startPoint.y(), cx1, cy1, cx2, cy2, dx, dy, distance, atEndpoint);
	return QPointF(dx, dy);
}

bool JRouter::calcOneStep(SeedTree * from, SeedTree * to, int & currentDirection, QPointF & startPoint) {
	QRectF fromTileRect, toTileRect;
	tileToQRect(from->seed, fromTileRect);
	tileToQRect(to->seed, toTileRect);

	int newDirection = SeedTree::None;
	if (toTileRect.right() == fromTileRect.left()) {
		newDirection = SeedTree::Left;
	}
	else if (toTileRect.left() == fromTileRect.right()) {
		newDirection = SeedTree::Right;
	}
	else if (toTileRect.bottom() == fromTileRect.top()) {
		newDirection = SeedTree::Up;
	}
	else if (toTileRect.top() == fromTileRect.bottom()) {
		newDirection = SeedTree::Down;
	}
	if (currentDirection == SeedTree::None) {
		currentDirection = newDirection;
	}
	else if (currentDirection != newDirection) {
		// turn; use this as the nearest
		qreal maxTop = qMax(fromTileRect.top(), toTileRect.top()) + m_halfWireWidthNeeded;
		qreal minBottom = qMin(fromTileRect.bottom(), toTileRect.bottom()) - m_halfWireWidthNeeded;
		qreal maxLeft = qMax(fromTileRect.left(), toTileRect.left()) + m_halfWireWidthNeeded;
		qreal minRight = qMin(fromTileRect.right(), toTileRect.right()) - m_halfWireWidthNeeded;
		switch (newDirection) {
			case SeedTree::Left:
				startPoint.setX(fromTileRect.left());
				startPoint.setY((maxTop + minBottom) / 2);
				break;
			case SeedTree::Right:
				startPoint.setX(fromTileRect.right());
				startPoint.setY((maxTop + minBottom) / 2);
				break;
			case SeedTree::Up:
				startPoint.setY(fromTileRect.top());
				startPoint.setX((maxLeft + minRight) / 2);
				break;
			case SeedTree::Down:
				startPoint.setY(fromTileRect.top());
				startPoint.setX((maxLeft + minRight) / 2);
				break;
		}
		return true;
	}

	return false;
}

bool enoughOverlapHorizontal(Tile* tile1, Tile* tile2, qreal widthNeeded) {
	return (qMin(RIGHT(tile1), RIGHT(tile2)) - qMax(LEFT(tile1), LEFT(tile2)) > realToTile(widthNeeded));
}

bool enoughOverlapVertical(Tile* tile1, Tile* tile2, qreal widthNeeded) {
	// remember that axes are switched
	return (qMin(YMAX(tile1), YMAX(tile2)) - qMax(YMIN(tile1), YMIN(tile2)) > realToTile(widthNeeded));
}

int checkAlready(Tile * tile, UserData data) {
	int type = TiGetType(tile);
	switch (type) {
		case NOTBOARD:
		case NONCONNECTOR:
		case TRACE:
		case CONNECTOR:
		case PART:
			break;
		default:
			return 0;
	}

	QList<Tile *> * alreadyTiled = (QList<Tile *> *) data;
	alreadyTiled->append(tile);
	return 0;
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

bool JRouter::findShortcut(TileRect & tileRect, bool useX, bool targetGreater, JSubedge * subedge, QList<QPointF> & allPoints, int ix)
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
	TiSrArea(NULL, subedge->edge->plane, &tileRect, checkAlready, &alreadyTiled);
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

bool JRouter::backPropagate(JSubedge * subedge, QList<Tile *> & path, QList<Wire *> & wires, bool forEmpty) {
	// path goes from start to end
	// root is end
	// leaf is start (again)
	SeedTree * root = new SeedTree;
	SeedTree * leaf = followPath(root, path);
	if (leaf == NULL) {
		// shouldn't happen
		return false;
	}

	// seedTreeList is in same order as path
	QList<SeedTree *> seedTreeList;
	QList<SeedTree *> revSeedTreeList;
	SeedTree * st = leaf;
	while(st) {
		seedTreeList.append(st);
		revSeedTreeList.push_front(st);
		QRectF r;
		tileToQRect(st->seed, r);
		DebugDialog::debug("seed", r);
		st = st->parent;
	}

	QPointF end1, end2;
	if (subedge->fromConnectorItem != NULL) {
		end1 = subedge->fromConnectorItem->sceneAdjustedTerminalPoint(NULL);
	}
	else {
		QPointF end2;
		if (subedge->toConnectorItem) {
			end2 = subedge->toConnectorItem->sceneAdjustedTerminalPoint(NULL);
		}
		else {
			// just have to guess
			QRectF r;
			tileToQRect(seedTreeList.at(0)->seed, r);
			end2 = r.center();
		}
		end1 = this->calcWireEndPoint(subedge->fromWire, end2, revSeedTreeList);
	}
	subedge->fromPoint = end1;
	if (forEmpty) {
		end2 = calcSpaceEndPoint(subedge, end1, seedTreeList);
	}
	else if (subedge->toConnectorItem) {
		end2 = subedge->toConnectorItem->sceneAdjustedTerminalPoint(NULL);
	}
	else {
		end2 = this->calcWireEndPoint(subedge->toWire, end1, seedTreeList);
	}
	subedge->toPoint = end2;
	
	// figure out first points from both ends
	QList<QPointF> allPoints;
	allPoints.append(end1);
	int j = drawOneStep(0, seedTreeList, allPoints);
	QList<QPointF> revAllPoints;
	revAllPoints.append(end2);
	int revJ = seedTreeList.count() - drawOneStep(0, revSeedTreeList, revAllPoints) - 1;
	if (j == revJ) {
		// reached the same tile
	}
	else if (j > revJ) {
		// crossed over
		j = seedTreeList.count() / 2;
		revJ = j + 1;
		allPoints.clear();
		revAllPoints.clear();
		allPoints.append(end1);
		revAllPoints.append(end2);
	}

	while (j < revJ) {
		j = drawOneStep(j, seedTreeList, allPoints);
	}

	QPointF startPoint = allPoints.last();
	QPointF endPoint = revAllPoints.last();
	if (startPoint.x() < endPoint.x() || startPoint.x() > endPoint.x()) {
		allPoints.append(QPointF(startPoint.x(), endPoint.y()));
		allPoints.append(QPointF(endPoint.x(), endPoint.y()));
	}
	else if (startPoint.y() < endPoint.y() || startPoint.y() > endPoint.y()) {
		allPoints.append(endPoint);
	}
	for (int i = revAllPoints.count() - 1; i >= 0; i--) {
		allPoints.append(revAllPoints.at(i));
	}

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
		if (qAbs(p1.x() - p2.x()) > 1 && qAbs(p1.y() - p2.y()) > 1) {
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

	removeCorners(allPoints, subedge);
	shortenUs(allPoints, subedge);

	foreach (QPointF p, allPoints) {
		DebugDialog::debug("allpoint after:", p);
	}


	for (int i = 0; i < allPoints.count() - 1; i++) {
		QPointF p1 = allPoints.at(i);
		QPointF p2 = allPoints.at(i + 1);
		Wire * trace = drawOneTrace(p1, p2, Wire::STANDARD_TRACE_WIDTH, subedge->edge->viewLayerSpec);
		wires.append(trace);
	}

	return true;
}

void JRouter::shortenUs(QList<QPointF> & allPoints, JSubedge * subedge)
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

void JRouter::removeCorners(QList<QPointF> & allPoints, JSubedge * subedge)
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
			if ((p0.x() < p1.x() && p1.x() < p3.x()) || (p0.x() > p1.x() && p1.x() > p3.x())) {
				// x must be monotonically increasing or decreasing
				// dogleg horizontal, vertical, horizontal
			}
			else {
				continue;
			}

			proposed.setX(p3.x());
			proposed.setY(p1.y());
			removeCorner = checkProposed(proposed, p1, p3, subedge, ix == 1 || ix + 3 == allPoints.count());
			if (!removeCorner) {
				proposed.setX(p0.x());
				proposed.setY(p2.y());
				removeCorner = checkProposed(proposed, p0, p2, subedge, ix == 1 || ix + 3 == allPoints.count());
			}
		}
		else if (p0.x() == p1.x()) {
			if ((p0.y() < p1.y() && p1.y() < p3.y()) || (p0.y() > p1.y() && p1.y() > p3.y())) {
				// y must be monotonically increasing or decreasing
				// dogleg vertical, horizontal, vertical
			}
			else {
				continue;
			}

			proposed.setY(p3.y());
			proposed.setX(p1.x());
			removeCorner = checkProposed(proposed, p1, p3, subedge, ix == 1 || ix + 3 == allPoints.count());
			if (!removeCorner) {
				proposed.setY(p0.y());
				proposed.setX(p2.x());
				removeCorner = checkProposed(proposed, p0, p2, subedge, ix == 1 || ix + 3 == allPoints.count());
			}
		}
		if (!removeCorner) continue;

		ix--;
		allPoints.replace(ix + 1, proposed);
		allPoints.removeAt(ix + 2);
		if (allPoints.count() > ix + 3) {
			allPoints.removeAt(ix + 2);
		}
	}
}

bool JRouter::checkProposed(const QPointF & proposed, const QPointF & p1, const QPointF & p3, JSubedge * subedge, bool atStartOrEnd) 
{
	if (atStartOrEnd) {
		Tile * tile = TiSrPoint(NULL, subedge->edge->plane, realToTile(proposed.x()), realToTile(proposed.y()));
		short result = checkCandidate(subedge, tile, false);
		if (result >= GridEntry::GOAL || result == GridEntry::SELF) {
			// don't want to draw traces within the target connector itself
			return false;
		}
	}

	QList<Tile *> alreadyTiled;
	TileRect tileRect;
	qreal x = proposed.x() - m_halfWireWidthNeeded;
	realsToTile(tileRect, x, qMin(p1.y(), p3.y()), x + m_wireWidthNeeded, qMax(p1.y(), p3.y()));
	TiSrArea(NULL, subedge->edge->plane, &tileRect, checkAlready, &alreadyTiled);
	qreal y = proposed.y() - m_halfWireWidthNeeded;
	realsToTile(tileRect, qMin(p1.x(), p3.x()), y, qMax(p1.x(), p3.x()), y + m_wireWidthNeeded);
	TiSrArea(NULL, subedge->edge->plane, &tileRect, checkAlready, &alreadyTiled);
	foreach (Tile * tile, alreadyTiled) {
		short result = checkCandidate(subedge, tile, false);
		if (result >= GridEntry::BLOCK) return false;
	}

	return true;
}

SeedTree * JRouter::followPath(SeedTree * & root, QList<Tile *> & path) {
	QList<SeedTree *> todoList;
	root->seed = path.last();
	root->parent = NULL;
	root->direction = SeedTree::None;
	root->directionChanges = 0;
	root->restrictionMax = std::numeric_limits<int>::max();
	root->restrictionMin = std::numeric_limits<int>::min();
	root->restricted = false;
	todoList.append(root);
	SeedTree * candidate = NULL;

	while (todoList.count() > 0) {
		SeedTree * currentSeedTree = todoList.takeFirst();
		TileRect currentSeedRect;
		TiToRect(currentSeedTree->seed, &currentSeedRect);
		DebugDialog::debug(QString("from seed %1 %2 %3 %4 %5 %6 %7 %8")
								.arg(TiGetWave(currentSeedTree->seed))
								.arg(currentSeedTree->restricted)
								.arg(currentSeedTree->restrictionMin)
								.arg(currentSeedTree->restrictionMax)
								.arg(currentSeedRect.xmini)
								.arg(currentSeedRect.ymini)
								.arg(currentSeedRect.xmaxi)
								.arg(currentSeedRect.ymaxi)
								);


		if (!currentSeedTree->restricted) {
			path.removeOne(currentSeedTree->seed);
		}

		foreach (Tile * seed, path) {
			if (TiGetWave(seed) > TiGetWave(currentSeedTree->seed)) {
				// must go toward successor nodes
				continue;
			}

			SeedTree::Direction direction = SeedTree::None;

			QRectF nextRect;
			tileToQRect(seed, nextRect);
			DebugDialog::debug(QString("    try seed %1").arg(TiGetWave(seed)), nextRect);

			if (LEFT(seed) == currentSeedRect.xmaxi) {
				direction = SeedTree::Right;
				if (!enoughOverlapVertical(seed, currentSeedTree->seed, m_wireWidthNeeded)) {
					continue;
				}
				if (currentSeedTree->restricted) {
					if (currentSeedTree->direction != direction) continue;
					int rmin = qMax(currentSeedTree->restrictionMin, YMIN(seed));
					int rmax = qMin(currentSeedTree->restrictionMax, YMAX(seed));
					if (rmax - rmin < realToTile(m_wireWidthNeeded)) {
						continue;
					}
				}
			}
			else if (RIGHT(seed) == currentSeedRect.xmini) {
				direction = SeedTree::Left;
				if (!enoughOverlapVertical(seed, currentSeedTree->seed, m_wireWidthNeeded)) {
					continue;
				}
				if (currentSeedTree->restricted) {
					if (currentSeedTree->direction != direction) continue;
					int rmin = qMax(currentSeedTree->restrictionMin, YMIN(seed));
					int rmax = qMin(currentSeedTree->restrictionMax, YMAX(seed));
					if (rmax - rmin < realToTile(m_wireWidthNeeded)) {
						continue;
					}
				}
			}
			else if (YMAX(seed) == currentSeedRect.ymini) {
				direction = SeedTree::Up;
				if (!enoughOverlapHorizontal(seed, currentSeedTree->seed, m_wireWidthNeeded)) {
					continue;
				}
				if (currentSeedTree->restricted) {
					if (currentSeedTree->direction != direction) continue;
					int rmin = qMax(currentSeedTree->restrictionMin, LEFT(seed));
					int rmax = qMin(currentSeedTree->restrictionMax, RIGHT(seed));
					if (rmax - rmin < realToTile(m_wireWidthNeeded)) {
						continue;
					}
				}
			}
			else if (YMIN(seed) == currentSeedRect.ymaxi) {
				direction = SeedTree::Down;
				if (!enoughOverlapHorizontal(seed, currentSeedTree->seed, m_wireWidthNeeded)) {
					continue;
				}
				if (currentSeedTree->restricted) {
					if (currentSeedTree->direction != direction) continue;
					int rmin = qMax(currentSeedTree->restrictionMin, LEFT(seed));
					int rmax = qMin(currentSeedTree->restrictionMax, RIGHT(seed));
					if (rmax - rmin < realToTile(m_wireWidthNeeded)) {
						continue;
					}
				}
			}
			else {
				continue;
			}

			SeedTree * newst = new SeedTree;
			newst->seed = seed;
			newst->restrictionMin = std::numeric_limits<int>::min();
			newst->restrictionMax = std::numeric_limits<int>::max();
			newst->restricted = false;
			DebugDialog::debug(QString("    next seed %1").arg(TiGetWave(seed)), nextRect);
			newst->direction = direction;
			switch(direction) {
				case SeedTree::Up:
				case SeedTree::Down:
					if (YMAX(seed) - YMIN(seed) < realToTile(m_wireWidthNeeded)) {
						newst->restrictionMax = qMin(currentSeedTree->restrictionMax, RIGHT(currentSeedTree->seed));
						newst->restrictionMin = qMax(currentSeedTree->restrictionMin, LEFT(currentSeedTree->seed));
						newst->restricted = true;
					}
					break;
				case SeedTree::Left:
				case SeedTree::Right:
					if (RIGHT(seed) - LEFT(seed) < realToTile(m_wireWidthNeeded)) {
						newst->restrictionMax = qMin(currentSeedTree->restrictionMax, YMAX(currentSeedTree->seed));
						newst->restrictionMin = qMax(currentSeedTree->restrictionMin, YMIN(currentSeedTree->seed));
						newst->restricted = true;
					}
					break;
				default:
					break;
			}

			if (currentSeedTree->direction == SeedTree::None || currentSeedTree->direction == direction) {
				newst->directionChanges = currentSeedTree->directionChanges;
			}
			else {
				newst->directionChanges = currentSeedTree->directionChanges + 1;
			}
			newst->parent = currentSeedTree;
			currentSeedTree->children.append(newst);
			if (TiGetWave(seed) == 0) {
				if (candidate == NULL) {
					candidate = newst;
				}
				else if (newst->directionChanges < candidate->directionChanges) {
					candidate = newst;
				}
			}
			else {
				todoList.append(newst);
			}
		}
	}

	return candidate;
}


bool JRouter::propagate(JSubedge * subedge, QList<Tile *> & path, bool forEmpty) {

	DebugDialog::debug("((((((((((((((((((((((((((((");
	if (subedge->fromConnectorItem) {
		DebugDialog::debug(QString("starting from connectoritem %1 %2 %3 %4 %5")
								.arg(subedge->fromConnectorItem->connectorSharedID())
								.arg(subedge->fromConnectorItem->connectorSharedName())
								.arg(subedge->fromConnectorItem->attachedToTitle())
								.arg(subedge->fromConnectorItem->attachedToID())
								.arg(subedge->fromConnectorItem->attachedToInstanceTitle())
								);
	}

	Tile * firstTile = TiSrPoint(NULL, subedge->edge->plane, realToTile(subedge->fromPoint.x()), realToTile(subedge->fromPoint.y()));
	if (firstTile == NULL) {
		// shouldn't happen
		return false;
	}

	TiSetWave(firstTile, 0);
	QList<Tile *> seeds;
	seeds.append(firstTile);

	int ix = 0;
	while (ix < seeds.count()) {
		if (m_cancelTrace || m_stopTrace || m_cancelled) break;

		Tile * seed = seeds[ix++];
		
		GridEntry * gridEntry = dynamic_cast<GridEntry *>(TiGetClient(seed));
		if (gridEntry != NULL && gridEntry->isVisible()) {
			// been here already
			continue;			
		}

		// TILE math reverses y-axis!
		qreal x1 = tileToReal(LEFT(seed));
		qreal y1 = tileToReal(YMIN(seed));
		qreal x2 = tileToReal(RIGHT(seed));
		qreal y2 = tileToReal(YMAX(seed));
			
		short fof = checkCandidate(subedge, seed, forEmpty);
		QRectF r(x1, y1, x2 - x1, y2 - y1);

		DebugDialog::debug(QString("wave:%1 fof:%2").arg(TiGetWave(seed)).arg(fof), r);
		gridEntry = drawGridItem(x1, y1, x2, y2, fof, gridEntry);
		TiSetClient(seed, gridEntry);
		//TODO: processEvents should only happen every once in a while
		ProcessEventBlocker::processEvents();
		if (fof == GridEntry::GOAL) {
			path.append(seed);
			return true;		// yeeha!
		}
		if (fof > GridEntry::GOAL) {
			continue;			// blocked
		}

		DebugDialog::debug("=================", r);
		path.append(seed);
		seedNext(seed, seeds);
	}

	return false;
}

void JRouter::appendIf(Tile * seed, Tile * next, QList<Tile *> & seeds, bool (*enoughOverlap)(Tile*, Tile*, qreal)) {
	
	if (TiGetClient(next) != NULL && TiGetClient(next)->isVisible()) {
		return;			// already visited
	}

	if (TiGetType(next) == BUFFER) {
		return;
	}

	if (TiGetType(next) == NOTBOARD) {
		if (TiGetClient(next) == NULL) {
			qreal x1 = tileToReal(LEFT(next));
			qreal y1 = tileToReal(YMIN(next));
			qreal x2 = tileToReal(RIGHT(next));
			qreal y2 = tileToReal(YMAX(next));
			TiSetClient(next, drawGridItem(x1, y1, x2, y2, GridEntry::NOTBOARD, NULL));
		}

		return;		// outside board boundaries
	}

	if (!enoughOverlap(seed, next, m_wireWidthNeeded)) {
		return;	// not wide/high enough 
	}

	TiSetWave(next, TiGetWave(seed) + 1);
	seeds.append(next);
}


void JRouter::seedNext(Tile * seed, QList<Tile *> & seeds) {
	int tWidthNeeded = realToTile(m_wireWidthNeeded);
	if ((RIGHT(seed) < realToTile(m_maxRect.right())) && (YMAX(seed) - YMIN(seed) > tWidthNeeded)) {
		Tile * next = TR(seed);
		appendIf(seed, next, seeds, enoughOverlapVertical);
		while (true) {
			next = LB(next);
			if (YMAX(next) <= YMIN(seed)) {
				break;
			}

			appendIf(seed, next, seeds, enoughOverlapVertical);
		}
	}

	if ((LEFT(seed) > realToTile(m_maxRect.left())) && (YMAX(seed) - YMIN(seed) > tWidthNeeded)) {
		Tile * next = BL(seed);
		appendIf(seed, next, seeds, enoughOverlapVertical);
		while (true) {
			next = RT(next);
			if (YMIN(next) >= YMAX(seed)) {
				break;
			}

			appendIf(seed, next, seeds, enoughOverlapVertical);
		}
	}

	if ((YMAX(seed) < realToTile(m_maxRect.bottom())) && (RIGHT(seed) - LEFT(seed) > tWidthNeeded)) {	
		Tile * next = RT(seed);
		appendIf(seed, next, seeds, enoughOverlapHorizontal);
		while (true) {
			next = BL(next);
			if (RIGHT(next) <= LEFT(seed)) {
				break;
			}

			appendIf(seed, next, seeds, enoughOverlapHorizontal);
		}
	}

	if ((YMIN(seed) > realToTile(m_maxRect.top())) && (RIGHT(seed) - LEFT(seed) > tWidthNeeded)) {		
		Tile * next = LB(seed);
		appendIf(seed, next, seeds, enoughOverlapHorizontal);
		while (true) {
			next = TR(next);
			if (LEFT(next) >= RIGHT(seed)) {
				break;
			}

			appendIf(seed, next, seeds, enoughOverlapHorizontal);
		}
	}
}

short JRouter::checkCandidate(JSubedge * subedge, Tile * tile, bool forEmpty) 
{	
	switch (TiGetType(tile)) {
		case SPACE:
		case TINYSPACE:
			return checkSpace(subedge, tile, forEmpty);

		case CONNECTOR:
			if (!m_sketchWidget->autorouteCheckConnectors()) {
				return GridEntry::IGNORE;
			}
			return checkConnector(subedge, tile, dynamic_cast<ConnectorItem *>(TiGetBody(tile)), forEmpty);

		case TRACE:
			if (!m_sketchWidget->autorouteCheckWires()) {
				return GridEntry::IGNORE;
			}

			return checkTrace(subedge, tile, dynamic_cast<Wire *>(TiGetBody(tile)), forEmpty);

		case PART:
			if (!m_sketchWidget->autorouteCheckParts()) {
				return GridEntry::IGNORE;
			}

			return GridEntry::BLOCK;

		case NONCONNECTOR:
		case NOTBOARD:
		case BUFFER:
			return GridEntry::BLOCK;

		default:
			// shouldn't happen:
			return GridEntry::IGNORE;
	}
}

struct EmptyThing {
	QList<int> x_s;
	QList<int> x2_s;
	QList<int> y_s;
	int maxY;
};

int collectXandY(Tile * tile, UserData data) {
	EmptyThing * emptyThing = (EmptyThing *) data;
	int type = TiGetType(tile);
	if (type == SPACE || type == TINYSPACE) {
		if (!emptyThing->x_s.contains(LEFT(tile))) {
			emptyThing->x_s.append(LEFT(tile));
		}
		if (!emptyThing->x2_s.contains(RIGHT(tile))) {
			emptyThing->x2_s.append(RIGHT(tile));
		}
		if (YMIN(tile) <= emptyThing->maxY && !emptyThing->y_s.contains(YMIN(tile))) {
			emptyThing->y_s.append(YMIN(tile));
		}
	}

	return 0;
}

int allEmpty(Tile * tile, UserData) {
	int type = TiGetType(tile);
	if (type == SPACE || type == TINYSPACE) {
		return 0;
	}

	return 1;		// not empty; will stop the search
}


short JRouter::checkSpace(JSubedge * subedge, Tile * tile, bool forEmpty) 
{
	short result = TiGetType(tile) == SPACE ? GridEntry::EMPTY : GridEntry::TINY;

	if (!forEmpty) {
		return result;
	}

	TileRect tileRect;
	TiToRect(tile, &tileRect);

	QSizeF sizeNeeded = m_sketchWidget->jumperItemSize();
	int tWidthNeeded = realToTile(sizeNeeded.width() + KeepoutSpace + KeepoutSpace);
	int tHeightNeeded = realToTile(sizeNeeded.height() + KeepoutSpace + KeepoutSpace);
	if (tileRect.xmaxi - tileRect.xmini  < tWidthNeeded) {
		return result;
	}

	// need to figure out how to minimize the distance between this space and the previous wire;
	// need to save the set of open spaces found....

	if (tileRect.ymaxi - tileRect.ymini  >= tHeightNeeded) {
		DebugDialog::debug("empty GOAL");
		return GridEntry::GOAL;
	}

	TileRect searchRect = tileRect;
	searchRect.ymini = qMax(realToTile(m_maxRect.top()), tileRect.ymini - tHeightNeeded + (tileRect.ymaxi - tileRect.ymini));
	searchRect.ymaxi = qMin(realToTile(m_maxRect.bottom()), tileRect.ymini + tHeightNeeded);

	EmptyThing emptyThing;
	emptyThing.maxY = tileRect.ymini;
	TiSrArea(tile, subedge->edge->plane, &searchRect, collectXandY, &emptyThing);

	foreach (int y, emptyThing.y_s) {
		foreach (int x, emptyThing.x_s) {
			searchRect.xmini = x;
			searchRect.xmaxi = x + tWidthNeeded;
			searchRect.ymini = y;
			searchRect.ymaxi = y + tHeightNeeded;
			if (TiSrArea(tile, subedge->edge->plane, &searchRect, allEmpty, NULL) == 0) {
				return GridEntry::GOAL;
			}
		}
	}

	return result;
}

short JRouter::checkTrace(JSubedge * subedge, Tile * tile, Wire * candidateWire, bool forEmpty) {
	Q_UNUSED(tile);

	//if (candidateWire == NULL) return GridEntry::IGNORE;
	//if (!candidateWire->isVisible()) return GridEntry::IGNORE;
	//if (candidateWire->hidden()) return GridEntry::IGNORE;
	//if (!m_sketchWidget->sameElectricalLayer2(candidateWire->viewLayerID(), viewLayerID)) return GridEntry::IGNORE;
		
	DebugDialog::debug(QString("candidate wire %1, x1:%2 y1:%3, x2:%4 y2:%5")
			.arg(candidateWire->id())
			.arg(candidateWire->pos().x())
			.arg(candidateWire->pos().y())
			.arg(candidateWire->line().p2().x())
			.arg(candidateWire->line().p2().y()) );		

	if (forEmpty) {
		DebugDialog::debug("for empty BLOCK");
		return GridEntry::BLOCK;
	}

	if (subedge->forward) {
		if (subedge->edge->fromTraces.contains(candidateWire)) {
			DebugDialog::debug("SAFE");
			return GridEntry::SAFE;
		}
		if (subedge->edge->toTraces.contains(candidateWire)) {
			subedge->toWire = candidateWire;
			DebugDialog::debug("GOAL");
			return GridEntry::GOAL;
		}
	}
	else {
		if (subedge->edge->toTraces.contains(candidateWire)) {
			DebugDialog::debug("SAFE");
			return GridEntry::SAFE;
		}
		if (subedge->edge->fromTraces.contains(candidateWire)) {
			subedge->toWire = candidateWire;
			DebugDialog::debug("GOAL");
			return GridEntry::GOAL;
		}
	}

	DebugDialog::debug("BLOCK");

	return GridEntry::BLOCK;
}



short JRouter::checkConnector(JSubedge * subedge, Tile * tile, ConnectorItem * candidateConnectorItem, bool forEmpty) {
	Q_UNUSED(tile);

	//if (candidateConnectorItem == NULL) return GridEntry::IGNORE;
	//if (!candidateConnectorItem->attachedTo()->isVisible()) return GridEntry::IGNORE;
	//if (candidateConnectorItem->attachedTo()->hidden()) return GridEntry::IGNORE;
	//if (!m_sketchWidget->sameElectricalLayer2(candidateConnectorItem->attachedToViewLayerID(), viewLayerID)) return GridEntry::IGNORE;

	DebugDialog::debug(QString("candidate connectoritem %1 %2 %3 %4 %5")
							.arg(candidateConnectorItem->connectorSharedID())
							.arg(candidateConnectorItem->connectorSharedName())
							.arg(candidateConnectorItem->attachedToTitle())
							.arg(candidateConnectorItem->attachedToID())
							.arg(candidateConnectorItem->attachedToInstanceTitle())
							);


	if (candidateConnectorItem == subedge->fromConnectorItem) {
		DebugDialog::debug("SELF");
		return GridEntry::SELF;
	}

	if (forEmpty) {
		DebugDialog::debug("forEmpty block");
		return GridEntry::BLOCK;
	}

	/*
		candidateWire = dynamic_cast<Wire *>(candidateConnectorItem->attachedTo());
		if (candidateWire != NULL) {
			// handle this from the wire rather than the connector
			return GridEntry::IGNORE;
		}
	*/

	if (subedge->forward) {
		if (subedge->edge->fromConnectorItems.contains(candidateConnectorItem)) {
			DebugDialog::debug("OWNSIDE");
			return GridEntry::OWNSIDE;			// still a blocker
		}
	}
	else {
		if (subedge->edge->toConnectorItems.contains(candidateConnectorItem)) {
			DebugDialog::debug("OWNSIDE");
			return GridEntry::OWNSIDE;			// still a blocker
		}
	}

	if (subedge->forward) {
		if (subedge->edge->toConnectorItems.contains(candidateConnectorItem)) {
			subedge->toConnectorItem = candidateConnectorItem;
			DebugDialog::debug("GOAL");
			return GridEntry::GOAL;			
		}
	}
	else {
		if (subedge->edge->fromConnectorItems.contains(candidateConnectorItem)) {
			subedge->toConnectorItem = candidateConnectorItem;
			DebugDialog::debug("GOAL");
			return GridEntry::GOAL;			
		}
	}

	DebugDialog::debug("BLOCK");

	return GridEntry::BLOCK;
}


GridEntry * JRouter::drawGridItem(qreal x1, qreal y1, qreal x2, qreal y2, short type, GridEntry * gridEntry) 
{
	int alpha = 128;
	if (gridEntry == NULL) {
		gridEntry = new GridEntry(x1, y1, x2 - x1, y2 - y1, type, NULL);
		gridEntry->setZValue(m_sketchWidget->getTopZ());
	}
	else {
		gridEntry->setRect(x1, y1, x2 - x1, y2 - y1);
	}

	QColor c;
	switch (type) {
		case GridEntry::EMPTY:
		case GridEntry::IGNORE:
		case GridEntry::SAFE:	
			{
			//QString traceColor = m_sketchWidget->traceColor(ViewLayer::WireOnTop_TwoLayers);
			c.setNamedColor(ViewLayer::Copper1Color);
			c.setAlpha(alpha);
			}
			break;
		case GridEntry::SELF:
			c = QColor(0, 255, 0, alpha);
			break;
		case GridEntry::OWNSIDE:
		case GridEntry::BLOCK:
			c = QColor(255, 0, 0, alpha);
			break;
		case GridEntry::GOAL:
			c = QColor(0, 255, 0, alpha);
			break;
		case GridEntry::NOTBOARD:
			c = QColor(0, 0, 0, alpha);
			break;
		case GridEntry::TINY:
			c = QColor(0, 0, 255, alpha);
			break;
	}

	gridEntry->setPen(c);
	gridEntry->setBrush(QBrush(c));
	if (gridEntry->scene() == NULL) {
		m_sketchWidget->scene()->addItem(gridEntry);
	}
	gridEntry->show();
	return gridEntry;
}


void JRouter::collectEdges(QList<JEdge *> & edges, Plane * plane0, Plane * plane1, ViewLayer::ViewLayerID copper0, ViewLayer::ViewLayerID copper1)
{
	foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
		VirtualWire * vw = dynamic_cast<VirtualWire *>(item);
		if (vw == NULL) continue;

		ConnectorItem * from = vw->connector0()->firstConnectedToIsh();
		ConnectorItem * otherFrom = from->getCrossLayerConnectorItem();
		if (!m_sketchWidget->sameElectricalLayer2(copper0, from->attachedToViewLayerID())) {
			ConnectorItem * temp = from;
			from = otherFrom;
			otherFrom = temp;
		}
		if (!from) {
			continue;
		}

		ConnectorItem * to = vw->connector1()->firstConnectedToIsh();
		ConnectorItem * otherTo = to->getCrossLayerConnectorItem();
		if (!m_sketchWidget->sameElectricalLayer2(copper0, to->attachedToViewLayerID())) {
			ConnectorItem * temp = to;
			to = otherTo;
			otherTo = temp;
		}
		if (!to) {
			continue;
		}

		JEdge * edge0 = makeEdge(from, to, ViewLayer::Bottom, copper0, plane0, vw);
		edge0->id = edges.count();
		edges.append(edge0);
		if (m_bothSidesNow && otherFrom != NULL && otherTo != NULL) {
			JEdge * edge1 = makeEdge(otherFrom, otherTo, ViewLayer::Top, copper1, plane1, vw);
			edge1->id = edges.count();
			edges.append(edge1);
			edge0->linkedEdge = edge1;
			edge1->linkedEdge = edge0;
		}
	}
}

JEdge * JRouter::makeEdge(ConnectorItem * from, ConnectorItem * to, 
						  ViewLayer::ViewLayerSpec viewLayerSpec, ViewLayer::ViewLayerID viewLayerID, 
						  Plane * plane, VirtualWire * vw) {
	JEdge * edge = new JEdge;
	edge->linkedEdge = NULL;
	edge->from = from;
	edge->to = to;
	edge->viewLayerSpec = viewLayerSpec;
	edge->viewLayerID = viewLayerID;
	edge->plane = plane;
	edge->routed = false;
	edge->vw = vw;
	edge->jumperItem = NULL;
	QPointF pi = from->sceneAdjustedTerminalPoint(NULL);
	QPointF pj = to->sceneAdjustedTerminalPoint(NULL);
	double px = pi.x() - pj.x();
	double py = pi.y() - pj.y();
	edge->distance = (px * px) + (py * py);
	return edge;
}

void JRouter::expand(ConnectorItem * originalConnectorItem, QList<ConnectorItem *> & connectorItems, QSet<Wire *> & visited) 
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

void JRouter::cleanUp() {
	foreach (QList<ConnectorItem *> * connectorItems, m_allPartConnectorItems) {
		delete connectorItems;
	}
	m_allPartConnectorItems.clear();
}

void JRouter::clearTraces(PCBSketchWidget * sketchWidget, bool deleteAll, QUndoCommand * parentCommand) {
	QList<Wire *> oldTraces;
	QList<JumperItem *> oldJumperItems;
	if (sketchWidget->usesJumperItem()) {
		foreach (QGraphicsItem * item, sketchWidget->scene()->items()) {
			JumperItem * jumperItem = dynamic_cast<JumperItem *>(item);
			if (jumperItem == NULL) continue;

			if (deleteAll || jumperItem->autoroutable()) {
				oldJumperItems.append(jumperItem);

				// now deal with the traces connecting the jumperitem to the part
				QList<ConnectorItem *> both;
				foreach (ConnectorItem * ci, jumperItem->connector0()->connectedToItems()) both.append(ci);
				foreach (ConnectorItem * ci, jumperItem->connector1()->connectedToItems()) both.append(ci);
				foreach (ConnectorItem * connectorItem, both) {
					Wire * w = dynamic_cast<Wire *>(connectorItem->attachedTo());
					if (w == NULL) continue;

					if (w->getTrace()) {
						QList<Wire *> wires;
						QList<ConnectorItem *> ends;
						w->collectChained(wires, ends);
						foreach (Wire * wire, wires) {
							wire->setAutoroutable(true);
						}
					}
				}
			}
		}
	}

	foreach (QGraphicsItem * item, sketchWidget->scene()->items()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire != NULL) {		
			if (wire->getTrace()) {
				if (deleteAll || wire->getAutoroutable()) {
					oldTraces.append(wire);
				}
			}
			/*
			else if (wire->getRatsnest()) {
				if (parentCommand) {
					sketchWidget->makeChangeRoutedCommand(wire, false, sketchWidget->getRatsnestOpacity(false), parentCommand);
				}
				wire->setRouted(false);
				wire->setOpacity(sketchWidget->getRatsnestOpacity(false));	
			}
			*/
			continue;
		}

	}

	if (parentCommand) {
		addUndoConnections(sketchWidget, false, oldTraces, parentCommand);
		foreach (Wire * wire, oldTraces) {
			sketchWidget->makeDeleteItemCommand(wire, BaseCommand::SingleView, parentCommand);
		}
		foreach (JumperItem * jumperItem, oldJumperItems) {
			sketchWidget->makeDeleteItemCommand(jumperItem, BaseCommand::CrossView, parentCommand);
		}
	}

	
	foreach (Wire * wire, oldTraces) {
		sketchWidget->deleteItem(wire, true, false, false);
	}
	foreach (JumperItem * jumperItem, oldJumperItems) {
		sketchWidget->deleteItem(jumperItem, true, true, false);
	}
}

void JRouter::updateRoutingStatus() {
	RoutingStatus routingStatus;
	routingStatus.zero();
	m_sketchWidget->updateRoutingStatus(routingStatus, false);
}

JumperItem * JRouter::drawJumperItem(JEdge * edge, ItemBase * board) 
{
	// TODO: check across planes...

	QPointF fp = edge->from->sceneAdjustedTerminalPoint(NULL);
	QPointF tp = edge->to->sceneAdjustedTerminalPoint(NULL);

	QList<JSubedge *> fromSubedges, toSubedges;
	foreach (ConnectorItem * from, edge->fromConnectorItems) {
		QPointF p1 = from->sceneAdjustedTerminalPoint(NULL);
		fromSubedges.append(makeSubedge(edge, p1, from, NULL, tp, true));
	}
	foreach (Wire * from, edge->fromTraces) {
		QPointF p1 = from->connector0()->sceneAdjustedTerminalPoint(NULL);
		QPointF p2 = from->connector1()->sceneAdjustedTerminalPoint(NULL);
		fromSubedges.append(makeSubedge(edge, (p1 + p2) / 2, NULL, from, tp, true));
	}
	// reverse direction
	foreach (ConnectorItem * to, edge->toConnectorItems) {
		QPointF p1 = to->sceneAdjustedTerminalPoint(NULL);
		toSubedges.append(makeSubedge(edge, p1, to, NULL, fp, false));
	}
	foreach (Wire * to, edge->toTraces) {
		QPointF p1 = to->connector0()->sceneAdjustedTerminalPoint(NULL);
		QPointF p2 = to->connector1()->sceneAdjustedTerminalPoint(NULL);
		toSubedges.append(makeSubedge(edge, (p1 + p2) / 2, NULL, to, fp, false));
	}

	DebugDialog::debug(QString("\n\nedge from %1 %2 %3 to %4 %5 %6, %7")
		.arg(edge->from->attachedToTitle())
		.arg(edge->from->attachedToID())
		.arg(edge->from->connectorSharedID())
		.arg(edge->to->attachedToTitle())
		.arg(edge->to->attachedToID())
		.arg(edge->to->connectorSharedID())
		.arg(edge->distance) );

	QList<Wire *> fromWires;
	QList<Wire *> toWires;
	JSubedge * fromSubedge = NULL;
	JSubedge * toSubedge = NULL;
	Tile * fromTile = NULL;
	Tile * toTile = NULL;

	foreach (JSubedge * subedge, fromSubedges) {
		fromTile = drawTrace(subedge, fromWires, true); 
		if (fromTile) {
			fromSubedge = subedge;
			break;
		}
	}

	hideTiles();

	// TODO: two jumper items may try to share the same tiles
	// so should insert the tiles from the first jumper now

	if (fromSubedge != NULL) {
		foreach (JSubedge * subedge, toSubedges) {
			toTile = drawTrace(subedge, toWires, true);
			if (toTile) {
				toSubedge = subedge;
				break;
			}
		}
	}

	hideTiles();

	if (fromSubedge != NULL && toSubedge != NULL) {
		long newID = ItemBase::getNextID();
		ViewGeometry viewGeometry;
		ItemBase * itemBase = m_sketchWidget->addItem(m_sketchWidget->paletteModel()->retrieveModelPart(ModuleIDNames::jumperModuleIDName), 
												  edge->from->attachedTo()->viewLayerSpec(), 
												  BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
		if (itemBase == NULL) {
			// we're in trouble
			return NULL;
		}

		JumperItem * jumperItem = dynamic_cast<JumperItem *>(itemBase);

		QPointF fromDestPoint = fromWires.last()->connector1()->sceneAdjustedTerminalPoint(NULL);
		QPointF toDestPoint = toWires.last()->connector1()->sceneAdjustedTerminalPoint(NULL);

		QSizeF sizeNeeded = m_sketchWidget->jumperItemSize();
		qreal widthNeeded = sizeNeeded.width() + KeepoutSpace + KeepoutSpace;
		qreal heightNeeded = sizeNeeded.height() + KeepoutSpace + KeepoutSpace;

		fromDestPoint = findNearestSpace(fromTile, widthNeeded, heightNeeded, edge->plane, fromDestPoint);
		toDestPoint = findNearestSpace(toTile, widthNeeded, heightNeeded, edge->plane, toDestPoint); 
		jumperItem->resize(fromDestPoint, toDestPoint);

		if (board) {
			jumperItem->addSticky(board, true);
			board->addSticky(jumperItem, true);
		}

		m_sketchWidget->scene()->addItem(jumperItem);
		fromSubedge->toConnectorItem = jumperItem->connector0();
		QList<Tile *> already;
		addTile(jumperItem->connector0(), CONNECTOR, edge->plane, already);
		hookUpWires(fromSubedge, fromWires);

		toSubedge->toConnectorItem = jumperItem->connector1();
		addTile(jumperItem->connector1(), CONNECTOR, edge->plane, already);
		hookUpWires(toSubedge, toWires);
		edge->jumperItem = jumperItem;
	}
	else {
		foreach (Wire * wire, fromWires) {
			delete wire;
		}
		foreach (Wire * wire, toWires) {
			delete wire;
		}
	}

	foreach (JSubedge * subedge, fromSubedges) {
		delete subedge;
	}
	fromSubedges.clear();

	foreach (JSubedge * subedge, toSubedges) {
		delete subedge;
	}
	toSubedges.clear();

	return edge->jumperItem;
}

void JRouter::restoreOriginalState(QUndoCommand * parentCommand) {
	QUndoStack undoStack;
	QList<JEdge *> edges;
	addToUndo(parentCommand, edges);
	undoStack.push(parentCommand);
	undoStack.undo();
}

void JRouter::addToUndo(Wire * wire, QUndoCommand * parentCommand) {
	if (!wire->getAutoroutable()) {
		// it was here before the autoroute, so don't add it again
		return;
	}

	AddItemCommand * addItemCommand = new AddItemCommand(m_sketchWidget, BaseCommand::SingleView, ModuleIDNames::wireModuleIDName, wire->viewLayerSpec(), wire->getViewGeometry(), wire->id(), false, -1, parentCommand);
	new CheckStickyCommand(m_sketchWidget, BaseCommand::SingleView, wire->id(), false, CheckStickyCommand::RemoveOnly, parentCommand);
	
	new WireWidthChangeCommand(m_sketchWidget, wire->id(), wire->width(), wire->width(), parentCommand);
	new WireColorChangeCommand(m_sketchWidget, wire->id(), wire->colorString(), wire->colorString(), wire->opacity(), wire->opacity(), parentCommand);
	addItemCommand->turnOffFirstRedo();
}

void JRouter::addToUndo(QUndoCommand * parentCommand, QList<JEdge *> & edges) 
{
	QList<Wire *> wires;
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
	}

	foreach (JEdge * edge, edges) {	
		JumperItem * jumperItem = edge->jumperItem;
		if (jumperItem == NULL) continue;

		jumperItem->saveParams();
		QPointF pos, c0, c1;
		jumperItem->getParams(pos, c0, c1);

		new AddItemCommand(m_sketchWidget, BaseCommand::CrossView, ModuleIDNames::jumperModuleIDName, jumperItem->viewLayerSpec(), jumperItem->getViewGeometry(), jumperItem->id(), false, -1, parentCommand);
		new ResizeJumperItemCommand(m_sketchWidget, jumperItem->id(), pos, c0, c1, pos, c0, c1, parentCommand);
		new CheckStickyCommand(m_sketchWidget, BaseCommand::SingleView, jumperItem->id(), false, CheckStickyCommand::RemoveOnly, parentCommand);

		m_sketchWidget->createWire(jumperItem->connector0(), edge->from, ViewGeometry::NoFlag, false, BaseCommand::CrossView, parentCommand);
		m_sketchWidget->createWire(jumperItem->connector1(), edge->to, ViewGeometry::NoFlag, false, BaseCommand::CrossView, parentCommand);

	}

	addUndoConnections(m_sketchWidget, true, wires, parentCommand);
}

void JRouter::addUndoConnections(PCBSketchWidget * sketchWidget, bool connect, QList<Wire *> & wires, QUndoCommand * parentCommand) 
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
												ViewLayer::specFromID(wire->viewLayerID()),
												connect, parentCommand);
			ccc->setUpdateConnections(false);
		}
		ConnectorItem * connector0 = wire->connector0();
		foreach (ConnectorItem * toConnectorItem, connector0->connectedToItems()) {
			ChangeConnectionCommand * ccc = new ChangeConnectionCommand(sketchWidget, BaseCommand::SingleView, toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
												wire->id(), connector0->connectorSharedID(),
												ViewLayer::specFromID(wire->viewLayerID()),
												connect, parentCommand);
			ccc->setUpdateConnections(false);
		}
	}
}

TraceWire * JRouter::drawOneTrace(QPointF fromPos, QPointF toPos, int width, ViewLayer::ViewLayerSpec viewLayerSpec)
{
	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	viewGeometry.setLoc(fromPos);
	QLineF line(0, 0, toPos.x() - fromPos.x(), toPos.y() - fromPos.y());
	viewGeometry.setLine(line);
	viewGeometry.setTrace(true);
	viewGeometry.setAutoroutable(true);

	ItemBase * trace = m_sketchWidget->addItem(m_sketchWidget->paletteModel()->retrieveModelPart(ModuleIDNames::wireModuleIDName), 
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

	// TODO: for debugging
	ProcessEventBlocker::processEvents();

	return traceWire;
}

void JRouter::clearEdges(QList<JEdge *> & edges) {
	foreach (JEdge * edge, edges) {
		if (edge->jumperItem) {
			m_sketchWidget->deleteItem(edge->jumperItem->id(), true, false, false);
		}
		delete edge;
	}
	edges.clear();
}

void JRouter::doCancel(QUndoCommand * parentCommand) {
	clearTraces(m_sketchWidget, false, NULL);
	restoreOriginalState(parentCommand);
	cleanUp();
}

void JRouter::updateProgress(int num, int denom) 
{
	emit setProgressValue((int) MaximumProgress * (m_currentProgressPart + (num / (qreal) denom)) / (qreal) m_maximumProgressPart);
}

JSubedge * JRouter::makeSubedge(JEdge * edge, QPointF p1, ConnectorItem * from, Wire * fromWire, QPointF p2, bool forward) 
{
	JSubedge * subedge = new JSubedge;
	subedge->edge = edge;
	subedge->fromConnectorItem = from;
	subedge->toConnectorItem = NULL;
	subedge->fromWire = fromWire;
	subedge->toWire = NULL;
	subedge->distance = (p1.x() - p2.x()) * (p1.x() - p2.x()) + (p1.y() - p2.y()) * (p1.y() - p2.y());	
	subedge->fromPoint = p1;
	subedge->toPoint = p2;
	subedge->forward = forward;
	return subedge;
}

Tile * JRouter::addTile(NonConnectorItem * nci, int type, Plane * thePlane, QList<Tile *> & alreadyTiled) 
{
	QRectF r = nci->attachedTo()->mapRectToScene(nci->rect());
	TileRect tileRect;
	realsToTile(tileRect, r.left(), r.top(), r.right(), r.bottom());
	DebugDialog::debug(QString("   add tile %1").arg((long) nci, 0, 16), r);
	return insertTile(thePlane, tileRect, alreadyTiled, nci, type, false);
}

int prepDeleteTile(Tile * tile, UserData data) {
	switch(TiGetType(tile)) {
		case DUMMYLEFT:
		case DUMMYRIGHT:
		case DUMMYTOP:
		case DUMMYBOTTOM:
			return 0;
	}

	//DebugDialog::debug(QString("tile %1 %2 %3 %4").arg(LEFT(tile)).arg(YMIN(tile)).arg(RIGHT(tile)).arg(YMAX(tile)));
	QSet<Tile *> * tiles = (QSet<Tile *> *) data;
	tiles->insert(tile);

	return 0;
}


void JRouter::hideTiles() 
{
	foreach (QGraphicsItem * item, m_sketchWidget->items()) {
		GridEntry * gridEntry = dynamic_cast<GridEntry *>(item);
		if (gridEntry) gridEntry->setVisible(false);
	}
}

void JRouter::clearTiles(Plane * thePlane) 
{
	if (thePlane == NULL) return;

	foreach (QGraphicsItem * item, m_sketchWidget->items()) {
		GridEntry * gridEntry = dynamic_cast<GridEntry *>(item);
		if (gridEntry) delete gridEntry;
	}

	QSet<Tile *> tiles;
	TileRect tileRect;
	realsToTile(tileRect, m_maxRect.left(), m_maxRect.top(),  m_maxRect.right(), m_maxRect.bottom());
	TiSrArea(NULL, thePlane, &tileRect, prepDeleteTile, &tiles);
	foreach (Tile * tile, tiles) {
		TiFree(tile);
	}

	TiFreePlane(thePlane);
}

void JRouter::displayBadTiles(QList<Tile *> & alreadyTiled) {
	foreach (Tile * tile, alreadyTiled) {
		qreal x1 = tileToReal(LEFT(tile));
		qreal y1 = tileToReal(YMIN(tile));
		qreal x2 = tileToReal(RIGHT(tile));
		qreal y2 = tileToReal(YMAX(tile));
			
		drawGridItem(x1, y1, x2, y2, GridEntry::BLOCK, dynamic_cast<GridEntry *>(TiGetClient(tile)));
	}
}

Tile * JRouter::insertTile(Plane * thePlane, TileRect & tileRect, QList<Tile *> & alreadyTiled, QGraphicsItem * item, int type, bool clip) 
{
	DebugDialog::debug(QString("insert tile xmin:%1 xmax:%2 ymin:%3 ymax:%4").
		arg(tileRect.xmini).arg(tileRect.xmaxi).arg(tileRect.ymini).arg(tileRect.ymaxi));

	TiSrArea(NULL, thePlane, &tileRect, checkAlready, &alreadyTiled);
	if (alreadyTiled.count() > 0) {
		if (clip) {
			clipInsertTile(thePlane, tileRect, alreadyTiled, item, type);
			if (alreadyTiled.count() > 0) {
				DebugDialog::debug("!!!!!!!!!!!!!! wire overlaps !!!!!!!!!!!!!!!");
			}
			return NULL;
		}

		// overlap not allowed: no wiggle room
		return NULL;
	}

	Tile * tile = TiInsertTile(thePlane, &tileRect, item, type);
	handleChangedTiles(thePlane, tileRect);
	return tile;
}

void JRouter::handleChangedTiles(Plane * thePlane, TileRect & tileRect) {
	// make a copy in case we make further changes
	QSet<Tile *> copyChangedTiles(ChangedTiles);
	ChangedTiles.clear();

	DebugDialog::debug(QString("tile %1 %2 %3 %4").arg(tileRect.xmini).arg(tileRect.ymini).arg(tileRect.xmaxi).arg(tileRect.ymaxi));

	handleChangedTilesAux(thePlane, copyChangedTiles);
	ChangedTiles.clear();
}

void JRouter::handleChangedTilesAux(Plane * thePlane, QSet<Tile *> & tiles) 
{
	foreach (Tile * theTile, tiles) {
		if (TiGetType(theTile) != SPACE) continue;
		if (YMAX(theTile) - YMIN(theTile) > realToTile(m_wireWidthNeeded)) continue;

		TileRect theTileRect;
		TiToRect(theTile, &theTileRect);

		DebugDialog::debug(QString("   changed tile %1 %2 %3 %4").arg(LEFT(theTile)).arg(YMIN(theTile)).arg(RIGHT(theTile)).arg(YMAX(theTile)));
		
		bool didInsert = false;
		int newRight = RIGHT(theTile); 
		for (Tile * tp = RT(theTile); RIGHT(tp) > LEFT(theTile); tp = BL(tp)) {
			if (TiGetType(tp) == SPACE || TiGetType(tp) == TINYSPACE) {
				DebugDialog::debug(QString("   using %1 %2 %3 %4").arg(LEFT(tp)).arg(YMIN(tp)).arg(RIGHT(tp)).arg(YMAX(tp)));
				if (RIGHT(tp) < newRight) {
					TileRect tinyRect;
					tinyRect.xmini = RIGHT(tp);
					tinyRect.xmaxi = newRight;
					tinyRect.ymini = theTileRect.ymini;
					tinyRect.ymaxi = theTileRect.ymaxi;
					TiInsertTile(thePlane, &tinyRect, NULL, TINYSPACE);
					didInsert = true;
					break;			// tiles will be restructured now; get out of the loop
				}
				newRight = LEFT(tp);
			}
		}
		if (!didInsert) {
			int newLeft = LEFT(theTile); 
			for (Tile * tp = LB(theTile); LEFT(tp) < RIGHT(theTile); tp = TR(tp)) {
				if (TiGetType(tp) == SPACE || TiGetType(tp) == TINYSPACE) {
					DebugDialog::debug(QString("   using %1 %2 %3 %4").arg(LEFT(tp)).arg(YMIN(tp)).arg(RIGHT(tp)).arg(YMAX(tp)));
					if (LEFT(tp) > newLeft) {
						TileRect tinyRect;
						tinyRect.xmini = newLeft;
						tinyRect.xmaxi = LEFT(tp);
						tinyRect.ymini = theTileRect.ymini;
						tinyRect.ymaxi = theTileRect.ymaxi;
						TiInsertTile(thePlane, &tinyRect, NULL, TINYSPACE);
						didInsert = true;
						break;			// tiles will be restructured now; get out of the loop
					}
					newLeft = RIGHT(tp);
				}
			}
		}

		if (!didInsert) {
			DebugDialog::debug("have to fix this one later");
		}
	
	}
}

Tile * JRouter::clipInsertTile(Plane * thePlane, TileRect & tileRect, QList<Tile *> & alreadyTiled, QGraphicsItem * item, int type) 
{
	DebugDialog::debug(QString("clip insert %1 %2 %3 %4").arg(tileRect.xmini).arg(tileRect.ymini).arg(tileRect.xmaxi).arg(tileRect.ymaxi));
	foreach (Tile * intersectingTile, alreadyTiled) {
		int type = TiGetType(intersectingTile);
		switch (type) {
			case NOTBOARD:
			case NONCONNECTOR:
			case PART:
				// overlap not allowed
				return NULL;
		}
	}

	QList<ConnectorItem *> equipotential;
	Wire * w = dynamic_cast<Wire *>(item);
	if (w) {
		equipotential.append(w->connector0());
	}
	else {
		ConnectorItem * ci = dynamic_cast<ConnectorItem *>(item);
		equipotential.append(ci);
	}
	ConnectorItem::collectEqualPotential(equipotential, false, ViewGeometry::NoFlag);
		
	foreach (Tile * intersectingTile, alreadyTiled) {
		QGraphicsItem * bodyItem = TiGetBody(intersectingTile);
		ConnectorItem * ci = dynamic_cast<ConnectorItem *>(bodyItem);
		if (ci != NULL) {
			if (!equipotential.contains(ci)) {
				// overlap not allowed
				TileRect tr;
				TiToRect(intersectingTile, &tr);
				DebugDialog::debug(QString("intersecting %1 %2 %3 %4").arg(tr.xmini).arg(tr.ymini).arg(tr.xmaxi).arg(tr.ymaxi));
				return NULL;
			}
		}
		else {
			Wire * w = dynamic_cast<Wire *>(bodyItem);
			if (!equipotential.contains(w->connector0())) {
				// overlap not allowed
				TileRect tr;
				TiToRect(intersectingTile, &tr);
				DebugDialog::debug(QString("intersecting %1 %2 %3 %4").arg(tr.xmini).arg(tr.ymini).arg(tr.xmaxi).arg(tr.ymaxi));
				return NULL;
			}
		}
	}

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
	}

	handleChangedTiles(thePlane, tileRect);

	alreadyTiled.clear();
	return NULL;
}

void JRouter::clearGridEntries() {
	foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
		GridEntry * gridEntry = dynamic_cast<GridEntry *>(item);
		if (gridEntry == NULL) continue;

		delete gridEntry;
	}
}

QPointF JRouter::findNearestSpace(Tile * tile, qreal widthNeeded, qreal heightNeeded, Plane * thePlane, const QPointF & nearPoint) 
{
	qreal l = tileToReal(LEFT(tile));
	qreal t = tileToReal(YMIN(tile));
	qreal r = tileToReal(RIGHT(tile));
	qreal b = tileToReal(YMAX(tile));

	if (b - t >= heightNeeded && r - l >= widthNeeded ) {
		qreal cy, cx;
		if (nearPoint.y() <= (b + t) / 2) {
			cy = qMax(nearPoint.y() - (heightNeeded / 2), t) + (heightNeeded / 2);
		}
		else {
			cy = qMin(nearPoint.y() + (heightNeeded / 2), b) - (heightNeeded / 2);
		}
		if (nearPoint.x() <= (r + l) / 2) {
			cx = qMax(nearPoint.x() - (widthNeeded / 2), l) + (widthNeeded / 2);
		}
		else {
			cy = qMin(nearPoint.x() + (widthNeeded / 2), r) - (widthNeeded / 2);
		}
		return QPointF(cx, cy);
	}
	else {
		// need to consider tiles above and below the originating tile
		// the potential region above and below being the amount left over from the height needed minus the height of this tile
		// so we collect all the empty tiles in that region
		// then we generate candidate rects by using the collected set of y coordinates, paired with the left and (right - needed width) x coordinates
		// then we make sure those candidates are actually all space tiles, and if so, compare the distance with the nearPoint

		int tWidthNeeded = realToTile(widthNeeded);
		int tHeightNeeded = realToTile(heightNeeded);
		TileRect searchRect;
		TiToRect(tile, &searchRect);
		searchRect.ymini = qMax(realToTile(m_maxRect.top()), YMIN(tile) - tHeightNeeded + (YMAX(tile) - YMIN(tile)));
		searchRect.ymaxi = qMin(realToTile(m_maxRect.bottom()), YMIN(tile) + tHeightNeeded);

		EmptyThing emptyThing;
		emptyThing.maxY = YMIN(tile);
		TiSrArea(tile, thePlane, &searchRect, collectXandY, &emptyThing);

		qreal bestD = std::numeric_limits<double>::max();
		qreal bestX, bestY;

		QList<QPoint> points;
		foreach (int y, emptyThing.y_s) {
			foreach (int x, emptyThing.x_s) {
				points.append(QPoint(x, y));
			}
			foreach (int x, emptyThing.x2_s) {
				points.append(QPoint(x - tWidthNeeded, y));
			}
		}
		foreach (QPoint p, points) {
			searchRect.xmini = p.x();
			searchRect.xmaxi = p.x() + tWidthNeeded;
			searchRect.ymini = p.y();
			searchRect.ymaxi = p.y() + tHeightNeeded;
			if (TiSrArea(tile, thePlane, &searchRect, allEmpty, NULL) == 0) {
				qreal cx = tileToReal(p.x()) + (widthNeeded / 2);
				qreal cy = tileToReal(p.y()) + (heightNeeded / 2);
				qreal d = (cx - nearPoint.x()) * (cx - nearPoint.x()) + (cy - nearPoint.y()) * (cy - nearPoint.y());
				if (d < bestD) {
					bestD = d;
					bestX = cx;
					bestY = cy;
				}
			}
		}

		return QPointF(bestX, bestY);
	}
}

void JRouter::reorderEdges(QList<JEdge *> & edges, QHash<Wire *, JEdge *> & tracesToEdges) {
	int ix = 0;
	while (ix < edges.count()) {
		JEdge * edge = edges.at(ix++);
		if (edge->routed) continue;
		if (edge->linkedEdge && edge->linkedEdge->routed) continue;

		int minIndex = edges.count();
		foreach (QGraphicsItem * item, m_sketchWidget->scene()->collidingItems(edge->vw)) {
			TraceWire * traceWire = dynamic_cast<TraceWire *>(item);
			if (traceWire == NULL) continue;

			JEdge * tedge = tracesToEdges.value(traceWire, NULL);
			if (tedge != NULL) {
				int tix = edges.indexOf(tedge);
				minIndex = qMin(tix, minIndex);
			}
		}

		if (minIndex < ix) {
			edges.removeOne(edge);
			edges.insert(minIndex, edge);
		}
	}
}
