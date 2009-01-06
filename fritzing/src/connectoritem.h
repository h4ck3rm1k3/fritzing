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

#ifndef CONNECTORITEM_H
#define CONNECTORITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsSceneHoverEvent>
#include <QPen>
#include <QBrush>
#include <QXmlStreamWriter>

#include "connector.h"
#include "itembase.h"

class ConnectorItem : public QGraphicsRectItem
{

public:
	ConnectorItem(Connector *, ItemBase* attachedTo);
	~ConnectorItem();

	Connector * connector();
	ItemBase * attachedTo();
	void connectorHover(class ItemBase *, bool hovering);
	void connectTo(ConnectorItem *);
	int connectionsCount();
	void attachedMoved();
	ConnectorItem * removeConnection(ItemBase *);
	void removeConnection(ConnectorItem *, bool emitChange);
	ConnectorItem * firstConnectedToIsh();
	void setTerminalPoint(QPointF);
	QPointF terminalPoint();
	QPointF adjustedTerminalPoint();
	QPointF sceneAdjustedTerminalPoint();
	bool connectedTo(ConnectorItem *);
	const QList<ConnectorItem *> & connectedToItems();
	void setHidden(bool hidden);
	ConnectorItem * overConnectorItem();
	void setOverConnectorItem(ConnectorItem *);
	long attachedToID();
	int attachedToItemType();
	const QString & attachedToTitle();
	const QString & connectorStuffID();
	const QString & connectorStuffName();
	const QString & busID();
	ModelPartStuff * modelPartStuff();
	ModelPart * modelPart();
	virtual class Bus * bus();
	void tempConnectTo(ConnectorItem * item);
	void tempRemove(ConnectorItem * item);
	void setCircular(bool);
	void setOpacity(qreal);
	Connector::ConnectorType connectorType();
	bool chained();
	virtual void saveInstance(QXmlStreamWriter & );
	bool maleToFemale(ConnectorItem * other);
	bool wiredTo(ConnectorItem *);
	class Wire * wiredTo(ConnectorItem *, ViewGeometry::WireFlags);
	void setDirty(bool dirty);
	bool isDirty();
	void setBaseTooltip(const QString &);

protected:
	virtual void hoverEnterEvent( QGraphicsSceneHoverEvent * event );
	virtual void hoverLeaveEvent( QGraphicsSceneHoverEvent * event );
	virtual void restoreColor();
	virtual void setHoverColor();
	virtual void setNormalColor();
	virtual void setConnectedColor();
	void setColorAux(QBrush brush, QPen pen, bool paint);
	void setColorAux(const QColor &color, bool paint=true);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
	virtual void writeTopLevelAttributes(QXmlStreamWriter & writer);
	virtual void writeOtherElements(QXmlStreamWriter & writer);
	void updateTooltip();

protected:
	Connector * m_connector;
	ItemBase * m_attachedTo;
	QList<ConnectorItem *> m_connectedTo;
	QPointF m_terminalPoint;
	bool m_hidden;
	bool m_paint;
	ConnectorItem * m_overConnectorItem;
	qreal m_opacity;
	bool m_circular;
	bool m_dirty;
	QString m_baseTooltip;

public:
	static void collectEqualPotential(QList<ConnectorItem *> & connectorItems);
	static void collectEqualPotentialParts(QList<ConnectorItem *> & connectorItems, ViewGeometry::WireFlags flags);
	static void collectParts(QList<ConnectorItem *> & connectorItems, QList<ConnectorItem *> & partsConnectors);

public:
	static QPen normalPen;
	static QPen hoverPen;
	static QPen connectedPen;
	static QBrush hoverBrush;
	static QBrush normalBrush;
	static QBrush connectedBrush;
};

#endif
