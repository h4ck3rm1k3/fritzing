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

#include "connectoritem.h"

#include <QBrush>
#include <QPen>
#include <QColor>
#include <limits>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QTooltip>

#include "../sketch/infographicsview.h"
#include "../debugdialog.h"
#include "bus.h"
#include "../items/wire.h"
#include "../items/virtualwire.h"
#include "../model/modelpart.h"
#include "../utils/graphicsutils.h"
#include "../utils/graphutils.h"
#include "../utils/ratsnestcolors.h"
#include "ercdata.h"

/////////////////////////////////////////////////////////

QList<ConnectorItem *> ConnectorItem::m_equalPotentialDisplayItems;

const QList<ConnectorItem *> ConnectorItem::emptyConnectorItemList;

static double MAX_DOUBLE = std::numeric_limits<double>::max();

bool wireLessThan(ConnectorItem * c1, ConnectorItem * c2)
{
	if (c1->connectorType() == c2->connectorType()) {
		// if they're the same type return the topmost
		return c1->zValue() > c2->zValue();
	}
	if (c1->connectorType() == Connector::Female) {
		// choose the female first
		return true;
	}
	if (c2->connectorType() == Connector::Female) {
		// choose the female first
		return false;
	}
	if (c1->connectorType() == Connector::Male) {
		// choose the male first
		return true;
	}
	if (c2->connectorType() == Connector::Male) {
		// choose the male first
		return false;
	}
	if (c1->connectorType() == Connector::Pad) {
		// choose the pad first
		return true;
	}
	if (c2->connectorType() == Connector::Pad) {
		// choose the pad first
		return false;
	}

	// Connector::Wire last
	return c1->zValue() > c2->zValue();

}

/////////////////////////////////////////////////////////

ConnectorItem::ConnectorItem( Connector * connector, ItemBase * attachedTo )
	: NonConnectorItem(attachedTo)
{
	m_bendable = m_bigDot = m_hybrid = false;
	m_lineItem = NULL;
	m_marked = false;
	m_checkedEffectively = false;
	m_hoverEnterSpaceBarWasPressed = m_spaceBarWasPressed = false;
	m_overConnectorItem = NULL;
	m_connectorHovering = false;
	m_connector = connector;
	if (connector != NULL) {
		connector->addViewItem(this);
	}
    setAcceptHoverEvents(true);
    this->setCursor(Qt::CrossCursor);

	//DebugDialog::debug(QString("%1 attached to %2")
			//.arg(this->connector()->connectorShared()->id())
			//.arg(attachedTo->modelPartShared()->title()) );
}

ConnectorItem::~ConnectorItem() {
	m_equalPotentialDisplayItems.removeOne(this);
	//DebugDialog::debug(QString("deleting connectorItem %1").arg((long) this, 0, 16));
	foreach (ConnectorItem * connectorItem, m_connectedTo) {
		if (connectorItem != NULL) {
			//DebugDialog::debug(QString("temp remove %1 %2").arg(this->attachedToID()).arg(connectorItem->attachedToID()));
			connectorItem->tempRemove(this, this->attachedToID() != connectorItem->attachedToID());
		}
	}
	if (this->connector() != NULL) {
		this->connector()->removeViewItem(this);
	}
}

void ConnectorItem::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) {

	/*
	QRectF sbr = this->sceneBoundingRect();
	QPointF p = event->scenePos();

	debugInfo(QString("hover %1, %2 %3 %4 %5, %6 %7")
		.arg((long) this, 0, 16)
		.arg(sbr.left())
		.arg(sbr.top())
		.arg(sbr.width())
		.arg(sbr.height())
		.arg(p.x())
		.arg(p.y())
		);
	*/

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL && infoGraphicsView->spaceBarIsPressed()) {
		m_hoverEnterSpaceBarWasPressed = true;
		event->ignore();
		return;
	}

	m_hoverEnterSpaceBarWasPressed = false;
	setHoverColor();
	if (infoGraphicsView != NULL) {
		infoGraphicsView->hoverEnterConnectorItem(event, this);
	}
	if (this->m_attachedTo != NULL) {
		m_attachedTo->hoverEnterConnectorItem(event, this);
	}
}

void ConnectorItem::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) {
	if (m_hoverEnterSpaceBarWasPressed) {
		event->ignore();
		return;
	}

	restoreColor(false, 0, true);
	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->hoverLeaveConnectorItem(event, this);
	}
	if (this->m_attachedTo != NULL) {
		m_attachedTo->hoverLeaveConnectorItem(event, this);
	}
}

void ConnectorItem::hoverMoveEvent ( QGraphicsSceneHoverEvent * event ) {
	if (m_hoverEnterSpaceBarWasPressed) {
		event->ignore();
		return;
	}

	if (this->m_attachedTo != NULL) {
		m_attachedTo->hoverMoveConnectorItem(event, this);
	}
}

Connector * ConnectorItem::connector() {
	return m_connector;
}

void ConnectorItem::clearConnectorHover() {
	m_connectorHovering = false;
}

void ConnectorItem::connectorHover(ItemBase * itemBase, bool hovering) {
	m_connectorHovering = hovering;
	if (hovering) {
		setHoverColor();			// could make this light up buses as well
	}
	else {
		restoreColor(false, 0, true);
	}
	if (this->m_attachedTo != NULL) {
		m_attachedTo->connectorHover(this, itemBase, hovering);
	}
}

bool ConnectorItem::connectorHovering() {
	return m_connectorHovering;
}

void ConnectorItem::connectTo(ConnectorItem * connected) {
	if (m_connectedTo.contains(connected)) return;

	m_connectedTo.append(connected);
	//DebugDialog::debug(QString("connect to cc:%4 this:%1 to:%2 %3").arg((long) this, 0, 16).arg((long) connected, 0, 16).arg(connected->attachedTo()->modelPartShared()->title()).arg(m_connectedTo.count()) );
	restoreColor(true, 0, true);
	if (m_attachedTo != NULL) {
		m_attachedTo->connectionChange(this, connected, true);
	}

	updateTooltip();
}

ConnectorItem * ConnectorItem::removeConnection(ItemBase * itemBase) {
	for (int i = 0; i < m_connectedTo.count(); i++) {
		if (m_connectedTo[i]->attachedTo() == itemBase) {
			ConnectorItem * removed = m_connectedTo[i];
			m_connectedTo.removeAt(i);
			if (m_attachedTo != NULL) {
				m_attachedTo->connectionChange(this, removed, false);
			}
			restoreColor(true, 0, true);
			DebugDialog::debug(QString("remove from:%1 to:%2 count%3")
				.arg((long) this, 0, 16)
				.arg(itemBase->modelPartShared()->title())
				.arg(m_connectedTo.count()) );
			updateTooltip();
			return removed;
		}
	}

	return NULL;
}

void ConnectorItem::removeConnection(ConnectorItem * connectedItem, bool emitChange) {
	if (connectedItem == NULL) return;

	m_connectedTo.removeOne(connectedItem);
	restoreColor(true, 0, true);
	if (emitChange) {
		m_attachedTo->connectionChange(this, connectedItem, false);
	}
	updateTooltip();
}

void ConnectorItem::tempConnectTo(ConnectorItem * item, bool applyColor) {
	if (!m_connectedTo.contains(item)) m_connectedTo.append(item);
	updateTooltip();

	if(applyColor) restoreColor(true, 0, true);
}

void ConnectorItem::tempRemove(ConnectorItem * item, bool applyColor) {
	m_connectedTo.removeOne(item);
	updateTooltip();

	if(applyColor) restoreColor(true, 0, true);
}

void ConnectorItem::restoreColor(bool doBuses, int busConnectionCount, bool doCross) 
{
	setMarked(true);
	if (!attachedTo()->isEverVisible()) return;

	/*
		DebugDialog::debug(QString("restore color dobus:%1 bccount:%2 docross:%3 cid:'%4' '%5' id:%6 '%7' vid:%8 vlid:%9")
		.arg(doBuses)
		.arg(busConnectionCount)
		.arg(doCross)
		.arg(this->connectorSharedID())
		.arg(this->connectorSharedName())
		.arg(this->attachedToID())
		.arg(this->attachedToInstanceTitle())
		.arg(this->attachedToViewIdentifier())
		.arg(this->attachedToViewLayerID())
		);
		*/

	int connectedToCount = busConnectionCount;
	if (attachedToItemType() == ModelPart::Wire) {
		doBuses = doCross = false;
		foreach (ConnectorItem * toConnectorItem, m_connectedTo) {
			if (toConnectorItem->attachedTo()->isEverVisible()) {
				connectedToCount = 1;
				break;
			}
		}
	}
	else if (connectedToCount == 0) {
		connectedToCount = isConnectedToPart() ? 1 : 0;
	}

	if (doBuses) {
		Bus * b = bus();
		if (b != NULL) {
			QList<ConnectorItem *> busConnectedItems;
			attachedTo()->busConnectorItems(b, busConnectedItems);
			busConnectedItems.removeOne(this);
			foreach (ConnectorItem * busConnectorItem, busConnectedItems) {
				busConnectorItem->restoreColor(false, connectedToCount, true);
			}
		}
	}

	if (doCross) {
		ConnectorItem * crossConnectorItem = getCrossLayerConnectorItem();
		if (crossConnectorItem) {
			crossConnectorItem->restoreColor(false, connectedToCount, false);
		}
	}

	QString how;
	if (connectedToCount <= 0) {
		if (connectorType() == Connector::Female) {
			setNormalColor();
			how = "normal";	
		}
		else {
			setUnconnectedColor();
			how = "unconnected";
		}
	}
	else {
		setConnectedColor();
		how = "connected";
	}

	/*
	DebugDialog::debug(QString("restore color dobus:%1 bccount:%2 docross:%3 cid:'%4' '%5' id:%6 '%7' vid:%8 vlid:%9 %10")
		.arg(doBuses)
		.arg(busConnectionCount)
		.arg(doCross)
		.arg(this->connectorSharedID())
		.arg(this->connectorSharedName())
		.arg(this->attachedToID())
		.arg(this->attachedToInstanceTitle())
		.arg(this->attachedToViewIdentifier())
		.arg(this->attachedToViewLayerID())
		.arg(how)
	);

	*/
}

void ConnectorItem::setConnectedColor() {
	if (m_attachedTo == NULL) return;

	QBrush * brush = NULL;
	QPen * pen = NULL;
	m_attachedTo->getConnectedColor(this, brush, pen, m_opacity, m_negativePenWidth, m_negativeOffsetRect);
	//DebugDialog::debug(QString("set connected %1 %2").arg(attachedToID()).arg(pen->width()));
	setColorAux(*brush, *pen, true);
}

void ConnectorItem::setNormalColor() {
	if (m_attachedTo == NULL) return;

	QBrush * brush = NULL;
	QPen * pen = NULL;
	m_attachedTo->getNormalColor(this, brush, pen, m_opacity, m_negativePenWidth, m_negativeOffsetRect);
	//DebugDialog::debug(QString("set normal %1 %2").arg(attachedToID()).arg(pen->width()));
	setColorAux(*brush, *pen, false);
}

void ConnectorItem::setUnconnectedColor() {
	if (m_attachedTo == NULL) return;

	QBrush * brush = NULL;
	QPen * pen = NULL;
	//DebugDialog::debug(QString("set unconnected %1").arg(attachedToID()) );
	m_attachedTo->getUnconnectedColor(this, brush, pen, m_opacity, m_negativePenWidth, m_negativeOffsetRect);
	setColorAux(*brush, *pen, true);
}

void ConnectorItem::setHoverColor() {
	if (m_attachedTo == NULL) return;

	QBrush * brush = NULL;
	QPen * pen = NULL;
	m_attachedTo->getHoverColor(this, brush, pen, m_opacity, m_negativePenWidth, m_negativeOffsetRect);
	setColorAux(*brush, *pen, true);
}

void ConnectorItem::setColorAux(QBrush brush, QPen pen, bool paint) {
	m_paint = paint;
	this->setBrush(brush);
	this->setPen(pen);
	update();
}

void ConnectorItem::setColorAux(const QColor &color, bool paint) {
	m_paint = paint;
	this->setBrush(QBrush(color));
	this->setPen(QPen(color));
	update();
}

void ConnectorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {

	// not clear when this code actually gets invoked
	//DebugDialog::debug("in connectorItem mouseReleaseEvent");
	clearEqualPotentialDisplay();

	if (m_bendable) {
		QGraphicsRectItem::mouseReleaseEvent(event);
		ConnectorItem * to = releaseDrag();
		return;
	}

	if (this->m_attachedTo != NULL && m_attachedTo->acceptsMouseReleaseConnectorEvent(this, event)) {
		m_attachedTo->mouseReleaseConnectorEvent(this, event);
		return;
	}

	QGraphicsRectItem::mouseReleaseEvent(event);
}

void ConnectorItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
	if (this->m_attachedTo != NULL && m_attachedTo->acceptsMouseDoubleClickConnectorEvent(this, event)) {
		m_attachedTo->mouseDoubleClickConnectorEvent(this, event);
		return;
	}

	QGraphicsRectItem::mouseDoubleClickEvent(event);
}

void ConnectorItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {

	if (m_bendable) {
		QGraphicsRectItem::mouseMoveEvent(event);

		QPointF p = this->mapToParent(rect().topLeft() + m_terminalPoint);
		if (p != m_originalPoint) {
			m_lineItem->setLine(m_originalPoint.x(), m_originalPoint.y(), p.x(), p.y());
			m_lineItem->setVisible(true);
		}
		else {
			m_lineItem->setVisible(false);
		}

		QList<ConnectorItem *> exclude;
		findConnectorUnder(true, true, exclude, true, this);

		return;
	}

	if (this->m_attachedTo != NULL && m_attachedTo->acceptsMouseMoveConnectorEvent(this, event)) {
		m_attachedTo->mouseMoveConnectorEvent(this, event);
		return;
	}

	QGraphicsRectItem::mouseMoveEvent(event);
}

void ConnectorItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if (event->button() != Qt::LeftButton) {
		QGraphicsRectItem::mousePressEvent(event);
		return;
	}

	if (m_attachedTo->filterMousePressConnectorEvent(this, event)) {
		event->ignore();
		return;
	}

	clearEqualPotentialDisplay();

	InfoGraphicsView *infographics = InfoGraphicsView::getInfoGraphicsView(this);
	if (infographics != NULL && infographics->spaceBarIsPressed()) {
		event->ignore();
		return;
	}

	m_equalPotentialDisplayItems.append(this);
	collectEqualPotential(m_equalPotentialDisplayItems, true, ViewGeometry::NoFlag);
	//m_equalPotentialDisplayItems.removeAt(0);									// not sure whether to leave the clicked one in or out of the list
	foreach (ConnectorItem * connectorItem, m_equalPotentialDisplayItems) {
		connectorItem->showEqualPotential(true);
	}

	if (m_bendable) {
		QGraphicsRectItem::mousePressEvent(event);
		return;
	}


	if (this->m_attachedTo != NULL && m_attachedTo->acceptsMousePressConnectorEvent(this, event)) {
		m_attachedTo->mousePressConnectorEvent(this, event);
		return;
	}

	QGraphicsRectItem::mousePressEvent(event);
}

int ConnectorItem::connectionsCount() {
	return m_connectedTo.count();
}


void ConnectorItem::attachedMoved() {
	//DebugDialog::debug("attached moved");
	foreach (ConnectorItem * toConnector, m_connectedTo) {
		ItemBase * itemBase = toConnector->attachedTo();
		if (itemBase == NULL) continue;
		if (itemBase->parentItem()) {
			// part of a group so don't move it separately
			continue;
		}

		itemBase->connectedMoved(this, toConnector);
	}
}

ConnectorItem * ConnectorItem::firstConnectedToIsh() {
	if (m_connectedTo.count() <= 0) return NULL;

	foreach (ConnectorItem * connectorItem, m_connectedTo) {
		if (!connectorItem->attachedTo()->getRatsnest()) return connectorItem;
	}

	return NULL;
}

void ConnectorItem::setTerminalPoint(QPointF p) {
	m_terminalPoint = p;
}

QPointF ConnectorItem::terminalPoint() {
	return m_terminalPoint;
}

QPointF ConnectorItem::adjustedTerminalPoint() {
	return m_terminalPoint + this->rect().topLeft();
}

QPointF ConnectorItem::sceneAdjustedTerminalPoint(ConnectorItem * connectee) {

	if ((connectee != NULL) && !m_circular && !m_shape.isEmpty() && (connectee->attachedToItemType() == ModelPart::Wire)) {
		Wire * wire = dynamic_cast<Wire *>(connectee->attachedTo());
		if ((wire != NULL) && !wire->getRatsnest()) {
			QPointF anchor = wire->otherConnector(connectee)->sceneAdjustedTerminalPoint(NULL);
			double newX = 0, newY = 0, newDistance = MAX_DOUBLE;
			int count = m_shape.elementCount();

			QPointF prev;
			for (int i = 0; i < count; i++) {
				QPainterPath::Element el = m_shape.elementAt(i);
				if (el.isMoveTo()) {
					prev = this->mapToScene(QPointF(el));
				}
				else {
					QPointF current = this->mapToScene(QPointF(el));
					double candidateX, candidateY, candidateDistance;
					bool atEndpoint;
					GraphicsUtils::distanceFromLine(anchor.x(), anchor.y(), prev.x(), prev.y(), current.x(), current.y(), 
										candidateX, candidateY, candidateDistance, atEndpoint);
					if (candidateDistance < newDistance) {
						newX = candidateX;
						newY = candidateY;
						newDistance = candidateDistance;
						//DebugDialog::debug(QString("anchor:%1,%2; new:%3,%4; %5").arg(anchor.x()).arg(anchor.y()).arg(newX).arg(newY).arg(newDistance));
					}

					prev = current;
				}
			}

			//DebugDialog::debug(QString("anchor:%1,%2; new:%3,%4; %5\n\n").arg(anchor.x()).arg(anchor.y()).arg(newX).arg(newY).arg(newDistance));
			return QPointF(newX, newY);
		}
	}

	return this->mapToScene(m_terminalPoint + this->rect().topLeft());
}

bool ConnectorItem::connectedTo(ConnectorItem * connectorItem) {
	return this->m_connectedTo.contains(connectorItem);
}

const QList< QPointer<ConnectorItem> > & ConnectorItem::connectedToItems() {
	return m_connectedTo;
}

void ConnectorItem::setHidden(bool hide) {
	m_hidden = hide;

	setHiddenOrInactive();
}

void ConnectorItem::setHybrid(bool h) {
	m_hybrid = h;
	setHiddenOrInactive();
}

bool ConnectorItem::isHybrid() {
	return m_hybrid;
}

void ConnectorItem::setBendable(QColor color, qreal strokeWidth) {
	m_bendable = true;
	setFlag(QGraphicsItem::ItemIsMovable, true);
	m_originalPoint = this->mapToParent(rect().topLeft() + m_terminalPoint);
	m_lineItem = new QGraphicsLineItem(parentItem());
	m_lineItem->setVisible(false);
	m_lineItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
	m_lineItem->setFlag(QGraphicsItem::ItemIsMovable, false);
	m_lineItem->setAcceptedMouseButtons(Qt::NoButton);
	m_lineItem->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	QPen pen(color);
	pen.setCapStyle(Qt::RoundCap);
	pen.setWidthF(strokeWidth);
	m_lineItem->setPen(pen);
	setAcceptedMouseButtons(Qt::LeftButton);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

bool ConnectorItem::isBendable() {
	return m_bendable;
}

void ConnectorItem::setBigDot(bool bd) {
	m_bigDot = bd;
}

bool ConnectorItem::isBigDot() {
	return m_bigDot;
}

void ConnectorItem::setInactive(bool inactivate) {
	m_inactive = inactivate;
	setHiddenOrInactive();
}

void ConnectorItem::setHiddenOrInactive() {
	if (m_hidden || m_inactive || m_hybrid) {
		this->setAcceptedMouseButtons(Qt::NoButton);
		this->unsetCursor();
		setAcceptHoverEvents(false);
	}
	else {
		this->setAcceptedMouseButtons(ALLMOUSEBUTTONS);
		this->setCursor(Qt::CrossCursor);
		setAcceptHoverEvents(true);
	}
	this->update();
}

ConnectorItem * ConnectorItem::overConnectorItem() {
	return m_overConnectorItem;
}

void ConnectorItem::setOverConnectorItem(ConnectorItem * connectorItem) {
	m_overConnectorItem = connectorItem;
}


const QString & ConnectorItem::connectorSharedID() {
	if (m_connector == NULL) return ___emptyString___;

	return m_connector->connectorSharedID();
}

ErcData * ConnectorItem::connectorSharedErcData() {
	if (m_connector == NULL) return NULL;

	return m_connector->connectorSharedErcData();
}

const QString & ConnectorItem::connectorSharedName() {
	if (m_connector == NULL) return ___emptyString___;

	return m_connector->connectorSharedName();
}

const QString & ConnectorItem::connectorSharedDescription() {
	if (m_connector == NULL) return ___emptyString___;

	return m_connector->connectorSharedDescription();
}

const QString & ConnectorItem::busID() {
	if (m_connector == NULL) return ___emptyString___;

	return m_connector->busID();
}

ModelPartShared * ConnectorItem::modelPartShared() {
	if (m_attachedTo == NULL) return NULL;

	return m_attachedTo->modelPartShared();
}


ModelPart * ConnectorItem::modelPart() {
	if (m_attachedTo == NULL) return NULL;

	return m_attachedTo->modelPart();
}

Bus * ConnectorItem::bus() {
	if (m_connector == NULL) return NULL;

	return m_connector->bus();
}

int ConnectorItem::attachedToItemType() {
	if (m_attachedTo == NULL) return ModelPart::Unknown;

	return m_attachedTo->itemType();
}

ViewLayer::ViewLayerID ConnectorItem::attachedToViewLayerID() {
	if (m_attachedTo == NULL) return ViewLayer::UnknownLayer;

	return m_attachedTo->viewLayerID();
}

ViewLayer::ViewLayerSpec ConnectorItem::attachedToViewLayerSpec() {
	if (m_attachedTo == NULL) return ViewLayer::UnknownSpec;

	return m_attachedTo->viewLayerSpec();
}

ViewIdentifierClass::ViewIdentifier ConnectorItem::attachedToViewIdentifier() {
	if (m_attachedTo == NULL) return ViewIdentifierClass::UnknownView;

	return m_attachedTo->viewIdentifier();
}

Connector::ConnectorType ConnectorItem::connectorType() {
	if (m_connector == NULL) return Connector::Unknown;

	return m_connector->connectorType();
}

bool ConnectorItem::chained() {
	foreach (ConnectorItem * toConnectorItem, m_connectedTo) {
		if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			return true;
		}
	}

	return false;
}

void ConnectorItem::writeTopLevelAttributes(QXmlStreamWriter & writer) {
	// do not write anything other than attributes in this routine.l
	writer.writeAttribute("layer", ViewLayer::viewLayerXmlNameFromID(attachedToViewLayerID()));
}

void ConnectorItem::saveInstance(QXmlStreamWriter & writer) {
	if (m_connectedTo.count() <= 0) {
		// no need to save if there's no connection
		return;
	}

	writer.writeStartElement("connector");
	writer.writeAttribute("connectorId", connectorSharedID());
	writeTopLevelAttributes(writer);

	writer.writeStartElement("geometry");
	writer.writeAttribute("x", QString::number(this->pos().x()));
	writer.writeAttribute("y", QString::number(this->pos().y()));
	writer.writeEndElement();

	if (m_connectedTo.count() > 0) {
		writer.writeStartElement("connects");
		foreach (ConnectorItem * connectorItem, this->m_connectedTo) {
			if (connectorItem->attachedTo()->getRatsnest()) continue;

			connectorItem->writeConnector(writer, "connect");
		}
		writer.writeEndElement();
	}

	writeOtherElements(writer);

	writer.writeEndElement();
}


void ConnectorItem::writeConnector(QXmlStreamWriter & writer, const QString & elementName)
{
	//DebugDialog::debug(QString("write connector %1").arg(this->attachedToID()));
	writer.writeStartElement(elementName);
	writer.writeAttribute("connectorId", connectorSharedID());
	writer.writeAttribute("modelIndex", QString::number(connector()->modelIndex()));
	writer.writeAttribute("layer", ViewLayer::viewLayerXmlNameFromID(attachedToViewLayerID()));
	writer.writeEndElement();
}

void ConnectorItem::writeOtherElements(QXmlStreamWriter & writer) {
	Q_UNUSED(writer);
}

bool ConnectorItem::maleToFemale(ConnectorItem * other) {
	
	if (this->connectorType() == Connector::Male && other->connectorType() == Connector::Female) return true;
	if (this->connectorType() == Connector::Female && other->connectorType() == Connector::Male) return true;

	return false;
}

bool ConnectorItem::wiredTo(ConnectorItem * target, ViewGeometry::WireFlags skipFlags) {
	QList<ConnectorItem *> connectorItems;
	connectorItems.append(this);
	collectEqualPotential(connectorItems, true, skipFlags);
	return connectorItems.contains(target);
}


Wire * ConnectorItem::directlyWiredTo(ConnectorItem * source, ConnectorItem * target, ViewGeometry::WireFlags flags) {
	 QList<ConnectorItem *> visited;
	 return directlyWiredToAux(source, target, flags, visited);
}

Wire * ConnectorItem::directlyWiredToAux(ConnectorItem * source, ConnectorItem * target, ViewGeometry::WireFlags flags, QList<ConnectorItem *> & visited) {
	if (visited.contains(source)) return NULL;

	QList<ConnectorItem *> equals;
	equals << source;

	ConnectorItem * cross = source->getCrossLayerConnectorItem();
	if (cross) {
		if (!visited.contains(cross)) {
			equals << cross;
		}
	}

	visited.append(equals);

	foreach (ConnectorItem * fromItem, equals) {
		foreach (ConnectorItem * toConnectorItem, fromItem->m_connectedTo) {
			ItemBase * toItem = toConnectorItem->attachedTo();
			if (toItem == NULL) {
				continue;			// shouldn't happen
			}

			if (toItem->itemType() != ModelPart::Wire) continue;

			Wire * wire = dynamic_cast<Wire *>(toItem);
			if (!wire->hasAnyFlag(flags)) continue;

			ConnectorItem * otherEnd = wire->otherConnector(toConnectorItem);
			bool isChained = false;
			foreach (ConnectorItem * otherConnectorItem, otherEnd->m_connectedTo) {
				if (target == otherConnectorItem) {
					return wire;
				}
				if (target->getCrossLayerConnectorItem() == otherConnectorItem) {
					return wire;
				}
				if (otherConnectorItem->attachedToItemType() == ModelPart::Wire) {
					//DebugDialog::debug(QString("wired from %1 to %2").arg(wire->id()).arg(otherConnectorItem->attachedToID()));
					isChained = true;
				}
			}

			if (isChained) {
				if (ConnectorItem::directlyWiredToAux(otherEnd, target, flags, visited)) {
					return wire;
				}
			}
		}
	}

	return false;
}

bool ConnectorItem::isConnectedToPart() {

	QList<ConnectorItem *> tempItems;
	tempItems << this;

	ConnectorItem * thisCrossConnectorItem = this->getCrossLayerConnectorItem();
	QList<ConnectorItem *> busConnectedItems;
	Bus * b = bus();
	if (b != NULL) {
		attachedTo()->busConnectorItems(b, busConnectedItems);
	}

	for (int i = 0; i < tempItems.count(); i++) {
		ConnectorItem * connectorItem = tempItems[i];

		if ((connectorItem != this) && 
			(connectorItem != thisCrossConnectorItem) && 
			!busConnectedItems.contains(connectorItem)) 
		{
			switch (connectorItem->attachedToItemType()) {
				case ModelPart::Symbol:
				case ModelPart::Jumper:
				case ModelPart::Part:
				case ModelPart::CopperFill:
				case ModelPart::Board:
				case ModelPart::Breadboard:
				case ModelPart::ResizableBoard:
				case ModelPart::Via:
					if (connectorItem->attachedTo()->isEverVisible()) {
						return true;
					}
					break;
				default:
					break;
			}
		}

		ConnectorItem * crossConnectorItem = connectorItem->getCrossLayerConnectorItem();
		if (crossConnectorItem != NULL) {
			if (!tempItems.contains(crossConnectorItem)) {
				tempItems.append(crossConnectorItem);
			}
		}

		foreach (ConnectorItem * cto, connectorItem->connectedToItems()) {
			if (tempItems.contains(cto)) continue;

			tempItems.append(cto);
		}

		Bus * bus = connectorItem->bus();
		if (bus != NULL) {
			QList<ConnectorItem *> busConnectedItems;
			connectorItem->attachedTo()->busConnectorItems(bus, busConnectedItems);
			foreach (ConnectorItem * busConnectedItem, busConnectedItems) {
				if (!tempItems.contains(busConnectedItem)) {
					tempItems.append(busConnectedItem);
				}
			}
		}
	}

	return false;
}

void ConnectorItem::collectEqualPotential(QList<ConnectorItem *> & connectorItems, bool crossLayers, ViewGeometry::WireFlags skipFlags) {
	// collects all the connectors at the same potential
	// allows direct connections or wired connections

	//DebugDialog::debug("__________________");

	QList<ConnectorItem *> tempItems = connectorItems;
	connectorItems.clear();

	for (int i = 0; i < tempItems.count(); i++) {
		ConnectorItem * connectorItem = tempItems[i];
		//connectorItem->debugInfo("testing");

		Wire * fromWire = (connectorItem->attachedToItemType() == ModelPart::Wire) ? dynamic_cast<Wire *>(connectorItem->attachedTo()) : NULL;
		if (fromWire != NULL) {
			if (fromWire->hasAnyFlag(skipFlags)) {
				// don't add this kind of wire
				continue;
			}
		}
		else {
			if (crossLayers) {
				ConnectorItem * crossConnectorItem = connectorItem->getCrossLayerConnectorItem();
				if (crossConnectorItem != NULL) {
					if (!tempItems.contains(crossConnectorItem)) {
						tempItems.append(crossConnectorItem);
					}
				}
			}
		}

		// this one's a keeper
		connectorItems.append(connectorItem);

		foreach (ConnectorItem * cto, connectorItem->connectedToItems()) {
			if (tempItems.contains(cto)) continue;

			if ((skipFlags & ViewGeometry::NormalFlag) && (fromWire == NULL) && (cto->attachedToItemType() != ModelPart::Wire)) {
				// direct (part-to-part) connections not allowed
				continue;
			}

			tempItems.append(cto);
		}

		Bus * bus = connectorItem->bus();
		if (bus != NULL) {
			QList<ConnectorItem *> busConnectedItems;
			connectorItem->attachedTo()->busConnectorItems(bus, busConnectedItems);
			foreach (ConnectorItem * busConnectedItem, busConnectedItems) {
				if (!tempItems.contains(busConnectedItem)) {
					tempItems.append(busConnectedItem);
				}
			}
		}
	}
}

void ConnectorItem::collectParts(QList<ConnectorItem *> & connectorItems, QList<ConnectorItem *> & partsConnectors, bool includeSymbols, ViewLayer::ViewLayerSpec viewLayerSpec)
{
	if (connectorItems.count() == 0) return;

	//DebugDialog::debug("___________________________");
	switch (viewLayerSpec) {
		case ViewLayer::Top:
		case ViewLayer::Bottom:
		case ViewLayer::TopAndBottom:
			break;
		default:
			DebugDialog::debug(QString("collect parts unknown spec %1").arg(viewLayerSpec));
			viewLayerSpec = ViewLayer::TopAndBottom;
			break;
	}
	
	foreach (ConnectorItem * connectorItem, connectorItems) {
		if (connectorItem->isHybrid()) {
			continue;
		}

		ItemBase * candidate = connectorItem->attachedTo();
		switch (candidate->itemType()) {
			case ModelPart::Symbol:
				if (!includeSymbols) break;
			case ModelPart::Jumper:
			case ModelPart::Part:
			case ModelPart::CopperFill:
			case ModelPart::Board:
			case ModelPart::ResizableBoard:
			case ModelPart::Via:
				collectPart(connectorItem, partsConnectors, viewLayerSpec);
				break;
			default:
				break;
		}
	}
}

void ConnectorItem::collectPart(ConnectorItem * connectorItem, QList<ConnectorItem *> & partsConnectors, ViewLayer::ViewLayerSpec viewLayerSpec) {
	if (partsConnectors.contains(connectorItem)) return;
				
	ConnectorItem * crossConnectorItem = connectorItem->getCrossLayerConnectorItem();
	if (crossConnectorItem != NULL) {
		if (partsConnectors.contains(crossConnectorItem)) {
			return;
		}
		
		if (viewLayerSpec == ViewLayer::TopAndBottom) {
			partsConnectors.append(crossConnectorItem);
			
			/*
			DebugDialog::debug(QString("collecting both: %1 %2 %3 %4")
				.arg(crossConnectorItem->attachedToID())
				.arg(crossConnectorItem->connectorSharedID())
				.arg(crossConnectorItem->attachedToViewLayerID())
				.arg((long)crossConnectorItem->attachedTo(), 0, 16) );
			*/
				
		}
		else if (viewLayerSpec == ViewLayer::Top) {
			if (connectorItem->attachedToViewLayerID() == ViewLayer::Copper1) {
			}
			else {
				connectorItem = crossConnectorItem;
			}
		}
		else if (viewLayerSpec == ViewLayer::Bottom) {
			if (connectorItem->attachedToViewLayerID() == ViewLayer::Copper0) {
			}
			else {
				connectorItem = crossConnectorItem;
			}
		}
	}

	/*
	DebugDialog::debug(QString("collecting part: %1 %2 %3 %4")
		.arg(connectorItem->attachedToID())
		.arg(connectorItem->connectorSharedID())
		.arg(connectorItem->attachedToViewLayerID())
		.arg((long) connectorItem->attachedTo(), 0, 16) );
	*/	

	partsConnectors.append(connectorItem);
}

void ConnectorItem::updateTooltip() {
	QList<ConnectorItem *> connectors;
	if (!attachedToItemType() == ModelPart::Wire) {
		connectors.append(this);
	}

	foreach(ConnectorItem * toConnectorItem, m_connectedTo) {
		if (!toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			connectors.append(toConnectorItem);
		}
	}

	if (connectors.count() == 0) {
		setToolTip("");
		return;
	}

	if (connectors.count() == 1) {
		setToolTip(connectors[0]->m_baseTooltip);
		return;
	}

	QString connections = QString("<ul style='margin-left:0;padding-left:0;'>");
	foreach(ConnectorItem * connectorItem, connectors) {
		connections += QString("<li style='margin-left:0;padding-left:0;'>") + "<b>" + connectorItem->attachedTo()->label() + "</b> " + connectorItem->connectorSharedName() + "</li>";
	}
	connections += "</ul>";

    setToolTip(ItemBase::ITEMBASE_FONT_PREFIX + connections + ItemBase::ITEMBASE_FONT_SUFFIX);

}

void ConnectorItem::setBaseTooltip(const QString & tooltip) {
	m_baseTooltip = tooltip;
	setToolTip(tooltip);
}

void ConnectorItem::clearConnector() {
	m_connector = NULL;
}


bool ConnectorItem::connectionIsAllowed(ConnectorItem * other) {
	if (!connector()->connectionIsAllowed(other->connector())) return false;
	if (!m_attachedTo->connectionIsAllowed(other)) return false;
	foreach (ConnectorItem * toConnectorItem, connectedToItems()) {
		if (!toConnectorItem->attachedTo()->connectionIsAllowed(other)) {
			return false;
		}
	}

	return true;
}

void ConnectorItem::prepareGeometryChange() {
	QGraphicsRectItem::prepareGeometryChange();
}

void ConnectorItem::showEqualPotential(bool show) {
	if (!show) {
		restoreColor(false, 0, true);
		return;
	}

	QBrush * brush = NULL;
	QPen * pen = NULL;
	m_attachedTo->getEqualPotentialColor(this, brush, pen, m_opacity, m_negativePenWidth, m_negativeOffsetRect);
	//DebugDialog::debug(QString("set normal %1 %2").arg(attachedToID()).arg(pen->width()));
	setColorAux(*brush, *pen, true);

}

void ConnectorItem::clearEqualPotentialDisplay() {
	foreach (ConnectorItem * connectorItem, m_equalPotentialDisplayItems) {
		connectorItem->showEqualPotential(false);
	}
	m_equalPotentialDisplayItems.clear();
}

bool ConnectorItem::isEverVisible() {
	return m_attachedTo->isEverVisible();
}

bool ConnectorItem::isGrounded(ConnectorItem * c1, ConnectorItem * c2) {
	QList<ConnectorItem *> connectorItems;
	if (c1 != NULL) {
		connectorItems.append(c1);
	}
	if (c2 != NULL) {
		connectorItems.append(c2);
	}
	collectEqualPotential(connectorItems, true, ViewGeometry::NoFlag);

	foreach (ConnectorItem * end, connectorItems) {
		if (end->isGrounded()) return true;

	}

	return false;
}

bool ConnectorItem::isGrounded() {
	QString name = connectorSharedName();
	return ((name.compare("gnd", Qt::CaseInsensitive) == 0) || 
			(name.compare("ground", Qt::CaseInsensitive) == 0));
}

ConnectorItem * ConnectorItem::getCrossLayerConnectorItem() {
	if (m_connector == NULL) return NULL;
	if (m_attachedTo == NULL) return NULL;
	if (m_attachedTo->viewIdentifier() != ViewIdentifierClass::PCBView) return NULL;

	ViewLayer::ViewLayerID viewLayerID = attachedToViewLayerID();
	if (viewLayerID == ViewLayer::Copper0) {
		return m_connector->connectorItemByViewLayerID(this->attachedToViewIdentifier(), ViewLayer::Copper1);
	}
	if (viewLayerID == ViewLayer::Copper1) {
		return m_connector->connectorItemByViewLayerID(this->attachedToViewIdentifier(), ViewLayer::Copper0);
	}

	return NULL;
}

bool ConnectorItem::isInLayers(ViewLayer::ViewLayerSpec viewLayerSpec) {
	return ViewLayer::copperLayers(viewLayerSpec).contains(attachedToViewLayerID());
}

bool ConnectorItem::isCrossLayerConnectorItem(ConnectorItem * candidate) {
	if (candidate == NULL) return false;

	ConnectorItem * cross = getCrossLayerConnectorItem();
	return cross == candidate;
}

bool ConnectorItem::isCrossLayerFrom(ConnectorItem * candidate) {
	return !ViewLayer::canConnect(this->attachedToViewLayerID(), candidate->attachedToViewLayerID());
}

void ConnectorItem::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) 
{
	if (m_hybrid) return;
	if (doNotPaint()) return;

	if (!m_checkedEffectively) {
		if (!m_circular && m_shape.isEmpty()) {
			if (this->attachedTo()->viewIdentifier() == ViewIdentifierClass::PCBView) {
				QRectF r = rect();
				m_effectivelyCircular = qAbs(r.width() - r.height()) < 0.01;
				m_effectivelyRectangular = !m_effectivelyCircular;
			}
		}
		m_checkedEffectively = true;
	}

	NonConnectorItem::paint(painter, option, widget);
}

ConnectorItem * ConnectorItem::chooseFromSpec(ViewLayer::ViewLayerSpec viewLayerSpec) {
	ConnectorItem * crossConnectorItem = getCrossLayerConnectorItem();
	if (crossConnectorItem == NULL) return this;

	ViewLayer::ViewLayerID basis = ViewLayer::Copper0;
	switch (viewLayerSpec) {
		case ViewLayer::Top:
			basis = ViewLayer::Copper1;
			break;
		case ViewLayer::Bottom:
			basis = ViewLayer::Copper0;
			break;
		default:
			DebugDialog::debug(QString("unusual viewLayerSpec %1").arg(viewLayerSpec));
			basis = ViewLayer::Copper0;
			break;
	}

	if (this->attachedToViewLayerID() == basis) {
		return this;
	}
	if (crossConnectorItem->attachedToViewLayerID() == basis) {
		return crossConnectorItem;	
	}
	return this;
}

bool ConnectorItem::connectedToWires() {
	foreach (ConnectorItem * toConnectorItem, connectedToItems()) {
		if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			return true;
		}
	}

	ConnectorItem * crossConnectorItem = getCrossLayerConnectorItem();
	if (crossConnectorItem == NULL) return false;

	foreach (ConnectorItem * toConnectorItem, crossConnectorItem->connectedToItems()) {
		if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			return true;
		}
	}

	return false;
}

void ConnectorItem::displayRatsnest(QList<ConnectorItem *> & partConnectorItems) {
	bool formerColorWasNamed = false;
	bool gotFormerColor = false;
	QColor formerColor;

	VirtualWire * vw = NULL;
	foreach (ConnectorItem * fromConnectorItem, partConnectorItems) {
		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
			vw = qobject_cast<VirtualWire *>(toConnectorItem->attachedTo());
			if (vw != NULL) break;
		}
		if (vw != NULL) break;
	}

	if (vw != NULL) {
		formerColorWasNamed = vw->colorWasNamed();
		formerColor = vw->color();
		gotFormerColor = true;
		clearRatsnestDisplay(partConnectorItems);
	}

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView == NULL) return;

	if (partConnectorItems.count() < 2) return;

	QStringList connectorNames;
	ConnectorItem::collectConnectorNames(partConnectorItems, connectorNames);
	QColor color;
	bool colorWasNamed = RatsnestColors::findConnectorColor(connectorNames, color);
	if (!colorWasNamed) {
		if (!formerColorWasNamed && gotFormerColor) {
			color = formerColor;
		}
		else {
			infoGraphicsView->getRatsnestColor(color);
		}
	}

	ConnectorPairHash result;
	GraphUtils::chooseRatsnestGraph(partConnectorItems, result);

	foreach (ConnectorItem * key, result.uniqueKeys()) {
		foreach (ConnectorItem * value, result.values(key)) {
			VirtualWire * vw = infoGraphicsView->makeOneRatsnestWire(key, value, false, color);
			if (vw) {
				vw->setColorWasNamed(colorWasNamed);
			}
		}
	}
}

void ConnectorItem::clearRatsnestDisplay(QList<ConnectorItem *> & connectorItems) {

	QSet<VirtualWire *> ratsnests;
	foreach (ConnectorItem * fromConnectorItem, connectorItems) {
		if (fromConnectorItem == NULL) continue;

		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
			VirtualWire * vw = qobject_cast<VirtualWire *>(toConnectorItem->attachedTo());
			if (vw != NULL) {
				ratsnests.insert(vw);
			}
		}
	}

	foreach (VirtualWire * vw, ratsnests.values()) {
		ConnectorItem * c1 = vw->connector0()->firstConnectedToIsh();
		if (c1 != NULL) {
			vw->connector0()->tempRemove(c1, false);
			c1->tempRemove(vw->connector0(), false);
		}

		ConnectorItem * c2 = vw->connector1()->firstConnectedToIsh();
		if (c2 != NULL) {
			vw->connector1()->tempRemove(c2, false);
			c2->tempRemove(vw->connector1(), false);
		}

		vw->scene()->removeItem(vw);
		delete vw;
	}
}


void ConnectorItem::collectConnectorNames(QList<ConnectorItem *> & connectorItems, QStringList & connectorNames) 
{
	foreach(ConnectorItem * connectorItem, connectorItems) {
		if (!connectorNames.contains(connectorItem->connectorSharedName())) {
			connectorNames.append(connectorItem->connectorSharedName());
			//DebugDialog::debug("name " + connectorItem->connectorSharedName());
		}
	}
}

bool ConnectorItem::marked() {
	return m_marked;
}

void ConnectorItem::setMarked(bool m) {
	m_marked = m;
}

qreal ConnectorItem::calcClipRadius() {
	if (m_circular) {
		return radius() - (strokeWidth() / 2.0);
	}

	if (m_effectivelyCircular) {
		qreal rad = rect().width() / 2;
		return rad - (rad / 5);
	}

	return 0;
}

bool ConnectorItem::isEffectivelyCircular() {
	return m_circular || m_effectivelyCircular;
}

void ConnectorItem::debugInfo(const QString & msg) 
{

#ifndef QT_NO_DEBUG
	DebugDialog::debug(QString("%1 cid:%2 %3 %4 id:%5 %6 vlid:%7 vid:%8 spec:%9 flg:%10 hy:%11")
		.arg(msg)
		.arg(this->connectorSharedID())
		.arg(this->connectorSharedName())
		.arg(this->attachedToTitle())
		.arg(this->attachedToID())
		.arg(this->attachedToInstanceTitle())
		.arg(this->attachedToViewLayerID())
		.arg(this->attachedToViewIdentifier())
		.arg(this->attachedToViewLayerSpec())
		.arg(this->attachedTo()->getViewGeometry().wireFlags())
		.arg(this->m_hybrid)
	);
#else
	Q_UNUSED(msg);
#endif
}

qreal ConnectorItem::minDimension() {
	QRectF r = this->boundingRect();
	return qMin(r.width(), r.height());
}

ConnectorItem * ConnectorItem::findConnectorUnder(bool useTerminalPoint, bool allowAlready, const QList<ConnectorItem *> & exclude, bool displayDragTooltip, ConnectorItem * other)
{
	QList<QGraphicsItem *> items = useTerminalPoint
		? this->scene()->items(this->sceneAdjustedTerminalPoint(NULL))
		: this->scene()->items(mapToScene(this->rect()));
	QList<ConnectorItem *> candidates;
	// for the moment, take the topmost ConnectorItem that doesn't belong to me
	foreach (QGraphicsItem * item, items) {
		ConnectorItem * connectorItemUnder = dynamic_cast<ConnectorItem *>(item);
		if (connectorItemUnder == NULL) continue;
		if (connectorItemUnder->connector() == NULL) continue;			// shouldn't happen
		if (attachedTo()->childItems().contains(connectorItemUnder)) continue;		// don't use own connectors
		if (!this->connectionIsAllowed(connectorItemUnder)) {
			continue;
		}
		if (!allowAlready) {
			if (connectorItemUnder->connectedToItems().contains(this)) {
				continue;		// already connected
			}
		}
		if (exclude.contains(connectorItemUnder)) continue;


		candidates.append(connectorItemUnder);
	}

	ConnectorItem * candidate = NULL;
	if (candidates.count() == 1) {
		candidate = candidates[0];
	}
	else if (candidates.count() > 0) {
		qSort(candidates.begin(), candidates.end(), wireLessThan);
		candidate = candidates[0];
	}

	if (m_overConnectorItem != NULL && candidate != m_overConnectorItem) {
		m_overConnectorItem->connectorHover(NULL, false);
	}
	if (candidate != NULL && candidate != m_overConnectorItem) {
		candidate->connectorHover(NULL, true);
	}

	m_overConnectorItem = candidate;

	if (candidate == NULL) {
		if (this->connectorHovering()) {
			this->connectorHover(NULL, false);
		}
	}
	else {
		if (!this->connectorHovering()) {
			this->connectorHover(NULL, true);
		}
	}

	if (displayDragTooltip) {
		displayTooltip(m_overConnectorItem, other);
	}

	return m_overConnectorItem;
}

void ConnectorItem::displayTooltip(ConnectorItem * ci, ConnectorItem * other)
{
	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView == NULL) return;

	// Activate tooltip for destination connector. based on a patch submitted by bryant.mairs
	QString text;
	if (ci && ci->connectorHovering()) {
		if (other) {
			text = QString("%1: %2\n%3: %4")
						.arg(other->attachedToInstanceTitle())
						.arg(other->connectorSharedName())
						.arg(ci->attachedToInstanceTitle())
						.arg(ci->connectorSharedName());
		}
		else {
			text = QString("%1: %2").arg(ci->attachedToInstanceTitle()).arg(ci->connectorSharedName());
		}
	}
	else {
		if (other) {
			text = QString("%1: %2")
				.arg(other->attachedToInstanceTitle())
				.arg(other->connectorSharedName());
		}
	}
    // Now use Qt's tooltip functionality to display our tooltip.
    // The tooltip text is first cleared as only a change in tooltip
    // text will update its position.
    // A rect is generated to smooth out position updates.
    // NOTE: Increasing this rect will cause the tooltip to disappear
    // and not reappear until another pixel move after the move that
    // disabled it.
    QPoint sp = QCursor::pos();
    QToolTip::showText(sp, "", infoGraphicsView);
	if (!text.isEmpty()) {
		QPoint q = infoGraphicsView->mapFromGlobal(sp);
		QRect r(q.x(), q.y(), 1, 1);
		QToolTip::showText(sp, text, infoGraphicsView, r);
	}
}

ConnectorItem * ConnectorItem::releaseDrag() {
	ConnectorItem * result = m_overConnectorItem;
	if (m_overConnectorItem != NULL) {
		m_overConnectorItem->connectorHover(NULL, false);

		// clean up
		setOverConnectorItem(NULL);
		clearConnectorHover();
		restoreColor(false, 0, true);
	}
	attachedTo()->clearConnectorHover();
	return result;
}
