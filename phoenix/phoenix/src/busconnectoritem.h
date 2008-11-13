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

