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
	void restoreConnections(QDomElement & element, QHash<long, ItemBase *> newItems);
	bool connectedTo(ConnectorItem *);
	const QList<ConnectorItem *> & connectedToItems();
	void setHidden(bool hidden);
	ConnectorItem * overConnectorItem();
	void setOverConnectorItem(ConnectorItem *);
	long attachedToID();
	int attachedToItemType();
	const QString & attachedToTitle();
	virtual const QString & connectorStuffID();
	const QString & busID();
	ModelPartStuff * modelPartStuff();
	ModelPart * modelPart();
	virtual class Bus * bus();
	void tempConnectTo(ConnectorItem * item);
	void tempRemove(ConnectorItem * item);
	virtual void adjustConnectedItems();
	void setCircular(bool);
	void setOpacity(qreal);
	Connector::ConnectorType connectorType();
	bool chained();
	void setChained(bool chained);
	virtual void saveInstance(QXmlStreamWriter & );
	bool maleToFemale(ConnectorItem * other);
	bool wiredTo(ConnectorItem *);
	class Wire * wiredTo(ConnectorItem *, ViewGeometry::WireFlags);
	void setDirty(bool dirty);
	bool isDirty();

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
	bool m_chained;
	bool m_dirty;

public:
	static QPen normalPen;
	static QPen hoverPen;
	static QPen connectedPen;
	static QBrush hoverBrush;
	static QBrush normalBrush;
	static QBrush connectedBrush;
};

#endif
