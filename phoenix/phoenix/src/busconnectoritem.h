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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/

/*
 *  busconnectoritem.h
 *  Fritzing
 *
 *  Created by Jonathan Cohen on 9/11/08.
 *  Copyright 2008 FHP. All rights reserved.
 *
 */

#include "connectoritem.h"
#include "bettertimer.h"
#include <QGraphicsItemGroup>
#include <QVariant>

class BusConnectorItem : public QObject, public ConnectorItem
{
	Q_OBJECT

public:
	BusConnectorItem( ItemBase * busOwner, class Bus * bus, ConnectorItem * tokenHolder );
	~BusConnectorItem();
	
	void setTokenHolder(ConnectorItem *);
	void merge(BusConnectorItem *);
	bool isMergedWith(BusConnectorItem *);
	const QString & busID();
	void adjustConnectedItems();
	ConnectorItem * tokenHolder();
	bool isMerged();
	void unmerge(BusConnectorItem *);
	const QList<BusConnectorItem *> & merged();
	class Bus * bus();
	const QString & connectorStuffID();
	void mergeGraphics(BusConnectorItem * child, bool hookTokenHolder);
	void unmergeGraphics(BusConnectorItem * child, bool hookTokenHolder, ItemBase::ViewIdentifier, QPointF childPos);
	void mergeGraphicsDelay(BusConnectorItem * child, bool hookTokenHolder, ItemBase::ViewIdentifier);
	bool initialized();
	void initialize(ItemBase::ViewIdentifier viewIdentifier, ConnectorItem * tokenHolder);
	void updateVisibility(ItemBase::ViewIdentifier viewIdentifier);
	void setOwner(ItemBase *);

	 // for debugging
	/*
	void setRect(QRectF);
	void setPos(QPointF);

	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void setVisible(bool visible);
	*/

public:
	static void collectEqualPotential(QList<ConnectorItem *> & connectorItems, QList<BusConnectorItem *> & busConnectorItems, bool addMerged, ViewGeometry::WireFlags skipWires);
	static void saveSelection(QGraphicsScene *);
	static void restoreSelection();
	static void collectParts(QList<ConnectorItem *> & connectorItems, QList<ConnectorItem *> & partsConnectors);

protected:
	void setHoverColor();
	void setNormalColor();
	void setConnectedColor();
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void initGraphics(ItemBase::ViewIdentifier viewIdentifier);
	void writeTopLevelAttributes(QXmlStreamWriter & writer);
	void writeOtherElements(QXmlStreamWriter & writer);

protected:
	ConnectorItem * m_tokenHolder;
	class Bus * m_bus;
	QList<BusConnectorItem *> m_merged;
	bool m_initialized;
	ItemBase * m_owner;

protected slots:
	void mergeGraphicsSlot(class BetterTimer *);

protected:
	static QList<QGraphicsItem *> m_savedItems;
};


class BusConnectorItemGroup : public QObject, public QGraphicsItemGroup 
{
	Q_OBJECT

public:
	BusConnectorItemGroup(QGraphicsItem * parent = NULL);

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void adjustConnectorsConnectedItems();

protected slots:
	void posChangedSlot();
	

};

class BusConnectorTimer : public BetterTimer {

public:
	BusConnectorTimer(class BusConnectorItem *, bool hookTokenHolder, ItemBase::ViewIdentifier);
	class BusConnectorItem * busConnectorItem();
	bool hookTokenHolder();
	ItemBase::ViewIdentifier viewIdentifier();

protected:
	class BusConnectorItem * m_busConnectorItem;
	bool m_hookTokenHolder;
	ItemBase::ViewIdentifier m_viewIdentifier;
};

