/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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
	bool canDeleteItem(QGraphicsItem * item, int count);
	bool canCopyItem(QGraphicsItem * item, int count);
	void createTrace(Wire *);
	void excludeFromAutoroute(bool exclude);
	void selectAllExcludedTraces();
	void updateRoutingStatus(CleanUpWiresCommand*, RoutingStatus &, bool manual);
	bool hasAnyNets();
	void forwardRoutingStatus(const RoutingStatus &);
	void addDefaultParts();
	void showEvent(QShowEvent * event);
	void initWire(Wire *, int penWidth);
	virtual bool autorouteTypePCB();
	virtual double getKeepout();
	virtual const QString & traceColor(ConnectorItem *);
	virtual const QString & traceColor(ViewLayer::ViewLayerSpec);
	virtual void ensureTraceLayersVisible();
	virtual void ensureTraceLayerVisible();
	bool canChainMultiple();
	void setNewPartVisible(ItemBase *);
	virtual bool usesJumperItem();
	void setClipEnds(class ClipableWire *, bool);
	void showGroundTraces(bool show);
        virtual double getLabelFontSizeTiny();
	virtual double getLabelFontSizeSmall();
	virtual double getLabelFontSizeMedium();
	virtual double getLabelFontSizeLarge();
	ViewLayer::ViewLayerID getWireViewLayerID(const ViewGeometry & viewGeometry, ViewLayer::ViewLayerSpec);
	ItemBase * findBoard();
	double getRatsnestOpacity(Wire *);
	virtual double getRatsnestOpacity(bool);
	void updateRoutingStatus(RoutingStatus &, bool manual);
	void setBoardLayers(int, bool redraw);
	long setUpSwap(ItemBase *, long newModelIndex, const QString & newModuleID, ViewLayer::ViewLayerSpec, bool doEmit, bool noFinalChangeWiresCommand, QUndoCommand * parentCommand);
	void loadFromModelParts(QList<ModelPart *> & modelParts, BaseCommand::CrossViewType, QUndoCommand * parentCommand, 
							bool offsetPaste, const QRectF * boundingRect, bool seekOutsideConnections, QList<long> & newIDs);
	virtual bool isInLayers(ConnectorItem *, ViewLayer::ViewLayerSpec);
	bool routeBothSides();
	virtual bool sameElectricalLayer(ViewLayer::ViewLayerID, ViewLayer::ViewLayerID);
	virtual bool sameElectricalLayer2(ViewLayer::ViewLayerID, ViewLayer::ViewLayerID);
	void changeTraceLayer();
	void changeLayer(long id, double z, ViewLayer::ViewLayerID viewLayerID);
	void deleteSelected(Wire *);
	void jumperItemHack();
	VirtualWire * makeOneRatsnestWire(ConnectorItem * source, ConnectorItem * dest, bool routed, QColor color);
	void getRatsnestColor(QColor &);
	void updateNet(Wire*);
	bool acceptsTrace(const ViewGeometry & viewGeometry);
	ItemBase * placePartDroppedInOtherView(ModelPart *, ViewLayer::ViewLayerSpec, const ViewGeometry & viewGeometry, long id, SketchWidget * dropOrigin);
	void autorouterSettings();
	void getViaSize(double & ringThickness, double & holeSize);
    void deleteItem(ItemBase *, bool deleteModelPart, bool doEmit, bool later);
	virtual double getTraceWidth();
	virtual double getAutorouterTraceWidth();
	void getBendpointWidths(class Wire *, double w, double & w1, double & w2, bool & negativeOffsetRect);
	double getSmallerTraceWidth(double minDim);
	bool groundFill(QUndoCommand * parentCommand);
	QString generateCopperFillUnit(ItemBase * itemBase, QPointF whereToStart);
	double getWireStrokeWidth(Wire *, double wireWidth);

public:
	static QSizeF jumperItemSize();
	static void getDefaultViaSize(QString & ringThickness, QString & holeSize);

public slots:
	void resizeBoard(double w, double h, bool doEmit);
	void showLabelFirstTime(long itemID, bool show, bool doEmit);
	void changeBoardLayers(int layers, bool doEmit);


public:
	enum CleanType {
		noClean,
		ninetyClean
	};

	CleanType cleanType();

protected:
	void setWireVisible(Wire * wire);
	// void checkAutorouted();
	ViewLayer::ViewLayerID multiLayerGetViewLayerID(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier, ViewLayer::ViewLayerSpec, QDomElement & layers, QString & layerName);
	bool canChainWire(Wire *);
	bool canDragWire(Wire * wire);
	void createTrace(Wire * fromWire, const QString & commandString, ViewGeometry::WireFlag);
	bool createOneTrace(Wire * wire, ViewGeometry::WireFlag flag, bool allowAny, QList<Wire *> & done, QUndoCommand * parentCommand);
	const QString & hoverEnterPartConnectorMessage(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	bool modifyNewWireConnections(Wire * dragWire, ConnectorItem * fromOnWire, ConnectorItem * from, ConnectorItem * to, QUndoCommand * parentCommand);
	ViewLayer::ViewLayerID getDragWireViewLayerID(ConnectorItem *);
	bool canDropModelPart(ModelPart * modelPart);
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
	void getLabelFont(QFont &, QColor &, ViewLayer::ViewLayerSpec);
	void connectSymbolPrep(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, ConnectorItem * & target1, ConnectorItem * & target2);
	void connectSymbols(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, QUndoCommand * parentCommand);
	void makeWiresChangeConnectionCommands(const QList<Wire *> & wires, QUndoCommand * parentCommand);
	double defaultGridSizeInches();
	ViewLayer::ViewLayerID getLabelViewLayerID(ViewLayer::ViewLayerSpec);
	ViewLayer::ViewLayerSpec wireViewLayerSpec(ConnectorItem *);
	int isBoardLayerChange(ItemBase * itemBase, const QString & newModuleID, bool master);
	void removeWire(Wire * w, QList<ConnectorItem *> & ends, QList<Wire *> & done, QUndoCommand * parentCommand);
	bool resizingJumperItemPress(QGraphicsItem * item);
	bool resizingJumperItemRelease();
	bool resizingBoardPress(QGraphicsItem * item);
	bool resizingBoardRelease();
	void resizeBoard();
	void resizeJumperItem();
	void dragWireChanged(class Wire* wire, ConnectorItem * from, ConnectorItem * to);
	bool checkUpdateRatsnest(QList<ConnectorItem *> & connectorItems);
	QPoint calcFixedToCenterItemOffset(const QRect & viewPortRect, const QSizeF & helpSize);
	void dealWithDefaultParts();
	void changeTrace(Wire * wire, ConnectorItem * from, ConnectorItem * to, QUndoCommand * parentCommand);
	void checkDeleteTrace(CleanUpWiresCommand* command);
	void clearSmdTraces(QList<ItemBase *> & smds, 	QList<Wire *> & already, QUndoCommand * parentCommand);
	bool connectorItemHasSpec(ConnectorItem * connectorItem, ViewLayer::ViewLayerSpec spec);
	ViewLayer::ViewLayerSpec createWireViewLayerSpec(ConnectorItem * from, ConnectorItem * to);
	Wire * createTempWireForDragging(Wire * fromWire, ModelPart * wireModel, ConnectorItem * connectorItem, ViewGeometry & viewGeometry, ViewLayer::ViewLayerSpec);
	void prereleaseTempWireForDragging(Wire*);

signals:
	void subSwapSignal(SketchWidget *, ItemBase *, ViewLayer::ViewLayerSpec, QUndoCommand * parentCommand);
	void updateLayerMenuSignal();
	void boardDeletedSignal();

protected:
	static void calcDistances(Wire * wire, QList<ConnectorItem *> & ends);
	static void clearDistances();
	static int calcDistance(Wire * wire, ConnectorItem * end, int distance, QList<Wire *> & distanceWires, bool & fromConnector0);
	static int calcDistanceAux(ConnectorItem * from, ConnectorItem * to, int distance, QList<Wire *> & distanceWires);

protected slots:
	void alignJumperItem(class JumperItem *, QPointF &);
	void wire_wireSplit(class Wire*, QPointF newPos, QPointF oldPos, QLineF oldLine);

protected:
	RoutingStatus m_routingStatus;
	CleanType m_cleanType;
	QPointF m_jumperDragOffset;
	QPointer<class JumperItem> m_resizingJumperItem;
	QPointer<class ResizableBoard> m_resizingBoard;

protected:
	static QSizeF m_jumperItemSize;
	static const char * FakeTraceProperty;
};

#endif
