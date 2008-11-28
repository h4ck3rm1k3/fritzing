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

#ifndef WIRE_H
#define WIRE_H


#include <QGraphicsLineItem>
#include <QGraphicsItemGroup>
#include <QLineF>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QHash>

#include "modelpart.h"
#include "itembase.h"
#include "viewgeometry.h"
#include "viewlayer.h"
#include "graphicssvglineitem.h"

//class Wire : public QObject, public QGraphicsLineItem, public virtual ItemBase

class Wire : public ItemBase
{
Q_OBJECT

public:
	Wire(ModelPart *, ItemBase::ViewIdentifier, const ViewGeometry &, long id, QMenu * itemMenu);
	virtual ~Wire();

	void saveGeometry();
	bool itemMoved();
	void saveInstanceLocation(QXmlStreamWriter &);
	void writeGeometry(QXmlStreamWriter &);
	void moveItem(ViewGeometry & viewGeometry);
	void rotateItem(qreal degrees);
	void hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, class ConnectorItem * item);
	void hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, class ConnectorItem * item);

	void initDragEnd(ConnectorItem * dragEnd);
	void connectedMoved(ConnectorItem * from, ConnectorItem * to);
	void setLineAnd(QLineF line, QPointF pos, bool useLine);
	void setLine(QLineF line);				// helpful for debugging
	void setLine(qreal x1, qreal y1, qreal x2, qreal y2);				// helpful for debugging
	ConnectorItem * otherConnector(ConnectorItem *);
	ConnectorItem * connector0();
	ConnectorItem * connector1();
	ItemBase * layerKinChief();
	void setUp(ViewLayer::ViewLayerID viewLayerID, const LayerHash & viewLayers);
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

	bool hasFlag(ViewGeometry::WireFlag);
	bool hasAnyFlag(ViewGeometry::WireFlags);
	ViewGeometry::WireFlags wireFlags();
	void setWireFlags(ViewGeometry::WireFlags);

	QString colorString();
	void setColorString(QString, qreal opacity);
	void setColor(QColor &, qreal opacity);
	qreal opacity();
	void setOpacity(qreal opacity);
	const QColor * color();
	void setWidth(int);
	int width();
	void setExtras(QDomElement &);
	void setColor(QDomElement & element);
	Wire * findJumperOrTraced(ViewGeometry::WireFlags flags, QList<ConnectorItem *>  & ends);
	bool draggingEnd();
	void connectsWithin(QSet<ItemBase *> & in, QHash<Wire *, ConnectorItem *> & out);
	void simpleConnectedMoved(ConnectorItem * to);
	void simpleConnectedMoved(ConnectorItem * from, ConnectorItem * to);
	qint64 chainedID();
	void setChainedID(qint64);

public:
	static QString moduleIDName;
	static void initNames();
	static QRgb getRgb(const QString & name);
	static const QColor * netColor(ItemBase::ViewIdentifier);

protected:
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
	void initEnds(const ViewGeometry &, QRectF defaultRect);
	void connectionChange(ConnectorItem *);
	void mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
 	virtual class FSvgRenderer * setUpConnectors(class ModelPart *, ItemBase::ViewIdentifier);
	void collectChained(ConnectorItem * connectorItem, QList<Wire *> & chained, QList<ConnectorItem *> & ends);
	void setConnector0Rect();
	void setConnector1Rect();
	void collectWiresAux(QList<Wire *> & wires, ConnectorItem * start);
	void setShadowColor(QColor &);
	bool connectsWithin(ConnectorItem * connectorItem, QSet<ItemBase *> & in, QList<Wire *> & wires);
	void calcNewLine(ConnectorItem * from, ConnectorItem * to, QPointF & p1, QPointF & p2);

protected:
	static void makeHues(int hue1, int hue2, int maxCount, int currentCount, QList<QColor *> & hues);
	static void makeHue(int hue, QList<QColor *> & hues, int currentCount);


protected:
	QPointF m_wireDragOrigin;
	bool m_dragEnd;
	bool m_drag0;
	class ConnectorItem *m_connectorHover;
	class ConnectorItem *m_connector0;
	class ConnectorItem *m_connector1;
	bool m_grabbedMouse;
	QString m_colorName;
	bool m_autoroutable;
	QPen m_shadowPen;
	qreal m_opacity;
	qint64 m_chainedID;

public:
	static QList<QString> colorNames;
	static QHash<QString, QString> colorTrans;

protected:
	static QHash<QString, QString> shadowColors;
	static QHash<QString, QString> colors;

signals:
	void wireChangedSignal(Wire* me, QLineF oldLine, QLineF newLine, QPointF oldPos, QPointF newPos, ConnectorItem * from, ConnectorItem * to);
	void wireSplitSignal(Wire* me, QPointF newPos, QPointF oldPos, QLineF oldLine);
	void wireJoinSignal(Wire* me, ConnectorItem * clickedConnectorItem);
};

#endif
