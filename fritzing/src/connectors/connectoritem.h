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
#include <QPointer>

#include "connector.h"
#include "../items/itembase.h"

class ConnectorItem : public QObject, public QGraphicsRectItem
{
Q_OBJECT

public:
	ConnectorItem(Connector *, ItemBase* attachedTo);
	~ConnectorItem();

	Connector * connector();
	ItemBase * attachedTo();
	void connectorHover(class ItemBase *, bool hovering);
	bool connectorHovering();
	void clearConnectorHover();
	void connectTo(ConnectorItem *);
	int connectionsCount();
	void attachedMoved();
	ConnectorItem * removeConnection(ItemBase *);
	void removeConnection(ConnectorItem *, bool emitChange);
	ConnectorItem * firstConnectedToIsh();
	void setTerminalPoint(QPointF);
	QPointF terminalPoint();
	QPointF adjustedTerminalPoint();
	QPointF sceneAdjustedTerminalPoint(ConnectorItem * anchor);
	bool connectedTo(ConnectorItem *);
	const QList< QPointer<ConnectorItem> > & connectedToItems();
	void setHidden(bool hidden);
	ConnectorItem * overConnectorItem();
	void setOverConnectorItem(ConnectorItem *);
	long attachedToID();
	int attachedToItemType();
	const QString & attachedToTitle();
	const QString & connectorSharedID();
	const QString & connectorSharedName();
	ErcData * connectorSharedErcData();
	const QString & busID();
	ModelPartShared * modelPartShared();
	ModelPart * modelPart();
	class Bus * bus();
	void tempConnectTo(ConnectorItem * item, bool applyColor);
	void tempRemove(ConnectorItem * item, bool applyColor);
	void setCircular(bool);
	Connector::ConnectorType connectorType();
	bool chained();
	void saveInstance(QXmlStreamWriter & );
	void writeConnector(QXmlStreamWriter & writer, const QString & elementName);
	bool maleToFemale(ConnectorItem * other);
	bool wiredTo(ConnectorItem *);
	class Wire * wiredTo(ConnectorItem *, ViewGeometry::WireFlags);
	void setBaseTooltip(const QString &);
	void clearConnector();
	void setIgnoreAncestorFlag(bool);
	void setIgnoreAncestorFlagIfExternal(bool);
	bool connectionIsAllowed(ConnectorItem * other);
	void setChosen(bool);
	void prepareGeometryChange();
	bool isExternal();
	void setRadius(qreal radius, qreal strokeWidth);
	qreal radius();
	qreal strokeWidth();
	void restoreColor(bool doBuses, int busConnectedCount);
	void showEqualPotential(bool show);
	void setShape(QPainterPath &);
	void setHoverColor();
	bool isGrounded();

protected:
	void hoverEnterEvent( QGraphicsSceneHoverEvent * event );
	void hoverLeaveEvent( QGraphicsSceneHoverEvent * event );
	void hoverMoveEvent( QGraphicsSceneHoverEvent * event );
	void setNormalColor();
	void setConnectedColor();
	void setUnconnectedColor();
	void setChosenColor();
	void setColorAux(QBrush brush, QPen pen, bool paint);
	void setColorAux(const QColor &color, bool paint=true);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
	void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
	void writeTopLevelAttributes(QXmlStreamWriter & writer);
	void writeOtherElements(QXmlStreamWriter & writer);
	void updateTooltip();
	bool sceneEvent(QEvent *event);
        class Wire * wiredToAux(ConnectorItem * target, ViewGeometry::WireFlags flags, QList<ConnectorItem *> & visited);
        bool wiredToAux(ConnectorItem * target, QList<ConnectorItem *> & visited);
	QPainterPath shape() const;
	bool isEverVisible();

protected:
	Connector * m_connector;
	QPointer<ItemBase> m_attachedTo;
	QList< QPointer<ConnectorItem> > m_connectedTo;
	QPointF m_terminalPoint;
	bool m_hidden;
	bool m_paint;
	bool m_chosen;
	QPointer<ConnectorItem> m_overConnectorItem;
	qreal m_opacity;
	bool m_circular;
	QString m_baseTooltip;
	bool m_connectorHovering;
	bool m_ignoreAncestorFlag;
	bool m_spaceBarWasPressed;
	bool m_hoverEnterSpaceBarWasPressed;
	qreal m_radius;
	qreal m_strokeWidth;
	int m_negativePenWidth;
	QPainterPath m_shape;
	
protected:	
	static QList<ConnectorItem *>  m_equalPotentialDisplayItems;

public:
	static void collectEqualPotential(QList<ConnectorItem *> & connectorItems, ViewGeometry::WireFlags skipFlags = ViewGeometry::TraceJumperRatsnestFlags);
	static void collectEqualPotentialParts(QList<ConnectorItem *> & connectorItems, ViewGeometry::WireFlags flags, bool includeSymbols);
	static void collectParts(QList<ConnectorItem *> & connectorItems, QList<ConnectorItem *> & partsConnectors, bool includeSymbols);
	static void clearEqualPotentialDisplay();
	static bool isGrounded(ConnectorItem * c1, ConnectorItem * c2);

public:
	static const QList<ConnectorItem *> emptyConnectorItemList;
};

#endif
