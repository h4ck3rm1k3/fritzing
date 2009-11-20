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

You should have received a copy of the GNU General Public Licensetriple
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************

$Revision$:
$Author$:
$Date$

********************************************************************/

#ifndef ITEMBASE_H
#define ITEMBASE_H

#include <QXmlStreamWriter>
#include <QPointF>
#include <QSize>
#include <QHash>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsItem>
#include <QPointer>
#include <QUrl>
#include <QMap>

#include "../viewgeometry.h"
#include "../viewlayer.h"
#include "../viewidentifierclass.h"
#include "../utils/misc.h"
#include "graphicssvglineitem.h"

class ConnectorItem;

typedef QMultiHash<ConnectorItem *, ConnectorItem *> ConnectorPairHash;

class ItemBase : public GraphicsSvgLineItem
{
Q_OBJECT

public:
	static const QString ITEMBASE_FONT_PREFIX;
	static const QString ITEMBASE_FONT_SUFFIX;
	static QHash<QString, QString> TranslatedPropertyNames;

public:
	static void initNames();
	static void cleanup();
	static ItemBase * extractTopLevelItemBase(QGraphicsItem * thing);
	static QString partInstanceDefaultTitle;
	static QString moduleInstanceDefaultTitle;
	static QList<ItemBase *> emptyList;
	static QString translatePropertyName(const QString & key);
	static void setReferenceModel(class ReferenceModel *);

public:
	ItemBase(class ModelPart*, ViewIdentifierClass::ViewIdentifier, const ViewGeometry &, long id, QMenu * itemMenu);
	virtual ~ItemBase();

	qreal z();
	virtual void saveGeometry() = 0;
	ViewGeometry & getViewGeometry();
	virtual bool itemMoved() = 0;
	QSizeF size();
	class ModelPart * modelPart();
	void setModelPart(class ModelPart *);
	class ModelPartShared * modelPartShared();
	virtual void writeXml(QXmlStreamWriter &) {}
	virtual void saveInstance(QXmlStreamWriter &);
	virtual void saveInstanceLocation(QXmlStreamWriter &) = 0;
	virtual void writeGeometry(QXmlStreamWriter &);
	virtual void moveItem(ViewGeometry &) = 0;
	virtual void setItemPos(QPointF & pos);
	virtual void rotateItem(qreal degrees);
	virtual void flipItem(Qt::Orientations orientation);
	void transformItem(const QTransform &);
	virtual void transformItem2(const QMatrix &);
	virtual void removeLayerKin();
	ViewIdentifierClass::ViewIdentifier viewIdentifier();
	QString & viewIdentifierName();
	ViewLayer::ViewLayerID viewLayerID();
	void setViewLayerID(ViewLayer::ViewLayerID, const LayerHash & viewLayers);
	void setViewLayerID(const QString & layerName, const LayerHash & viewLayers);
	bool topLevel();
	void collectConnectors(ConnectorPairHash & connectorHash, QGraphicsScene * scene);
	virtual void collectConnectors(QList<ConnectorItem *> & connectors);

	virtual void busConnectorItems(class Bus * bus, QList<ConnectorItem *> & items);
	virtual void setHidden(bool hidden);
	bool hidden();
	ConnectorItem * findConnectorItemNamed(const QString & connectorID);
	void updateConnections(ConnectorItem *);
	virtual void updateConnections();
	virtual const QString & title();
	bool getVirtual();
	const QHash<QString, class Bus *> & buses();
	void addBusConnectorItem(class Bus *, ConnectorItem *);
	void clearBusConnectorItems();
	int itemType() const;					// wanted this to return ModelPart::ItemType but couldn't figure out how to get it to compile
	virtual bool sticky();
	virtual void setSticky(bool);
	virtual void addSticky(ItemBase *, bool stickem);
	virtual ItemBase * stuckTo();
	virtual QList<ItemBase *> & stickyList();
	virtual bool alreadySticking(ItemBase * itemBase);
	virtual bool stickyEnabled(ItemBase * stickTo);
	ConnectorItem * anyConnectorItem();
	bool isConnectedTo(ItemBase * other);
	virtual QString instanceTitle();
	QString label();
	virtual void updateTooltip();
	void setTooltip();
	void setConnectorTooltips();
	void removeTooltip();
	virtual bool hasConnectors();
	bool hasConnections();
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
	virtual QString toolTip2();
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void collectWireConnectees(QSet<class Wire *> & wires);
	virtual void collectFemaleConnectees(QSet<ItemBase *> & items);
	void prepareGeometryChange();
	virtual void resetID();
	void updateConnectionsAux();
	void updateExternalConnections();
	virtual void blockSyncKinMoved(bool block);
	virtual ItemBase * lowerConnectorLayerVisible(ItemBase *);
	virtual void hoverEnterEvent( QGraphicsSceneHoverEvent * event );
	virtual void hoverLeaveEvent( QGraphicsSceneHoverEvent * event );
	void hoverMoveEvent( QGraphicsSceneHoverEvent * event );
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
	virtual void figureHover();
	virtual QString retrieveSvg(ViewLayer::ViewLayerID, QHash<QString, class SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi);
	virtual void slamZ(qreal newZ);
	bool isEverVisible();
	void setEverVisible(bool);
	virtual bool connectionIsAllowed(ConnectorItem *);
	virtual bool collectExtraInfoHtml(const QString & family, const QString & prop, const QString & value, bool collectValues, QString & returnProp, QString & returnValue);
	virtual QString getProperty(const QString & key);
	ConnectorItem * rightClickedConnector();
	virtual bool canEditPart();
	virtual bool hasCustomSVG();
	virtual void setProp(const QString & prop, const QString & value);
	bool isObsolete();
	virtual QObject * createPlugin(QWidget * parent, const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);
	void prepareProps();

public:
	virtual void getConnectedColor(ConnectorItem *, QBrush * &, QPen * &, qreal & opacity, qreal & negativePenWidth);
	virtual void getUnconnectedColor(ConnectorItem *, QBrush * &, QPen * &, qreal & opacity, qreal & negativePenWidth);
	virtual void getNormalColor(ConnectorItem *, QBrush * &, QPen * &, qreal & opacity, qreal & negativePenWidth);
	virtual void getChosenColor(ConnectorItem *, QBrush * &, QPen * &, qreal & opacity, qreal & negativePenWidth);
	virtual void getHoverColor(ConnectorItem *, QBrush * &, QPen * &, qreal & opacity, qreal & negativePenWidth);
	virtual void getEqualPotentialColor(ConnectorItem *, QBrush * &, QPen * &, qreal & opacity, qreal & negativePenWidth);

protected:
	static QPen normalPen;
	static QPen hoverPen;
	static QPen connectedPen;
	static QPen unconnectedPen;
	static QPen chosenPen;
	static QPen equalPotentialPen;
	static QBrush hoverBrush;
	static QBrush normalBrush;
	static QBrush connectedBrush;
	static QBrush unconnectedBrush;
	static QBrush chosenBrush;
	static QBrush equalPotentialBrush;
	static const qreal normalConnectorOpacity;

public:
	static QColor connectedColor();
	static QColor unconnectedColor();
	static QColor standardConnectedColor();
	static QColor standardUnconnectedColor();
	static void setConnectedColor(QColor &);
	static void setUnconnectedColor(QColor &);

public:
	virtual void hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	virtual void hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	virtual void hoverMoveConnectorItem(QGraphicsSceneHoverEvent * event, class ConnectorItem * item);
	void hoverEnterConnectorItem();
	void hoverLeaveConnectorItem();
	virtual void connectorHover(ConnectorItem *, ItemBase *, bool hovering);
	void clearConnectorHover();
	virtual bool filterMousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	virtual void mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	virtual void mouseDoubleClickConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	virtual void mouseMoveConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	virtual void mouseReleaseConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	virtual bool acceptsMousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	virtual bool acceptsMouseDoubleClickConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	virtual bool acceptsMouseMoveConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	virtual bool acceptsMouseReleaseConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
	virtual void connectionChange(ConnectorItem * onMe, ConnectorItem * onIt, bool connect);
	virtual void connectedMoved(ConnectorItem * from, ConnectorItem * to);
	virtual ItemBase * layerKinChief();
	virtual const QList<ItemBase *> & layerKin();
	virtual void findConnectorsUnder() = 0;
	virtual ConnectorItem* newConnectorItem(class Connector *connector);
	virtual void setInstanceTitle(const QString &title);

public slots:
	void showPartLabel(bool show, ViewLayer *);
	void partLabelChanged(const QString &newText);
	qint64 id();
	void swapEntry(const QString & text);

public:
	static bool zLessThan(ItemBase * & p1, ItemBase * & p2);
	static qint64 getNextID();
	static qint64 getNextID(qint64 fromIndex);
	static class FSvgRenderer * setUpImage(class ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier, ViewLayer::ViewLayerID, class LayerAttributes &);

protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event );
	ConnectorItem * findConnectorUnder(ConnectorItem* , ConnectorItem * lastUnderConnector, bool useTerminalPoint, bool allowAlready, const QList<ConnectorItem *> & exclude);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	virtual void paintHover(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget); 
	QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant & value);

	virtual QStringList collectValues(const QString & family, const QString & prop);

	void setInstanceTitleTooltip(const QString& text);
	virtual void setDefaultTooltip();
	void setInstanceTitleAux(const QString & title);
	void ensureUniqueTitle(QString &title);
	int getNextTitle(QList<QGraphicsItem*> & items, const QString &title);
	void saveLocAndTransform(QXmlStreamWriter & streamWriter);

protected:
 	QSizeF m_size;
	qint64 m_id;
	ViewGeometry m_viewGeometry;
	QPointer<ModelPart> m_modelPart;
	ViewIdentifierClass::ViewIdentifier m_viewIdentifier;
	ViewLayer::ViewLayerID m_viewLayerID;
	int m_connectorHoverCount;
	int m_connectorHoverCount2;
	int m_hoverCount;
	bool m_hidden;
	QHash<class Bus *, QList <ConnectorItem *> * > m_busConnectorItems;
	bool m_sticky;
	QList<ItemBase *> m_stickyList;
	QMenu *m_itemMenu;
	bool m_canFlipHorizontal;
	bool m_canFlipVertical;
	bool m_zUninitialized;
	QPointer<class PartLabel> m_partLabel;
	bool m_spaceBarWasPressed;
	bool m_hoverEnterSpaceBarWasPressed;
	bool m_everVisible;
	ConnectorItem * m_rightClickedConnector;
	QMap<QString, QString> m_propsMap;

protected:
	static long nextID;
	const static QColor hoverColor;
	const static qreal hoverOpacity;
	const static QColor connectorHoverColor;
	const static qreal connectorHoverOpacity;
	static QString SvgFilesDir;
	static QPointer<class ReferenceModel> referenceModel;

};
#endif
