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


// TODO:
//	backPropagate: 
//		tighten path between connectors once trace has succeeded?
//		turn corners into 45's?
//	wire bendpoint is not a blocker if wire is ownside
//	insert new traces
//	make DRC available from trace menu
//	schematic view: blocks parts, not traces
//	schematic view: come up with a max board size
//	fix up cancel/stop
//	placing jumpers: if double-sided, must make sure there is room on both sides
//	space finder needs to be tweaked?
//	use tile to place vias
//		tile both sides
//		for each onside tile
//			do jumper search, but goal is empty space that has overlapping tile on the other side
//				when such tile is found, do normal trace search on the other side from via to connector
//	option to turn off propagation feedback
//	deal with using traces as source and dest
//		why does start wire look screwy
//		figure out end wire point
//	remove debugging output and extra calls to processEvents
//	consider using lastTrace instead of lastTracePoint, then extend the wire
//	bugs: 
//		sometimes takes a longer route than expected; why?
//		off-by-one weirdness with rasterizer
//		weird slanted line in one trace with stepper-motor-unrouted example
//	new double-sided strategy:
//		collect all edges from both sides and expand them from both sides so there is still a single router pass
//		if there are jumpers at the end then ripup edges to there and move that edge upward
//	need to put a border no-go area around the board
//	need to rethink border outline?
//	rethink dealing with non-manhattan wires?
//	still seeing a few thin tiles going across the board
//	put in trace connectors, but make the tile size small


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
static const qreal FloatingPointFudge = .001;
static qreal KeepoutSpace = 0;

static inline qreal TWODECIMALS(qreal d) { return qRound(d * 100) / 100.0; }

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

struct JumperItemStruct {
	JEdge * edge;
	ItemBase * partForBounds;
	JumperItem * jumperItem;
	ViewLayer::ViewLayerID fromViewLayerID;
	ViewLayer::ViewLayerID toViewLayerID;
	Plane * plane;
	bool deleted;
};

bool edgeLessThan(JEdge * e1, JEdge * e2)
{
	if (e1->ground == e2->ground) {
		return e1->distance < e2->distance;
	}

	return e2->ground;
}

bool subedgeLessThan(JSubedge * e1, JSubedge * e2)
{
	return e1->distance < e2->distance;
}

bool edgeGreaterThan(JEdge * e1, JEdge * e2)
{
	return e1->distance > e2->distance;
}

void tileToRect(Tile * tile, QRectF & rect) {
	TileRect tileRect;
	TiToRect(tile, &tileRect);
	rect.setCoords(tileRect.xmin, tileRect.ymin, tileRect.xmax, tileRect.ymax);
}

bool tileRectIntersects(TileRect * tile1, TileRect * tile2)
{
    qreal l1 = tile1->xmin;
    qreal r1 = tile1->xmin;
	qreal w1 = tile1->xmax - tile1->xmin;
    if (w1 < 0)
        l1 += w1;
    else
        r1 += w1;
    if (l1 == r1) // null rect
        return false;

    qreal l2 = tile2->xmin;
    qreal r2 = tile2->xmin;
	qreal w2 = tile2->xmax - tile2->xmin;
    if (w2 < 0)
        l2 += w2;
    else
        r2 += w2;
    if (l2 == r2) // null rect
        return false;

    if (l1 >= r2 || l2 >= r1)
        return false;

    qreal t1 = tile1->ymin;
    qreal b1 = tile1->ymin;
	qreal h1 = tile1->ymax - tile1->ymin;
    if (h1 < 0)
        t1 += h1;
    else
        b1 += h1;
    if (t1 == b1) // null rect
        return false;

    qreal t2 = tile2->ymin;
    qreal b2 = tile2->ymin;
	qreal h2 = tile2->ymax - tile2->ymin;
    if (h2 < 0)
        t2 += h2;
    else
        b2 += h2;
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

qreal wireXFromY(Wire * wire, qreal y, qreal xhint) {
	QPointF p1 = wire->pos();
	QPointF p2 = wire->line().p2() + p1;
	if (p1.x() == p2.x()) return p1.x();
	if (p1.y() == p2.y()) {
		if (xhint >= qMin(p1.x(), p2.x()) && xhint <= qMax(p1.x(), p2.x())) {
			return xhint;
		}
		if (qAbs(xhint - p1.x()) <= qAbs(xhint - p2.x())) {
			return p1.x();
		}
		return p2.x();
	}
	qreal dy = p2.y() - p1.y();
	qreal dx = p2.x() - p1.x();
	return ((y - p1.y()) * dx / dy) + p1.x(); 
}

qreal wireYFromX(Wire * wire, qreal x, qreal yhint) {
	QPointF p1 = wire->pos();
	QPointF p2 = wire->line().p2() + p1;
	if (p1.y() == p2.y()) return p1.y();
	if (p1.x() == p2.x()) {
		if (yhint >= qMin(p1.y(), p2.y()) && yhint <= qMax(p1.y(), p2.y())) {
			return yhint;
		}
		if (qAbs(yhint - p1.y()) <= qAbs(yhint - p2.y())) {
			return p1.y();
		}
		return p2.y();
	}
	qreal dy = p2.y() - p1.y();
	qreal dx = p2.x() - p1.x();
	return ((x - p1.x()) * dy / dx) + p1.y(); 
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
	KeepoutSpace = 0.01 * FSvgRenderer::printerScale();			// 10 mils space
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

	TiSetChangedFunc(CollectChangedTiles);
	TiSetFreeFunc(RemoveChangedTiles);

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
	// associate ConnectorItem with index
	QHash<ConnectorItem *, int> indexer;
	m_sketchWidget->collectAllNets(indexer, m_allPartConnectorItems, false, m_bothSidesNow);

	if (m_allPartConnectorItems.count() == 0) {
		return;
	}

	// will list connectors on both sides separately
	routingStatus.m_netCount = m_allPartConnectorItems.count();

	QList<JEdge *> edges;
	QVector<int> netCounters(m_allPartConnectorItems.count());
	m_viewLayerSpec = ViewLayer::Bottom;

	if (m_cancelled || m_stopTrace) {
		restoreOriginalState(parentCommand);
		cleanUp();
		return;
	}

	ProcessEventBlocker::processEvents(); // to keep the app  from freezing

	// TODO: if double-sided, tile both planes first and bail on drc overlap

	QList<JumperItemStruct *> jumperItemStructs;
	Plane * plane0 = runEdges(edges, jumperItemStructs, netCounters, routingStatus);
	clearEdges(edges);

	if (m_cancelled) {
		clearJumperItemStructs(jumperItemStructs);
		clearTiles(plane0);
		doCancel(parentCommand);
		return;
	}

	Plane * plane1 = NULL;
	if (m_bothSidesNow) {
		emit wantTopVisible();
		ProcessEventBlocker::processEvents();
		m_viewLayerSpec = ViewLayer::Top;
		m_currentProgressPart++;
		runEdges(edges, jumperItemStructs, netCounters, routingStatus);
		clearEdges(edges);
	}

	if (m_cancelled) {
		clearJumperItemStructs(jumperItemStructs);
		clearTiles(plane0);
		clearTiles(plane1);
		doCancel(parentCommand);
		return;
	}

	m_currentProgressPart++;
	fixupJumperItems(jumperItemStructs);

	cleanUp();

	addToUndo(parentCommand, jumperItemStructs);

	clearJumperItemStructs(jumperItemStructs);
	clearTiles(plane0);
	clearTiles(plane1);
	new CleanUpWiresCommand(m_sketchWidget, CleanUpWiresCommand::RedoOnly, parentCommand);

	m_sketchWidget->pushCommand(parentCommand);
	m_sketchWidget->repaint();
	DebugDialog::debug("\n\n\nautorouting complete\n\n\n");
}

Plane * JRouter::runEdges(QList<JEdge *> & edges, 
						   QList<struct JumperItemStruct *> & jumperItemStructs, 
						   QVector<int> & netCounters, RoutingStatus & routingStatus)
{
	ViewGeometry vg;
	vg.setTrace(true);
	ViewLayer::ViewLayerID viewLayerID = m_sketchWidget->getWireViewLayerID(vg, m_viewLayerSpec);

	collectEdges(netCounters, edges, viewLayerID);
	// sort the edges by distance
	qSort(edges.begin(), edges.end(), edgeLessThan);


	ItemBase * board = NULL;
	if (m_sketchWidget->autorouteNeedsBounds()) {
		board = m_sketchWidget->findBoard();
	}

	QList<Tile *> alreadyTiled;
	Plane * thePlane = tilePlane(board, viewLayerID, alreadyTiled);
	if (alreadyTiled.count() > 0) {
		m_cancelled = true;
		displayBadTiles(alreadyTiled);
		QMessageBox::warning(NULL, QObject::tr("Fritzing"), QObject::tr("Cannot autoroute: parts or traces are overlapping"));
		clearTiles(thePlane);
		return NULL;
	}

	int edgesDone = 0;
	foreach (JEdge * edge, edges) {		
		expand(edge->from, edge->fromConnectorItems, edge->fromTraces);
		expand(edge->to, edge->toConnectorItems, edge->toTraces);

		QPointF fp = edge->from->sceneAdjustedTerminalPoint(NULL);
		QPointF tp = edge->to->sceneAdjustedTerminalPoint(NULL);

		QList<JSubedge *> subedges;
		foreach (ConnectorItem * from, edge->fromConnectorItems) {
			QPointF p1 = from->sceneAdjustedTerminalPoint(NULL);
			subedges.append(makeSubedge(edge, p1, from, NULL, tp, /* edge->to */ NULL, true));
		}
		foreach (Wire * from, edge->fromTraces) {
			QPointF p1 = from->connector0()->sceneAdjustedTerminalPoint(NULL);
			QPointF p2 = from->connector1()->sceneAdjustedTerminalPoint(NULL);
			subedges.append(makeSubedge(edge, (p1 + p2) / 2, NULL, from, tp, /* edge->to */ NULL, true));
		}
		// reverse direction
		foreach (ConnectorItem * to, edge->toConnectorItems) {
			QPointF p1 = to->sceneAdjustedTerminalPoint(NULL);
			subedges.append(makeSubedge(edge, p1, to, NULL, fp, /* edge->from */ NULL, false));
		}
		foreach (Wire * to, edge->toTraces) {
			QPointF p1 = to->connector0()->sceneAdjustedTerminalPoint(NULL);
			QPointF p2 = to->connector1()->sceneAdjustedTerminalPoint(NULL);
			subedges.append(makeSubedge(edge, (p1 + p2) / 2, NULL, to, fp, /* edge->from */ NULL, false));
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

			routedFlag = traceSubedge(subedge, thePlane, board, viewLayerID);
			if (routedFlag) break;
		}

		foreach (JSubedge * subedge, subedges) {
			delete subedge;
		}
		subedges.clear();

		if (!routedFlag && !m_stopTrace) {
			if (m_sketchWidget->usesJumperItem()) {
				if (!alreadyJumper(jumperItemStructs, edge->from, edge->to)) {
					JumperItemStruct * jumperItemStruct = new JumperItemStruct;
					jumperItemStruct->jumperItem = NULL;
					jumperItemStruct->edge = edge;
					jumperItemStruct->partForBounds = board;
					jumperItemStruct->deleted = false;
					jumperItemStruct->plane = thePlane;
					jumperItemStructs.append(jumperItemStruct);
				}
			}
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

	foreach (JumperItemStruct * jumperItemStruct, jumperItemStructs) {
		edges.removeOne(jumperItemStruct->edge);
	}

	return thePlane;
}

Plane * JRouter::tilePlane(ItemBase * board, ViewLayer::ViewLayerID viewLayerID, QList<Tile *> & alreadyTiled) {
	Tile * bufferTile = TiAlloc();
	TiSetType(bufferTile, BUFFER);
	TiSetBody(bufferTile, NULL);

	if (board) {
		m_maxRect = board->boundingRect();
		m_maxRect.translate(board->pos());
	}
	else {
		m_maxRect = m_sketchWidget->scene()->itemsBoundingRect();
		m_maxRect.adjust(-m_maxRect.width() / 4, -m_maxRect.height() / 4, m_maxRect.width() / 4, m_maxRect.height() / 4);
	}

	QRectF bufferRect(m_maxRect);
	bufferRect.adjust(-m_maxRect.width(), -m_maxRect.height(), m_maxRect.width(), m_maxRect.height());

	LEFT(bufferTile) = bufferRect.left();
	YMIN(bufferTile) = bufferRect.top();		// TILE is Math Y-axis not computer-graphic Y-axis

	Plane * thePlane = TiNewPlane(bufferTile);

	RIGHT(bufferTile) = bufferRect.right();
	YMAX(bufferTile) = bufferRect.bottom();		// TILE is Math Y-axis not computer-graphic Y-axis


	TileRect boardRect;
	boardRect.xmin = m_maxRect.left();
	boardRect.xmax = m_maxRect.right();
	boardRect.ymin = m_maxRect.top();
	boardRect.ymax = m_maxRect.bottom();
	QList<Tile *> already;
	insertTile(thePlane, boardRect, already, NULL, SPACE, false);
	// if board is not rectangular, add tiles for the outside edges;

	if (board) {
		qreal factor = m_wireWidthNeeded;
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
		svg += "</svg>";
		GroundPlaneGenerator gpg;
		QList<QRect> rects;
		gpg.getBoardRects(svg, board, FSvgRenderer::printerScale() / factor, rects);
		QPointF boardPos = board->pos();
		foreach (QRect r, rects) {
			TileRect tileRect;
			tileRect.xmin = (r.left() * factor) + boardPos.x();
			tileRect.xmax = (r.right() * factor) + boardPos.x();
			tileRect.ymin = (r.top() * factor) + boardPos.y();		// TILE is Math Y-axis not computer-graphic Y-axis
			// note off-by-one weirdness
			tileRect.ymax = ((r.bottom() + 1) * factor) + boardPos.y();  
			//insertTile(thePlane, tileRect, alreadyTiled, NULL, NOTBOARD, false);
			if (alreadyTiled.count() > 0) {
				return thePlane;
			}
		}
	}

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

	return thePlane;
}

bool clipRect(TileRect * r, TileRect * clip, QList<TileRect> & rects) {
	if (!tileRectIntersects(r, clip)) return false;

	if (r->ymin < clip->ymin)
	{
		TileRect s;
		s.xmin = r->xmin;
		s.ymin = r->ymin; 
		s.xmax = r->xmax;
		s.ymax = clip->ymin;
		rects.append(s);
		r->ymin = clip->ymin;
	}
	if (r->ymax > clip->ymax)
	{
		TileRect s;
		s.xmin = r->xmin;
		s.ymin = clip->ymax;
		s.xmax = r->xmax;
		s.ymax = r->ymax;
		rects.append(s);
		r->ymax = clip->ymax;
	}
	if (r->xmin < clip->xmin)
	{
		TileRect s;
		s.xmin = r->xmin;
		s.ymin = r->ymin;
		s.xmax = clip->xmin;
		s.ymax = r->ymax;
		rects.append(s);
		r->xmin = clip->xmin;
	}
	if (r->xmax > clip->xmax)
	{
		TileRect s;
		s.xmin = clip->xmax;
		s.ymin = r->ymin;
		s.xmax = r->xmax;
		s.ymax = r->ymax;
		rects.append(s);
		r->xmax = clip->xmax;
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
		tileRect.xmin = c.x() - (w->width() / 2);
		tileRect.xmax = tileRect.xmin + w->width();
		tileRect.ymin = c.y() - (w->width() / 2);
		tileRect.ymax = tileRect.ymin + w->width(); 
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
			tileRect.xmin = qMin(p1.x(), p2.x()) - (w->width() / 2);
			tileRect.xmax = tileRect.xmin + w->width() + dx;
			tileRect.ymin = qMin(p1.y(), p2.y());
			tileRect.ymax = tileRect.ymin + dy;

			tileOneWire(thePlane, tileRect, alreadyTiled, w);
			if (alreadyTiled.count() > 0) break;
		}
		else if (dy < 1.0) {
			// horizontal line
			TileRect tileRect;
			tileRect.xmin = qMin(p1.x(), p2.x());
			tileRect.xmax = qMax(p1.x(), p2.x());
			tileRect.ymin = qMin(p1.y(), p2.y()) - (w->width() / 2);
			tileRect.ymax = tileRect.ymin + w->width() + dy;

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
			tileRect.xmin = r.left();
			tileRect.xmax = r.right();
			tileRect.ymin = r.top();
			tileRect.ymax = r.bottom();
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

void JRouter::fixupJumperItems(QList<JumperItemStruct *> & jumperItemStructs) {
	if (jumperItemStructs.count() <= 0) return;

	if (m_bothSidesNow) {
		// clear any jumpers that have been routed on the other side
		foreach (JumperItemStruct * jumperItemStruct, jumperItemStructs) {
			ConnectorItem * from = jumperItemStruct->edge->from;
			ConnectorItem * to = jumperItemStruct->edge->to;
			if (from->wiredTo(to, ViewGeometry::NotTraceFlags)) {
				jumperItemStruct->deleted = true;
			}
		}
	}

	int jumpersDone = 0;
	foreach (JumperItemStruct * jumperItemStruct, jumperItemStructs) {
		if (!jumperItemStruct->deleted) {
			drawJumperItem(jumperItemStruct);
		}

		updateProgress(++jumpersDone, jumperItemStructs.count());
	}
}

bool JRouter::traceSubedge(JSubedge* subedge, Plane * thePlane, ItemBase * partForBounds, ViewLayer::ViewLayerID viewLayerID) 
{
	Tile * tile = NULL;

	QList<Wire *> wires;
	tile = drawTrace(subedge, thePlane, viewLayerID, wires, false);	
	if (tile) {

		// TODO: handle wire stickyness


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
			subedge->fromConnectorItem = splitTrace(subedge->fromWire, subedge->fromPoint);
		}
		if (subedge->toConnectorItem == NULL) {
			subedge->toConnectorItem = splitTrace(subedge->toWire, subedge->toPoint);
		}

		// hook everyone up
		hookUpWires(subedge, wires, thePlane);
	}

	hideTiles();

	return (tile != NULL);
}

ConnectorItem * JRouter::splitTrace(Wire * wire, QPointF point) 
{
	// split the trace at point
	QLineF originalLine = wire->line();
	QLineF newLine(QPointF(0,0), point - wire->pos());
	wire->setLine(newLine);
	TraceWire * splitWire = drawOneTrace(point, originalLine.p2() + wire->pos(), wire->width(), m_viewLayerSpec);
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

	//if (partForBounds) {
		//splitWire->addSticky(partForBounds, true);
		//partForBounds->addSticky(splitWire, true);
	//}

	if (!wire->getAutoroutable()) {
		// TODO: deal with undo
	}

	return splitWire->connector0();
}


void JRouter::hookUpWires(JSubedge * subedge, QList<Wire *> & wires, Plane * thePlane) {
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
	tileWire(wires[0], thePlane, beenThere, alreadyTiled);
}

Tile * JRouter::drawTrace(JSubedge * subedge, Plane * thePlane, ViewLayer::ViewLayerID viewLayerID, QList<Wire *> & wires, bool forEmpty) 
{
	QList<Tile *> path;
	Tile * tile = NULL;
	bool result = propagate(subedge, path, thePlane, viewLayerID, forEmpty);
	if (result) {
		tile = path.last();
		result = backPropagate(subedge, path, thePlane, viewLayerID, wires, forEmpty);
		if (!result) tile = NULL;
	}

	// clear the cancel flag if it's been set so the next trace can proceed
	m_cancelTrace = false;
	return tile;
}

QPointF JRouter::drawLastNotVertical(const QPointF & startPoint, const QPointF & nextPoint, const QPointF & lastTracePoint, QList<Wire *> & wires) {
	if (startPoint.x() == lastTracePoint.x() && startPoint.x() == nextPoint.x()) return lastTracePoint;

	Wire * trace = drawOneTrace(lastTracePoint, startPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
	wires.append(trace);
	return startPoint;
}

QPointF JRouter::drawLastNotHorizontal(const QPointF & startPoint, const QPointF & nextPoint, const QPointF & lastTracePoint, QList<Wire *> & wires) {
	if (startPoint.y() == lastTracePoint.y() && startPoint.y() == nextPoint.y()) return lastTracePoint;

	Wire * trace = drawOneTrace(lastTracePoint, startPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
	wires.append(trace);
	return startPoint;
}


void JRouter::drawDirection(QPointF & startPoint, QPointF & lastTracePoint, QPointF & endPoint, QRectF & fromTileRect, QRectF & toTileRect, 
							QList<Wire *> & wires, ConnectorItem * terminalConnectorItem, Wire * terminalWire) 
{
	if (toTileRect.right() == fromTileRect.left()) {
		endPoint.setX(fromTileRect.left());
		drawDirectionHorizontal(startPoint, lastTracePoint, endPoint, fromTileRect, toTileRect, wires, terminalConnectorItem, terminalWire);
	}
	else if (toTileRect.left() == fromTileRect.right()) {
		endPoint.setX(fromTileRect.right());
		drawDirectionHorizontal(startPoint, lastTracePoint, endPoint, fromTileRect, toTileRect, wires, terminalConnectorItem, terminalWire);
	}
	else if (toTileRect.bottom() == fromTileRect.top()) {
		endPoint.setY(fromTileRect.top());
		drawDirectionVertical(startPoint, lastTracePoint, endPoint, fromTileRect, toTileRect, wires, terminalConnectorItem, terminalWire);
	}
	else if (toTileRect.top() == fromTileRect.bottom()) {
		endPoint.setY(fromTileRect.bottom());
		drawDirectionVertical(startPoint, lastTracePoint, endPoint, fromTileRect, toTileRect, wires, terminalConnectorItem, terminalWire);
	}
	else {
		// shouldn't happen
	}
}

void JRouter::drawDirectionVertical(QPointF & startPoint, QPointF & lastTracePoint, QPointF & endPoint, QRectF & fromTileRect, QRectF & toTileRect, 
									QList<Wire *> & wires, ConnectorItem * terminalConnectorItem, Wire * terminalWire) 
{
	qreal maxLeft = qMax(fromTileRect.left(), toTileRect.left()) + m_halfWireWidthNeeded;
	qreal minRight = qMin(fromTileRect.right(), toTileRect.right()) - m_halfWireWidthNeeded;
	qreal originalY = endPoint.y();

	bool normalX = true;
	if (terminalConnectorItem) {
		endPoint = terminalConnectorItem->sceneAdjustedTerminalPoint(NULL);
		normalX = (endPoint.x() < maxLeft || endPoint.x() > minRight);
		if (normalX) {
			DebugDialog::debug("end point jog");
		}
	}

	if (startPoint.y() == endPoint.y() && terminalWire == NULL) {
		// u-shape: need three lines

		if (normalX) {
			if (toTileRect.right() < startPoint.x()) {
				endPoint.setX(minRight);
			}
			else {
				endPoint.setX(maxLeft);
			}
		}
		QPointF midPoint1;
		midPoint1.setX(startPoint.x());
		midPoint1.setY(startPoint.y() == fromTileRect.top() ? startPoint.y() + m_halfWireWidthNeeded : startPoint.y() - m_halfWireWidthNeeded);
		startPoint = drawLastNotVertical(startPoint, midPoint1, lastTracePoint, wires);
		Wire * trace = drawOneTrace(startPoint, midPoint1, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
		wires.append(trace);
		QPointF midPoint2;
		midPoint2.setY(midPoint1.y());
		midPoint2.setX(endPoint.x());
		trace = drawOneTrace(midPoint1, midPoint2, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
		wires.append(trace);
		//trace = drawOneTrace(midPoint2, endPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
		//wires.append(trace);
		lastTracePoint = midPoint2;
		return;
	}

	bool isTurn = false;
	if (startPoint.x() == fromTileRect.left()) {
		isTurn = true;
		if (normalX) {
			endPoint.setX(maxLeft);
		}
	}
	else if (startPoint.x() == fromTileRect.right()) {
		isTurn = true;
		if (normalX) {
			endPoint.setX(minRight);
		}
	}
	if (isTurn) {
		if (terminalWire) {
			endPoint.setY(wireYFromX(terminalWire, endPoint.x(), startPoint.y()));
		}
		QPointF midPoint;
		midPoint.setY(startPoint.y());
		midPoint.setX(endPoint.x());
		startPoint = drawLastNotHorizontal(startPoint, midPoint, lastTracePoint, wires);
		Wire *trace = drawOneTrace(startPoint, midPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
		wires.append(trace);
		//trace = drawOneTrace(midPoint, endPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
		//wires.append(trace);
		lastTracePoint = midPoint;
		return;
	}

	if (startPoint.x() <= minRight && startPoint.x() >= maxLeft) {
		if (normalX) {
			endPoint.setX(startPoint.x());
		}
		if (terminalWire != NULL) {
			endPoint.setY(wireYFromX(terminalWire, endPoint.x(), startPoint.y()));
		}
		startPoint = drawLastNotVertical(startPoint, endPoint, lastTracePoint, wires);
		//Wire *trace = drawOneTrace(startPoint, endPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
		//wires.append(trace);
		lastTracePoint = startPoint;
		return;
	}

	if (normalX) {
		if (minRight < startPoint.x()) {
			endPoint.setX(minRight);
		}
		else {
			endPoint.setX(maxLeft);
		}
	}
	if (terminalWire) {
		endPoint.setY(wireYFromX(terminalWire, endPoint.x(), startPoint.y()));
	}

	QPointF midPoint1;
	midPoint1.setX(startPoint.x());
	midPoint1.setY((originalY + startPoint.y()) / 2);
	startPoint = drawLastNotVertical(startPoint, midPoint1, lastTracePoint, wires);
	Wire * trace = drawOneTrace(startPoint, midPoint1, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
	wires.append(trace);
	QPointF midPoint2;
	midPoint2.setX(endPoint.x());
	midPoint2.setY(midPoint1.y());
	trace = drawOneTrace(midPoint1, midPoint2, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
	wires.append(trace);
	//trace = drawOneTrace(midPoint2, endPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
	//wires.append(trace);
	lastTracePoint = midPoint2;
}

void JRouter::drawDirectionHorizontal(QPointF & startPoint, QPointF & lastTracePoint, QPointF & endPoint, QRectF & fromTileRect, QRectF & toTileRect, 
									  QList<Wire *> & wires, ConnectorItem * terminalConnectorItem, Wire * terminalWire) 
{
	qreal maxTop = qMax(fromTileRect.top(), toTileRect.top()) + m_halfWireWidthNeeded;
	qreal minBottom = qMin(fromTileRect.bottom(), toTileRect.bottom()) - m_halfWireWidthNeeded;
	qreal originalX = endPoint.x();

	bool normalY = true;
	if (terminalConnectorItem) {
		endPoint = terminalConnectorItem->sceneAdjustedTerminalPoint(NULL);
		normalY = (endPoint.y() < maxTop || endPoint.y() > minBottom);
		if (normalY) {
			DebugDialog::debug("end point jog");
		}
	}

	if (startPoint.x() == endPoint.x() && terminalWire == NULL) {
		// u-shape: need three lines

		if (normalY) {
			if (toTileRect.bottom() < startPoint.y()) {
				endPoint.setY(minBottom);
			}
			else {
				endPoint.setY(maxTop);
			}
		}
		QPointF midPoint1;
		midPoint1.setY(startPoint.y());
		midPoint1.setX(startPoint.x() == fromTileRect.left() ? startPoint.x() + m_halfWireWidthNeeded : startPoint.x() - m_halfWireWidthNeeded);
		startPoint = drawLastNotHorizontal(startPoint, midPoint1, lastTracePoint, wires);
		Wire * trace = drawOneTrace(startPoint, midPoint1, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
		wires.append(trace);
		QPointF midPoint2;
		midPoint2.setX(midPoint1.x());
		midPoint2.setY(endPoint.y());
		trace = drawOneTrace(midPoint1, midPoint2, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
		wires.append(trace);
		//trace = drawOneTrace(midPoint2, endPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
		//wires.append(trace);
		lastTracePoint = midPoint2;
		return;
	}

	bool isTurn = false;
	if (startPoint.y() == fromTileRect.top()) {
		isTurn = true;
		if (normalY) {
			endPoint.setY(maxTop);
		}
	}
	else if (startPoint.y() == fromTileRect.bottom()) {
		isTurn = true;
		if (normalY) {
			endPoint.setY(minBottom);
		}
	}
	if (isTurn) {
		if (terminalWire) {
			endPoint.setX(wireXFromY(terminalWire, endPoint.y(), startPoint.x()));
		}
		QPointF midPoint;
		midPoint.setX(startPoint.x());
		midPoint.setY(endPoint.y());
		startPoint = drawLastNotVertical(startPoint, midPoint, lastTracePoint, wires);
		Wire *trace = drawOneTrace(startPoint, midPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
		wires.append(trace);
		//trace = drawOneTrace(midPoint, endPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
		///wires.append(trace);
		lastTracePoint = midPoint;
		return;
	}

	if (startPoint.y() <= minBottom && startPoint.y() >= maxTop) {
		if (normalY) {
			endPoint.setY(startPoint.y());
		}
		if (terminalWire) {
			endPoint.setX(wireXFromY(terminalWire, endPoint.y(), startPoint.x()));
		}
		startPoint = drawLastNotHorizontal(startPoint, endPoint, lastTracePoint, wires);
		//Wire *trace = drawOneTrace(startPoint, endPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
		//wires.append(trace);
		lastTracePoint = startPoint;
		return;
	}

	if (normalY) {
		if (minBottom < startPoint.y()) {
			endPoint.setY(minBottom);
		}
		else {
			endPoint.setY(maxTop);
		}
	}
	if (terminalWire) {
		endPoint.setX(wireXFromY(terminalWire, endPoint.y(), startPoint.x()));
	}
	QPointF midPoint1;
	midPoint1.setY(startPoint.y());
	midPoint1.setX((startPoint.x() + originalX) / 2);
	startPoint = drawLastNotHorizontal(startPoint, midPoint1, lastTracePoint, wires);
	Wire * trace = drawOneTrace(startPoint, midPoint1, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
	wires.append(trace);
	QPointF midPoint2;
	midPoint2.setY(endPoint.y());
	midPoint2.setX(midPoint1.x());
	trace = drawOneTrace(midPoint1, midPoint2, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
	wires.append(trace);
	//trace = drawOneTrace(midPoint2, endPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
	//wires.append(trace);
	lastTracePoint = midPoint2;
}

bool enoughOverlapHorizontal(Tile* tile1, Tile* tile2, qreal widthNeeded) {
	return (qMin(RIGHT(tile1), RIGHT(tile2)) - qMax(LEFT(tile1), LEFT(tile2)) > widthNeeded - FloatingPointFudge);
}

bool enoughOverlapVertical(Tile* tile1, Tile* tile2, qreal widthNeeded) {
	// remember that axes are switched
	return (qMin(YMAX(tile1), YMAX(tile2)) - qMax(YMIN(tile1), YMIN(tile2)) > widthNeeded - FloatingPointFudge);
}

struct SeedTree {
	enum Direction {
		None = 0,
		Left,
		Up,
		Right,
		Down
	};
	
	Tile * seed;
	SeedTree * parent;
	Direction direction;
	int directionChanges;
	QList<SeedTree *> children;
};

bool JRouter::backPropagate(JSubedge * subedge, QList<Tile *> & path, Plane * thePlane, ViewLayer::ViewLayerID viewLayerID, QList<Wire *> & wires, bool forEmpty) {
	// TODO: handle wire as destination

	Q_UNUSED(thePlane);

	SeedTree * root = new SeedTree;
	SeedTree * destination = followPath(root, path);
	if (destination == NULL) {
		// shouldn't happen
		return false;
	}

	SeedTree * from = destination;
	// TODO: may be wire here
	QPointF startPoint = subedge->fromPoint;
	QPointF lastTracePoint = startPoint;
	while (from) {
		SeedTree * to = from->parent;
		QPointF endPoint;
		QRectF fromTileRect, toTileRect;
		tileToRect(from->seed, fromTileRect);

		if (to == NULL) {
			if (lastTracePoint != startPoint) {
				Wire * trace = drawOneTrace(lastTracePoint, startPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
				wires.append(trace);
			}
		}
		else {
			tileToRect(to->seed, toTileRect);
			ConnectorItem * terminalConnectorItem = NULL;
			Wire * terminalWire = NULL;
			if (to->parent == NULL && !forEmpty) {
				terminalConnectorItem = subedge->toConnectorItem;
				terminalWire = subedge->toWire;
			}
			drawDirection(startPoint, lastTracePoint, endPoint, fromTileRect, toTileRect, wires, terminalConnectorItem, terminalWire);
		}

		QGraphicsItem * item = TiGetClient(from->seed);
		if (item != NULL) item->setVisible(false);

		// TODO: process events just for debugging
		ProcessEventBlocker::processEvents();	

		startPoint = endPoint;
		from = to;
	}
			
	return true;
}

SeedTree * JRouter::followPath(SeedTree * & root, QList<Tile *> & path) {
	QList<SeedTree *> todoList;
	root->seed = path.last();
	root->parent = NULL;
	root->direction = SeedTree::None;
	root->directionChanges = 0;
	todoList.append(root);
	SeedTree * candidate = NULL;

	while (todoList.count() > 0) {
		SeedTree * currentSeedTree = todoList.takeFirst();
		//path.removeOne(currentSeedTree->seed);

		foreach (Tile * seed, path) {
			if (TiGetWave(seed) >= TiGetWave(currentSeedTree->seed)) {
				continue;
			}

			SeedTree::Direction direction = SeedTree::None;

			if (LEFT(seed) == RIGHT(currentSeedTree->seed)) {
				direction = SeedTree::Right;
				if (!enoughOverlapVertical(seed, currentSeedTree->seed, m_wireWidthNeeded)) {
					continue;
				}
			}
			else if (RIGHT(seed) == LEFT(currentSeedTree->seed)) {
				direction = SeedTree::Left;
				if (!enoughOverlapVertical(seed, currentSeedTree->seed, m_wireWidthNeeded)) {
					continue;
				}
			}
			else if (YMAX(seed) == YMIN(currentSeedTree->seed)) {
				direction = SeedTree::Down;
				if (!enoughOverlapHorizontal(seed, currentSeedTree->seed, m_wireWidthNeeded)) {
					continue;
				}
			}
			else if (YMIN(seed) == YMAX(currentSeedTree->seed)) {
				direction = SeedTree::Up;
				if (!enoughOverlapHorizontal(seed, currentSeedTree->seed, m_wireWidthNeeded)) {
					continue;
				}
			}
			else {
				continue;
			}

			SeedTree * newst = new SeedTree;
			newst->seed = seed;
			newst->direction = direction;
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


bool JRouter::propagate(JSubedge * subedge, QList<Tile *> & path, Plane* thePlane, ViewLayer::ViewLayerID viewLayerID, bool forEmpty) {

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

	Tile * firstTile = TiSrPoint(NULL, thePlane, subedge->fromPoint.x(), subedge->fromPoint.y());
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
		qreal x1 = LEFT(seed);
		qreal y1 = YMIN(seed);
		qreal x2 = RIGHT(seed);
		qreal y2 = YMAX(seed);
			
		short fof = checkCandidate(subedge, seed, viewLayerID, thePlane, forEmpty);
		QRectF r(x1, y1, x2 - x1, y2 - y1);
		DebugDialog::debug("=================", r);

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
			qreal x1 = LEFT(next);
			qreal y1 = YMIN(next);
			qreal x2 = RIGHT(next);
			qreal y2 = YMAX(next);
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
	if ((RIGHT(seed) < m_maxRect.right()) && (YMAX(seed) - YMIN(seed) > m_wireWidthNeeded - FloatingPointFudge)) {
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

	if ((LEFT(seed) > m_maxRect.left()) && (YMAX(seed) - YMIN(seed) > m_wireWidthNeeded - FloatingPointFudge)) {
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

	if ((YMAX(seed) < m_maxRect.bottom()) && (RIGHT(seed) - LEFT(seed) > m_wireWidthNeeded - FloatingPointFudge)) {	
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

	if ((YMIN(seed) > m_maxRect.top()) && (RIGHT(seed) - LEFT(seed) > m_wireWidthNeeded - FloatingPointFudge)) {		
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

short JRouter::checkCandidate(JSubedge * subedge, Tile * tile, ViewLayer::ViewLayerID viewLayerID, Plane * thePlane, bool forEmpty) 
{	
	switch (TiGetType(tile)) {
		case SPACE:
		case TINYSPACE:
			return checkSpace(subedge, tile, viewLayerID, thePlane, forEmpty);

		case CONNECTOR:
			if (!m_sketchWidget->autorouteCheckConnectors()) {
				return GridEntry::IGNORE;
			}
			return checkConnector(subedge, tile, viewLayerID, dynamic_cast<ConnectorItem *>(TiGetBody(tile)), forEmpty);

		case TRACE:
			if (!m_sketchWidget->autorouteCheckWires()) {
				return GridEntry::IGNORE;
			}

			return checkTrace(subedge, tile, viewLayerID, dynamic_cast<Wire *>(TiGetBody(tile)), forEmpty);

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
	QList<qreal> x_s;
	QList<qreal> x2_s;
	QList<qreal> y_s;
	qreal maxY;
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


short JRouter::checkSpace(JSubedge * subedge, Tile * tile, ViewLayer::ViewLayerID viewLayerID, Plane * thePlane, bool forEmpty) 
{
	Q_UNUSED(subedge);
	Q_UNUSED(viewLayerID);
	
	short result = TiGetType(tile) == SPACE ? GridEntry::EMPTY : GridEntry::TINY;

	if (!forEmpty) {
		return result;
	}

	TileRect tileRect;
	TiToRect(tile, &tileRect);

	QSizeF sizeNeeded = m_sketchWidget->jumperItemSize();
	qreal widthNeeded = sizeNeeded.width() + KeepoutSpace + KeepoutSpace;
	qreal heightNeeded = sizeNeeded.height() + KeepoutSpace + KeepoutSpace;
	if (tileRect.xmax - tileRect.xmin  < widthNeeded) {
		return result;
	}

	// need to figure out how to minimize the distance between this space and the previous wire;
	// need to save the set of open spaces found....

	if (tileRect.ymax - tileRect.ymin  >= heightNeeded) {
		DebugDialog::debug("empty GOAL");
		return GridEntry::GOAL;
	}

	TileRect searchRect = tileRect;
	searchRect.ymin = qMax(m_maxRect.top(), tileRect.ymin - heightNeeded + (tileRect.ymax - tileRect.ymin));
	searchRect.ymax = qMin(m_maxRect.bottom(), tileRect.ymin + heightNeeded);

	EmptyThing emptyThing;
	emptyThing.maxY = tileRect.ymin;
	TiSrArea(tile, thePlane, &searchRect, collectXandY, &emptyThing);

	foreach (qreal y, emptyThing.y_s) {
		foreach (qreal x, emptyThing.x_s) {
			searchRect.xmin = x;
			searchRect.xmax = x + widthNeeded;
			searchRect.ymin = y;
			searchRect.ymax = y + heightNeeded;
			if (TiSrArea(tile, thePlane, &searchRect, allEmpty, NULL) == 0) {
				return GridEntry::GOAL;
			}
		}
	}

	return result;
}

short JRouter::checkTrace(JSubedge * subedge, Tile * tile, ViewLayer::ViewLayerID viewLayerID, Wire * candidateWire, bool forEmpty) {
	Q_UNUSED(viewLayerID);
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



short JRouter::checkConnector(JSubedge * subedge, Tile * tile, ViewLayer::ViewLayerID viewLayerID, ConnectorItem * candidateConnectorItem, bool forEmpty) {
	Q_UNUSED(viewLayerID);
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


void JRouter::collectEdges(QVector<int> & netCounters, QList<JEdge *> & edges, ViewLayer::ViewLayerID viewLayerID) {

	for (int i = 0; i < m_allPartConnectorItems.count(); i++) {
		netCounters[i] = (m_allPartConnectorItems[i]->count() - 1) * 2;			// since we use two connectors at a time on a net
	}

	foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
		VirtualWire * vw = dynamic_cast<VirtualWire *>(item);
		if (vw == NULL) continue;

		ConnectorItem * from = vw->connector0()->firstConnectedToIsh();
		if (!m_sketchWidget->sameElectricalLayer2(viewLayerID, from->attachedToViewLayerID())) {
			from = from->getCrossLayerConnectorItem();
		}
		if (!from) {
			DebugDialog::debug("something's fishy 1");
			continue;
		}
		ConnectorItem * to = vw->connector1()->firstConnectedToIsh();
		if (!m_sketchWidget->sameElectricalLayer2(viewLayerID, to->attachedToViewLayerID())) {
			to = to->getCrossLayerConnectorItem();
		}
		if (!to) {
			DebugDialog::debug("something's fishy 2");
			continue;
		}
		JEdge * edge = new JEdge;
		edge->from = from;
		edge->to = to;
		QPointF pi = from->sceneAdjustedTerminalPoint(NULL);
		QPointF pj = to->sceneAdjustedTerminalPoint(NULL);
		double px = pi.x() - pj.x();
		double py = pi.y() - pj.y();
		edge->distance = (px * px) + (py * py);
		edge->ground = false;			// TODO: figure out which set of part connectors this belongs to and check isGrounded()
		edges.append(edge);
	}
}

void JRouter::expand(ConnectorItem * originalConnectorItem, QList<ConnectorItem *> & connectorItems, QSet<Wire *> & visited) 
{
	Bus * bus = originalConnectorItem->bus();
	if (bus == NULL) {
		connectorItems.append(originalConnectorItem);
	}
	else {
		originalConnectorItem->attachedTo()->busConnectorItems(bus, connectorItems);
	}

	// TODO: worry about side?

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

JumperItem * JRouter::drawJumperItem(JumperItemStruct * jumperItemStruct) 
{
	// TODO: check across planes...

	QPointF fp = jumperItemStruct->edge->from->sceneAdjustedTerminalPoint(NULL);
	QPointF tp = jumperItemStruct->edge->to->sceneAdjustedTerminalPoint(NULL);

	QList<JSubedge *> fromSubedges, toSubedges;
	foreach (ConnectorItem * from, jumperItemStruct->edge->fromConnectorItems) {
		QPointF p1 = from->sceneAdjustedTerminalPoint(NULL);
		fromSubedges.append(makeSubedge(jumperItemStruct->edge, p1, from, NULL, tp, /* jumperItemStruct->edge->to */ NULL, true));
	}
	foreach (Wire * from, jumperItemStruct->edge->fromTraces) {
		QPointF p1 = from->connector0()->sceneAdjustedTerminalPoint(NULL);
		QPointF p2 = from->connector1()->sceneAdjustedTerminalPoint(NULL);
		fromSubedges.append(makeSubedge(jumperItemStruct->edge, (p1 + p2) / 2, NULL, from, tp, /* jumperItemStruct->edge->to */ NULL, true));
	}
	// reverse direction
	foreach (ConnectorItem * to, jumperItemStruct->edge->toConnectorItems) {
		QPointF p1 = to->sceneAdjustedTerminalPoint(NULL);
		toSubedges.append(makeSubedge(jumperItemStruct->edge, p1, to, NULL, fp, /* jumperItemStruct->edge->from */ NULL, false));
	}
	foreach (Wire * to, jumperItemStruct->edge->toTraces) {
		QPointF p1 = to->connector0()->sceneAdjustedTerminalPoint(NULL);
		QPointF p2 = to->connector1()->sceneAdjustedTerminalPoint(NULL);
		toSubedges.append(makeSubedge(jumperItemStruct->edge, (p1 + p2) / 2, NULL, to, fp, /* jumperItemStruct->edge->from */ NULL, false));
	}

	DebugDialog::debug(QString("\n\nedge from %1 %2 %3 to %4 %5 %6, %7")
		.arg(jumperItemStruct->edge->from->attachedToTitle())
		.arg(jumperItemStruct->edge->from->attachedToID())
		.arg(jumperItemStruct->edge->from->connectorSharedID())
		.arg(jumperItemStruct->edge->to->attachedToTitle())
		.arg(jumperItemStruct->edge->to->attachedToID())
		.arg(jumperItemStruct->edge->to->connectorSharedID())
		.arg(jumperItemStruct->edge->distance) );

	QList<Wire *> fromWires;
	QList<Wire *> toWires;
	JSubedge * fromSubedge = NULL;
	JSubedge * toSubedge = NULL;
	Tile * fromTile = NULL;
	Tile * toTile = NULL;

	foreach (JSubedge * subedge, fromSubedges) {
		fromTile = drawTrace(subedge, jumperItemStruct->plane, jumperItemStruct->fromViewLayerID, fromWires, true); 
		if (fromTile) {
			fromSubedge = subedge;
			break;
		}
	}

	if (fromSubedge != NULL) {
		foreach (JSubedge * subedge, toSubedges) {
			toTile = drawTrace(subedge, jumperItemStruct->plane, jumperItemStruct->toViewLayerID, toWires, true);
			if (toTile) {
				toSubedge = subedge;
				break;
			}
		}
	}

	if (fromSubedge != NULL && toSubedge != NULL) {
		long newID = ItemBase::getNextID();
		ViewGeometry viewGeometry;
		ItemBase * itemBase = m_sketchWidget->addItem(m_sketchWidget->paletteModel()->retrieveModelPart(ModuleIDNames::jumperModuleIDName), 
												  jumperItemStruct->edge->from->attachedTo()->viewLayerSpec(), 
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

		fromDestPoint = findNearestSpace(fromTile, widthNeeded, heightNeeded, jumperItemStruct->plane, fromDestPoint);
		toDestPoint = findNearestSpace(toTile, widthNeeded, heightNeeded, jumperItemStruct->plane, toDestPoint); 
		jumperItem->resize(fromDestPoint, toDestPoint);

		if (jumperItemStruct->partForBounds) {
			jumperItem->addSticky(jumperItemStruct->partForBounds, true);
			jumperItemStruct->partForBounds->addSticky(jumperItem, true);
		}

		m_sketchWidget->scene()->addItem(jumperItem);
		fromSubedge->toConnectorItem = jumperItem->connector0();
		hookUpWires(fromSubedge, fromWires, jumperItemStruct->plane);

		toSubedge->toConnectorItem = jumperItem->connector1();
		hookUpWires(toSubedge, toWires, jumperItemStruct->plane);
		jumperItemStruct->jumperItem = jumperItem;
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

	return jumperItemStruct->jumperItem;
}

void JRouter::restoreOriginalState(QUndoCommand * parentCommand) {
	QUndoStack undoStack;
	QList<struct JumperItemStruct *> jumperItemStructs;
	addToUndo(parentCommand, jumperItemStructs);
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

void JRouter::addToUndo(QUndoCommand * parentCommand, QList<JumperItemStruct *> & jumperItemStructs) 
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

	foreach (JumperItemStruct * jumperItemStruct, jumperItemStructs) {	
		JumperItem * jumperItem = jumperItemStruct->jumperItem;
		if (jumperItem == NULL) continue;

		jumperItem->saveParams();
		QPointF pos, c0, c1;
		jumperItem->getParams(pos, c0, c1);

		new AddItemCommand(m_sketchWidget, BaseCommand::CrossView, ModuleIDNames::jumperModuleIDName, jumperItem->viewLayerSpec(), jumperItem->getViewGeometry(), jumperItem->id(), false, -1, parentCommand);
		new ResizeJumperItemCommand(m_sketchWidget, jumperItem->id(), pos, c0, c1, pos, c0, c1, parentCommand);
		new CheckStickyCommand(m_sketchWidget, BaseCommand::SingleView, jumperItem->id(), false, CheckStickyCommand::RemoveOnly, parentCommand);

		m_sketchWidget->createWire(jumperItem->connector0(), jumperItemStruct->edge->from, ViewGeometry::NoFlag, false, BaseCommand::CrossView, parentCommand);
		m_sketchWidget->createWire(jumperItem->connector1(), jumperItemStruct->edge->to, ViewGeometry::NoFlag, false, BaseCommand::CrossView, parentCommand);

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

void JRouter::reduceColinearWires(QList<Wire *> & wires)
{
	if (wires.count() < 2) return;

	for (int i = 0; i < wires.count() - 1; i++) {
		Wire * w0 = wires[i];
		Wire * w1 = wires[i + 1];

		QPointF fromPos = w0->connector0()->sceneAdjustedTerminalPoint(NULL);
		QPointF toPos = w1->connector1()->sceneAdjustedTerminalPoint(NULL);

		if (qAbs(fromPos.y() - toPos.y()) < .001 || qAbs(fromPos.x() - toPos.x()) < .001) {
			TraceWire * traceWire = drawOneTrace(fromPos, toPos, 5, w0->viewLayerSpec());
			if (traceWire == NULL)continue;

			m_sketchWidget->deleteItem(wires[i], true, false, false);
			m_sketchWidget->deleteItem(wires[i + 1], true, false, false);

			wires[i] = traceWire;
			wires.removeAt(i + 1);
			i--;								// don't forget to check the new wire
		}
	}
}

void JRouter::reduceWires(QList<Wire *> & wires, ConnectorItem * from, ConnectorItem * to, const QPolygonF & boundingPoly)
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

Wire * JRouter::reduceWiresAux(QList<Wire *> & wires, ConnectorItem * from, ConnectorItem * to, QPointF fromPos, QPointF toPos, const QPolygonF & boundingPoly)
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

	TraceWire * traceWire = drawOneTrace(fromPos, toPos, 5, m_viewLayerSpec);
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

			if (!m_sketchWidget->sameElectricalLayer2(candidateConnectorItem->attachedToViewLayerID(), traceWire->viewLayerID())) {
				// needs to be on the same layer
				continue;
			}

			intersects = true;
			break;
		}

		NonConnectorItem * nonConnectorItem = dynamic_cast<NonConnectorItem *>(item);
		if (nonConnectorItem) {
			if (dynamic_cast<ConnectorItem *>(item) == NULL) {
				intersects = true;
				break;
			}
		}
	}	
	if (intersects) {
		m_sketchWidget->deleteItem(traceWire, true, false, false);
		return NULL;
	}

	traceWire->setWireWidth(Wire::STANDARD_TRACE_WIDTH, m_sketchWidget);									// restore normal width
	return traceWire;
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
		delete edge;
	}
	edges.clear();
}

void JRouter::doCancel(QUndoCommand * parentCommand) {
	clearTraces(m_sketchWidget, false, NULL);
	restoreOriginalState(parentCommand);
	cleanUp();
}

bool JRouter::alreadyJumper(QList<struct JumperItemStruct *> & jumperItemStructs, ConnectorItem * from, ConnectorItem * to) {
	foreach (JumperItemStruct * jumperItemStruct, jumperItemStructs) {
		if (jumperItemStruct->edge->from == from && jumperItemStruct->edge->to == to) {
			return true;
		}
		if (jumperItemStruct->edge->to == from && jumperItemStruct->edge->from == to) {
			return true;
		}
	}

	if (m_bothSidesNow) {
		from = from->getCrossLayerConnectorItem();
		to = to->getCrossLayerConnectorItem();
		if (from != NULL && to != NULL) {
			foreach (JumperItemStruct * jumperItemStruct, jumperItemStructs) {
				if (jumperItemStruct->edge->from == from && jumperItemStruct->edge->to == to) {
					return true;
				}
				if (jumperItemStruct->edge->to == from && jumperItemStruct->edge->from == to) {
					return true;
				}
			}
		}
	}

	return false;
}

void JRouter::updateProgress(int num, int denom) 
{
	emit setProgressValue((int) MaximumProgress * (m_currentProgressPart + (num / (qreal) denom)) / (qreal) m_maximumProgressPart);
}

JSubedge * JRouter::makeSubedge(JEdge * edge, QPointF p1, ConnectorItem * from, Wire * fromWire, QPointF p2, ConnectorItem * to, bool forward) 
{
	JSubedge * subedge = new JSubedge;
	subedge->edge = edge;
	subedge->fromConnectorItem = from;
	subedge->toConnectorItem = to;
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
	QRectF r = nci->rect();
	QRectF r2 = nci->attachedTo()->mapRectToScene(r);
	TileRect tileRect;
	tileRect.xmin = r2.left();
	tileRect.xmax = r2.right();
	tileRect.ymin = r2.top();		// TILE is Math Y-axis not computer-graphic Y-axis
	tileRect.ymax = r2.bottom(); 
	DebugDialog::debug(QString("   add tile %1").arg((long) nci, 0, 16), r2);
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
	tileRect.xmax = m_maxRect.right();
	tileRect.xmin = m_maxRect.left();
	tileRect.ymax = m_maxRect.bottom();
	tileRect.ymin = m_maxRect.top();
	TiSrArea(NULL, thePlane, &tileRect, prepDeleteTile, &tiles);
	foreach (Tile * tile, tiles) {
		TiFree(tile);
	}

	TiFreePlane(thePlane);
}

void JRouter::displayBadTiles(QList<Tile *> & alreadyTiled) {
	foreach (Tile * tile, alreadyTiled) {
		qreal x1 = LEFT(tile);
		qreal y1 = YMIN(tile);
		qreal x2 = RIGHT(tile);
		qreal y2 = YMAX(tile);
			
		drawGridItem(x1, y1, x2, y2, GridEntry::BLOCK, dynamic_cast<GridEntry *>(TiGetClient(tile)));
	}
}

struct CheckAlreadyStruct {
	QGraphicsItem * item;
	int type;
	QList<Tile *> * alreadyTiled;
};

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

	CheckAlreadyStruct * checkAlreadyStruct = (CheckAlreadyStruct *) data;
	checkAlreadyStruct->alreadyTiled->append(tile);
	return 0;
}

Tile * JRouter::insertTile(Plane * thePlane, TileRect & tileRect, QList<Tile *> & alreadyTiled, QGraphicsItem * item, int type, bool clip) {
	tileRect.xmin = TWODECIMALS(tileRect.xmin);
	tileRect.xmax = TWODECIMALS(tileRect.xmax);
	tileRect.ymin = TWODECIMALS(tileRect.ymin);
	tileRect.ymax = TWODECIMALS(tileRect.ymax);

	DebugDialog::debug(QString("insert tile xmin:%1 xmax:%2 ymin:%3 ymax:%4").
		arg(tileRect.xmin).arg(tileRect.xmax).arg(tileRect.ymin).arg(tileRect.ymax));

	CheckAlreadyStruct checkAlreadyStruct;
	checkAlreadyStruct.item = item;
	checkAlreadyStruct.type = type;
	checkAlreadyStruct.alreadyTiled = &alreadyTiled;
	TiSrArea(NULL, thePlane, &tileRect, checkAlready, &checkAlreadyStruct);
	if (alreadyTiled.count() > 0) {
		if (clip) {
			clipInsertTile(thePlane, tileRect, alreadyTiled, item, type);
			if (alreadyTiled.count() > 0) {
				DebugDialog::debug("wire overlaps");
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

	foreach (Tile * changedTile, copyChangedTiles) {
		if (TiGetType(changedTile) != SPACE) continue;

		if (YMAX(changedTile) - YMIN(changedTile) > m_wireWidthNeeded - FloatingPointFudge) continue;

		TileRect changedTileRect;
		TiToRect(changedTile, &changedTileRect);

		DebugDialog::debug(QString("changed tile %1 %2 %3 %4").arg(LEFT(changedTile)).arg(YMIN(changedTile)).arg(RIGHT(changedTile)).arg(YMAX(changedTile)));
		
		Tile* leftAbove = NULL;
		Tile* leftBelow = NULL;
		Tile* rightAbove = NULL;
		Tile* rightBelow = NULL;
		if (changedTileRect.xmin < tileRect.xmin && changedTileRect.xmax > tileRect.xmin) {
			leftAbove = TiSrPoint(changedTile, thePlane, changedTileRect.xmin + FloatingPointFudge, changedTileRect.ymin - FloatingPointFudge);
			leftBelow = TiSrPoint(changedTile, thePlane, changedTileRect.xmin + FloatingPointFudge, changedTileRect.ymax + FloatingPointFudge);
		}
		if (changedTileRect.xmin < tileRect.xmax && changedTileRect.xmax > tileRect.xmax) {
			rightAbove = TiSrPoint(changedTile, thePlane, changedTileRect.xmax - FloatingPointFudge, changedTileRect.ymin - FloatingPointFudge);
			rightBelow = TiSrPoint(changedTile, thePlane, changedTileRect.xmax - FloatingPointFudge, changedTileRect.ymax + FloatingPointFudge);
		}

		if (leftAbove) {
			bool joinAbove = (TiGetType(leftAbove) == SPACE && LEFT(leftAbove) == changedTileRect.xmin && RIGHT(leftAbove) == tileRect.xmin);
			bool joinBelow = (TiGetType(leftBelow) == SPACE && LEFT(leftBelow) == changedTileRect.xmin && RIGHT(leftBelow) == tileRect.xmin);
			if (joinAbove || joinBelow) {
				TileRect tinyRect;
				tinyRect.xmin = tileRect.xmin;
				tinyRect.xmax = qMin(tileRect.xmax, changedTileRect.xmax);
				tinyRect.ymin = changedTileRect.ymin;
				tinyRect.ymax = changedTileRect.ymax;
				TiInsertTile(thePlane, &tinyRect, NULL, TINYSPACE);
			}
		}

		if (rightAbove) {
			bool joinAbove = (TiGetType(rightAbove) == SPACE && RIGHT(rightAbove) == changedTileRect.xmax && LEFT(rightAbove) == tileRect.xmax);
			bool joinBelow = (TiGetType(rightBelow) == SPACE && RIGHT(rightBelow) == changedTileRect.xmax && LEFT(rightBelow) == tileRect.xmax);
			if (joinAbove || joinBelow) {
				TileRect tinyRect;
				tinyRect.xmin = qMax(tileRect.xmin, changedTileRect.xmin);
				tinyRect.xmax = tileRect.xmax;
				tinyRect.ymin = changedTileRect.ymin;
				tinyRect.ymax = changedTileRect.ymax;
				TiInsertTile(thePlane, &tinyRect, NULL, TINYSPACE);
			}
		}
	}

	ChangedTiles.clear();
}

Tile * JRouter::clipInsertTile(Plane * thePlane, TileRect & tileRect, QList<Tile *> & alreadyTiled, QGraphicsItem * item, int type) 
{
	DebugDialog::debug(QString("clip insert %1 %2 %3 %4").arg(tileRect.xmin).arg(tileRect.ymin).arg(tileRect.xmax).arg(tileRect.ymax));
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
				DebugDialog::debug(QString("intersecting %1 %2 %3 %$").arg(tr.xmin).arg(tr.ymin).arg(tr.xmax).arg(tr.ymax));
				return NULL;
			}
		}
		else {
			Wire * w = dynamic_cast<Wire *>(bodyItem);
			if (!equipotential.contains(w->connector0())) {
				// overlap not allowed
				TileRect tr;
				TiToRect(intersectingTile, &tr);
				DebugDialog::debug(QString("intersecting %1 %2 %3 %4").arg(tr.xmin).arg(tr.ymin).arg(tr.xmax).arg(tr.ymax));
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

void JRouter::clearJumperItemStructs(QList<JumperItemStruct *> jumperItemStructs) {
	foreach (JumperItemStruct * jumperItemStruct, jumperItemStructs) {
		if (jumperItemStruct->jumperItem) {
			m_sketchWidget->deleteItem(jumperItemStruct->jumperItem->id(), true, false, false);
		}
		delete jumperItemStruct->edge;
		delete jumperItemStruct;
	}
	jumperItemStructs.clear();
}

QPointF JRouter::findNearestSpace(Tile * tile, qreal widthNeeded, qreal heightNeeded, Plane * thePlane, const QPointF & nearPoint) 
{
	TileRect tileRect;
	TiToRect(tile, &tileRect);

	if (tileRect.ymax - tileRect.ymin  >= heightNeeded && tileRect.xmax - tileRect.xmin  >= widthNeeded ) {
		qreal cy, cx;
		if (nearPoint.y() <= (tileRect.ymax + tileRect.ymin) / 2) {
			cy = qMax(nearPoint.y() - (heightNeeded / 2), tileRect.ymin) + (heightNeeded / 2);
		}
		else {
			cy = qMin(nearPoint.y() + (heightNeeded / 2), tileRect.ymax) - (heightNeeded / 2);
		}
		if (nearPoint.x() <= (tileRect.xmax + tileRect.xmin) / 2) {
			cx = qMax(nearPoint.x() - (widthNeeded / 2), tileRect.xmin) + (widthNeeded / 2);
		}
		else {
			cy = qMin(nearPoint.x() + (widthNeeded / 2), tileRect.xmax) - (widthNeeded / 2);
		}
		return QPointF(cx, cy);
	}
	else {
		TileRect searchRect = tileRect;
		searchRect.ymin = qMax(m_maxRect.top(), tileRect.ymin - heightNeeded + (tileRect.ymax - tileRect.ymin));
		searchRect.ymax = qMin(m_maxRect.bottom(), tileRect.ymin + heightNeeded);

		EmptyThing emptyThing;
		emptyThing.maxY = tileRect.ymin;
		TiSrArea(tile, thePlane, &searchRect, collectXandY, &emptyThing);

		qreal bestD = std::numeric_limits<double>::max();
		qreal bestX, bestY;

		QList<QPointF> points;
		foreach (qreal y, emptyThing.y_s) {
			foreach (qreal x, emptyThing.x_s) {
				points.append(QPointF(x, y));
			}
			foreach (qreal x, emptyThing.x2_s) {
				points.append(QPointF(x - widthNeeded, y));
			}
		}
		foreach (QPointF p, points) {
			searchRect.xmin = p.x();
			searchRect.xmax = p.x() + widthNeeded;
			searchRect.ymin = p.y();
			searchRect.ymax = p.y() + heightNeeded;
			if (TiSrArea(tile, thePlane, &searchRect, allEmpty, NULL) == 0) {
				qreal cx = p.x() + (widthNeeded / 2);
				qreal cy = p.y() + (heightNeeded / 2);
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

