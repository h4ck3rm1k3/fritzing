/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2009 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "infographicsview.h"
#include "debugdialog.h"
#include "bus.h"
#include "wire.h"
#include "modelpart.h"

QPen ConnectorItem::normalPen(QColor(255,0,0));
QPen ConnectorItem::hoverPen(QColor(0, 0, 255));
QPen ConnectorItem::connectedPen(QColor(0, 255, 0));
QPen ConnectorItem::chosenPen(QColor(255,0,0));

QBrush ConnectorItem::normalBrush(QColor(255,0,0));
QBrush ConnectorItem::hoverBrush(QColor(0,0,255));
QBrush ConnectorItem::connectedBrush(QColor(0,255,0));
QBrush ConnectorItem::chosenBrush(QColor(255,0,0));


ConnectorItem::ConnectorItem( Connector * connector, ItemBase * attachedTo )
	: QGraphicsRectItem(attachedTo)
{
	m_hoverEnterSpaceBarWasPressed = m_spaceBarWasPressed = false;
	m_chosen = false;
	m_ignoreAncestorFlag = false;
	m_opacity = 0.4;
	m_circular = false;
	m_overConnectorItem = NULL;
	m_connectorHovering = false;
	m_hidden = false;
	m_attachedTo = attachedTo;
	m_connector = connector;
	if (connector != NULL) {
		connector->addViewItem(this);
	}
	restoreColor();
    setAcceptsHoverEvents(true);
    this->setCursor(Qt::CrossCursor);

	//DebugDialog::debug(QString("%1 attached to %2")
			//.arg(this->connector()->connectorShared()->id())
			//.arg(attachedTo->modelPartShared()->title()) );
}

ConnectorItem::~ConnectorItem() {
	//DebugDialog::debug(QString("deleting connectorItem %1").arg((long) this, 0, 16));
	for (int i = 0; i < m_connectedTo.count(); i++) {
		m_connectedTo[i]->tempRemove(this, true);
	}
	if (this->connector() != NULL) {
		this->connector()->removeViewItem(this);
	}
}

void ConnectorItem::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) {
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

	restoreColor();
	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->hoverLeaveConnectorItem(event, this);
	}
}

Connector * ConnectorItem::connector() {
	return m_connector;
}

ItemBase * ConnectorItem::attachedTo() {
	return m_attachedTo;
}

void ConnectorItem::clearConnectorHover() {
	m_connectorHovering = false;
	restoreColor();
	if (this->m_attachedTo != NULL) {
		m_attachedTo->clearConnectorHover();
	}
}

void ConnectorItem::connectorHover(ItemBase * itemBase, bool hovering) {
	m_connectorHovering = hovering;
	if (hovering) {
		setHoverColor();
	}
	else {
		restoreColor();
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
	restoreColor();
	if (m_attachedTo != NULL) {
		m_attachedTo->connectionChange(this);
	}

	updateTooltip();
}

void ConnectorItem::tempConnectTo(ConnectorItem * item, bool applyColor) {
	m_connectedTo.append(item);
	updateTooltip();

	if(applyColor) restoreColor();
}

void ConnectorItem::tempRemove(ConnectorItem * item, bool applyColor) {
	m_connectedTo.removeOne(item);
	updateTooltip();

	if(applyColor) restoreColor();
}

void ConnectorItem::restoreColor() {
	if (m_chosen) {
		setChosenColor();
		return;
	}

	if (m_connectedTo.count() <= 0) {
		setNormalColor();
		return;
	}

	setConnectedColor();
}

void ConnectorItem::setConnectedColor() {
	setColorAux(connectedBrush, connectedPen, true);
}

void ConnectorItem::setNormalColor() {
	setColorAux(normalBrush, normalPen, false);
}

void ConnectorItem::setChosenColor() {
	setColorAux(chosenBrush, chosenPen, true);
}

void ConnectorItem::setHoverColor() {
	setColorAux(hoverBrush, hoverPen, true);
}

void ConnectorItem::setChosen(bool chosen)
{
	m_chosen = chosen;
	restoreColor();
}


void ConnectorItem::setColorAux(QBrush brush, QPen pen, bool paint) {
	this->setBrush(brush);
	this->setPen(pen);
	m_paint = paint;
}

void ConnectorItem::setColorAux(const QColor &color, bool paint) {
	this->setBrush(QBrush(color));
	this->setPen(QPen(color));
	m_paint = paint;
}

void ConnectorItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	InfoGraphicsView *infographics = InfoGraphicsView::getInfoGraphicsView(this);
	if (infographics != NULL && infographics->spaceBarIsPressed()) { 
		event->ignore();
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

ConnectorItem * ConnectorItem::removeConnection(ItemBase * itemBase) {
	for (int i = 0; i < m_connectedTo.count(); i++) {
		if (m_connectedTo[i]->attachedTo() == itemBase) {
			ConnectorItem * removed = m_connectedTo[i];
			m_connectedTo.removeAt(i);
			if (m_attachedTo != NULL) {
				m_attachedTo->connectionChange(this);
			}
			restoreColor();
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
	restoreColor();
	if (emitChange) {
		m_attachedTo->connectionChange(this);
	}
	updateTooltip();
}

ConnectorItem * ConnectorItem::firstConnectedToIsh() {
	if (m_connectedTo.count() <= 0) return NULL;

	foreach (ConnectorItem * connectorItem, m_connectedTo) {
		if (!connectorItem->attachedTo()->getVirtual()) return connectorItem;
	}

	return NULL;
}

void ConnectorItem::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) {
	if (m_hidden  || !m_paint) return;

	painter->setOpacity(m_opacity);
	if (m_circular) {
		painter->setPen(pen());
		painter->setBrush(brush());
		painter->drawEllipse(rect());
	}
	else {
		QGraphicsRectItem::paint(painter, option, widget);
	}

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

QPointF ConnectorItem::sceneAdjustedTerminalPoint() {
	return this->mapToScene(m_terminalPoint + this->rect().topLeft());
}

bool ConnectorItem::connectedTo(ConnectorItem * connectorItem) {
	return this->m_connectedTo.contains(connectorItem);
}

const QList<ConnectorItem *> & ConnectorItem::connectedToItems() {
	return m_connectedTo;
}

void ConnectorItem::setHidden(bool hide) {
	m_hidden = hide;
	if (hide) {
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

long ConnectorItem::attachedToID() {
	if (attachedTo() == NULL) return -1;
	return attachedTo()->id();
}

const QString & ConnectorItem::attachedToTitle() {
	if (attachedTo() == NULL) return ___emptyString___;
	return attachedTo()->title();
}

const QString & ConnectorItem::connectorSharedID() {
	if (m_connector == NULL) return ___emptyString___;

	return m_connector->connectorSharedID();
}

const QString & ConnectorItem::connectorSharedName() {
	if (m_connector == NULL) return ___emptyString___;

	return m_connector->connectorSharedName();
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

void ConnectorItem::setCircular(bool circular) {
	m_circular = circular;
}

void ConnectorItem::setOpacity(qreal opacity) {
	m_opacity = opacity;
}

int ConnectorItem::attachedToItemType() {
	if (m_attachedTo == NULL) return ModelPart::Unknown;

	return m_attachedTo->itemType();
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
	writer.writeAttribute("layer", ViewLayer::viewLayerXmlNameFromID(attachedTo()->viewLayerID()));
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
			connectorItem->writeConnector(writer, "connect");
		}
		writer.writeEndElement();
	}

	writeOtherElements(writer);

	writer.writeEndElement();
}


void ConnectorItem::writeConnector(QXmlStreamWriter & writer, const QString & elementName) 
{
	writer.writeStartElement(elementName);
	writer.writeAttribute("connectorId", connectorSharedID());
	if (attachedTo()->parentItem() == NULL) {
		writer.writeAttribute("modelIndex", QString::number(connector()->modelIndex()));
	}
	else {
		// if connectorItem is part of a module, then we need to save references up the tree
		QList<QGraphicsItem *> parents;
		QGraphicsItem * parent = attachedTo();
		while (parent) {
			parents.push_front(parent);
			parent = parent->parentItem();
		}
		ItemBase * itemBase = dynamic_cast<ItemBase *>(parents[0]);
		writer.writeStartElement("mp");
		// write the local file modelIndex first
		writer.writeAttribute("i", QString::number(itemBase->modelPart()->modelIndex()));
		for (int i = 1; i < parents.count(); i++) {
			itemBase = dynamic_cast<ItemBase *>(parents[i]);
			writer.writeStartElement("mp");
			// now write the indices from the original file
			writer.writeAttribute("i", QString::number(itemBase->modelPart()->originalModelIndex()));
		}	
		for (int i = 0; i < parents.count(); i++) {
			writer.writeEndElement();
		}
	}
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

Wire * ConnectorItem::wiredTo(ConnectorItem * target, ViewGeometry::WireFlags flags) {
	foreach (ConnectorItem * toConnectorItem, m_connectedTo) {
		ItemBase * toItem = toConnectorItem->attachedTo();
		if (toItem == NULL) {
			continue;			// shouldn't happen
		}

		if (toItem->itemType() != ModelPart::Wire) continue;

		Wire * wire = dynamic_cast<Wire *>(toItem);
		if (!wire->hasAnyFlag(flags)) continue;

		ConnectorItem * otherEnd = wire->otherConnector(toConnectorItem);
		bool chained = false;
		foreach (ConnectorItem * otherConnectorItem, otherEnd->m_connectedTo) {
			if (target == otherConnectorItem) {
				return wire;
			}
			if (otherConnectorItem->attachedToItemType() == ModelPart::Wire) {
				chained = true;
			}
		}

		if (chained) {
			if (otherEnd->wiredTo(target, flags)) {
				return wire;
			}
		}
	}

	return NULL;
}

bool ConnectorItem::wiredTo(ConnectorItem * target)
{
	foreach (ConnectorItem * toConnectorItem, m_connectedTo) {
		if (target == toConnectorItem)
		{
			return true;
		}
	}

	foreach (ConnectorItem * toConnectorItem, m_connectedTo) {
		if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			ConnectorItem * otherConnector = dynamic_cast<Wire *>(toConnectorItem->attachedTo())->otherConnector(toConnectorItem);
			if (otherConnector->wiredTo(target)) {
				return true;
			}
		}
	}

	return false;
}

void ConnectorItem::collectEqualPotential(QList<ConnectorItem *> & connectorItems, ViewGeometry::WireFlags skipFlags) {
	// collects all the connectors at the same potential
	// allows direct connections or wired connections

	QList<ConnectorItem *> tempItems = connectorItems;
	connectorItems.clear();

	for (int i = 0; i < tempItems.count(); i++) {
		ConnectorItem * connectorItem = tempItems[i];
		//DebugDialog::debug(QString("testing %1 %2 %3").arg(connectorItem->attachedToID()).arg(connectorItem->attachedToTitle()).arg(connectorItem->connectorSharedID()) );

		Wire * fromWire = (connectorItem->attachedToItemType() == ModelPart::Wire) ? dynamic_cast<Wire *>(connectorItem->attachedTo()) : NULL;
		if (fromWire != NULL && fromWire->hasAnyFlag(skipFlags)) {
			// don't add this kind of wire
			continue;
		}

		// this one's a keeper
		connectorItems.append(connectorItem);

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
}

void ConnectorItem::collectEqualPotentialParts(QList<ConnectorItem *> & connectorItems, ViewGeometry::WireFlags flags) {
	// collects all the connectors at the same potential
	// which are directly reached by the given wire type

	collectEqualPotential(connectorItems);
	QList<ConnectorItem *> partConnectorItems;
	collectParts(connectorItems, partConnectorItems);
	connectorItems.clear();
	for (int i = 0; i < partConnectorItems.count() - 1; i++) {
		ConnectorItem * ci = partConnectorItems[i];
		for (int j = i + 1; j < partConnectorItems.count(); j++) {
			ConnectorItem * cj = partConnectorItems[j];
			if (ci->wiredTo(cj, flags)) {
				if (!connectorItems.contains(ci)) {
					connectorItems.append(ci);
				}
				if (!connectorItems.contains(cj)) {
					connectorItems.append(cj);
				}
			}
		}
	}
}

void ConnectorItem::collectParts(QList<ConnectorItem *> & connectorItems, QList<ConnectorItem *> & partsConnectors)
{
	foreach (ConnectorItem * connectorItem, connectorItems) {
		ItemBase * candidate = connectorItem->attachedTo();
		switch (candidate->itemType()) {
			case ModelPart::Part:
			case ModelPart::Board:
			case ModelPart::ResizableBoard:
				if (!partsConnectors.contains(connectorItem)) {
					//DebugDialog::debug(QString("collecting part %1 %2").arg(candidate->id()).arg(connectorItem->connectorSharedID()) );
					partsConnectors.append(connectorItem);
				}
				break;
			default:
				break;
		}
	}
}

void ConnectorItem::updateTooltip() {
	if (m_connectedTo.count() == 0) {
		setToolTip(m_baseTooltip);
		return;
	}

	QString connections;
	foreach(ConnectorItem * toConnectorItem, m_connectedTo) {
		connections += "<br />&nbsp;&nbsp;" + toConnectorItem->attachedToTitle() + ":" + toConnectorItem->connectorSharedName();
	}

	setToolTip(m_baseTooltip + ITEMBASE_FONT_PREFIX + connections + ITEMBASE_FONT_SUFFIX);

}

void ConnectorItem::setBaseTooltip(const QString & tooltip) {
	m_baseTooltip = tooltip;
	setToolTip(tooltip);
}

void ConnectorItem::setRectAux(qreal x1, qreal y1, qreal x2, qreal y2) {
	qreal width = x2-x1;
	qreal height = y2-y1;
	setRect(x1,y1,width,height);
}

void ConnectorItem::clearConnector() {
	m_connector = NULL;
}

bool ConnectorItem::sceneEvent(QEvent *event)
{
	if (!m_ignoreAncestorFlag) {
		return QGraphicsRectItem::sceneEvent(event);
	}

	// the rest of this is copied from QGraphicsItem::sceneEvent
	// this seemed to be the only way to be able to allow some items in a group to respond directly to events
	// while others pass their events up to the parent

	if (!this->isVisible()) {
		return true;
	}

    switch (event->type()) {
    case QEvent::FocusIn:
        focusInEvent(static_cast<QFocusEvent *>(event));
        break;
    case QEvent::FocusOut:
        focusOutEvent(static_cast<QFocusEvent *>(event));
        break;
    case QEvent::GraphicsSceneContextMenu:
        contextMenuEvent(static_cast<QGraphicsSceneContextMenuEvent *>(event));
        break;
    case QEvent::GraphicsSceneDragEnter:
        dragEnterEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneDragMove:
        dragMoveEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneDragLeave:
        dragLeaveEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneDrop:
        dropEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneHoverEnter:
        hoverEnterEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
        break;
    case QEvent::GraphicsSceneHoverMove:
        hoverMoveEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
        break;
    case QEvent::GraphicsSceneHoverLeave:
        hoverLeaveEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseMove:
        mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneMousePress:
        mousePressEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseRelease:
        mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseDoubleClick:
        mouseDoubleClickEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneWheel:
        wheelEvent(static_cast<QGraphicsSceneWheelEvent *>(event));
        break;
    case QEvent::KeyPress:
        keyPressEvent(static_cast<QKeyEvent *>(event));
        break;
    case QEvent::KeyRelease:
        keyReleaseEvent(static_cast<QKeyEvent *>(event));
        break;
    case QEvent::InputMethod:
        inputMethodEvent(static_cast<QInputMethodEvent *>(event));
        break;
    default:
        return false;
    }

    return true;
}

void ConnectorItem::setIgnoreAncestorFlag(bool ignore) {
	m_ignoreAncestorFlag = ignore;
}

void ConnectorItem::setIgnoreAncestorFlagIfExternal(bool ignore) {
	if (m_connector == NULL) {
		DebugDialog::debug("setIgnoreAncestorFlagIfExternal: connector is null");
		return;
	}

	if (m_connector->external()) {
		m_ignoreAncestorFlag = ignore;
	}
}

bool ConnectorItem::connectionIsAllowed(ConnectorItem * other) {
	bool result = connector()->connectionIsAllowed(other->connector());
	if (!result) return false;

	if (other->attachedTo()->parentItem() != NULL) {
		// other's part is in a group
		// TODO: what to do with external connectors once a group is inside a group?
		if (!other->connector()->external()) {
			return false;
		}
	}

	/*
	// code to disallow a wire connecting two items in the same group, but why not?
	if (other->m_ignoreAncestorFlag) {
		if (this->attachedToItemType() == ModelPart::Wire) {
			ConnectorItem * wireOther = dynamic_cast<Wire *>(this->attachedTo())->otherConnector(this);
			foreach (ConnectorItem * wireOtherConnectedToItem, wireOther->connectedToItems()) {
				if (wireOtherConnectedToItem->m_ignoreAncestorFlag) {
					if (other->commonAncestorItem(wireOtherConnectedToItem) != NULL) {
						// don't allow wire connection between grouped items
						return false;
					}
				}
			}
		}
	}
	*/

	return true;
}

void ConnectorItem::prepareGeometryChange() {
	QGraphicsRectItem::prepareGeometryChange();
}

bool ConnectorItem::isExternal() {
	if (m_connector == NULL) return false;

	return m_connector->external();
}

