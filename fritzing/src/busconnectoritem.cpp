/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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
 *  busconnectoritem.cpp
 *  Fritzing
 *
 *  Created by Jonathan Cohen on 9/11/08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#include <QApplication>

#include "busconnectoritem.h"
#include "bus.h"
#include "connector.h"
#include "modelpart.h"
#include "debugdialog.h"
#include "wire.h"
#include "bettertimer.h"

QList<QGraphicsItem *> BusConnectorItem::m_savedItems;

static QPen _normalPen(QColor(0,0,0));
static QPen _hoverPen(QColor(80, 80, 80));
static QPen _connectedPen(QColor(0, 0, 0));
static QBrush _normalBrush(QColor(0,0,0));
static QBrush _hoverBrush(QColor(80,80,80));
static QBrush _connectedBrush(QColor(0,0,0));

static int size = 12;

BusConnectorItem::BusConnectorItem( ItemBase * busOwner, Bus * bus, ConnectorItem * tokenHolder ) :
	ConnectorItem(NULL, NULL)
{
	m_connector = bus->busConnector();
	m_owner = m_attachedTo = busOwner;
		
	m_bus = bus;
	if (bus != NULL) {
		bus->addViewItem(this);
	}
	m_tokenHolder = tokenHolder;
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setRect(QRectF(0, 0, size, size));
	m_terminalPoint.setX(size / 2);
	m_terminalPoint.setY(size / 2);
    this->setCursor(Qt::SizeAllCursor);
	//setOpacity(1.0);
	setCircular(true);
	m_paint = true;
	m_initialized = false;
	setParentItem(NULL);

}

BusConnectorItem::~BusConnectorItem() {
	if (m_owner != NULL) {
		m_owner->removeBusConnectorItem(m_bus);
	}
	if (m_bus != NULL) {
		m_bus->removeViewItem(this);
	}
}

void BusConnectorItem::setConnectedColor() {
	this->setBrush(_connectedBrush);
	this->setPen(_connectedPen);
}

void BusConnectorItem::setNormalColor() {
	this->setBrush(_normalBrush);
	this->setPen(_normalPen);
}

void BusConnectorItem::setHoverColor() {
	this->setBrush(_hoverBrush);
	this->setPen(_hoverPen);
}

void BusConnectorItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	Q_UNUSED(event);
	// do not call parent class mousePressEvent here or you will draw a wire
	saveSelection(scene());
}

void BusConnectorItem::saveSelection(QGraphicsScene * scene) {
	m_savedItems = scene->selectedItems();
	scene->clearSelection();
}

void BusConnectorItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	ConnectorItem::mouseMoveEvent(event);
	adjustConnectedItems();
}

void BusConnectorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	ConnectorItem::mouseReleaseEvent(event);
	restoreSelection();
}

void BusConnectorItem::restoreSelection() {
	foreach (QGraphicsItem * savedItem, m_savedItems) {
		savedItem->setSelected(true);
	}
	m_savedItems.clear();
}

void BusConnectorItem::merge(BusConnectorItem * that) {
	if (!that->m_merged.contains(this)) {
		that->m_merged.append(this);
	}
	if (!this->m_merged.contains(that)) {
		this->m_merged.append(that);
	}

	this->bus()->merge(that->bus());
}

void BusConnectorItem::unmerge(BusConnectorItem * that) {
	that->m_merged.removeOne(this);
	this->m_merged.removeOne(that);

	this->bus()->unmerge(that->bus());
}

bool BusConnectorItem::isMergedWith(BusConnectorItem * that) {
	return m_merged.contains(that);
}

bool BusConnectorItem::isMerged() {
	return m_merged.count() > 0;
}

const QString & BusConnectorItem::busID() {
	if (m_bus == NULL) return ___emptyString___;
	
	return m_bus->id();
}

void BusConnectorItem::adjustConnectedItems() {
	ConnectorItem::adjustConnectedItems();
	for (int i = 0; i < childItems().count(); i++) {
		BusConnectorItem * bci = dynamic_cast<BusConnectorItem *>(childItems()[i]);
		if (bci == NULL) continue;

		bci->adjustConnectedItems();
	}
}

ConnectorItem * BusConnectorItem::tokenHolder() {
	return m_tokenHolder;
}


const QList<BusConnectorItem *> & BusConnectorItem::merged() {
	return m_merged;
}

Bus * BusConnectorItem::bus() {
	return m_bus;
}


const QString & BusConnectorItem::connectorStuffID() {
	return busID();
}

void BusConnectorItem::writeTopLevelAttributes(QXmlStreamWriter & writer) {
	ConnectorItem::writeTopLevelAttributes(writer);

	if (m_tokenHolder == NULL) {
		// this shouldn't happen
		return;
	}

	// do not write anything other than attributes in this routine.
	writer.writeAttribute("busId", connectorStuffID());
	
	// probably most of this will become obsolete
	writer.writeAttribute("tokenHolderConnectorID", m_tokenHolder->connectorStuffID());
	writer.writeAttribute("tokenHolderModelIndex", QString::number(m_tokenHolder->connector()->modelIndex()));
	BusConnectorItem * bci = dynamic_cast<BusConnectorItem *>(this->parentItem());
	if (bci != NULL) {
		writer.writeAttribute("mergeParentBusID", bci->busID());
		writer.writeAttribute("mergeParentModelIndex", QString::number(bci->connector()->modelIndex()) );
	}

}


void BusConnectorItem::writeOtherElements(QXmlStreamWriter & writer) {
	Q_UNUSED(writer);
	if (m_merged.count() <= 0) return;
	
	writer.writeStartElement("merged");
	foreach (BusConnectorItem * bci, m_merged) {
		writer.writeStartElement("bus");
		writer.writeAttribute("busId", bci->bus()->id());
		writer.writeAttribute("modelIndex", QString::number(bci->bus()->modelPart()->modelIndex()));
		writer.writeEndElement();
	}
	writer.writeEndElement();
}

/*

void BusConnectorItem::setRect(QRectF rect) {
	ConnectorItem::setRect(rect);
	if (parentItem()) {
		DebugDialog::debug(QString("setting rect %1 %2 %3 %4").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height()) );
	}
}

void BusConnectorItem::setPos(QPointF p) {
	ConnectorItem::setPos(p);
	if (parentItem()) {
		DebugDialog::debug(QString("setting pos %1 %2").arg(p.x()).arg(p.y()) );
	}
}

QVariant BusConnectorItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
     if (change == ItemPositionChange && scene()) {
		 QPointF p = value.toPointF();
		 DebugDialog::debug(QString("got busconnector pos change %1 %2 %3").arg(p.x()).arg(p.y()).arg(busID()) );
     }
     return ConnectorItem::itemChange(change, value);
}


void BusConnectorItem::setVisible(bool visible) {
	ConnectorItem::setVisible(visible);
	DebugDialog::debug(QString("setting visible %1 %2").arg(visible).arg(busID()) );
}

*/


void BusConnectorItem::collectEqualPotential(QList<ConnectorItem *> & connectorItems, QList<BusConnectorItem *> & busConnectorItems, bool addMerged, ViewGeometry::WireFlags skipWires) {
	// collects all the connectors at the same potential
	// assumes first connector item does not belong to the skipWires category

	for (int i = 0; i < connectorItems.count(); i++) {
		ConnectorItem * connectorItem = connectorItems[i];
		//DebugDialog::debug(QString("testing %1 %2 %3").arg(connectorItem->attachedToID()).arg(connectorItem->attachedToTitle()).arg(connectorItem->connectorStuffID()) );
		BusConnectorItem * busConnectorItem = dynamic_cast<BusConnectorItem *>(connectorItem);
		if (busConnectorItem != NULL && !busConnectorItems.contains(busConnectorItem)) {
			busConnectorItems.append(busConnectorItem);
		}

		foreach (ConnectorItem * cto, connectorItem->connectedToItems()) {
			if (connectorItems.contains(cto)) continue;

			bool append = true;
			if (skipWires != ViewGeometry::NoFlag) {
				if (cto->attachedToItemType() == ModelPart::Wire) {
					Wire * wire = dynamic_cast<Wire *>(cto->attachedTo());
					if (!wire->hasAnyFlag(skipWires)) {
						append = false;
					}
				}
			}
			
			if (append) {
				connectorItems.append(cto);
			}
		}

		if (connectorItem->attachedToItemType() == ModelPart::Wire) {
			ConnectorItem * otherEnd = dynamic_cast<Wire *>(connectorItem->attachedTo())->otherConnector(connectorItem);
			if (!connectorItems.contains(otherEnd)) {
				//DebugDialog::debug(QString("adding %1 %2 %3").arg(otherEnd->attachedToID()).arg(otherEnd->attachedToTitle()).arg(otherEnd->connectorStuffID()) );
				connectorItems.append(otherEnd);
			}
		}

		if (addMerged && (busConnectorItem != NULL)) {
			foreach (BusConnectorItem * bci, busConnectorItem->merged()) {
				if (!connectorItems.contains(bci)) {
					connectorItems.append(bci);
				}
			}
		}
	}
}

void BusConnectorItem::collectParts(QList<ConnectorItem *> & connectorItems, QList<ConnectorItem *> & partsConnectors)
{
	foreach (ConnectorItem * connectorItem, connectorItems) {
		if (dynamic_cast<BusConnectorItem *>(connectorItem) != NULL) continue;

		ItemBase * candidate = connectorItem->attachedTo();
		if (candidate->itemType() == ModelPart::Part) {
			if (!partsConnectors.contains(connectorItem)) {
				//DebugDialog::debug(QString("collecting part %1 %2").arg(candidate->id()).arg(connectorItem->connectorStuffID()) );
				partsConnectors.append(connectorItem);
			}
		}
	}
}


void BusConnectorItem::mergeGraphics(BusConnectorItem * child, bool hookTokenHolder) {
	BusConnectorItemGroup * group = dynamic_cast<BusConnectorItemGroup *>(this->parentItem());
	if (group == NULL) {
		group = new BusConnectorItemGroup(NULL);
		group->setFlag(QGraphicsItem::ItemIsMovable, true);
		scene()->addItem(group);
		group->addToGroup(this);
		group->setPos(this->pos());
		this->setPos(QPointF(0,0));
		if (hookTokenHolder) {
			bool success = QObject::connect(m_tokenHolder->attachedTo(), SIGNAL(posChangedSignal()), 
											group, SLOT(posChangedSlot()) );

			DebugDialog::debug(QString("merge connect result %1 %2 %3 %4").arg(m_tokenHolder->attachedToTitle())
				.arg(m_tokenHolder->attachedToID())
				.arg(m_tokenHolder->connectorStuffID())
				.arg(success) );
		}
	}

	if (child != NULL) {
		if (child->parentItem() != group) {
			QGraphicsItemGroup * oldGroup = dynamic_cast<QGraphicsItemGroup *>(child->parentItem());
			if (oldGroup != NULL) {
				foreach (QGraphicsItem * childItem, oldGroup->childItems()) {
					group->addToGroup(childItem);
					childItem->setPos(0,0);
					dynamic_cast<BusConnectorItem *>(childItem)->adjustConnectedItems();
				}
				scene()->removeItem(oldGroup);
				delete oldGroup;
			}
			else {
				group->addToGroup(child);
				child->setPos(QPointF(0, 0));
				child->adjustConnectedItems();
			}
		}
	}
	
}

void BusConnectorItem::unmergeGraphics(BusConnectorItem * child, bool hookTokenHolder, ItemBase::ViewIdentifier viewIdentifier, QPointF childPos) {
	BusConnectorItemGroup * group = dynamic_cast<BusConnectorItemGroup *>(this->parentItem());
	if (group == NULL) return;

	if (child->parentItem() == group) {
		group->removeFromGroup(child);
		child->setPos(childPos);
		child->mergeGraphicsDelay(NULL, hookTokenHolder, viewIdentifier);
	}
}

void BusConnectorItem::mergeGraphicsDelay(BusConnectorItem * child, bool hookTokenHolder, ItemBase::ViewIdentifier viewIdentifier)
{
	BusConnectorTimer * timer = new BusConnectorTimer(child, hookTokenHolder, viewIdentifier);
	timer->setSingleShot(true);
	timer->setInterval(10);
	connect(timer, SIGNAL(betterTimeout(BetterTimer *)), this, SLOT(mergeGraphicsSlot(BetterTimer *)) );
	timer->start();
}

void BusConnectorItem::mergeGraphicsSlot(BetterTimer * betterTimer) {
	BusConnectorTimer * busConnectorTimer = dynamic_cast<BusConnectorTimer *>(betterTimer);
	if (busConnectorTimer != NULL) {
		if (busConnectorTimer->busConnectorItem() == NULL) {
			initGraphics(busConnectorTimer->viewIdentifier());
		}
		mergeGraphics(busConnectorTimer->busConnectorItem(), busConnectorTimer->hookTokenHolder());
		/*
		DebugDialog::debug(QString("bc post get pos %1 %2 %3 %4 %5")
				.arg(this->mapToScene(this->pos()).x())
				.arg(this->mapToScene(this->pos()).y())
				.arg(this->m_tokenHolder->attachedToID())
				.arg(this->m_tokenHolder->attachedToTitle())
				.arg(this->m_tokenHolder->connectorStuffID()) );
		*/
	}
}

void BusConnectorItem::initGraphics(ItemBase::ViewIdentifier viewIdentifier) 
{
	switch (viewIdentifier) {
	case ItemBase::PCBView:
	case ItemBase::SchematicView:				// make it the same for now
		{
			QRectF rect = this->rect();
			rect.moveTo(0,0);
			QPointF p = m_tokenHolder->sceneAdjustedTerminalPoint() - rect.center();
			this->setPos(p);
			
			DebugDialog::debug(QString("bc pre  get pos %1 %2 %3 %4 %5 %6")
					.arg(p.x())
					.arg(p.y())
					.arg(viewIdentifier)
					.arg(m_tokenHolder->attachedToID())
					.arg(m_tokenHolder->attachedToTitle())
					.arg(m_tokenHolder->connectorStuffID()) );
			setVisible(false);
		}			
		break;
	/*
	case ItemBase::SchematicView:
		{
			// find a reasonable place to stick a bus connector near one of the borders of the part attached to the bus
			// based on shortest distance to one of the borders
			QPointF pos = m_tokenHolder->adjustedTerminalPoint();
			QRectF rect = m_tokenHolder->attachedTo()->boundingRect();
			static QPoint points[] = {QPoint(-20, 0), QPoint(0, -20), QPoint(20, 0), QPoint(0, 20) };
			qreal dt = qAbs(pos.y() - rect.top());
			qreal db =  qAbs(pos.y() - rect.bottom());
			qreal dl = qAbs(pos.x() - rect.left());
			qreal dr = qAbs(pos.x() - rect.right());
			int x, y, yix, xix;
			if (dt <= db) {
				yix = 1;
				y = dt;
			}
			else {
				yix = 3;
				y = db;
			}
			if (dl <= dr) {
				xix = 0;
				x = dl;
			}
			else {
				xix = 2;
				x = dr;
			}
			int ix = (x <= y) ? xix : yix;

			QPointF bpos = m_tokenHolder->mapToScene(pos + points[ix] - this->rect().center());
			DebugDialog::debug(QString("bpos %1 %2").arg(bpos.x()).arg(bpos.y()) );
			QRectF r = this->rect();
			r.moveTo(bpos);
			//setRect(r);
			setPos(bpos);
			setVisible(true);
		}
		break;
	*/
	default:
		break;
	}
}

bool BusConnectorItem::initialized() {
	return m_initialized;
}

void BusConnectorItem::initialize(ItemBase::ViewIdentifier viewIdentifier, ConnectorItem * tokenHolder) {

	if (m_initialized && (tokenHolder == m_tokenHolder)) return;

	// not sure why, but when calling mergeGraphics directly from here
	// the resulting busConnectorItem wasn't located correctly
	// putting in a delay between creating the busConnectorItem and
	// attaching it to a group fixed the location bug
	// TODO: QApplication::processEvents might have the same effect

	m_initialized = true;
	m_tokenHolder = tokenHolder;
	if (this->scene() == NULL) {
		m_attachedTo->scene()->addItem(this);
	}
	switch (viewIdentifier) {
		case ItemBase::BreadboardView:
			// hide busConnectors in Breadboard view
			setVisible(false);
			break;
		case ItemBase::PCBView:
			// hide busConnectors in PCB view, but...
			mergeGraphicsDelay(NULL, true, viewIdentifier);
			setVisible(true);
			break;
		case ItemBase::SchematicView:
			//mergeGraphicsDelay(NULL, false, viewIdentifier);
			// make it the same for now
			mergeGraphicsDelay(NULL, true, viewIdentifier);
			setVisible(true);
			break;
		default:
			m_initialized = false;
			break;
	}

}

void BusConnectorItem::updateVisibility(ItemBase::ViewIdentifier viewIdentifier) {
	if (viewIdentifier == ItemBase::SchematicView && false) {   // make this a no-op for now
		// hide degenerate buses in Schematic view
		bool vis = connectionsCount() > 1;
		setVisible(vis);
		foreach (ConnectorItem * connectorItem, connectedToItems()) {
			connectorItem->attachedTo()->setVisible(vis);
		}
	}
}

//////////////////////////////////

BusConnectorItemGroup::BusConnectorItemGroup(QGraphicsItem * parent) : QGraphicsItemGroup(parent) 
{
}

void BusConnectorItemGroup::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	BusConnectorItem::saveSelection(scene());
	QGraphicsItemGroup::mousePressEvent(event);
}

void BusConnectorItemGroup::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsItemGroup::mouseMoveEvent(event);
	adjustConnectorsConnectedItems();
}

void BusConnectorItemGroup::adjustConnectorsConnectedItems() 
{
	foreach (QGraphicsItem * childItem, childItems()) {
		BusConnectorItem * bci = dynamic_cast<BusConnectorItem *>(childItem);
		if (bci == NULL) continue;

		bci->adjustConnectedItems();
	}
}

void BusConnectorItemGroup::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsItemGroup::mouseReleaseEvent(event);
	BusConnectorItem::restoreSelection();
}


void BusConnectorItemGroup::posChangedSlot() {
	//DebugDialog::debug("got signal from item");

	foreach (QGraphicsItem * childItem, childItems()) {
		BusConnectorItem * bci = dynamic_cast<BusConnectorItem *>(childItem);
		if (bci == NULL) continue;

		QRectF rect = bci->rect();
		QPointF p = bci->tokenHolder()->sceneAdjustedTerminalPoint() - rect.center();
		this->setPos(p);   //  
		break;
	}

	adjustConnectorsConnectedItems();
}

void BusConnectorItem::setOwner(ItemBase * owner) {
	m_owner = owner;
}


/////////////////////////////////////////

BusConnectorTimer::BusConnectorTimer(BusConnectorItem *busConnectorItem, bool hookTokenHolder, ItemBase::ViewIdentifier viewIdentifier) 
: BetterTimer(NULL) 
{
	m_busConnectorItem = busConnectorItem;
	m_hookTokenHolder = hookTokenHolder;
	m_viewIdentifier = viewIdentifier;
}

BusConnectorItem * BusConnectorTimer::busConnectorItem() {
	return m_busConnectorItem;
}


bool BusConnectorTimer::hookTokenHolder() {
	return m_hookTokenHolder;
}

ItemBase::ViewIdentifier BusConnectorTimer::viewIdentifier() {
	return m_viewIdentifier;
}

