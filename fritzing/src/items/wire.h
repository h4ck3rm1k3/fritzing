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

#ifndef WIRE_H
#define WIRE_H


#include <QGraphicsLineItem>
#include <QLineF>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QHash>

#include "itembase.h"

class Wire : public ItemBase
{
Q_OBJECT

public:
	Wire(class ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry &, long id, QMenu * itemMenu);
	virtual ~Wire();

	void saveGeometry();
	bool itemMoved();
	void saveInstanceLocation(QXmlStreamWriter &);
	void writeGeometry(QXmlStreamWriter &);
	void moveItem(ViewGeometry & viewGeometry);
	void hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, class ConnectorItem * item);
	void hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, class ConnectorItem * item);

	void initDragEnd(ConnectorItem * dragEnd, QPointF scenePos, bool shiftModifier);
	void connectedMoved(ConnectorItem * from, ConnectorItem * to);
	void setLineAnd(QLineF line, QPointF pos, bool useLine);
	void setLine(QLineF line);				// helpful for debugging
	void setLine(qreal x1, qreal y1, qreal x2, qreal y2);				// helpful for debugging
	ConnectorItem * otherConnector(ConnectorItem *);
	ConnectorItem * connector0();
	ConnectorItem * connector1();
	void setUp(ViewLayer::ViewLayerID viewLayerID, const LayerHash & viewLayers, class InfoGraphicsView *);
	void findConnectorsUnder();
	void collectChained(QList<Wire *> &, QList<ConnectorItem *> & ends, QList<ConnectorItem *> & uniqueEnds);
	void collectWires(QList<Wire *> & wires);
	bool stickyEnabled(ItemBase * stickTo);
	void setPcbPenBrush(QBrush & brush);
	bool getTrace();
	void setTrace(bool);
	bool getRouted();
	void setRouted(bool);
	bool getVirtual();
	void setVirtual(bool);
	bool getJumper();
	void setJumper(bool);
	bool getRatsnest();
	void setRatsnest(bool);
	void setAutoroutable(bool);
	bool getAutoroutable();
	void setNormal(bool);
	bool getNormal();


	bool hasFlag(ViewGeometry::WireFlag);
	bool hasAnyFlag(ViewGeometry::WireFlags);
	ViewGeometry::WireFlags wireFlags();
	void setWireFlags(ViewGeometry::WireFlags);

	QString colorString();
	QString hexString();
	void setColorString(QString, qreal opacity);
	void setColor(QColor &, qreal opacity);
	qreal opacity();
	void setOpacity(qreal opacity);
	const QColor * color();
	void setWidth(int);
	void setPenWidth(int width);
	int width();
	void setExtras(QDomElement &);
	void setColor(QDomElement & element);
	Wire * findJumperOrTraced(ViewGeometry::WireFlags flags, QList<ConnectorItem *>  & ends);
	bool draggingEnd();
	void simpleConnectedMoved(ConnectorItem * to);
	void simpleConnectedMoved(ConnectorItem * from, ConnectorItem * to);
	void setCanChainMultiple(bool);
	bool canChangeColor();
	bool canChangeWidth();
	void collectDirectWires(QList<Wire *> & wires);
	bool isGrounded();

public:
	static const qreal ROUTED_OPACITY;
	static const qreal UNROUTED_OPACITY;

public:
	static void initNames();
	static QRgb getRgb(const QString & name);
	static const QColor * netColor(ViewIdentifierClass::ViewIdentifier);
	static void cleanup();
	static void getColor(QColor & color, const QString & name);

protected:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEventAux(QPointF eventPos);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
	void paintHover(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget); 
	void initEnds(const ViewGeometry &, QRectF defaultRect, class InfoGraphicsView *);
	void connectionChange(ConnectorItem *);
	void mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	void mouseDoubleClickConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	void mouseMoveConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	void mouseReleaseConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	bool acceptsMouseDoubleClickConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	bool acceptsMouseMoveConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	bool acceptsMouseReleaseConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
 	virtual class FSvgRenderer * setUpConnectors(class ModelPart *, ViewIdentifierClass::ViewIdentifier);
	void collectChained(ConnectorItem * connectorItem, QList<Wire *> & chained, QList<ConnectorItem *> & ends);
	void setConnector0Rect();
	void setConnector1Rect();
	void collectWiresAux(QList<Wire *> & wires, ConnectorItem * start);
	void setShadowColor(QColor &);
	void calcNewLine(ConnectorItem * from, ConnectorItem * to, QPointF & p1, QPointF & p2);
	void collectDirectWires(ConnectorItem * connectorItem, QList<Wire *> & wires);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void getConnectedColor(ConnectorItem *, QBrush * &, QPen * &, qreal & opacity, int & negativePenWidth);
	bool connectionIsAllowed(ConnectorItem *);
	bool releaseDrag();


protected:
	QPointF m_wireDragOrigin;
	bool m_dragEnd;
	bool m_drag0;
	QPointer<class ConnectorItem> m_connectorHover;
	QPointer<class ConnectorItem> m_connector0;
	QPointer<class ConnectorItem> m_connector1;
	QString m_colorName;
	QPen m_shadowPen;
	QBrush m_shadowBrush;
	QPen m_bendpointPen;
	int m_bendpointWidth;
	qreal m_opacity;
	bool m_canChainMultiple;
	QPointF m_dragEndInitialPos;
	bool m_dragEndShiftModifier;
	Constraint m_dragEndConstraint;

public:
	static QList<QString> colorNames;
	static QHash<QString, QString> colorTrans;
	static QList<QString> widthNames;
	static QHash<QString, qreal> widthTrans;


protected:
	static QHash<QString, QString> shadowColors;
	static QHash<QString, QString> colors;

signals:
	void wireChangedSignal(Wire* me, QLineF oldLine, QLineF newLine, QPointF oldPos, QPointF newPos, ConnectorItem * from, ConnectorItem * to);
	void wireSplitSignal(Wire* me, QPointF newPos, QPointF oldPos, QLineF oldLine);
	void wireJoinSignal(Wire* me, ConnectorItem * clickedConnectorItem);
};

#endif
