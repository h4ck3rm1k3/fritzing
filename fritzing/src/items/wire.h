/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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
#include <QMenu>

#include "itembase.h"

class WireMenu : public QMenu {
	Q_OBJECT

public:
	WireMenu(const QString & title, QWidget * parent = 0);
	void setWire(Wire *);
	Wire * wire();

protected:
	Wire * m_wire;
};

class WireAction : public QAction {
	Q_OBJECT

public:
	WireAction(QAction *);
	WireAction(const QString & text, QObject * parent);

	void setWire(Wire *);
	Wire * wire();

protected:
	Wire * m_wire;
};

class Wire : public ItemBase
{
Q_OBJECT

public:
	Wire(class ModelPart *, ViewIdentifierClass::ViewIdentifier, const ViewGeometry &, long id, QMenu * itemMenu, bool initLabel);
	virtual ~Wire();


	// for debugging
	//void setLine(QLineF line);				
	//void setLine(qreal x1, qreal y1, qreal x2, qreal y2);	
	//void setPos(const QPointF & pos);
	// for debugging


	void saveGeometry();
	bool itemMoved();
	void saveInstanceLocation(QXmlStreamWriter &);
	void writeGeometry(QXmlStreamWriter &);
	void moveItem(ViewGeometry & viewGeometry);
	void hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, class ConnectorItem * item);
	void hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, class ConnectorItem * item);

	void initDragEnd(ConnectorItem * dragEnd, QPointF scenePos);
	void connectedMoved(ConnectorItem * from, ConnectorItem * to);
	void setLineAnd(QLineF line, QPointF pos, bool useLine);
	ConnectorItem * otherConnector(ConnectorItem *);
	ConnectorItem * connector0();
	ConnectorItem * connector1();
	virtual class FSvgRenderer * setUp(ViewLayer::ViewLayerID viewLayerID, const LayerHash & viewLayers, class InfoGraphicsView *);
	void findConnectorsUnder();
	void collectChained(QList<Wire *> &, QList<ConnectorItem *> & ends);
	void collectWires(QList<Wire *> & wires);
	bool stickyEnabled();
	void setPcbPenBrush(QBrush & brush);
	bool getRouted();
	void setRouted(bool);
	bool getRatsnest();
	void setRatsnest(bool);
	void setAutoroutable(bool);
	bool getAutoroutable();
	void setNormal(bool);
	bool getNormal();
	bool getTrace();


	bool hasFlag(ViewGeometry::WireFlag);
	bool hasAnyFlag(ViewGeometry::WireFlags);
	ViewGeometry::WireFlags wireFlags();
	void setWireFlags(ViewGeometry::WireFlags);

	QString colorString();
	QString hexString();
	void setColorString(QString, qreal opacity);
	virtual void setColor(const QColor &, qreal opacity);
	qreal opacity();
	void setOpacity(qreal opacity);
	const QColor & color();
	void setWireWidth(qreal, InfoGraphicsView *);
	void setPenWidth(qreal width, InfoGraphicsView *);
	qreal width();
	qreal mils();
	void setExtras(QDomElement &, InfoGraphicsView *);
	Wire * findTraced(ViewGeometry::WireFlags flags, QList<ConnectorItem *>  & ends);
	bool draggingEnd();
	void simpleConnectedMoved(ConnectorItem * to);
	void simpleConnectedMoved(ConnectorItem * from, ConnectorItem * to);
	void setCanChainMultiple(bool);
	bool canChangeColor();
	void collectDirectWires(QList<Wire *> & wires);
	bool isGrounded();
	bool collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget);
	bool hasPartLabel();
	PluralType isPlural();
	virtual bool canSwitchLayers();
	void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
	bool hasPartNumberProperty();

protected slots:
	void colorEntry(const QString & text);

public:
	static qreal STANDARD_TRACE_WIDTH;
	static qreal HALF_STANDARD_TRACE_WIDTH;

public:
	static void initNames();
	static QRgb getRgb(const QString & name);
	static void cleanup();

protected:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEventAux(QPointF eventPos, bool shiftModifier);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void paintHover(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget); 
	void initEnds(const ViewGeometry &, QRectF defaultRect, class InfoGraphicsView *);
	void connectionChange(ConnectorItem * onMe, ConnectorItem * onIt, bool connect);
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
	void getConnectedColor(ConnectorItem *, QBrush * &, QPen * &, qreal & opacity, qreal & negativePenWidth, bool & negativeOffsetRect);
	bool connectionIsAllowed(ConnectorItem *);
	bool releaseDrag();
	void setIgnoreSelectionChange(bool);
	virtual void setColorFromElement(QDomElement & element);
	void checkVisibility(ConnectorItem * onMe, ConnectorItem * onIt, bool connect);

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
	QPen m_bendpoint2Pen;
	qreal m_bendpointWidth;
	qreal m_bendpoint2Width;
	bool m_negativeOffsetRect;
	qreal m_opacity;
	bool m_canChainMultiple;
	bool m_ignoreSelectionChange;
	bool m_markedDeleted;

public:
	static QStringList colorNames;
	static QHash<QString, QString> colorTrans;
	static QHash<int, QString> widthTrans;
	static QList<int> widths;

protected:
	static QHash<QString, QString> shadowColors;
	static QHash<QString, QString> colors;

signals:
	void wireChangedSignal(Wire* me, QLineF oldLine, QLineF newLine, QPointF oldPos, QPointF newPos, ConnectorItem * from, ConnectorItem * to);
	void wireSplitSignal(Wire* me, QPointF newPos, QPointF oldPos, QLineF oldLine);
	void wireJoinSignal(Wire* me, ConnectorItem * clickedConnectorItem);
};

#endif
