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



#ifndef ITEMBASE_H
#define ITEMBASE_H


#include <QHash>
#include <QXmlStreamWriter>
#include <QPointF>
#include <QSize>
#include <QHash>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsItem>

#include "viewgeometry.h"
#include "viewlayer.h"
#include "misc.h"
#include "graphicssvglineitem.h"
#include "partinstancestuff.h"

typedef QMultiHash<class ConnectorItem *, class ConnectorItem *> ConnectorPairHash;

class ItemBase : public GraphicsSvgLineItem
{
Q_OBJECT

public:
   enum ViewIdentifier {
    	IconView,
    	BreadboardView,
    	SchematicView,
    	PCBView,
    	AllViews,
    	ViewCount
   	};

	static QString rulerModuleIDName;
	static QString breadboardModuleIDName;
	static QString tinyBreadboardModuleIDName;
	static QString & viewIdentifierName(ViewIdentifier);
	static QString & viewIdentifierXmlName(ViewIdentifier);
	static QString & viewIdentifierNaturalName(ViewIdentifier);
	static void initNames();
	static ItemBase * extractTopLevelItemBase(QGraphicsItem * thing);
	static ItemBase * extractItemBase(QGraphicsItem * item);
	static ViewLayer::ViewLayerID defaultConnectorLayer(ItemBase::ViewIdentifier viewId);
	static QString partInstanceDefaultTitle;


public:
	ItemBase(class ModelPart*, ItemBase::ViewIdentifier, const ViewGeometry &, long id, bool topLevel, QMenu * itemMenu);
	virtual ~ItemBase();

	qreal z();
	virtual void saveGeometry() = 0;
	ViewGeometry & getViewGeometry();
	virtual bool itemMoved() = 0;
	QSizeF size();
	class ModelPart * modelPart();
	void setModelPart(class ModelPart *);
	class ModelPartStuff * modelPartStuff();
	virtual void writeXml(QXmlStreamWriter &) {}
	virtual void saveInstance(QXmlStreamWriter &);
	virtual void saveInstanceLocation(QXmlStreamWriter &) = 0;
	virtual void writeGeometry(QXmlStreamWriter &);
	virtual void moveItem(ViewGeometry &) = 0;
	virtual void setItemPos(QPointF & pos);
	virtual void rotateItem(qreal degrees) = 0;
	virtual void removeLayerKin();
	ItemBase::ViewIdentifier viewIdentifier();
	QString & viewIdentifierName();
	ViewLayer::ViewLayerID viewLayerID();
	void setViewLayerID(ViewLayer::ViewLayerID, const LayerHash & viewLayers);
	void setViewLayerID(const QString & layerName, const LayerHash & viewLayers);
	bool topLevel();
	void collectConnectors(ConnectorPairHash & connectorHash, QGraphicsScene * scene);
	void busConnectorItems(class Bus * bus, QList<class ConnectorItem *> & items);
	virtual void setHidden(bool hidden);
	bool hidden();
	ConnectorItem * findConnectorItemNamed(const QString & connectorID);
	virtual void updateConnections(ConnectorItem *);
	virtual void updateConnections();
	const QString & title();
	bool getVirtual();
	const QHash<QString, class Bus *> & buses();
	void addBusConnectorItem(class Bus *, class ConnectorItem *);
	void clearBusConnectorItems();
	int itemType();					// wanted this to return ModelPart::ItemType but couldn't figure out how to get it to compile
	bool sticky();
	void addSticky(ItemBase *, bool stickem);
	ItemBase * stuckTo();
	QHash<ItemBase *, QPointF> & sticking();
	bool alreadySticking(ItemBase * itemBase);
	ConnectorItem * anyConnectorItem();
	bool isConnectedTo(ItemBase * other);
	QString instanceTitle();
	QString label();
	virtual void updateTooltip();
	void setTooltip();
	void setConnectorTooltips();
	void removeTooltip();
	bool hasConnectors();
	bool canFlipHorizontal();
	void setCanFlipHorizontal(bool);
	bool canFlipVertical();
	void setCanFlipVertical(bool);
	virtual void clearModelPart();
	void clearPartLabel();
	bool isPartLabelVisible();
	void restorePartLabel(QDomElement & labelGeometry, ViewLayer::ViewLayerID);				// on loading from a file
	void movePartLabel(QPointF newPos, QPointF newOffset);												// coming down from the command object
	void partLabelMoved(QPointF oldPos, QPointF oldOffset, QPointF newPos, QPointF newOffset);			// coming up from the label
	void rotateFlipPartLabel(qreal degrees, Qt::Orientations);				// coming up from the label
	void doRotateFlipPartLabel(qreal degrees, Qt::Orientations);			// coming down from the command object
	bool isSwappable();

public:
	virtual bool stickyEnabled(ItemBase * stickTo);
	virtual void hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, class ConnectorItem * item);
	virtual void hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, class ConnectorItem * item);
	virtual void connectorHover(class ConnectorItem *, ItemBase *, bool hovering);
	void clearConnectorHover();
	virtual void mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	virtual bool acceptsMousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	virtual void connectionChange(ConnectorItem *);
	virtual void connectedMoved(ConnectorItem * from, ConnectorItem * to);
	virtual ItemBase * layerKinChief() = 0;
	virtual void findConnectorsUnder() = 0;
	virtual ConnectorItem* newConnectorItem(class Connector *connector);
	virtual void setInstanceTitle(const QString &title);

public slots:
	void showPartLabel(bool show, ViewLayer *, const QColor & textColor);
	void partLabelChanged(const QString &newText);
	qint64 id();

signals:
	void posChangedSignal();

public:
	static bool zLessThan(ItemBase * & p1, ItemBase * & p2);
	static qint64 getNextID();
	static qint64 getNextID(qint64 fromIndex);

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event );
	virtual void hoverEnterEvent( QGraphicsSceneHoverEvent * event );
	virtual void hoverLeaveEvent( QGraphicsSceneHoverEvent * event );
	void hoverMoveEvent( QGraphicsSceneHoverEvent * event );
	ConnectorItem * findConnectorUnder(ConnectorItem* , ConnectorItem * lastUnderConnector, bool useTerminalPoint);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	virtual void paintHover(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget); 
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
	QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant & value);


	void setInstanceTitleTooltip(const QString& text);
	void setDefaultTooltip();
	void setInstanceTitleAux(const QString & title);
	void ensureUniqueTitle(QString &title);
	int getNextTitle(QList<QGraphicsItem*> & items, const QString &title);


protected:
 	QSizeF m_size;
	qint64 m_id;
	ViewGeometry m_viewGeometry;
	class ModelPart* m_modelPart;
	ViewIdentifier m_viewIdentifier;
	ViewLayer::ViewLayerID m_viewLayerID;
	int m_connectorHoverCount;
	int m_connectorHoverCount2;
	int m_hoverCount;
	bool m_topLevel;
	bool m_hidden;
	QHash<class Bus *, QList <ConnectorItem *> * > m_busConnectorItems;
	bool m_sticky;
	QHash<ItemBase *, QPointF> m_stickyList;
	QMenu *m_itemMenu;
	bool m_canFlipHorizontal;
	bool m_canFlipVertical;
	bool m_zUninitialized;
	class PartLabel * m_partLabel;

protected:
	static long nextID;
	static QHash <ViewIdentifier, StringTriple * > names;
	const static QColor hoverColor;
	const static qreal hoverOpacity;
	const static QColor connectorHoverColor;
	const static qreal connectorHoverOpacity;

};
#endif
