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
//	in backpropagate, don't allow change of direction along short side
//		draw wires as feedback in all paths, count number of wires, erase paths we don't use?
//	backpropagate: tighten path between connectors once trace has succeeded?
//		turn corners into 45's?
//	wire bendpoint is not a blocker if wire is ownside
//	insert new traces
//	data structure handles DRC overlaps
//		make it available from trace menu
//	schematic view: blocks parts, not traces
//	schematic view: come up with a max board size
//	fix up cancel/stop
//	use tiles to place jumpers
//		foreach ownside tile
//			do normal trace search, except goal is an empty connector-sized region
//	use tile to place vias
//		tile both sides
//		for each onside tile
//			do jumper search, but goal is empty space that has overlapping tile on the other side
//				when such tile is found, do normal trace search on the other side from via to connector
//	option to turn off propagation feedback

#include "jrouter.h"
#include "../sketch/pcbsketchwidget.h"
#include "../debugdialog.h"
#include "../items/virtualwire.h"
#include "../items/tracewire.h"
#include "../items/jumperitem.h"
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
static const qreal MinWireSpace = 1.5;
static const qreal MinConnectorSpace = 1.0;

enum TileType {
	NOTBOARD = 1,
	NONCONNECTOR,
	TRACE,
	TRACECONNECTOR,
	CONNECTOR,
	PART,
	SPACE,
	GRIDALIGN
};

struct JumperItemStruct {
	ConnectorItem * from;
	ConnectorItem * to;
	ItemBase * partForBounds;
	QPolygonF boundingPoly;
	JumperItem * jumperItem;
	ViewLayer::ViewLayerID fromViewLayerID;
	ViewLayer::ViewLayerID toViewLayerID;
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

static int keepOut = 4;
static int boundingKeepOut = 4;

////////////////////////////////////////////////////////////////////

GridEntry::GridEntry(qreal x, qreal y, qreal w, qreal h, int wave, int flags, QGraphicsItem * parent) : QGraphicsRectItem(x, y, w, h, parent)
{
	setAcceptedMouseButtons(Qt::NoButton);
	setAcceptsHoverEvents(false);
	m_flags = flags;
	m_wave = wave;
}

////////////////////////////////////////////////////////////////////

JRouter::JRouter(PCBSketchWidget * sketchWidget)
{
	m_sketchWidget = sketchWidget;
	m_stopTrace = m_cancelTrace = m_cancelled = false;
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
	runEdges(edges, jumperItemStructs, netCounters, routingStatus);
	clearEdges(edges);

	if (m_cancelled) {
		doCancel(parentCommand);
		return;
	}

	if (m_bothSidesNow) {
		emit wantTopVisible();
		ProcessEventBlocker::processEvents();
		m_viewLayerSpec = ViewLayer::Top;
		m_currentProgressPart++;
		runEdges(edges, jumperItemStructs, netCounters, routingStatus);
		clearEdges(edges);
	}

	if (m_cancelled) {
		doCancel(parentCommand);
		return;
	}

	m_currentProgressPart++;
	fixupJumperItems(jumperItemStructs);

	cleanUp();


	addToUndo(parentCommand, jumperItemStructs);

	foreach (JumperItemStruct * jumperItemStruct, jumperItemStructs) {
		if (jumperItemStruct->jumperItem) {
			m_sketchWidget->deleteItem(jumperItemStruct->jumperItem->id(), true, false, false);
		}
		delete jumperItemStruct;
	}
	jumperItemStructs.clear();
	
	new CleanUpWiresCommand(m_sketchWidget, CleanUpWiresCommand::RedoOnly, parentCommand);

	m_sketchWidget->pushCommand(parentCommand);
	m_sketchWidget->repaint();
	DebugDialog::debug("\n\n\nautorouting complete\n\n\n");
}

void JRouter::runEdges(QList<JEdge *> & edges, 
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
		return;
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
			subedges.append(makeSubedge(edge, p1, from, tp, edge->to, true));
		}
		// reverse direction
		foreach (ConnectorItem * to, edge->toConnectorItems) {
			QPointF p1 = to->sceneAdjustedTerminalPoint(NULL);
			subedges.append(makeSubedge(edge, p1, to, fp, edge->from, false));
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
			if (!alreadyJumper(jumperItemStructs, edge->from, edge->to)) {
				if (m_sketchWidget->usesJumperItem()) {
					JumperItemStruct * jumperItemStruct = new JumperItemStruct();
					jumperItemStruct->jumperItem = NULL;
					jumperItemStruct->from = edge->from;
					jumperItemStruct->to = edge->to;
					jumperItemStruct->partForBounds = board;
					jumperItemStruct->boundingPoly = NULL;   // TODO: boundingPoly;
					jumperItemStruct->deleted = false;
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
			clearTiles(thePlane);
			return;
		}

		if (m_stopTrace) {
			break;
		}
	}

	clearTiles(thePlane);
}

Plane * JRouter::tilePlane(ItemBase * board, ViewLayer::ViewLayerID viewLayerID, QList<Tile *> & alreadyTiled) {
	Tile * boardTile = NULL;
	if (board) {
		boardTile = TiAlloc();

		TiSetBody(boardTile, board);
		TiSetType(boardTile, SPACE);
		m_maxRect = board->boundingRect();
		m_maxRect.translate(board->pos());

		LEFT(boardTile) = m_maxRect.left();
		BOTTOM(boardTile) = m_maxRect.top();		// TILE is Math Y-axis not computer-graphic Y-axis
	}

	Plane * thePlane = TiNewPlane(boardTile);
	if (boardTile) {
		RIGHT(boardTile) = m_maxRect.right();
		TOP(boardTile) = m_maxRect.bottom();		// TILE is Math Y-axis not computer-graphic Y-axis
	}

	// if board is not rectangular, add tiles for the outside edges;

	if (board) {
		qreal factor = Wire::STANDARD_TRACE_WIDTH;
		QHash<QString, SvgFileSplitter *> svgHash;
		QRectF boundingRect = board->boundingRect();
		QString svg = TextUtils::makeSVGHeader(FSvgRenderer::printerScale(), FSvgRenderer::printerScale(), boundingRect.width(), boundingRect.height());
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
			insertTile(thePlane, tileRect, alreadyTiled, NULL, NOTBOARD, false);
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

bool clipRect(QRectF & r, QRectF & clip, QList<QRectF> & rects) {
	if (!r.intersects(clip)) return false;

	if (r.top() < clip.top())
	{
		QRectF s(QPointF(r.left(), r.top()), QPointF(r.right(), clip.top()));
		rects.append(s);
		r.setTop(clip.top());
	}
	if (r.bottom() > clip.bottom())
	{
		QRectF s(QPointF(r.left(), clip.bottom()), QPointF(r.right(), r.bottom()));
		rects.append(s);
		r.setBottom(clip.bottom());
	}
	if (r.left() < clip.left())
	{
		QRectF s(QPointF(r.left(), r.top()), QPointF(clip.left(), r.bottom()));
		rects.append(s);
		r.setLeft(clip.left());
	}
	if (r.right() > clip.right())
	{
		QRectF s(QPointF(clip.right(), r.top()), QPointF(r.right(), r.bottom()));
		rects.append(s);
		r.setRight(clip.right());
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
			addTile(c0, TRACECONNECTOR, thePlane, alreadyTiled);
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
			QRectF r(p1.x() - (w->width() / 2), qMin(p1.y(), p2.y()), w->width(), dy);
			r.adjust(-MinWireSpace, -MinWireSpace, MinWireSpace, MinWireSpace);
			rects.append(r);
		}
		else if (dy < 1.0) {
			// horizontal line
			QRectF r(qMin(p1.x(), p2.x()), p1.y() - (w->width() / 2), dx, w->width());
			r.adjust(-MinWireSpace, -MinWireSpace, MinWireSpace, MinWireSpace);
			rects.append(r);
		}
		else {
			qreal angle = atan2(p2.y() - p1.y(), p2.x() - p1.x());
			if (dy >= dx) {
				sliceWireHorizontally(w, angle, p1, p2, rects);
			}
			else {
				sliceWireVertically(w, angle, p1, p2, rects);
			}
		}

		QList<ConnectorItem *> clipConnectorItems;
		foreach (ConnectorItem * connectorItem, w->connector0()->connectedToItems()) {
			clipConnectorItems.append(connectorItem);
		}
		foreach (ConnectorItem * connectorItem, w->connector1()->connectedToItems()) {
			clipConnectorItems.append(connectorItem);
		}
		clipConnectorItems.append(w->connector0());
		clipConnectorItems.append(w->connector1());
		QList<QRectF> clipRects;
		foreach (ConnectorItem * connectorItem, clipConnectorItems) {
			QRectF r = connectorItem->rect();
			QRectF clip = connectorItem->attachedTo()->mapRectToScene(r);
			clip.adjust(-MinConnectorSpace, -MinConnectorSpace, MinConnectorSpace, MinConnectorSpace);
			// ensure cliping rect goes to y pseudo-grid boundaries
			clip.setTop(qFloor(clip.top() / Wire::STANDARD_TRACE_WIDTH) * Wire::STANDARD_TRACE_WIDTH);
			clip.setBottom(qCeil(clip.bottom() / Wire::STANDARD_TRACE_WIDTH) * Wire::STANDARD_TRACE_WIDTH);
			bool redundant = false;
			for (int i = 0; i < clipRects.count(); i++) {
				if (clipRects[i] == clip) {
					redundant = true;
					break;
				}
				if (clipRects[i].contains(clip)) {
					redundant = true;
					break;
				}
				if (clip.contains(clipRects[i])) {
					clipRects.removeAt(i);
					break;
				}
			}
			if (redundant) continue;

			clipRects.append(clip);
		}

		int ix = 0;
		while (ix < rects.count()) {
			QRectF r = rects.at(ix++);
			bool clipped = false;
			foreach (QRectF clip, clipRects) {
				if (clipRect(r, clip, rects)) {
					clipped = true;
					break;
				}
			}
			if (clipped) continue;

			TileRect tileRect;
			tileRect.xmin = r.left();
			tileRect.xmax = r.right();
			tileRect.ymin = r.top();
			tileRect.ymax = r.bottom();
			DebugDialog::debug("tile wire", r);
			insertTile(thePlane, tileRect, alreadyTiled, w, TRACE, false);
			if (alreadyTiled.count() > 0) break;
		}
		if (alreadyTiled.count() > 0) break;
	}
}


void JRouter::sliceWireVertically(Wire * w, qreal angle, QPointF p1, QPointF p2, QList<QRectF> & rects) {
	// tiler gets confused when horizontally contiguous tiles are the same type, so join rects horizontally
	qreal x, y, xend, yend;
	qreal slantWidth = qAbs((w->width() + MinWireSpace + MinWireSpace) / cos(angle));
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
	qreal slantWidth = qAbs((w->width() + MinWireSpace + MinWireSpace) / sin(angle));
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
			ConnectorItem * from = jumperItemStruct->from;
			ConnectorItem * to = jumperItemStruct->to;
			if (from->wiredTo(to, ViewGeometry::NotTraceFlags)) {
				jumperItemStruct->deleted = true;
			}
		}
	}

	int jumpersDone = 0;
	foreach (JumperItemStruct * jumperItemStruct, jumperItemStructs) {
		if (!jumperItemStruct->deleted) {
			if (drawJumperItem(jumperItemStruct)) {
				m_sketchWidget->scene()->addItem(jumperItemStruct->jumperItem);

				TraceWire * traceWire = drawOneTrace(jumperItemStruct->jumperItem->connector0()->sceneAdjustedTerminalPoint(NULL), 
													 jumperItemStruct->from->sceneAdjustedTerminalPoint(NULL), 
													 Wire::STANDARD_TRACE_WIDTH, 
													 jumperItemStruct->from->attachedToViewLayerID() == ViewLayer::Copper0 ? ViewLayer::Bottom : ViewLayer::Top);
				traceWire->connector0()->tempConnectTo(jumperItemStruct->jumperItem->connector0(), true);
				jumperItemStruct->jumperItem->connector0()->tempConnectTo(traceWire->connector0(), true);
				traceWire->connector1()->tempConnectTo(jumperItemStruct->from, true);
				jumperItemStruct->from->tempConnectTo(traceWire->connector1(), true);

				traceWire = drawOneTrace(jumperItemStruct->jumperItem->connector1()->sceneAdjustedTerminalPoint(NULL), 
										 jumperItemStruct->to->sceneAdjustedTerminalPoint(NULL), 
										 Wire::STANDARD_TRACE_WIDTH, 
										 jumperItemStruct->to->attachedToViewLayerID() == ViewLayer::Copper0 ? ViewLayer::Bottom : ViewLayer::Top);
				traceWire->connector0()->tempConnectTo(jumperItemStruct->jumperItem->connector1(), true);
				jumperItemStruct->jumperItem->connector1()->tempConnectTo(traceWire->connector0(), true);
				traceWire->connector1()->tempConnectTo(jumperItemStruct->to, true);
				jumperItemStruct->to->tempConnectTo(traceWire->connector1(), true);
			}
		}

		updateProgress(++jumpersDone, jumperItemStructs.count());
	}
}

int deleteGridEntry(Tile * tile, UserData) {
	GridEntry * gridEntry = dynamic_cast<GridEntry *>(TiGetClient(tile));
	if (gridEntry == NULL) return 0;

	TiSetClient(tile, NULL);
	delete gridEntry;
	return 0;				// keep enumerating;
}

bool JRouter::traceSubedge(JSubedge* subedge, Plane * thePlane, ItemBase * partForBounds, ViewLayer::ViewLayerID viewLayerID) 
{
	bool routedFlag = false;

	TraceWire * splitWire = NULL;
	QLineF originalLine;
	if (subedge->from == NULL) {

		// TODO: starting from trace rather than connector

		/*
		// split the trace at subedge->point then restore it later
		originalLine = subedge->wire->line();
		QLineF newLine(QPointF(0,0), subedge->point - subedge->wire->pos());
		subedge->wire->setLine(newLine);
		splitWire = drawOneTrace(subedge->point, originalLine.p2() + subedge->wire->pos(), Wire::STANDARD_TRACE_WIDTH + 1, m_viewLayerSpec);
		from = splitWire->connector0();
		ProcessEventBlocker::processEvents();

		*/
	}

	QList<Wire *> wires;
	routedFlag = drawTrace(subedge, thePlane, viewLayerID, wires);	
	if (routedFlag) {
		//TODO: backtrace and convert QGraphicsRectItems into a set of wires
		//	on backtrace keep direction the same, 
		//		but if cleantype is noClean, then when you make a bend, 
		//		if all the intersecting grid regions contain a qgraphicsrect item
		//		then you can draw a straight line 

		// TODO: handle wire stickyness

		// TODO: backtrace to create a set of wires


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

		// hook everyone up
		if (wires.count() > 0) {
			subedge->from->tempConnectTo(wires[0]->connector0(), false);
			wires[0]->connector0()->tempConnectTo(subedge->from, false);
			int last = wires.count() - 1;
			subedge->to->tempConnectTo(wires[last]->connector1(), false);
			wires[last]->connector1()->tempConnectTo(subedge->to, false);
			for (int i = 0; i < last; i++) {
				ConnectorItem * c1 = wires[i]->connector1();
				ConnectorItem * c0 = wires[i + 1]->connector0();
				c1->tempConnectTo(c0, false);
				c0->tempConnectTo(c1, false);
			}
		}
	}


	// TODO: deal with routing from trace later

	/*
	if (subedge->wire != NULL) {
		if (routedFlag) {
			// hook up the split trace
			ConnectorItem * connector1 = subedge->wire->connector1();
			ConnectorItem * newConnector1 = splitWire->connector1();
			foreach (ConnectorItem * toConnectorItem, connector1->connectedToItems()) {
				connector1->tempRemove(toConnectorItem, false);
				toConnectorItem->tempRemove(connector1, false);
				newConnector1->tempConnectTo(toConnectorItem, false);
				toConnectorItem->tempConnectTo(newConnector1, false);
				if (partForBounds) {
					splitWire->addSticky(partForBounds, true);
					partForBounds->addSticky(splitWire, true);
				}
			}

			connector1->tempConnectTo(splitWire->connector0(), false);
			splitWire->connector0()->tempConnectTo(connector1, false);
		}
		else {
			// restore the old trace
			subedge->wire->setLine(originalLine);
			m_sketchWidget->deleteItem(splitWire, true, false, false);
		}
	}

	*/

	TileRect tileRect;
	tileRect.xmin = m_maxRect.left();
	tileRect.xmax = m_maxRect.right();
	tileRect.ymax = m_maxRect.bottom();
	tileRect.ymin = m_maxRect.top();
	TiSrArea(NULL, thePlane, &tileRect, deleteGridEntry, NULL);

	return routedFlag;
}

bool JRouter::drawTrace(JSubedge * subedge, Plane * thePlane, ViewLayer::ViewLayerID viewLayerID, QList<Wire *> & wires) 
{
	QList<Tile *> path;
	bool result = propagate(subedge, path, thePlane, viewLayerID);
	if (result) {
		backPropagate(subedge, path, thePlane, viewLayerID, wires);
	}

	// clear the cancel flag if it's been set so the next trace can proceed
	m_cancelTrace = false;
	return result;
}

struct SeedTree {
	Tile * seed;
	SeedTree * parent;
	QList<SeedTree *> children;
};

QPointF figureMidVertical(SeedTree * from, bool isDestination, QRectF & fromTileRect, QPointF & fromPoint, QPointF & toPoint) {
	if (isDestination) {
		return fromTileRect.center();
	}

	if (fromPoint.y() == TOP(from->seed) || fromPoint.y() == BOTTOM(from->seed)) {
		// in and out left;
		qreal x = (fromPoint.x() + toPoint.x()) / 2;
		qreal y = fromTileRect.center().y();
		return QPointF(x, y);
	}

	return QPointF(toPoint.x(), fromPoint.y());
}

QPointF figureMidHorizontal(SeedTree * from, bool isDestination, QRectF & fromTileRect, QPointF & fromPoint, QPointF & toPoint) {
	if (isDestination) {
		return fromTileRect.center();
	}

	if (fromPoint.x() == LEFT(from->seed) || fromPoint.x() == RIGHT(from->seed)) {
		// in and out left;
		qreal y = (fromPoint.y() + toPoint.y()) / 2;
		qreal x = fromTileRect.center().x();
		return QPointF(x, y);
	}

	return QPointF(fromPoint.x(), toPoint.y());
}


bool enoughOverlapHorizontal(Tile* tile1, Tile* tile2) {
	return (qMin(RIGHT(tile1), RIGHT(tile2)) - qMax(LEFT(tile1), LEFT(tile2)) > Wire::STANDARD_TRACE_WIDTH);
}

bool enoughOverlapVertical(Tile* tile1, Tile* tile2) {
	// remember that axes are switched
	return (qMin(TOP(tile1), TOP(tile2)) - qMax(BOTTOM(tile1), BOTTOM(tile2)) > Wire::STANDARD_TRACE_WIDTH);
}

bool JRouter::backPropagate(JSubedge * subedge, QList<Tile *> & path, Plane * thePlane, ViewLayer::ViewLayerID viewLayerID, QList<Wire *> & wires) {
	// TODO: handle wire as destination

	QList<SeedTree *> todoList;
	SeedTree * root = new SeedTree;
	root->seed = path.last();
	root->parent = NULL;
	todoList.append(root);
	SeedTree * destination = NULL;
	while (todoList.count() > 0 && destination == NULL) {
		SeedTree * currentSeedTree = todoList.takeFirst();

		// for now take the shortest path;

		foreach (Tile * seed, path) {
			if (TiGetWave(seed) != TiGetWave(currentSeedTree->seed) - 1) {
				continue;
			}

			if ((LEFT(seed) == RIGHT(currentSeedTree->seed)) ||
				(RIGHT(seed) == LEFT(currentSeedTree->seed))) 
			{
				if (!enoughOverlapVertical(seed, currentSeedTree->seed)) {
					continue;
				}
			}
			else if ((TOP(seed) == BOTTOM(currentSeedTree->seed)) ||
				     (BOTTOM(seed) == TOP(currentSeedTree->seed)))
			{
				if (!enoughOverlapHorizontal(seed, currentSeedTree->seed)) {
					continue;
				}
			}
			else {
				continue;
			}

			SeedTree * newst = new SeedTree;
			newst->seed = seed;
			newst->parent = currentSeedTree;
			todoList.append(newst);
			currentSeedTree->children.append(newst);
			if (TiGetWave(seed) == 0) {
				destination = newst;
				break;
			}
		}
	}

	SeedTree * from = destination;
	QPointF fromPoint;
	while (from) {
		SeedTree * to = from->parent;

		QPointF midPoint, toPoint;
		QRectF fromTileRect, toTileRect;
		tileToRect(from->seed, fromTileRect);
		if (to) {
			tileToRect(to->seed, toTileRect);
		}

		if (to == NULL) {
			// assume center for now; if it's a wire tile that may be wrong
			midPoint = fromTileRect.center();
		}
		else {
			// find the center of the overlap
			if (RIGHT(to->seed) == LEFT(from->seed)) {
				toPoint.setX(fromTileRect.left());
				qreal mid = qMax(fromTileRect.top(), toTileRect.top()) + qMin(fromTileRect.bottom(), toTileRect.bottom());
				toPoint.setY(mid / 2);
				midPoint = figureMidHorizontal(from, from == destination, fromTileRect, fromPoint, toPoint);
			}
			else if (LEFT(to->seed) == RIGHT(from->seed)) {
				toPoint.setX(fromTileRect.right());
				qreal mid = qMax(fromTileRect.top(), toTileRect.top()) + qMin(fromTileRect.bottom(), toTileRect.bottom());
				toPoint.setY(mid / 2);
				midPoint = figureMidHorizontal(from, from == destination, fromTileRect, fromPoint, toPoint);
			}
			else if (TOP(to->seed) == BOTTOM(from->seed)) {
				toPoint.setY(fromTileRect.top());
				qreal mid = qMax(fromTileRect.left(), toTileRect.left()) + qMin(fromTileRect.right(), toTileRect.right());
				toPoint.setX(mid / 2);
				midPoint = figureMidVertical(from, from == destination, fromTileRect, fromPoint, toPoint);
			}
			else if (BOTTOM(to->seed) == TOP(from->seed)) {
				toPoint.setY(fromTileRect.bottom());
				qreal mid = qMax(fromTileRect.left(), toTileRect.left()) + qMin(fromTileRect.right(), toTileRect.right());
				toPoint.setX(mid / 2);
				midPoint = figureMidVertical(from, from == destination, fromTileRect, fromPoint, toPoint);
			}
			else {
				// shouldn't happen
			}




		}

		if (from != destination) {
			TraceWire * trace = drawOneTrace(fromPoint, midPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
			wires.append(trace);
		}
		if (to != NULL) {
			TraceWire * trace = drawOneTrace(midPoint, toPoint, Wire::STANDARD_TRACE_WIDTH, m_viewLayerSpec);
			wires.append(trace);
		}

		QGraphicsItem * item = TiGetClient(from->seed);
		if (item != NULL) item->setVisible(false);

		// TODO: process events just for debugging
		ProcessEventBlocker::processEvents();	

		fromPoint = toPoint;
		from = to;
	}
	
	foreach (Wire * wire, wires) {
		delete wire;
	}
	wires.clear();
		
	return true;
}

bool JRouter::propagate(JSubedge * subedge, QList<Tile *> & path, Plane* thePlane, ViewLayer::ViewLayerID viewLayerID) {

	DebugDialog::debug("((((((((((((((((((((((((((((");
		DebugDialog::debug(QString("starting from connectoritem %1 %2 %3 %4 %5")
								.arg(subedge->from->connectorSharedID())
								.arg(subedge->from->connectorSharedName())
								.arg(subedge->from->attachedToTitle())
								.arg(subedge->from->attachedToID())
								.arg(subedge->from->attachedToInstanceTitle())
								);

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
		if (gridEntry != NULL) {
			continue;				
		}

		// TILE math reverses y-axis!
		qreal x1 = LEFT(seed);
		qreal y1 = BOTTOM(seed);
		qreal x2 = RIGHT(seed);
		qreal y2 = TOP(seed);
			
		short fof = checkCandidate(subedge, seed, viewLayerID);
		QRectF r(x1, y1, x2 - x1, y2 - y1);
		DebugDialog::debug("=================", r);

		DebugDialog::debug(QString("wave:%1 fof:%2").arg(TiGetWave(seed)).arg(fof), r);
		gridEntry = drawGridItem(x1, y1, x2, y2, TiGetWave(seed), fof);
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

void JRouter::appendIf(Tile * seed, Tile * next, QList<Tile *> & seeds, bool (*enoughOverlap)(Tile*, Tile*)) {
	
	if (TiGetClient(next) != NULL) {
		return;			// already visited
	}

	if (TiGetType(next) == NOTBOARD) {
		if (TiGetClient(next) == NULL) {
			qreal x1 = LEFT(next);
			qreal y1 = BOTTOM(next);
			qreal x2 = RIGHT(next);
			qreal y2 = TOP(next);
			drawGridItem(x1, y1, x2, y2, 0, GridEntry::NOTBOARD);
		}

		return;		// outside board boundaries
	}

	if (!enoughOverlap(seed, next)) {
		return;	// not wide/high enough 
	}

	TiSetWave(next, TiGetWave(seed) + 1);
	seeds.append(next);
}


void JRouter::seedNext(Tile * seed, QList<Tile *> & seeds) {
	if (TiGetType(seed) != GRIDALIGN && RIGHT(seed) < m_maxRect.right()) {
		Tile * next = TR(seed);
		appendIf(seed, next, seeds, enoughOverlapVertical);
		while (true) {
			next = LB(next);
			if (TOP(next) <= BOTTOM(seed)) {
				break;
			}

			appendIf(seed, next, seeds, enoughOverlapVertical);
		}
	}

	if (TiGetType(seed) != GRIDALIGN && LEFT(seed) > m_maxRect.left()) {
		Tile * next = BL(seed);
		appendIf(seed, next, seeds, enoughOverlapVertical);
		while (true) {
			next = RT(next);
			if (BOTTOM(next) >= TOP(seed)) {
				break;
			}

			appendIf(seed, next, seeds, enoughOverlapVertical);
		}
	}

	if (TOP(seed) < m_maxRect.bottom()) {		// reverse axis
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

	if (BOTTOM(seed) > m_maxRect.top()) {		// reverse axis
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

short JRouter::checkCandidate(JSubedge * subedge, Tile * tile, ViewLayer::ViewLayerID viewLayerID) 
{	
	switch (TiGetType(tile)) {
		case SPACE:
			return GridEntry::EMPTY;

		case GRIDALIGN:
			return GridEntry::ALIGN;

		case CONNECTOR:
			if (!m_sketchWidget->autorouteCheckConnectors()) {
				return GridEntry::IGNORE;
			}
			return checkConnector(subedge, tile, viewLayerID, dynamic_cast<ConnectorItem *>(TiGetBody(tile)));

		case TRACE:
			if (!m_sketchWidget->autorouteCheckWires()) {
				return GridEntry::IGNORE;
			}

			return checkTrace(subedge, tile, viewLayerID, dynamic_cast<Wire *>(TiGetBody(tile)));

		case TRACECONNECTOR:
			if (!m_sketchWidget->autorouteCheckWires()) {
				return GridEntry::IGNORE;
			}
			else {
				ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(TiGetBody(tile));
				return checkTrace(subedge, tile, viewLayerID, qobject_cast<Wire *>(connectorItem->attachedTo()));
			}

		case PART:
			if (!m_sketchWidget->autorouteCheckParts()) {
				return GridEntry::IGNORE;
			}

			return GridEntry::BLOCK;

		case NONCONNECTOR:
		case NOTBOARD:
			return GridEntry::BLOCK;

		default:
			// shouldn't happen:
			return GridEntry::IGNORE;
	}
}

short JRouter::checkTrace(JSubedge * subedge, Tile * tile, ViewLayer::ViewLayerID viewLayerID, Wire * candidateWire) {
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



short JRouter::checkConnector(JSubedge * subedge, Tile * tile, ViewLayer::ViewLayerID viewLayerID, ConnectorItem * candidateConnectorItem) {
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


	if (candidateConnectorItem == subedge->from) {
		DebugDialog::debug("SELF");
		return GridEntry::SELF;
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
			subedge->to = candidateConnectorItem;
			DebugDialog::debug("GOAL");
			return GridEntry::GOAL;			
		}
	}
	else {
		if (subedge->edge->fromConnectorItems.contains(candidateConnectorItem)) {
			subedge->to = candidateConnectorItem;
			DebugDialog::debug("GOAL");
			return GridEntry::GOAL;			
		}
	}

	DebugDialog::debug("BLOCK");

	return GridEntry::BLOCK;
}


GridEntry * JRouter::drawGridItem(qreal x1, qreal y1, qreal x2, qreal y2, int wave, short flag) 
{
	int alpha = 128;
	GridEntry * gridEntry = new GridEntry(x1, y1, x2 - x1, y2 - y1, wave, flag, NULL);
	gridEntry->setZValue(m_sketchWidget->getTopZ());

	QColor c;
	switch (flag) {
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
		case GridEntry::ALIGN:
			c = QColor(0, 0, 255, alpha);
			break;
	}

	gridEntry->setPen(c);
	gridEntry->setBrush(QBrush(c));
	m_sketchWidget->scene()->addItem(gridEntry);
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
	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	ItemBase * temp = m_sketchWidget->addItem(m_sketchWidget->paletteModel()->retrieveModelPart(ModuleIDNames::jumperModuleIDName), 
											  jumperItemStruct->from->attachedTo()->viewLayerSpec(), BaseCommand::SingleView, viewGeometry, newID, -1, NULL, NULL);
	if (temp == NULL) {
		// we're in trouble
		return NULL;
	}

	JumperItem * jumperItem = dynamic_cast<JumperItem *>(temp);

	QPointF candidate1;
	bool ok = findSpaceFor(jumperItemStruct->to, jumperItem, jumperItemStruct, candidate1);
	if (!ok) {
		m_sketchWidget->deleteItem(jumperItem, true, false, false);
		return NULL;
	}

	QPointF candidate0;
	ok = findSpaceFor(jumperItemStruct->from, jumperItem, jumperItemStruct, candidate0);
	if (!ok) {
		m_sketchWidget->deleteItem(jumperItem, true, false, false);
		return NULL;
	}

	jumperItem->resize(candidate0, candidate1);

	if (jumperItemStruct->partForBounds) {
		jumperItem->addSticky(jumperItemStruct->partForBounds, true);
		jumperItemStruct->partForBounds->addSticky(jumperItem, true);
	}

	jumperItemStruct->jumperItem = jumperItem;

	return jumperItem;
}

bool JRouter::findSpaceFor(ConnectorItem * & from, JumperItem * jumperItem, JumperItemStruct * jumperItemStruct, QPointF & candidate) 
{
	QSizeF jsz = jumperItem->footprintSize();
	QRectF fromR = from->rect();
	QPointF c = from->mapToScene(from->rect().center());
	qreal minRadius = (jsz.width() / 2) + (qSqrt((fromR.width() * fromR.width()) + (fromR.height() * fromR.height())) / 4) + 1;
	qreal maxRadius = minRadius * 5;

	QGraphicsEllipseItem * ellipse = NULL;
	QGraphicsLineItem * lineItem = NULL;

	// TODO: with double-sided routing, it's possible to range further away to find an empty spot
	// eventually could use a variant of maze-routing to find empty spots

	for (qreal radius = minRadius; radius <= maxRadius; radius += (minRadius / 2)) {
		for (int angle = 0; angle < 360; angle += 10) {
			if (m_cancelled || m_cancelTrace || m_stopTrace) {
				if (ellipse) delete ellipse;
				if (lineItem) delete lineItem;
				return false;
			}

			qreal radians = angle * 2 * M_PI / 360.0;
			candidate.setX(radius * cos(radians));
			candidate.setY(radius * sin(radians));
			candidate += c;
			if (!jumperItemStruct->boundingPoly.isEmpty()) {
				if (!jumperItemStruct->boundingPoly.containsPoint(candidate, Qt::OddEvenFill)) {
					continue;
				}

				bool inBounds = true;
				QPointF nearestBoundsIntersection;
				double nearestBoundsIntersectionDistance;
				QLineF l1(c, candidate);
				findNearestIntersection(l1, c,jumperItemStruct->boundingPoly, inBounds, nearestBoundsIntersection, nearestBoundsIntersectionDistance);
				if (!inBounds) {
					continue;
				}
			}

			// first look for a circular space
			if (ellipse == NULL) {
				ellipse = new QGraphicsEllipseItem(candidate.x() - (jsz.width() / 2), 
												   candidate.y() - (jsz.height() / 2), 
												   jsz.width(), jsz.height(), 
												   NULL, m_sketchWidget->scene());
			}
			else {
				ellipse->setRect(candidate.x() - (jsz.width() / 2), 
								 candidate.y() - (jsz.height() / 2), 
								 jsz.width(), jsz.height());
			}
			ProcessEventBlocker::processEvents();

			if (hasCollisions(jumperItem, ViewLayer::UnknownLayer, ellipse, NULL)) {
				continue;
			}

			if (lineItem == NULL) {
				lineItem = new QGraphicsLineItem(c.x(), c.y(), candidate.x(), candidate.y(), NULL, m_sketchWidget->scene());
				QPen pen = lineItem->pen();
				pen.setWidthF(Wire::STANDARD_TRACE_WIDTH + 1);
				pen.setCapStyle(Qt::RoundCap);
				lineItem->setPen(pen);
			}
			else {
				lineItem->setLine(c.x(), c.y(), candidate.x(), candidate.y());
			}
			ProcessEventBlocker::processEvents();
			
			if (!hasCollisions(jumperItem, from->attachedToViewLayerID(), lineItem, from)) {
				if (ellipse) delete ellipse;
				if (lineItem) delete lineItem;
				return true;
			}

			if (m_bothSidesNow) {
				ConnectorItem * from2 = from->getCrossLayerConnectorItem();
				if (from2) {
					if (!hasCollisions(jumperItem, from2->attachedToViewLayerID(), lineItem, from2)) {
						if (ellipse) delete ellipse;
						if (lineItem) delete lineItem;
						from = from2;
						return true;
					}
				}
			}
		}
	}

	if (ellipse) delete ellipse;
	if (lineItem) delete lineItem;
	return false;
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

		m_sketchWidget->createWire(jumperItem->connector0(), jumperItemStruct->from, ViewGeometry::NoFlag, false, BaseCommand::CrossView, parentCommand);
		m_sketchWidget->createWire(jumperItem->connector1(), jumperItemStruct->to, ViewGeometry::NoFlag, false, BaseCommand::CrossView, parentCommand);

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

void JRouter::findNearestIntersection(QLineF & l1, QPointF & fromPos, const QPolygonF & boundingPoly, bool & inBounds, QPointF & nearestBoundsIntersection, qreal & nearestBoundsIntersectionDistance) 
{
	inBounds = true;
	nearestBoundsIntersectionDistance = 0;
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
		if (jumperItemStruct->from == from && jumperItemStruct->to == to) {
			return true;
		}
		if (jumperItemStruct->to == from && jumperItemStruct->from == to) {
			return true;
		}
	}

	if (m_bothSidesNow) {
		from = from->getCrossLayerConnectorItem();
		to = to->getCrossLayerConnectorItem();
		if (from != NULL && to != NULL) {
			foreach (JumperItemStruct * jumperItemStruct, jumperItemStructs) {
				if (jumperItemStruct->from == from && jumperItemStruct->to == to) {
					return true;
				}
				if (jumperItemStruct->to == from && jumperItemStruct->from == to) {
					return true;
				}
			}
		}
	}

	return false;
}

bool JRouter::hasCollisions(JumperItem * jumperItem, ViewLayer::ViewLayerID viewLayerID, QGraphicsItem * lineOrEllipse, ConnectorItem * from) 
{
	foreach (QGraphicsItem * item, m_sketchWidget->scene()->collidingItems(lineOrEllipse)) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
		if (connectorItem != NULL) {
			if (connectorItem == from) continue;
			if (connectorItem->attachedTo() == jumperItem) continue;

			ItemBase * itemBase = connectorItem->attachedTo();
			if (m_sketchWidget->sameElectricalLayer2(itemBase->viewLayerID(), viewLayerID))
			{
				Wire * wire = dynamic_cast<Wire *>(itemBase);
				if (wire != NULL) {
					// handle this elsewhere
					continue;
				}

				return true;
			}
			else {
				continue;
			}
		}

		NonConnectorItem * nonConnectorItem = dynamic_cast<NonConnectorItem *>(item);
		if (nonConnectorItem != NULL) {
			if (dynamic_cast<ConnectorItem *>(item) == NULL) {
				if (m_sketchWidget->sameElectricalLayer2(nonConnectorItem->attachedTo()->viewLayerID(), viewLayerID))
				{
					return true;
				}
			}

			continue;
		}

		TraceWire * traceWire = dynamic_cast<TraceWire *>(item);
		if (traceWire == NULL) continue;
		if (!m_sketchWidget->sameElectricalLayer2(traceWire->viewLayerID(), viewLayerID)) continue;

		if (!from) return true;

		QList<Wire *> chainedWires;
		QList<ConnectorItem *> ends;
		traceWire->collectChained(chainedWires, ends);
		
		if (!ends.contains(from)) {
			return true;
		}
	}

	return false;
}

void JRouter::updateProgress(int num, int denom) 
{
	emit setProgressValue((int) MaximumProgress * (m_currentProgressPart + (num / (qreal) denom)) / (qreal) m_maximumProgressPart);
}

JSubedge * JRouter::makeSubedge(JEdge * edge, QPointF p1, ConnectorItem * from, QPointF p2, ConnectorItem * to, bool forward) 
{
	JSubedge * subedge = new JSubedge;
	subedge->edge = edge;
	subedge->from = from;
	subedge->to = to;
	subedge->fromWire = NULL;
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
	r2.adjust(-MinConnectorSpace, -MinConnectorSpace, MinConnectorSpace, MinConnectorSpace);
	QList<Tile *> already;
	TileRect tileRect;
	tileRect.xmin = r2.left();
	tileRect.xmax = r2.right();
	tileRect.ymin = r2.top();		// TILE is Math Y-axis not computer-graphic Y-axis
	tileRect.ymax = r2.bottom(); 
	DebugDialog::debug(QString("   add tile %1").arg((long) nci, 0, 16), r2);
	return insertTile(thePlane, tileRect, alreadyTiled, nci, type, true);
}


int prepDeleteTile(Tile * tile, UserData data) {
	switch(TiGetType(tile)) {
		case DUMMYLEFT:
		case DUMMYRIGHT:
		case DUMMYTOP:
		case DUMMYBOTTOM:
			return 0;
	}

	//DebugDialog::debug(QString("tile %1 %2 %3 %4").arg(LEFT(tile)).arg(BOTTOM(tile)).arg(RIGHT(tile)).arg(TOP(tile)));
	QSet<Tile *> * tiles = (QSet<Tile *> *) data;
	tiles->insert(tile);

	return 0;
}

void JRouter::clearTiles(Plane * thePlane) 
{
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
		qreal y1 = BOTTOM(tile);
		qreal x2 = RIGHT(tile);
		qreal y2 = TOP(tile);
			
		drawGridItem(x1, y1, x2, y2, 0, GridEntry::BLOCK);
	}
}

int checkAlready(Tile * tile, UserData data) {
	int type = TiGetType(tile);
	switch (type) {
		case NOTBOARD:
		case NONCONNECTOR:
		case TRACE:
		case TRACECONNECTOR:
		case CONNECTOR:
		case PART:
			break;
		default:
			return 0;
	}

	QList<Tile *> * tiles = (QList<Tile *> *) data;
	if (tiles == NULL) return 0;

	tiles->append(tile);
	return 0;
}

Tile * JRouter::insertTile(Plane * thePlane, TileRect & trueRect, QList<Tile *> & alreadyTiled, QGraphicsItem * item, int type, bool adjustToGrid) {
	TileRect tileRect = trueRect;
	if (adjustToGrid) {
		// make sure all tiles are on the grid in the y-axis
		// so that we're sure all horizontal traces will fit
		tileRect.ymin = qFloor(trueRect.ymin / Wire::STANDARD_TRACE_WIDTH) * Wire::STANDARD_TRACE_WIDTH;
		tileRect.ymax = qCeil(trueRect.ymax / Wire::STANDARD_TRACE_WIDTH) * Wire::STANDARD_TRACE_WIDTH;
	}
	else {
		int y1 = qRound(trueRect.ymin / Wire::STANDARD_TRACE_WIDTH);
		int y2 = qRound(trueRect.ymax / Wire::STANDARD_TRACE_WIDTH);
		trueRect.ymin = tileRect.ymin = y1 *  Wire::STANDARD_TRACE_WIDTH;
		trueRect.ymax = tileRect.ymax = y2 *  Wire::STANDARD_TRACE_WIDTH;
	}

	DebugDialog::debug(QString("insert tile xmin:%1 xmax:%2 ymin:%3 ymax:%4 aymin:%5 aymax:%6").
		arg(tileRect.xmin).arg(tileRect.xmax).arg(trueRect.ymin).arg(trueRect.ymax).arg(tileRect.ymin).arg(tileRect.ymax));

	TiSrArea(NULL, thePlane, &tileRect, checkAlready, &alreadyTiled);
	if (alreadyTiled.count() > 0) {
		foreach (Tile * intersectingTile, alreadyTiled) {
			TileRect intersectingRect;
			TiToRect(intersectingTile, &intersectingRect);
			DebugDialog::debug(QString("intersecting tile l:%1 t:%2 r:%3 b:%4 t:%5").arg(LEFT(intersectingTile))
				.arg(BOTTOM(intersectingTile)).arg(RIGHT(intersectingTile)).arg(TOP(intersectingTile)).arg(intersectingTile->ti_type));

			if (TiGetType(intersectingTile) != GRIDALIGN) {
				if (tileRectIntersects(&trueRect, &intersectingRect)) {
					return NULL;
				}
			}
			else {
				DebugDialog::debug("intersected grid align");
			}
		}
	}


	if (alreadyTiled.count() > 0) {
		// we are only intersecting GRIDALIGN tiles so deal with that here...
		alreadyTiled.clear();
	}


	Tile * tile = TiInsertTile(thePlane, &tileRect, item, type);
	if (tileRect.ymin < trueRect.ymin) {
		Tile * alignTile = TiSplitY_Bottom(tile, trueRect.ymin);
		TiSetType(alignTile, GRIDALIGN);
		TiSetBody(alignTile, item);
		DebugDialog::debug(QString("align tile min l:%1 t:%2 r:%3 b:%4")
				.arg(LEFT(alignTile)).arg(BOTTOM(alignTile)).arg(RIGHT(alignTile)).arg(TOP(alignTile)));
	}
	if (tileRect.ymax > trueRect.ymax) {
		Tile * alignTile = TiSplitY(tile, trueRect.ymax);
		TiSetType(alignTile, GRIDALIGN);
		TiSetBody(alignTile, item);		
		DebugDialog::debug(QString("align tile max l:%1 t:%2 r:%3 b:%4")
				.arg(LEFT(alignTile)).arg(BOTTOM(alignTile)).arg(RIGHT(alignTile)).arg(TOP(alignTile)));
	}

	if (tileRect.ymin < trueRect.ymin || tileRect.ymax > trueRect.ymax) {
		DebugDialog::debug(QString("new tile max l:%1 t:%2 r:%3 b:%4")
				.arg(LEFT(tile)).arg(BOTTOM(tile)).arg(RIGHT(tile)).arg(TOP(tile)));
	}


	return tile;
}

void JRouter::clearGridEntries() {
	foreach (QGraphicsItem * item, m_sketchWidget->scene()->items()) {
		GridEntry * gridEntry = dynamic_cast<GridEntry *>(item);
		if (gridEntry == NULL) continue;

		delete gridEntry;
	}
}

