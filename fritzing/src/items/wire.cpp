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

$Revision$:
$Author$:
$Date$

********************************************************************/

/* 

curvy To Do

	* pixel turds

	* undo/redo

	* save/load

	* copy/paste

	* make-straight function

	* export

	curvy to begin with? would have to vary with some function of angle and distance
		could convert control points to t values?

	turn curvature on/off per view

---------------------------------------------------------

later:

	clippable wire

	gerber

	autorouter warning in PCB view

	modify parameters (tension, unit area)?

*/

/////////////////////////////////////////////////////////////////

#include "wire.h"

#include <QLineF>
#include <QPen>
#include <QRadialGradient>
#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
#include <QList>
#include <QGraphicsItem>
#include <QSet>
#include <QComboBox>
#include <QToolTip>
#include <QApplication>

#include "../debugdialog.h"
#include "../sketch/infographicsview.h"
#include "../connectors/connectoritem.h"
#include "../connectors/svgidlayer.h"
#include "../fsvgrenderer.h"
#include "partlabel.h"
#include "../model/modelpart.h"
#include "../utils/graphicsutils.h"
#include "../utils/textutils.h"
#include "../utils/bezier.h"
#include "../utils/bezierdisplay.h"
#include "../utils/cursormaster.h"
#include "../layerattributes.h"

#include <stdlib.h>

QHash<QString, QString> Wire::colors;
QHash<QString, QString> Wire::shadowColors;
QHash<QString, QString> Wire::colorTrans;
QStringList Wire::colorNames;
QHash<int, QString> Wire::widthTrans;
QList<int> Wire::widths;
double Wire::STANDARD_TRACE_WIDTH;
double Wire::HALF_STANDARD_TRACE_WIDTH;
double Wire::THIN_TRACE_WIDTH;

const double DefaultHoverStrokeWidth = 4;

static Bezier UndoBezier;
static BezierDisplay * TheBezierDisplay = NULL;

////////////////////////////////////////////////////////////

bool alphaLessThan(QColor * c1, QColor * c2)
{
	return c1->alpha() < c2->alpha();
}

/////////////////////////////////////////////////////////////


WireMenu::WireMenu(const QString & title, QWidget * parent) : QMenu(title, parent) 
{
	m_wire = NULL;
}

void WireMenu::setWire(Wire * w) {
	m_wire = w;
}

Wire * WireMenu::wire() {
	return m_wire;
}

/////////////////////////////////////////////////////////////

WireAction::WireAction(QAction * action) : QAction(action) {
	m_wire = NULL;
	this->setText(action->text());
	this->setStatusTip(action->statusTip());
	this->setCheckable(action->isCheckable());
}

WireAction::WireAction(const QString & title, QObject * parent) : QAction(title, parent) {
	m_wire = NULL;
}

void WireAction::setWire(Wire * w) {
	m_wire = w;
}

Wire * WireAction::wire() {
	return m_wire;
}

/////////////////////////////////////////////////////////////

Wire::Wire( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,  const ViewGeometry & viewGeometry, long id, QMenu* itemMenu, bool initLabel)
	: ItemBase(modelPart, viewIdentifier, viewGeometry, id, itemMenu)
{
	m_bezier = NULL;
	m_canHaveCurve = true;
	m_hoverStrokeWidth = DefaultHoverStrokeWidth;
	m_connector0 = m_connector1 = NULL;
	m_partLabel = initLabel ? new PartLabel(this, NULL) : NULL;
	m_canChainMultiple = false;
    setFlag(QGraphicsItem::ItemIsSelectable, true );
	m_connectorHover = NULL;
	m_opacity = 1.0;
	m_ignoreSelectionChange = false;

	//DebugDialog::debug(QString("aix line %1 %2 %3 %4").arg(this->viewGeometry().line().x1())
													//.arg(this->viewGeometry().line().y1())
													//.arg(this->viewGeometry().line().x2())
													//.arg(this->viewGeometry().line().y2()) );
	//DebugDialog::debug(QString("aix loc %1 %2").arg(this->viewGeometry().loc().x())
														//.arg(this->viewGeometry().loc().y()) );

	setPos(m_viewGeometry.loc());

	m_dragCurve = m_dragEnd = false;
}

Wire::~Wire() {
	if (m_bezier) {
		delete m_bezier;
	}
}

FSvgRenderer * Wire::setUp(ViewLayer::ViewLayerID viewLayerID, const LayerHash &  viewLayers, InfoGraphicsView * infoGraphicsView) {
	ItemBase::setViewLayerID(viewLayerID, viewLayers);
	FSvgRenderer * svgRenderer = setUpConnectors(m_modelPart, m_viewIdentifier);
	if (svgRenderer != NULL) {
		initEnds(m_viewGeometry, svgRenderer->viewBox(), infoGraphicsView);
		setConnectorTooltips();
	}
	setZValue(this->z());

	return svgRenderer;
}

void Wire::saveGeometry() {
	m_viewGeometry.setSelected(this->isSelected());
	m_viewGeometry.setLine(this->line());
	m_viewGeometry.setLoc(this->pos());
	m_viewGeometry.setZ(this->zValue());
}


bool Wire::itemMoved() {
	if  (m_viewGeometry.loc() != this->pos()) return true;

	if (this->line().dx() != m_viewGeometry.line().dx()) return false;
	if (this->line().dy() != m_viewGeometry.line().dy()) return false;

	return (this->line() != m_viewGeometry.line());
}

void Wire::moveItem(ViewGeometry & viewGeometry) {
	this->setPos(viewGeometry.loc());
	this->setLine(viewGeometry.line());
}

void Wire::initEnds(const ViewGeometry & vg, QRectF defaultRect, InfoGraphicsView * infoGraphicsView) {
	bool gotOne = false;
	bool gotTwo = false;
	double penWidth = 1;
	foreach (ConnectorItem * item, cachedConnectorItems()) {
		// check the name or is order good enough?

		if (gotOne) {
			gotTwo = true;
			m_connector1 = item;
			break;
		}
		else {
			penWidth = item->rect().width();
			m_connector0 = item;
			gotOne = true;
		}
	}

	if (!gotTwo) {
		return;
	}

	if ((vg.line().length() == 0) && (vg.line().x1() == -1)) {
		this->setLine(defaultRect.left(), defaultRect.top(), defaultRect.right(), defaultRect.bottom());
	}
	else {
		this->setLine(vg.line());
	}

	setConnector0Rect();
	setConnector1Rect();
	m_viewGeometry.setLine(this->line());
	
   	QBrush brush(QColor(0, 0, 0));
	QPen pen(brush, penWidth, Qt::SolidLine, Qt::RoundCap);
	this->setPen(pen);

	m_pen.setCapStyle(Qt::RoundCap);
	m_shadowPen.setCapStyle(Qt::RoundCap);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->initWire(this, penWidth);
	}

	prepareGeometryChange();
}

void Wire::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) {
	if (m_hidden) return;

	ItemBase::paint(painter, option, widget);
}

void Wire::paintBody(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) 
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	QPainterPath painterPath;
	if (m_bezier && !m_bezier->isEmpty()) {
		QLineF line = this->line();
		painterPath.moveTo(line.p1());
		painterPath.cubicTo(m_bezier->cp0(), m_bezier->cp1(), line.p2());
		
		/*
		DebugDialog::debug(QString("c0x:%1,c0y:%2 c1x:%3,c1y:%4 p0x:%5,p0y:%6 p1x:%7,p1y:%8 px:%9,py:%10")
							.arg(m_controlPoints.at(0).x())
							.arg(m_controlPoints.at(0).y())
							.arg(m_controlPoints.at(1).x())
							.arg(m_controlPoints.at(1).y())
							.arg(m_line.p1().x())
							.arg(m_line.p1().y())
							.arg(m_line.p2().x())
							.arg(m_line.p2().y())
							.arg(pos().x())
							.arg(pos().y())

							);
		*/
	}


	painter->setOpacity(m_inactive ? m_opacity  / 2 : m_opacity);
	if (hasShadow()) {
		painter->save();
		painter->setPen(m_shadowPen);
		if (painterPath.isEmpty()) {
			QLineF line = this->line();
			painter->drawLine(line);
		}
		else {
			painter->drawPath(painterPath);
		}
		painter->restore();
	}
	   
	painter->setPen(m_pen);
	if (painterPath.isEmpty()) {
		painter->drawLine(getPaintLine());	
	}
	else {
		painter->drawPath(painterPath);
	}
}

void Wire::paintHover(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) 
{
	Q_UNUSED(widget);
	Q_UNUSED(option);
	painter->save();
	if ((m_connectorHoverCount > 0 && !(m_dragEnd || m_dragCurve)) || m_connectorHoverCount2 > 0) {
		painter->setOpacity(.50);
		painter->fillPath(this->hoverShape(), QBrush(connectorHoverColor));
	}
	else {
		painter->setOpacity(hoverOpacity);
		painter->fillPath(this->hoverShape(), QBrush(hoverColor));
	}
	painter->restore();
}

QPainterPath Wire::hoverShape() const
{
	return shapeAux(m_hoverStrokeWidth);
}

QPainterPath Wire::shape() const
{
	return shapeAux(m_pen.widthF());
}

QPainterPath Wire::shapeAux(double width) const
{
	QPainterPath path;
	if (m_line == QLineF()) {
	    return path;
	}
				
	path.moveTo(m_line.p1());
	if (m_bezier == NULL || m_bezier->isEmpty()) {
		path.lineTo(m_line.p2());
	}
	else {
		path.cubicTo(m_bezier->cp0(), m_bezier->cp1(), m_line.p2());
	}
	//DebugDialog::debug(QString("using hoverstrokewidth %1 %2").arg(m_id).arg(m_hoverStrokeWidth));
	return GraphicsUtils::shapeFromPath(path, m_pen, width, false);
}

QRectF Wire::boundingRect() const
{
	if (m_pen.widthF() == 0.0) {
	    const double x1 = m_line.p1().x();
	    const double x2 = m_line.p2().x();
	    const double y1 = m_line.p1().y();
	    const double y2 = m_line.p2().y();
	    double lx = qMin(x1, x2);
	    double rx = qMax(x1, x2);
	    double ty = qMin(y1, y2);
	    double by = qMax(y1, y2);
	    return QRectF(lx, ty, rx - lx, by - ty);
	}
	return hoverShape().controlPointRect();
}

void Wire::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
	//DebugDialog::debug("checking press event");
	emit wireSplitSignal(this, event->scenePos(), this->pos(), this->line());
}

void Wire::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	WireMenu * wireMenu = qobject_cast<WireMenu *>(m_itemMenu);
	if (wireMenu) {
		wireMenu->setWire(this);
	}
	ItemBase::mousePressEvent(event);
}

void Wire::initDragCurve(QPointF scenePos) {
	if (m_bezier == NULL) {
		m_bezier = new Bezier();
	}

	UndoBezier.copy(m_bezier);

	m_dragCurve = true;
	m_dragEnd = false;

	QPointF p0 = connector0()->sceneAdjustedTerminalPoint(NULL);
	QPointF p1 = connector1()->sceneAdjustedTerminalPoint(NULL);
	if (m_bezier->isEmpty()) {
		m_bezier->initToEnds(mapFromScene(p0), mapFromScene(p1));
	}
	else {
		m_bezier->set_endpoints(mapFromScene(p0), mapFromScene(p1));
	}

	m_bezier->initControlIndex(mapFromScene(scenePos), m_pen.widthF());
	TheBezierDisplay = new BezierDisplay;
	TheBezierDisplay->initDisplay(this, m_bezier);
}

bool Wire::initNewBendpoint(QPointF scenePos, Bezier & left, Bezier & right) {
	if (m_bezier == NULL || m_bezier->isEmpty()) {
		UndoBezier.clear();
		return false;
	}

	QPointF p0 = connector0()->sceneAdjustedTerminalPoint(NULL);
	QPointF p1 = connector1()->sceneAdjustedTerminalPoint(NULL);
	m_bezier->set_endpoints(mapFromScene(p0), mapFromScene(p1));
	UndoBezier.copy(m_bezier);

	double t = m_bezier->findSplit(mapFromScene(scenePos), m_pen.widthF());
	m_bezier->split(t, left, right);
	return true;
}

void Wire::initDragEnd(ConnectorItem * connectorItem, QPointF scenePos) {
	Q_UNUSED(scenePos);
	saveGeometry();
	QLineF line = this->line();
	m_drag0 = (connectorItem == m_connector0);
	m_dragEnd = true;
	m_dragCurve = false;
	if (m_drag0) {
		m_wireDragOrigin = line.p2();
 		//DebugDialog::debug(QString("drag near origin %1 %2").arg(m_wireDragOrigin.x()).arg(m_wireDragOrigin.y()) );
	}
	else {
		m_wireDragOrigin = line.p1();
 		//DebugDialog::debug(QString("drag far origin %1 %2").arg(m_wireDragOrigin.x()).arg(m_wireDragOrigin.y()) );
 		//DebugDialog::debug(QString("drag far other %1 %2").arg(line.p2().x()).arg(line.p2().y()) );
	}

	if (connectorItem->chained()) {
		QList<Wire *> chained;
		QList<ConnectorItem *> ends;
		collectChained(chained, ends);
		// already saved the first one
		for (int i = 1; i < chained.count(); i++) {
			chained[i]->saveGeometry();
		}
	}
}


void Wire::mouseReleaseConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	Q_UNUSED(event);
	Q_UNUSED(connectorItem);
	releaseDrag();
}

void Wire::mouseMoveConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	mouseMoveEventAux(this->mapFromItem(connectorItem, event->pos()), event->modifiers());
}


void Wire::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	mouseMoveEventAux(event->pos(), event->modifiers());
}

void Wire::mouseMoveEventAux(QPointF eventPos, Qt::KeyboardModifiers modifiers) {
	if (m_spaceBarWasPressed) {
		return;
	}

	if (m_dragCurve) {
		prepareGeometryChange();
		dragCurve(eventPos, modifiers);
		update();
		if (TheBezierDisplay) TheBezierDisplay->updateDisplay(this, m_bezier);
		return;
	}

	if (m_dragEnd == false) {
		return;
	}

	ConnectorItem * whichConnectorItem;
	ConnectorItem * otherConnectorItem;
	if (m_drag0) {
		whichConnectorItem = m_connector0;
		otherConnectorItem = m_connector1;
	}
	else {
		whichConnectorItem = m_connector1;
		otherConnectorItem = m_connector0;
	}

	if ((modifiers & Qt::ShiftModifier) != 0) {
		QPointF initialPos = mapFromScene(otherConnectorItem->sceneAdjustedTerminalPoint(NULL)); 
		bool bendpoint = isBendpoint(whichConnectorItem);
		if (bendpoint) {
			bendpoint = false;
			foreach (ConnectorItem * ci, whichConnectorItem->connectedToItems()) {
				Wire * w = qobject_cast<Wire *>(ci->attachedTo());
				ConnectorItem * oci = w->otherConnector(ci);
				QPointF otherInitialPos = mapFromScene(oci->sceneAdjustedTerminalPoint(NULL));
				QPointF p1(initialPos.x(), otherInitialPos.y());
				double d = GraphicsUtils::distanceSqd(p1, eventPos);
				if (d <= 144) {
					bendpoint = true;
					eventPos = p1;
					break;
				}
				p1.setX(otherInitialPos.x());
				p1.setY(initialPos.y());
				d = GraphicsUtils::distanceSqd(p1, eventPos);
				if (d <= 144) {
					bendpoint = true;
					eventPos = p1;
					break;
				}				
			}
		}

		if (!bendpoint) {
			eventPos = GraphicsUtils::calcConstraint(initialPos, eventPos);
		}

	}

	if (m_drag0) {
		QPointF p = this->mapToScene(eventPos);
		QGraphicsSvgItem::setPos(p.x(), p.y());
		this->setLine(0, 0, m_wireDragOrigin.x() - p.x() + m_viewGeometry.loc().x(),
							m_wireDragOrigin.y() - p.y() + m_viewGeometry.loc().y() );
		//DebugDialog::debug(QString("drag0 wdo:(%1,%2) p:(%3,%4) vg:(%5,%6) l:(%7,%8)")
		//			.arg(m_wireDragOrigin.x()).arg(m_wireDragOrigin.y())
		//			.arg(p.x()).arg(p.y())
		//			.arg(m_viewGeometry.loc().x()).arg(m_viewGeometry.loc().y())
		//			.arg(line().p2().x()).arg(line().p2().y())
		//	);
	}
	else {
		this->setLine(m_wireDragOrigin.x(), m_wireDragOrigin.y(), eventPos.x(), eventPos.y());
		//DebugDialog::debug(QString("drag1 wdo:(%1,%2) ep:(%3,%4) p:(%5,%6) l:(%7,%8)")
		//			.arg(m_wireDragOrigin.x()).arg(m_wireDragOrigin.y())
		//			.arg(eventPos.x()).arg(eventPos.y())
		//			.arg(pos().x()).arg(pos().y())
		//			.arg(line().p2().x()).arg(line().p2().y())
		//	);
	}
	setConnector1Rect();

	bool chained = false;
	foreach (ConnectorItem * toConnectorItem, whichConnectorItem->connectedToItems()) {
		Wire * chainedWire = qobject_cast<Wire *>(toConnectorItem->attachedTo());
		if (chainedWire == NULL) continue;

		chainedWire->simpleConnectedMoved(whichConnectorItem, toConnectorItem);
		chained = true;
	}

	if (!chained) {
		// don't allow wire to connect back to something the other end is already directly connected to
		QList<Wire *> wires;
		QList<ConnectorItem *> ends;
		collectChained(otherConnectorItem, wires, ends);
		for (int i = 0; i < wires.count(); i++) {
			Wire * w = wires[i];
			collectChained(w->m_connector1, wires, ends);
			collectChained(w->m_connector0, wires, ends);
		}
		ends.append(otherConnectorItem);
		foreach (Wire * w, wires) {
			ends.append(w->connector0());
			ends.append(w->connector1());
		}
		foreach (ConnectorItem * toConnectorItem, whichConnectorItem->connectedToItems()) {
			ends.removeOne(toConnectorItem);
		}

		ConnectorItem * originatingConnector = NULL;
		if (otherConnectorItem && otherConnectorItem->connectionsCount() > 0) {
			originatingConnector = otherConnectorItem->connectedToItems()[0];
		}
		whichConnectorItem->findConnectorUnder(false, true, ends, true, originatingConnector);
	}
}

void Wire::setConnector0Rect() {
	QRectF rect = m_connector0->rect();
	rect.moveTo(0 - (rect.width()  / 2.0)  ,
				0 - (rect.height()  / 2.0) );
	//DebugDialog::debug(QString("set connector rect %1 %2").arg(rect.width()).arg(rect.height()));
	m_connector0->setRect(rect);
}


void Wire::setConnector1Rect() {
	QRectF rect = m_connector1->rect();
	rect.moveTo(this->line().dx() - (rect.width()  / 2.0)  ,
				this->line().dy() - (rect.height()  / 2.0) );
	m_connector1->setRect(rect);
}

void Wire::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	if (m_spaceBarWasPressed) {
		return;
	}

	if (releaseDrag()) return;

	ItemBase::mouseReleaseEvent(event);
}

bool Wire::releaseDrag() {
	if (m_dragEnd == false && m_dragCurve == false) return false;

	if (m_dragCurve) {
		delete TheBezierDisplay;
		TheBezierDisplay = NULL;
		m_dragCurve = false;
		ungrabMouse();
		if (UndoBezier != *m_bezier) {
			emit wireChangedCurveSignal(this, &UndoBezier, m_bezier, false);
		}
		return true;
	}

	m_dragEnd = false;

	ConnectorItem * from = (m_drag0) ? m_connector0 : m_connector1;
	ConnectorItem * to = from->releaseDrag();

	QLineF newLine = this->line();
	QLineF oldLine = m_viewGeometry.line();
	QPointF oldPos = m_viewGeometry.loc();
	QPointF newPos = this->pos();
	if (newLine != oldLine || oldPos != newPos) {
		emit wireChangedSignal(this, oldLine, newLine, oldPos, newPos, from, to);
	}

	return true;
}


void Wire::saveInstanceLocation(QXmlStreamWriter & streamWriter)
{
	QLineF line = m_viewGeometry.line();
	QPointF loc = m_viewGeometry.loc();
	streamWriter.writeAttribute("x", QString::number(loc.x()));
	streamWriter.writeAttribute("y", QString::number(loc.y()));
	streamWriter.writeAttribute("x1", QString::number(line.x1()));
	streamWriter.writeAttribute("y1", QString::number(line.y1()));
	streamWriter.writeAttribute("x2", QString::number(line.x2()));
	streamWriter.writeAttribute("y2", QString::number(line.y2()));
	streamWriter.writeAttribute("wireFlags", QString::number(m_viewGeometry.flagsAsInt()));
}

void Wire::writeGeometry(QXmlStreamWriter & streamWriter) {
	ItemBase::writeGeometry(streamWriter);
	streamWriter.writeStartElement("wireExtras");
	streamWriter.writeAttribute("mils", QString::number(mils()));
	streamWriter.writeAttribute("color", m_pen.brush().color().name());
	streamWriter.writeAttribute("opacity", QString::number(m_opacity));
	if (m_bezier) m_bezier->write(streamWriter);
	streamWriter.writeEndElement();
}

void Wire::setExtras(QDomElement & element, InfoGraphicsView * infoGraphicsView)
{
	if (element.isNull()) return;

	bool ok;
	double w = element.attribute("width").toDouble(&ok);
	if (ok) {
		setWireWidth(w, infoGraphicsView, infoGraphicsView->getWireStrokeWidth(this, w));
	}
	else {
		w = element.attribute("mils").toDouble(&ok);
		if (ok) {
			double wpix = GraphicsUtils::mils2pixels(w, FSvgRenderer::printerScale());
			setWireWidth(wpix, infoGraphicsView, infoGraphicsView->getWireStrokeWidth(this, wpix));
		}
	}

	setColorFromElement(element);
	QDomElement bElement = element.firstChildElement("bezier");
	Bezier bezier = Bezier::fromElement(bElement);
	if (!bezier.isEmpty()) {
		prepareGeometryChange();
		m_bezier = new Bezier;
		m_bezier->copy(&bezier);
		QPointF p0 = connector0()->sceneAdjustedTerminalPoint(NULL);
		QPointF p1 = connector1()->sceneAdjustedTerminalPoint(NULL);
		m_bezier->set_endpoints(mapFromScene(p0), mapFromScene(p1));
	}

}

void Wire::setColorFromElement(QDomElement & element) {
	QString colorString = element.attribute("color");
	if (colorString.isNull() || colorString.isEmpty()) return;

	bool ok;
	double op = element.attribute("opacity").toDouble(&ok);
	if (!ok) {
		op = 1.0;
	}

	setColorString(colorString, op);
}

void Wire::hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event , ConnectorItem * item) {
	m_connectorHover = item;
	ItemBase::hoverEnterConnectorItem(event, item);
}

void Wire::hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item) {
	m_connectorHover = NULL;
	ItemBase::hoverLeaveConnectorItem(event, item);
}

void Wire::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) {
	ItemBase::hoverEnterEvent(event);
	QApplication::instance()->installEventFilter(this);
	CursorMaster::instance()->addCursor(this, cursor());
	//DebugDialog::debug("---wire set override cursor");
	updateCursor(event->modifiers());
}

void Wire::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) {
	QApplication::instance()->removeEventFilter(this);
	ItemBase::hoverLeaveEvent(event);
	//DebugDialog::debug("------wire restore override cursor");
	CursorMaster::instance()->removeCursor(this);
}


void Wire::connectionChange(ConnectorItem * onMe, ConnectorItem * onIt, bool connect) {
	checkVisibility(onMe, onIt, connect);

	bool movable = true;
	foreach (ConnectorItem * connectedTo, m_connector0->connectedToItems()) {
		if (connectedTo->attachedToItemType() != ModelPart::Wire) {
			movable = false;
			break;
		}
	}
	if (movable) {
		foreach (ConnectorItem * connectedTo, m_connector1->connectedToItems()) {
			if (connectedTo->attachedToItemType() != ModelPart::Wire) {
				movable = false;
				break;
			}
		}
	}
}

void Wire::mouseDoubleClickConnectorEvent(ConnectorItem * connectorItem) {
	int chained = 0;
	foreach (ConnectorItem * toConnectorItem, connectorItem->connectedToItems()) {
		if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			chained++;
		}
		else {
			return;
		}
	}


	if (chained == 1) {
		// near as I can tell, this is to eliminate the overrides from the connectorItem and then from the wire itself
		emit wireJoinSignal(this, connectorItem);
	}
}

void Wire::mousePressConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	//DebugDialog::debug("checking press connector event");

	if (m_canChainMultiple && event->modifiers() & altOrMetaModifier()) {
		// dragging a wire out of a bendpoint
		InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
		if (infoGraphicsView != NULL) {
			infoGraphicsView->mousePressConnectorEvent(connectorItem, event);
		}

		return;
	}


	connectorItem->setOverConnectorItem(NULL);
	initDragEnd(connectorItem, event->scenePos());

}

void Wire::simpleConnectedMoved(ConnectorItem * to) {
	// to is this wire, from is something else
	simpleConnectedMoved(to->firstConnectedToIsh(), to);
}

void Wire::simpleConnectedMoved(ConnectorItem * from, ConnectorItem * to)
{
	if (from == NULL) return;

	// to is this wire, from is something else
	QPointF p1, p2;
	calcNewLine(from, to, p1, p2);

	/*
	QPointF oldPos = this->pos();
	QPointF newPos = p1;
	QLineF oldLine = this->line();
	QLineF newLine(0, 0,  p2.x() - p1.x(), p2.y() - p1.y());
	if (qAbs(oldPos.x() - newPos.x()) > 1.75 ||
		qAbs(oldPos.y() - newPos.y()) > 1.75 ||
		qAbs(oldLine.x1() - newLine.x1()) > 1.75 ||
		qAbs(oldLine.x2() - newLine.x2()) > 1.75 ||
		qAbs(oldLine.y1() - newLine.y1()) > 1.75 ||
		qAbs(oldLine.y2() - newLine.y2()) > 1.75
		)
	{
		DebugDialog::debug("line changed");
		calcNewLine(from,to,p1,p2);
	}
	*/

	this->setPos(p1);
	this->setLine(0,0, p2.x() - p1.x(), p2.y() - p1.y() );
	//DebugDialog::debug(QString("set line %5: %1 %2, %3 %4, vis:%6 lyr:%7").arg(p1.x()).arg(p1.y()).arg(p2.x()).arg(p2.y()).arg(id()).arg(isVisible()).arg(m_viewIdentifier) );
	setConnector1Rect();
}

void Wire::calcNewLine(ConnectorItem * from, ConnectorItem * to, QPointF & p1, QPointF & p2) {
	// to is this wire, from is something else
	if (to == m_connector0) {
		p1 = from->sceneAdjustedTerminalPoint(to);
		ConnectorItem * otherFrom = m_connector1->firstConnectedToIsh();
		if (otherFrom == NULL) {
			p2 = m_connector1->mapToScene(m_connector1->rect().center());
		}
		else {
			p2 = otherFrom->sceneAdjustedTerminalPoint(m_connector1);
		}
	}
	else {
		p2 = from->sceneAdjustedTerminalPoint(to);
		ConnectorItem * otherFrom = m_connector0->firstConnectedToIsh();
		if (otherFrom == NULL) {
			p1 = m_connector0->mapToScene(m_connector0->rect().center());
		}
		else {
			p1 = otherFrom->sceneAdjustedTerminalPoint(m_connector0);
		}

	}
}

void Wire::connectedMoved(ConnectorItem * from, ConnectorItem * to) {
	// "from" is the connector on the part
	// "to" is the connector on the wire

	simpleConnectedMoved(from, to);
	return;

	/*
	DebugDialog::debug(QString("connected moved %1 %2, %3 %4")
		.arg(from->attachedToID())
		.arg(from->attachedToTitle())
		.arg(to->attachedToID())
		.arg(to->attachedToTitle())
		);
	*/

	ConnectorItem * otherEnd = otherConnector(to);
	bool chained = otherEnd->chained();
	QPointF p1, p2;
	if (chained) {
		// move both ends
		if (to == m_connector0) {
			p1 = from->sceneAdjustedTerminalPoint(m_connector0);
			p2 = this->line().p2() + p1;
		}
		else {
			p2 = from->sceneAdjustedTerminalPoint(m_connector1);
			p1 = p2 - this->line().p2();
		}
	}
	else {
		calcNewLine(from, to, p1, p2);
	}
	this->setPos(p1);
	this->setLine(0,0, p2.x() - p1.x(), p2.y() - p1.y() );
	//DebugDialog::debug(QString("set line %5: %1 %2, %3 %4, vis:%6 lyr:%7").arg(p1.x()).arg(p1.y()).arg(p2.x()).arg(p2.y()).arg(id()).arg(isVisible()).arg(m_viewIdentifier) );
	setConnector1Rect();

	if (chained) {
		foreach (ConnectorItem * otherEndTo, otherEnd->connectedToItems()) {
			if (otherEndTo->attachedToItemType() == ModelPart::Wire) {
				otherEndTo->attachedTo()->connectedMoved(otherEnd, otherEndTo);
			}
		}
	}
}


FSvgRenderer * Wire::setUpConnectors(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier) 
{
	clearConnectorItemCache();

	QString error;
	LayerAttributes layerAttributes;
	FSvgRenderer * renderer = ItemBase::setUpImage(modelPart, viewIdentifier, m_viewLayerID, m_viewLayerSpec, layerAttributes, error);
	if (renderer == NULL) {
		return NULL;
	}

	foreach (Connector * connector, m_modelPart->connectors().values()) {
		if (connector == NULL) continue;

		SvgIdLayer * svgIdLayer = connector->fullPinInfo(viewIdentifier, m_viewLayerID);
		if (svgIdLayer == NULL) continue;

		bool result = renderer->setUpConnector(svgIdLayer, false);
		if (!result) continue;

		ConnectorItem * connectorItem = newConnectorItem(connector);
		connectorItem->setRect(svgIdLayer->m_rect);
		connectorItem->setTerminalPoint(svgIdLayer->m_point);
		m_originalConnectorRect = svgIdLayer->m_rect;

		connectorItem->setCircular(true);
		//DebugDialog::debug(QString("terminal point %1 %2").arg(terminalPoint.x()).arg(terminalPoint.y()) );
	}

	return renderer;
}

/*
void Wire::setPos(const QPointF & pos) {
	ItemBase::setPos(pos);
}
*/


void Wire::setLineAnd(QLineF line, QPointF pos, bool useLine) {
	this->setPos(pos);
	if (useLine) this->setLine(line);

	setConnector1Rect();
}

ConnectorItem * Wire::otherConnector(ConnectorItem * oneConnector) {
	if (oneConnector == m_connector0) return m_connector1;

	return m_connector0;
}

ConnectorItem * Wire::connector0() {
	return m_connector0;
}

ConnectorItem * Wire::connector1() {
	return m_connector1;
}

void Wire::findConnectorsUnder() {
	foreach (ConnectorItem * connectorItem, cachedConnectorItems()) {
		connectorItem->findConnectorUnder(true, false, ConnectorItem::emptyConnectorItemList, false, NULL);
	}
}

void Wire::collectChained(QList<Wire *> & chained, QList<ConnectorItem *> & ends ) {
	chained.append(this);
	for (int i = 0; i < chained.count(); i++) {
		Wire * wire = chained[i];
		collectChained(wire->m_connector1, chained, ends);
		collectChained(wire->m_connector0, chained, ends);
	}
}

void Wire::collectChained(ConnectorItem * connectorItem, QList<Wire *> & chained, QList<ConnectorItem *> & ends) {
	if (connectorItem == NULL) return;

	foreach (ConnectorItem * connectedToItem, connectorItem->connectedToItems()) {
		Wire * wire = qobject_cast<Wire *>(connectedToItem->attachedTo());
		if (wire == NULL) {
			if (!ends.contains(connectedToItem)) {
				ends.append(connectedToItem);
			}
			continue;
		}

		if (chained.contains(wire)) continue;
		chained.append(wire);
	}
}

void Wire::collectWires(QList<Wire *> & wires) {
	if (wires.contains(this)) return;

	wires.append(this);
	//DebugDialog::debug(QString("collecting wire %1").arg(this->id()) );
	collectWiresAux(wires, m_connector0);
	collectWiresAux(wires, m_connector1);
}

void Wire::collectWiresAux(QList<Wire *> & wires, ConnectorItem * start) {
	foreach (ConnectorItem * toConnectorItem, start->connectedToItems()) {
		if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			qobject_cast<Wire *>(toConnectorItem->attachedTo())->collectWires(wires);
		}
	}

}

bool Wire::stickyEnabled()
{
	return (connector0()->connectionsCount() <= 0) && (connector1()->connectionsCount() <= 0);
}

bool Wire::getTrace() {
	return m_viewGeometry.getAnyTrace();
}

bool Wire::getRouted() {
	return m_viewGeometry.getRouted();
}

void Wire::setRouted(bool routed) {
	m_viewGeometry.setRouted(routed);
}

void Wire::setRatsnest(bool ratsnest) {
	m_viewGeometry.setRatsnest(ratsnest);
}

bool Wire::getRatsnest() {
	return m_viewGeometry.getRatsnest();
}

void Wire::setAutoroutable(bool ar) {
	m_viewGeometry.setAutoroutable(ar);
}

bool Wire::getAutoroutable() {
	return m_viewGeometry.getAutoroutable();
}

void Wire::setNormal(bool normal) {
	m_viewGeometry.setNormal(normal);
}

bool Wire::getNormal() {
	return m_viewGeometry.getNormal();
}

void Wire::setColor(const QColor & color, double op) {
	m_pen.setBrush(QBrush(color));
	m_opacity = op;
	m_colorName = color.name();
	this->update();
}

void Wire::setShadowColor(QColor & color) {
	m_shadowBrush = QBrush(color);
	m_shadowPen.setBrush(m_shadowBrush);
	m_bendpointPen.setBrush(m_shadowBrush);
	m_bendpoint2Pen.setBrush(m_shadowBrush);
	if (m_connector0) m_connector0->restoreColor(false, 0, false);
	if (m_connector0) m_connector1->restoreColor(false, 0, false);
	this->update();
}

const QColor & Wire::color() {
	return m_pen.brush().color();
}

void Wire::setWireWidth(double width, InfoGraphicsView * infoGraphicsView, double hoverStrokeWidth) {
	if (m_pen.widthF() == width) return;

	prepareGeometryChange();
	setPenWidth(width, infoGraphicsView, hoverStrokeWidth);
	if (m_connector0) m_connector0->restoreColor(false, 0, false);
	if (m_connector1) m_connector1->restoreColor(false, 0, false);
	update();
}

double Wire::width() {
	return m_pen.widthF();
}

double Wire::shadowWidth() {
	return m_shadowPen.widthF();
}

double Wire::mils() {
	return 1000 * m_pen.widthF() / FSvgRenderer::printerScale();
}

void Wire::setColorString(QString colorName, double op) {
	// sets a color using the name (.e. "red")
	// note: colorName is associated with a Fritzing color, not a Qt color

	QString colorString = colors.value(colorName, "");
	if (colorString.isEmpty()) {
		colorString = colorName;

		foreach (QString c, colors.keys()) {
			if (colors.value(c).compare(colorName, Qt::CaseInsensitive) == 0) {
				colorName = c;
				break;
			}
		}
	}

	QColor c;
	c.setNamedColor(colorString);
	setColor(c, op);
	m_colorName = colorName;

	colorString = shadowColors.value(colorName, "");
	if (colorString.isEmpty()) {
		colorString = colorName;
	}

	c.setNamedColor(colorString);
	setShadowColor(c);
}

QString Wire::hexString() {
	return m_pen.brush().color().name();
}

QString Wire::shadowHexString() {
	return m_shadowPen.brush().color().name();
}

QString Wire::colorString() {
	return m_colorName;
}

void Wire::initNames() {
	if (colors.count() > 0) return;

	widths << 16 << 24 << 32 << 48;
	widthTrans.insert(widths[0], tr("thin (16 mil)"));
	widthTrans.insert(widths[1], tr("standard (24 mil)"));
	widthTrans.insert(widths[2], tr("thick (32 mil)"));
	widthTrans.insert(widths[3], tr("extra thick (48 mil)"));

	THIN_TRACE_WIDTH = GraphicsUtils::mils2pixels(widths[0], FSvgRenderer::printerScale());
	STANDARD_TRACE_WIDTH = GraphicsUtils::mils2pixels(widths[1], FSvgRenderer::printerScale());
	HALF_STANDARD_TRACE_WIDTH = STANDARD_TRACE_WIDTH / 2.0;

    // need a list because a hash table doesn't guarantee order 
    colorNames.append(tr("blue"));
	colorNames.append(tr("red"));
    colorNames.append(tr("black"));
	colorNames.append(tr("yellow"));
	colorNames.append(tr("green"));
	colorNames.append(tr("grey"));
	colorNames.append(tr("white"));
	colorNames.append(tr("orange"));
    colorNames.append(tr("brown"));
    colorNames.append(tr("purple"));
    colorNames.append(tr("schematic black"));

	// need this hash table to translate from user's language to internal color name
    colorTrans.insert(tr("blue"), "blue");
	colorTrans.insert(tr("red"), "red");
    colorTrans.insert(tr("black"), "black");
	colorTrans.insert(tr("yellow"), "yellow");
	colorTrans.insert(tr("green"), "green");
	colorTrans.insert(tr("grey"), "grey");
	colorTrans.insert(tr("white"), "white");
	colorTrans.insert(tr("orange"), "orange");
	colorTrans.insert(tr("brown"), "brown");
    colorTrans.insert(tr("purple"), "purple");
    colorTrans.insert(tr("schematic black"), "schematic black");

    colors.insert("blue",	"#418dd9");
	colors.insert("red",	"#cc1414");
    colors.insert("black",	"#404040");
	colors.insert("yellow", "#ffe24d");
	colors.insert("green",	"#47cc79");
	colors.insert("grey",	"#999999");
	colors.insert("white",	"#ffffff");
	colors.insert("orange", "#ff7033");
	colors.insert("jumper", ViewLayer::JumperColor);
	colors.insert("trace",  ViewLayer::Copper0Color);    
	colors.insert("trace1",  ViewLayer::Copper1Color);    
	//colors.insert("unrouted", "#000000");
	//colors.insert("blackblack", "#000000");
	colors.insert("schematicGrey", "#9d9d9d");
    colors.insert("purple", "#ab58a2");
	colors.insert("brown", "#8c3b00");
	colors.insert("schematic black", "#000000");

    shadowColors.insert("blue",		"#1b5bb3");
	shadowColors.insert("red",		"#8c0000");
    shadowColors.insert("black",	"#000000");
	shadowColors.insert("yellow",	"#e6ab00");
	shadowColors.insert("green",	"#00a63d");
	shadowColors.insert("grey",		"#666666");
	shadowColors.insert("white",	"#999999");
	shadowColors.insert("orange",	"#d95821");
    shadowColors.insert("jumper",	"#2d6563");
	shadowColors.insert("trace",	"#d69b00");
	shadowColors.insert("trace1",   "#d69b00");    
	//shadowColors.insert("unrouted", "#000000");
    shadowColors.insert("purple",	"#7a3a73");
    shadowColors.insert("brown",	"#6c2710");
	shadowColors.insert("schematicGrey", "#1d1d1d");
	//shadowColors.insert("blackblack", "#000000");
	shadowColors.insert("schematic black", "#000000");
}

bool Wire::hasFlag(ViewGeometry::WireFlag flag)
{
	return m_viewGeometry.hasFlag(flag);
}

bool Wire::hasAnyFlag(ViewGeometry::WireFlags flags)
{
	return m_viewGeometry.hasAnyFlag(flags);
}

Wire * Wire::findTraced(ViewGeometry::WireFlags flags, QList<ConnectorItem *>  & ends) {
	QList<Wire *> chainedWires;
	this->collectChained(chainedWires, ends);
	if (ends.count() != 2) {
		DebugDialog::debug(QString("wire in jumper or trace must have two ends") );
		return NULL;
	}

	return ConnectorItem::directlyWiredTo(ends[0], ends[1], flags);
}

QRgb Wire::getRgb(const QString & name) {
	QString str = colors.value(name);
	QColor c;
	c.setNamedColor(str);
	return c.rgb();
}

void Wire::setWireFlags(ViewGeometry::WireFlags wireFlags) {
	m_viewGeometry.setWireFlags(wireFlags);
}

double Wire::opacity() {
	return m_opacity;
}

void Wire::setOpacity(double opacity) {
	m_opacity = opacity;
	this->update();
}

bool Wire::draggingEnd() {
	return m_dragEnd || m_dragCurve;
}

void Wire::setCanChainMultiple(bool can) {
	m_canChainMultiple = can;
}

bool Wire::canChangeColor() {
	if (getRatsnest()) return false;
	if (!getTrace()) return true;

	return (this->m_viewIdentifier == ViewIdentifierClass::SchematicView);
}

void Wire::collectDirectWires(QList<Wire *> & wires) {
	if (!wires.contains(this)) {
		wires.append(this);
	}

	collectDirectWires(m_connector0, wires);
	collectDirectWires(m_connector1, wires);
}

void Wire::collectDirectWires(ConnectorItem * connectorItem, QList<Wire *> & wires) {
	if (connectorItem->connectionsCount() != 1) return;

	ConnectorItem * toConnectorItem = connectorItem->connectedToItems()[0];
	if (toConnectorItem->attachedToItemType() != ModelPart::Wire) return;

	Wire * nextWire = qobject_cast<Wire *>(toConnectorItem->attachedTo());
	if (wires.contains(nextWire)) return;

	wires.append(nextWire);
	nextWire->collectDirectWires(nextWire->otherConnector(toConnectorItem), wires);
}

QVariant Wire::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedChange) {
		if (m_partLabel) {
			m_partLabel->update();
		}

		if (!m_ignoreSelectionChange) {
			QList<Wire *> chained;
			QList<ConnectorItem *> ends;
			collectChained(chained, ends);
			InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
			if (infoGraphicsView) {
				infoGraphicsView->setIgnoreSelectionChangeEvents(true);
			}
			// DebugDialog::debug(QString("original wire selected %1 %2").arg(value.toBool()).arg(this->id()));
			foreach (Wire * wire, chained) {
				if (wire != this ) {
					wire->setIgnoreSelectionChange(true);
					wire->setSelected(value.toBool());
					wire->setIgnoreSelectionChange(false);
					// DebugDialog::debug(QString("wire selected %1 %2").arg(value.toBool()).arg(wire->id()));
				}
			}
			if (infoGraphicsView) {
				infoGraphicsView->setIgnoreSelectionChangeEvents(false);
			}
		}
    }
    return ItemBase::itemChange(change, value);
}

void Wire::cleanup() {
}

void Wire::getConnectedColor(ConnectorItem * connectorItem, QBrush * &brush, QPen * &pen, double & opacity, double & negativePenWidth, bool & negativeOffsetRect) {

	connectorItem->setBigDot(false);
	int count = 0;
	bool bendpoint = true;
	foreach (ConnectorItem * toConnectorItem, connectorItem->connectedToItems()) {
		if (toConnectorItem->attachedToItemType() != ModelPart::Wire) {
			// for drawing a big dot on the end of a part connector in schematic view if the part is connected to more than one trace
			bendpoint = false;
			if (toConnectorItem->connectionsCount() > 1) {	
				InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
				if (infoGraphicsView != NULL && infoGraphicsView->hasBigDots()) {
					int c = 0;
					foreach (ConnectorItem * totoConnectorItem, toConnectorItem->connectedToItems()) {
						if (totoConnectorItem->attachedToItemType() == ModelPart::Wire) {
							Wire * w = qobject_cast<Wire *>(totoConnectorItem->attachedTo());
							if (w && w->getTrace()) {
								c++;
							}
						}
					}
					if (c > 1) {
						count = 2;
						break;
					}
				}
			}

			ItemBase::getConnectedColor(connectorItem, brush, pen, opacity, negativePenWidth, negativeOffsetRect);
			return;
		}

		count++;
	}

	if (count == 0) {
		ItemBase::getConnectedColor(connectorItem, brush, pen, opacity, negativePenWidth, negativeOffsetRect);
		return;
	}
	
	// connectorItem is a bendpoint or connects to a multiply connected connector

	//if (!bendpoint) {
		//DebugDialog::debug(QString("big dot %1 %2 %3").arg(this->id()).arg(connectorItem->connectorSharedID()).arg(count));
	//}

	brush = &m_shadowBrush;
	opacity = 1.0;
	if (count > 1) {
		// only ever reach here when drawing a connector that is connected to more than one trace
		pen = &m_bendpoint2Pen;
		negativePenWidth = m_bendpoint2Width;
		negativeOffsetRect = m_negativeOffsetRect;
		connectorItem->setBigDot(true);
	}
	else {
		negativeOffsetRect = m_negativeOffsetRect;
		negativePenWidth = m_bendpointWidth;
		pen = &m_bendpointPen;
	}
}

void Wire::setPenWidth(double w, InfoGraphicsView * infoGraphicsView, double hoverStrokeWidth) {
	m_hoverStrokeWidth = hoverStrokeWidth;
	//DebugDialog::debug(QString("setting hoverstrokewidth %1 %2").arg(m_id).arg(m_hoverStrokeWidth));
	m_pen.setWidthF(w);
	infoGraphicsView->getBendpointWidths(this, w, m_bendpointWidth, m_bendpoint2Width, m_negativeOffsetRect);
	m_bendpointPen.setWidthF(qAbs(m_bendpointWidth));
	m_bendpoint2Pen.setWidthF(qAbs(m_bendpoint2Width));
	m_shadowPen.setWidthF(w + 2);
}

bool Wire::connectionIsAllowed(ConnectorItem * to) {
	if (!ItemBase::connectionIsAllowed(to)) return false;

	Wire * w = qobject_cast<Wire *>(to->attachedTo());
	if (w == NULL) return true;

	if (w->getRatsnest()) return false;

	return m_viewIdentifier != ViewIdentifierClass::BreadboardView;
}

bool Wire::isGrounded() {
	return ConnectorItem::isGrounded(connector0(), connector1());
}

bool Wire::acceptsMouseDoubleClickConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *) {
	return true;
}

bool Wire::acceptsMouseMoveConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *) {
	return true;
}

bool Wire::acceptsMouseReleaseConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *) {
	return true;
}

void Wire::setIgnoreSelectionChange(bool ignore) {
	m_ignoreSelectionChange = ignore;
}

bool Wire::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget)
{
	if (prop.compare("width", Qt::CaseInsensitive) == 0) {
		// don't display width property
		return false;
	}

	if (prop.compare("color", Qt::CaseInsensitive) == 0) {
		returnProp = tr("color");
		if (canChangeColor()) {
			QComboBox * comboBox = new QComboBox(parent);
			comboBox->setEditable(false);
			comboBox->setEnabled(swappingEnabled);
			comboBox->setObjectName("infoViewComboBox");
			
			int ix = 0;
			QString englishCurrColor = colorString();
			foreach(QString transColorName, Wire::colorNames) {
				QString englishColorName = Wire::colorTrans.value(transColorName);
				comboBox->addItem(transColorName, QVariant(englishColorName));
				if (englishColorName.compare(englishCurrColor, Qt::CaseInsensitive) == 0) {
					comboBox->setCurrentIndex(ix);
				}
				ix++;
			}

			connect(comboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(colorEntry(const QString &)));
			returnWidget = comboBox;
			returnValue = comboBox->currentText();
			return true;
		}
		else {
			returnWidget = NULL;
			returnValue = colorString();
			return true;
		}
	}

	return ItemBase::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

void Wire::colorEntry(const QString & text) {
	Q_UNUSED(text);

	QComboBox * comboBox = qobject_cast<QComboBox *>(sender());
	if (comboBox == NULL) return;

	QString color = comboBox->itemData(comboBox->currentIndex()).toString();

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->changeWireColor(color);
	}
}

bool Wire::hasPartLabel() {
	
	return false;
}

ItemBase::PluralType Wire::isPlural() {
	return Plural;
}

void Wire::checkVisibility(ConnectorItem * onMe, ConnectorItem * onIt, bool connect) {
	if (connect) {
		if (!onIt->attachedTo()->isVisible()) {
			this->setVisible(false);
		}
		else {
			ConnectorItem * other = otherConnector(onMe);
			foreach (ConnectorItem * toConnectorItem, other->connectedToItems()) {
				if (toConnectorItem->attachedToItemType() == ModelPart::Wire) continue;

				if (!toConnectorItem->attachedTo()->isVisible()) {
					this->setVisible(false);
					break;
				}
			}
		}
	}
}

bool Wire::canSwitchLayers() {
	return false;
}

bool Wire::hasPartNumberProperty()
{
	return false;
}

bool Wire::rotationAllowed() {
	return false;
}

bool Wire::rotation45Allowed() {
	return false;
}

void Wire::addedToScene(bool temporary) {
	ItemBase::addedToScene(temporary);

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView == NULL) return;

	bool succeeded = connect(this, SIGNAL(wireChangedSignal(Wire*, const QLineF & , const QLineF & , QPointF, QPointF, ConnectorItem *, ConnectorItem *)	),
			infoGraphicsView, SLOT(wireChangedSlot(Wire*, const QLineF & , const QLineF & , QPointF, QPointF, ConnectorItem *, ConnectorItem *)),
			Qt::DirectConnection);		// DirectConnection means call the slot directly like a subroutine, without waiting for a thread or queue
	succeeded = connect(this, SIGNAL(wireChangedCurveSignal(Wire*, const Bezier *, const Bezier *, bool)),
			infoGraphicsView, SLOT(wireChangedCurveSlot(Wire*, const Bezier *, const Bezier *, bool)),
			Qt::DirectConnection);		// DirectConnection means call the slot directly like a subroutine, without waiting for a thread or queue
	succeeded = succeeded && connect(this, SIGNAL(wireSplitSignal(Wire*, QPointF, QPointF, const QLineF & )),
			infoGraphicsView, SLOT(wireSplitSlot(Wire*, QPointF, QPointF, const QLineF & )));
	succeeded = succeeded && connect(this, SIGNAL(wireJoinSignal(Wire*, ConnectorItem *)),
			infoGraphicsView, SLOT(wireJoinSlot(Wire*, ConnectorItem*)));
	if (!succeeded) {
		DebugDialog::debug("wire signal connect failed");
	}
}

void Wire::setConnectorDimensions(double width, double height) 
{
	setConnectorDimensionsAux(connector0(), width, height);
	setConnectorDimensionsAux(connector1(), width, height);
}

void Wire::setConnectorDimensionsAux(ConnectorItem * connectorItem, double width, double height) 
{
	QPointF p = connectorItem->rect().center();
	QRectF r(p.x() - (width / 2), p.y() - (height / 2), width, height);
	connectorItem->setRect(r);
	connectorItem->setTerminalPoint(r.center() - r.topLeft());
}

void Wire::originalConnectorDimensions(double & width, double & height) 
{
	width = m_originalConnectorRect.width();
	height = m_originalConnectorRect.height();
}

bool Wire::isBendpoint(ConnectorItem * connectorItem) {
	return connectorItem->isBendpoint();
}

double Wire::hoverStrokeWidth() {
	return m_hoverStrokeWidth;
}

const QLineF & Wire::getPaintLine() {
	return m_line;
}

/*!
    Returns the item's line, or a null line if no line has been set.

    \sa setLine()
*/
QLineF Wire::line() const
{
    return m_line;
}

/*!
    Sets the item's line to be the given \a line.

    \sa line()
*/
void Wire::setLine(const QLineF &line)
{
    if (m_line == line)
        return;
    prepareGeometryChange();
    m_line = line;
    update();
}

void Wire::setLine(double x1, double y1, double x2, double y2)
{ 
	setLine(QLineF(x1, y1, x2, y2)); 
}

/*!
    Returns the item's pen, or a black solid 0-width pen if no pen has
    been set.

    \sa setPen()
*/
QPen Wire::pen() const
{
    return m_pen;
}

/*!
    Sets the item's pen to \a pen. If no pen is set, the line will be painted
    using a black solid 0-width pen.

    \sa pen()
*/
void Wire::setPen(const QPen &pen)
{
	if (pen.widthF() != m_pen.widthF()) {
		prepareGeometryChange();
	}
    m_pen = pen;
    update();
}

bool Wire::canHaveCurve() {
	return m_canHaveCurve && (m_viewIdentifier == ViewIdentifierClass::BreadboardView);
}

void Wire::dragCurve(QPointF eventPos, Qt::KeyboardModifiers)
{
	m_bezier->recalc(eventPos);
}

void Wire::changeCurve(const Bezier * bezier)
{
	prepareGeometryChange();
	if (m_bezier == NULL) m_bezier = new Bezier;
	m_bezier->copy(bezier);
	update();
}

bool Wire::isCurved() {
	return (m_bezier != NULL) && !m_bezier->isEmpty();
}

const Bezier * Wire::curve() {
	return m_bezier;
}

const Bezier * Wire::undoCurve() {
	return &UndoBezier;
}

QPolygonF Wire::sceneCurve(QPointF offset) {
	QPolygonF poly;
	if (m_bezier == NULL) return poly;
	if (m_bezier->isEmpty()) return poly;

	poly.append(m_line.p1() + pos() - offset);
	poly.append(m_bezier->cp0() + pos() - offset);
	poly.append(m_bezier->cp1() + pos() - offset);
	poly.append(m_line.p2() + pos() - offset);
	return poly;
}

bool Wire::hasShadow() {
	if (getRatsnest()) return false;
	if (getTrace()) return false;
	return m_pen.widthF() != m_shadowPen.widthF();
}

bool Wire::eventFilter(QObject * object, QEvent * event)
{
	Q_UNUSED(object);
	if (!(m_dragEnd || m_dragCurve)) {
		if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
			InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);;
			if (infoGraphicsView) {
				QPoint p = infoGraphicsView->mapFromGlobal(QCursor::pos());
				QPointF r = infoGraphicsView->mapToScene(p);
				QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
				// DebugDialog::debug(QString("got key event %1").arg(keyEvent->modifiers()));
				updateCursor(keyEvent->modifiers());
			}
		}
	}

	return false;
}

void Wire::updateCursor(Qt::KeyboardModifiers modifiers)
{
	if (m_connectorHover) {
		return;
	}

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	bool segment = false;
	int totalConnections = 0;
	foreach (ConnectorItem * connectorItem, cachedConnectorItems()) {
		totalConnections += connectorItem->connectionsCount();
	}
	if (totalConnections == 2 && modifiers & altOrMetaModifier()) {
		segment = true;
		foreach (ConnectorItem * connectorItem, cachedConnectorItems()) {
			if (connectorItem->connectionsCount() != 1) {
				segment = false;
				break;
			}

			ConnectorItem * toConnectorItem = connectorItem->connectedToItems().at(0);
			if (toConnectorItem->attachedToItemType() != ModelPart::Wire) {
				segment = false;
				break;
			}
		}
	}
		
	if (segment) {
		// dragging a segment of wire between bounded by two other wires
		CursorMaster::instance()->addCursor(this, *CursorMaster::RubberbandCursor);
	}
	else if (totalConnections == 0) {
		// only in breadboard view
		CursorMaster::instance()->addCursor(this, *CursorMaster::MoveCursor);
	}
	else if (infoGraphicsView != NULL && infoGraphicsView->curvyWiresIndicated(modifiers)) {
		CursorMaster::instance()->addCursor(this, *CursorMaster::MakeCurveCursor);
	}
	else {
		CursorMaster::instance()->addCursor(this, *CursorMaster::NewBendpointCursor);
	}
}

bool Wire::canChainMultiple()
{
	return m_canChainMultiple;
}
