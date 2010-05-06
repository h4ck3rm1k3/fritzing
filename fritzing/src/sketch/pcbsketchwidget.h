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

#ifndef PCBSKETCHWIDGET_H
#define PCBSKETCHWIDGET_H

#include "sketchwidget.h"
#include <QVector>

class PCBSketchWidget : public SketchWidget
{
	Q_OBJECT

public:
    PCBSketchWidget(ViewIdentifierClass::ViewIdentifier, QWidget *parent=0);

	void addViewLayers();
	bool canDeleteItem(QGraphicsItem * item);
	bool canCopyItem(QGraphicsItem * item);
	void createJumper();
	void createTrace();
	void excludeFromAutoroute(bool exclude);
	bool ratsAllRouted();
	void selectAllExcludedTraces();
	void makeChangeRoutedCommand(Wire * wire, bool routed, qreal opacity, QUndoCommand * parentCommand);
	void clearRouting(QUndoCommand * parentCommand);
	void updateRatsnestStatus(CleanUpWiresCommand*, QUndoCommand *, RoutingStatus &);
	void forwardRoutingStatus(const RoutingStatus &);
	void addBoard();
	void setCurrent(bool current);
	void initWire(Wire *, int penWidth);
	virtual bool autorouteNeedsBounds();
	virtual bool autorouteCheckWires();
	virtual bool autorouteCheckConnectors();
	virtual bool autorouteCheckParts();
	const QString & traceColor();
	const QString & jumperColor();
	qreal jumperWidth();
	virtual void ensureTraceLayersVisible();
	virtual void ensureTraceLayerVisible();
	virtual void ensureJumperLayerVisible();
	bool canChainMultiple();
	void setNewPartVisible(ItemBase *);
	virtual void setJumperFlags(ViewGeometry & vg);
	virtual bool usesJumperItem();
	void setClipEnds(class ClipableWire *, bool);
	void showGroundTraces(bool show);
	virtual qreal getLabelFontSizeSmall();
	virtual qreal getLabelFontSizeMedium();
	virtual qreal getLabelFontSizeLarge();
	ViewLayer::ViewLayerID getWireViewLayerID(const ViewGeometry & viewGeometry, const LayerList & notLayers);
	ItemBase * findBoard();
	qreal getRatsnestOpacity(Wire *);
	virtual qreal getRatsnestOpacity(bool);
	void updateRatsnestColors(BaseCommand * command, QUndoCommand * parentCommand, bool forceUpdate, RoutingStatus &);
	void designRulesCheck();

public slots:
	void resizeBoard(qreal w, qreal h, bool doEmit);
	void showLabelFirstTime(long itemID, bool show, bool doEmit);


public:
	enum CleanType {
		noClean,
		ninetyClean
	};

	CleanType cleanType();

protected:
	void setWireVisible(Wire * wire);
	void makeWires(QList<ConnectorItem *> & partsConnectorItems, QList <Wire *> & ratsnestWires, Wire * & modelWire, RatsnestCommand *);
	// void checkAutorouted();
	ViewLayer::ViewLayerID multiLayerGetViewLayerID(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier, const LayerList & notLayers, QDomElement & layers, QString & layerName);
	bool canChainWire(Wire *);
	void createJumperOrTrace(const QString & commandString, ViewGeometry::WireFlag, const QString & colorString);
	void createOneJumperOrTrace(Wire * wire, ViewGeometry::WireFlag flag, bool allowAny, QList<Wire *> & done, 
								QUndoCommand * & parentCommand, const QString & commandString, const QString & colorString);
	const QString & hoverEnterPartConnectorMessage(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	bool modifyNewWireConnections(Wire * dragWire, ConnectorItem * fromOnWire, ConnectorItem * from, ConnectorItem * to, QUndoCommand * parentCommand);
	ViewLayer::ViewLayerID getDragWireViewLayerID(ConnectorItem *);
	void dealWithRatsnest(long fromID, const QString & fromConnectorID, 
								  long toID, const QString & toConnectorID,
								  bool connect, class RatsnestCommand *, bool doEmit);
	bool dealWithRatsnestAux(ConnectorItem * & from, ConnectorItem * & to,
							long fromID, const QString & fromConnectorID, 
							long toID, const QString & toConnectorID,
							bool connect, class RatsnestCommand *, bool doEmit);
	bool canDropModelPart(ModelPart * modelPart);
	virtual void removeRatsnestWires(QList< QList<ConnectorItem *>* > & allPartConnectorItems, CleanUpWiresCommand *);
	bool reviewDeletedConnections(QSet<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash * > & deletedConnections, QUndoCommand * parentCommand);
	bool alreadyRatsnest(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem);
	bool canCreateWire(Wire * dragWire, ConnectorItem * from, ConnectorItem * to);
	bool bothEndsConnected(Wire * wire, ViewGeometry::WireFlags, ConnectorItem * oneEnd, QList<Wire *> & wires, QList<ConnectorItem *> & partConnectorItems);
	bool doRatsnestOnCopy();
	void makeRatsnestViewGeometry(ViewGeometry & viewGeometry, ConnectorItem * source, ConnectorItem * dest); 
	ConnectorItem * lookForBreadboardConnection(ConnectorItem * connectorItem);
	ConnectorItem * findEmptyBusConnectorItem(ConnectorItem * busConnectorItem);
	long makeModifiedWire(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, BaseCommand::CrossViewType, ViewGeometry::WireFlags, QUndoCommand * parentCommand);
	void modifyNewWireConnectionsAux(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, QUndoCommand * parentCommand);
	void makeTwoWires(ConnectorItem * originalFromConnectorItem, ConnectorItem * fromConnectorItem,
						ConnectorItem * originalToConnectorItem, ConnectorItem * toConnectorItem, 
						QUndoCommand * parentCommand); 
	ConnectorItem * lookForNewBreadboardConnection(ConnectorItem * connectorItem, ItemBase * & newBreadboard);
	ConnectorItem * findNearestPartConnectorItem(ConnectorItem * fromConnectorItem);
	ConnectorItem * findEmptyBus(ItemBase * breadboard);
	bool bothEndsConnectedAux(Wire * wire, ViewGeometry::WireFlags flag, ConnectorItem * oneEnd, QList<Wire *> & wires, QList<ConnectorItem *> & partConnectorItems, QList<Wire *> & visited);
	void getLabelFont(QFont &, QColor &, const LayerList & notLayers);
	void connectSymbolPrep(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, ConnectorItem * & target1, ConnectorItem * & target2);
	void connectSymbols(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, QUndoCommand * parentCommand);
	void makeWiresChangeConnectionCommands(const QList<Wire *> & wires, QUndoCommand * parentCommand);
	Wire * makeOneRatsnestWire(ConnectorItem * source, ConnectorItem * dest, RatsnestCommand *, bool select);
	void collectConnectorNames(QList<ConnectorItem *> & connectorItems, QStringList & connectorNames);
	void recolor(QList<ConnectorItem *> & connectorItems, BaseCommand * command, QUndoCommand * parentCommand, bool forceUpdate); 
	void scoreOneNet(QList<ConnectorItem *> & connectorItems, RoutingStatus &);
	double defaultGridSizeInches();
	bool canAlignToTopLeft(ItemBase *);
	ViewLayer::ViewLayerID getLabelViewLayerID(const LayerList & notLayers);
	void setDRCVisibility(QGraphicsItem * item, QList<ConnectorItem *> & equipotentialConnectorItems, QHash<QGraphicsItem *, bool> & visibility);

signals:
	void setMaximumDRCProgress(int);
	void setDRCProgressValue(int);

protected:
	static void calcDistances(Wire * wire, QList<ConnectorItem *> & ends);
	static void clearDistances();
	static int calcDistance(Wire * wire, ConnectorItem * end, int distance, QList<Wire *> & distanceWires, bool & fromConnector0);
	static int calcDistanceAux(ConnectorItem * from, ConnectorItem * to, int distance, QList<Wire *> & distanceWires);
	static void transitiveClosure(QVector< QVector<bool> > & adjacency, int count);
	static int countMissing(QVector< QVector<bool> > & adjacency, int count);

protected slots:
	void cancelDRC();
	void stopDRC();

protected:
	RoutingStatus m_routingStatus;
	bool m_addBoard;
	QPointer<ItemBase> m_addedBoard;
	QString m_jumperColor;
	qreal m_jumperWidth;
	QString m_traceColor;
	CleanType m_cleanType;
	bool m_cancelDRC;

};

#endif
