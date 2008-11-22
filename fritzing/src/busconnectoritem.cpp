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

BusConnectorItem::BusConnectorItem( ItemBase * busOwner, Bus * bus) :
	ConnectorItem(NULL, NULL)
{
	m_connector = bus->busConnector();
	m_owner = m_attachedTo = busOwner;
		
	m_bus = bus;
	if (bus != NULL) {
		bus->addViewItem(this);
	}
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
	attachedMoved();
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

	// do not write anything other than attributes in this routine.
	writer.writeAttribute("busId", connectorStuffID());
}


void BusConnectorItem::writeOtherElements(QXmlStreamWriter & writer) {
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


void BusConnectorItem::collectEqualPotential(QList<ConnectorItem *> & connectorItems, QList<BusConnectorItem *> & busConnectorItems, bool addMerged, ViewGeometry::WireFlags keepWires) {
	// collects all the connectors at the same potential
	// assumes first connector item does not belong to the skipWires category

	for (int i = 0; i < connectorItems.count(); i++) {
		ConnectorItem * connectorItem = connectorItems[i];
		//DebugDialog::debug(QString("testing %1 %2 %3").arg(connectorItem->attachedToID()).arg(connectorItem->attachedToTitle()).arg(connectorItem->connectorStuffID()) );
		BusConnectorItem * busConnectorItem = dynamic_cast<BusConnectorItem *>(connectorItem);
		if (busConnectorItem != NULL && !busConnectorItems.contains(busConnectorItem)) {
			busConnectorItems.append(busConnectorItem);
		}

		Wire * fromWire = (connectorItem->attachedToItemType() == ModelPart::Wire) ? dynamic_cast<Wire *>(connectorItem->attachedTo()) : NULL;
		foreach (ConnectorItem * cto, connectorItem->connectedToItems()) {
			if (connectorItems.contains(cto)) continue;

			bool append = false;
			if (keepWires != ViewGeometry::NoFlag) {
				if ((fromWire != NULL) && fromWire->hasAnyFlag(keepWires)) {
					// since we're coming from the right kind of wire, append the target
					append = true;
				}
				else if (cto->attachedToItemType() == ModelPart::Wire) {
					Wire * toWire = dynamic_cast<Wire *>(cto->attachedTo());
					if (toWire->hasAnyFlag(keepWires)) {
						// since we're going to the right kind of wire, append the target
						append = true;
					}
				}
			}
			else {
				append = true;
				if ((fromWire != NULL) && fromWire->hasAnyFlag(ViewGeometry::NotTraceJumperRatsnestFlags)) {
					// since we're coming from the right kind of wire, append the target
					append = false;
				}
				else if (cto->attachedToItemType() == ModelPart::Wire) {
					Wire * toWire = dynamic_cast<Wire *>(cto->attachedTo());
					if (toWire->hasAnyFlag(ViewGeometry::NotTraceJumperRatsnestFlags)) {
						// since we're going to the right kind of wire, append the target
						append = false;
					}
				}
			}
			
			if (append) {
				connectorItems.append(cto);
			}
		}

		if (fromWire != NULL) {
			ConnectorItem * otherEnd = fromWire->otherConnector(connectorItem);
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
		if (candidate->itemType() == ModelPart::Part || candidate->itemType() == ModelPart::Board) {
			if (!partsConnectors.contains(connectorItem)) {
				//DebugDialog::debug(QString("collecting part %1 %2").arg(candidate->id()).arg(connectorItem->connectorStuffID()) );
				partsConnectors.append(connectorItem);
			}
		}
	}
}


bool BusConnectorItem::initialized() {
	return m_initialized;
}

bool BusConnectorItem::isBusConnector() {
	return true;
}

void BusConnectorItem::initialize(ItemBase::ViewIdentifier viewIdentifier) {

	if (m_initialized) return;

	// not sure why, but when calling mergeGraphics directly from here
	// the resulting busConnectorItem wasn't located correctly
	// putting in a delay between creating the busConnectorItem and
	// attaching it to a group fixed the location bug
	// TODO: QApplication::processEvents might have the same effect

	m_initialized = true;
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
			setVisible(false);
			break;
		case ItemBase::SchematicView:
			// make it the same for now
			setVisible(false);
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

