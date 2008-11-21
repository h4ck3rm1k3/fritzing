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
	BusConnectorItem( ItemBase * busOwner, class Bus * bus );
	~BusConnectorItem();
	
	void merge(BusConnectorItem *);
	bool isMergedWith(BusConnectorItem *);
	const QString & busID();
	bool isMerged();
	void unmerge(BusConnectorItem *);
	const QList<BusConnectorItem *> & merged();
	class Bus * bus();
	const QString & connectorStuffID();
	bool isBusConnector();
	bool initialized();
	void initialize(ItemBase::ViewIdentifier viewIdentifier);
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
	static void collectEqualPotential(QList<ConnectorItem *> & connectorItems, QList<BusConnectorItem *> & busConnectorItems, bool addMerged, ViewGeometry::WireFlags keepWires);
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
	void writeTopLevelAttributes(QXmlStreamWriter & writer);
	void writeOtherElements(QXmlStreamWriter & writer);

protected:
	class Bus * m_bus;
	QList<BusConnectorItem *> m_merged;
	bool m_initialized;
	ItemBase * m_owner;

protected:
	static QList<QGraphicsItem *> m_savedItems;
};

