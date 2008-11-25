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



#ifndef SKETCHWIDGET_H
#define SKETCHWIDGET_H

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QUndoStack>
#include <QRubberBand>
#include <QGraphicsEllipseItem>
#include <QSet>
#include <QHash>
#include <QTimer>

#include "paletteitem.h"
#include "palettemodel.h"
#include "sketchmodel.h"
#include "viewgeometry.h"
#include "infographicsview.h"
#include "viewlayer.h"
#include "misc.h"
#include "commands.h"

QT_BEGIN_NAMESPACE
class QDragEnterEvent;
class QDropEvent;
QT_END_NAMESPACE

struct ItemCount {
	int selCount;
	int itemsCount;
	int selRotatable;
	int selHFlipable;
	int selVFlipable;
};

struct BusMergeThing {
	class BusConnectorItem * bci;
	QString busID;
	ItemBase * itemBase;
};

class SketchWidget : public InfoGraphicsView
{
	Q_OBJECT

public:
    SketchWidget(ItemBase::ViewIdentifier, QWidget *parent=0, int size=500, int minSize=100);

    QUndoStack* undoStack();
    ItemBase * addItem(ModelPart *, BaseCommand::CrossViewType, const ViewGeometry &, long id, PaletteItem* item=0);
	ItemBase * addItem(const QString & moduleID, BaseCommand::CrossViewType, const ViewGeometry &, long id);
    void deleteItem(long id, bool deleteModelPart, bool doEmit);
    void deleteItem(ItemBase *, bool deleteModelPart, bool doEmit);
    void moveItem(long id, ViewGeometry &);
    void rotateItem(long id, qreal degrees);
    void flipItem(long id, Qt::Orientations orientation);
    void selectItem(long id, bool state, bool updateInfoView=true);
    void selectDeselectAllCommand(bool state);
    void changeWire(long fromID, QLineF line, QPointF pos, bool useLine);   //  const QString & fromConnectorID, long toID, const QString & toConnectorID, bool dragend
    void cut();
    void copy();
    void paste();
    void duplicate();
    void setPaletteModel(PaletteModel *);
    void setRefModel(ReferenceModel *refModel);
    void setSketchModel(SketchModel *);
    void setUndoStack(class WaitPushUndoStack *);
    void clearSelection();
    void loadFromModel();
    ItemBase* loadFromModel(ModelPart *, const ViewGeometry&);
    void changeZ(QHash<long, RealPair * >, qreal (*pairAccessor)(RealPair *) );
    void relativeZoom(qreal step);
    void absoluteZoom(qreal percent);
	void sendToBack();
	void sendBackward();
	void bringForward();
	void bringToFront();
	void fitInWindow();
	void rotateX(qreal degrees);
	void flip(Qt::Orientations orientation);
	void group();
	void deleteItem();
	PaletteItem *getSelectedPart();

    void addViewLayer(ViewLayer *);
    void updateLayerMenu(QMenu * layerMenu, QAction * showAll, QAction * hideAll);
    void setAllLayersVisible(bool visible);
    void setLayerVisible(ViewLayer * viewLayer, bool visible);
	void setLayerVisible(ViewLayer::ViewLayerID viewLayerID, bool visible);
    bool layerIsVisible(ViewLayer::ViewLayerID);
	void sortSelectedByZ(QList<ItemBase *> & bases);
	void sortAnyByZ(const QList<QGraphicsItem *> & items, QList<ItemBase *> & bases);
 	void mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *);
 	void setBackground(QColor);
 	const QColor& background();
 	void setItemMenu(QMenu*);
	void changeConnection(long fromID,
						  const QString & fromConnectorID,
						  long toID, const QString & toConnectorID,
						  bool connect, bool doEmit, bool seekLayerKin,
						  bool fromBusConnector, bool chain);

 	ItemCount calcItemCount();
	class BusConnectorItem * initializeBusConnectorItem(long busOwnerID, const QString & busID, bool doEmit);
	void mergeBuses(long bus1OwnerID, const QString & bus1ID, QPointF bus1Pos,
					long bus2OwnerID, const QString & bus2ID, QPointF bus2Pos,
					bool merge, bool doEmit);

 	qreal currentZoom();
	ItemBase::ViewIdentifier viewIdentifier();
	void setViewLayerIDs(ViewLayer::ViewLayerID part, ViewLayer::ViewLayerID wire, ViewLayer::ViewLayerID connector, ViewLayer::ViewLayerID ruler);
	void stickem(long stickTargetID, long stickSourceID, bool stick);
	void stickyScoop(ItemBase * stickyOne, QUndoCommand * parentCommand);
	void checkNewSticky(ItemBase * itemBase);
	void checkSticky(ItemBase * item, QUndoCommand * parentCommand);
	void setChainDrag(bool);
	void hoverEnterItem(QGraphicsSceneHoverEvent * event, ItemBase * item);
	void hoverLeaveItem(QGraphicsSceneHoverEvent * event, ItemBase * item);
	void hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	void hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	void cleanUpWires(bool doEmit);

	void setItemTooltip(long id, const QString &newTooltip);

	void setInfoViewOnHover(bool infoViewOnHover);
	PaletteModel * paletteModel();
    virtual ItemBase * addItemAux(ModelPart *, const ViewGeometry &, long id, PaletteItem * paletteItem, bool doConnectors);

    void viewItemInfo(long id);
    bool swappingEnabled();

	virtual void addViewLayers();
	void addPcbViewLayers();
	void addSchematicViewLayers();
	void addBreadboardViewLayers();

	void changeWireColor(long wireId, const QString& color, qreal opacity);
	void changeWireWidth(long wireId, int width);
	void changeWireFlags(long wireId, ViewGeometry::WireFlags wireFlags);
	void setIgnoreSelectionChangeEvents(bool);
	void createJumper();
	void createTrace();
	void hideConnectors(bool hide);
	void saveLayerVisibility();
	void restoreLayerVisibility();
	bool ratsAllRouted();
	virtual void updateRatsnestStatus();
	void ensureLayerVisible(ViewLayer::ViewLayerID);
	void clearRouting(QUndoCommand * parentCommand);

	const QString &selectedModuleID();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
	bool dragEnterEventAux(QDragEnterEvent *event);
	virtual bool canDropModelPart(ModelPart * modelPart);

    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent* event);
	class SelectItemCommand* stackSelectionState(bool pushIt, QUndoCommand * parentCommand);
    PaletteItem* addPartItem(ModelPart * modelPart, PaletteItem * paletteItem, bool doConnectors);
	ItemBase * findItem(long id);
	void clearHoldingSelectItem();
	bool startZChange(QList<ItemBase *> & bases);
	void continueZChange(QList<ItemBase *> & bases, int start, int end, bool (*test)(int current, int start), int inc, const QString & text);
	void continueZChangeMax(QList<ItemBase *> & bases, int start, int end, bool (*test)(int current, int start), int inc, const QString & text);
	void continueZChangeAux(QList<ItemBase *> & bases, const QString & text);
	ViewLayer::ViewLayerID getWireViewLayerID(const ViewGeometry & viewGeometry);
	ViewLayer::ViewLayerID getPartViewLayerID();
	ViewLayer::ViewLayerID getRulerViewLayerID();
	ViewLayer::ViewLayerID getConnectorViewLayerID();
	void dragMoveHighlightConnector(QDragMoveEvent *event);

	void addToScene(ItemBase * item, ViewLayer::ViewLayerID viewLayerID);
	ConnectorItem * findConnectorItem(ItemBase * item, const QString & connectorID, bool seekLayerKin);
	void updateAllLayersActions(QAction * showAllAction, QAction * hideAllAction);
	bool checkMoved();

	long createWire(ConnectorItem * from, ConnectorItem * to, bool toIsBus, ViewGeometry::WireFlags, bool addItNow, BaseCommand::CrossViewType, QUndoCommand * parentCommand);
	class Bus * hookToBus(ConnectorItem * from, ConnectorItem * busTo, QUndoCommand * parentCommand);
	void prepMergeBuses(ConnectorItem * start, QUndoCommand* parentCommand);
	void changeConnectionAux(long fromID, const QString & fromConnectorID,
						  long toID, const QString & toConnectorID,
						  bool connect, bool seekLayerKin,
						  bool fromBusConnector, bool chain);


	void pasteDuplicateAux(QString undoStackMessage);
	void cutDeleteAux(QString undoStackMessage);
	void extendChangeConnectionCommand(long fromID, const QString & fromConnectorID,
									   long toID, const QString & toConnectorID,
									   bool connect, bool seekLayerKin, QUndoCommand * parent);
	void extendChangeConnectionCommand(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem,
										bool connect, bool seekLayerKin, QUndoCommand * parentCommand);

	void disconnectVirtualWire(ConnectorItem * fromConnectorItem, QUndoCommand * parentCommand);
	void deleteVirtualWires(QSet<class VirtualWire *> & virtualWires, QUndoCommand * parentCommand);
	void disconnectVirtualWires(QSet<class VirtualWire *> & virtualWires, QUndoCommand * parentCommand);
	void reorgBuses(QList<BusConnectorItem *> & busConnectorItems, QUndoCommand * parentCommand);


	void keyPressEvent ( QKeyEvent * event );
	void makeDeleteItemCommand(ItemBase * itemBase, QUndoCommand * parentCommand);
	void clearTemporaries();
	void dragWireChanged(class Wire* wire, ConnectorItem * from, ConnectorItem * to);
	void killDroppingItem();
	ViewLayer::ViewLayerID getViewLayerID(ModelPart *);
	ItemBase * overSticky(ItemBase *);
	void cleanUpWiresAux();
	void tempDisconnectWire(ConnectorItem * fromConnectorItem, QMultiHash<ConnectorItem *, ConnectorItem *> & connectionState);
	virtual void cleanUpWire(Wire * wire, QList<Wire *> & wires);
	virtual void setNewPartVisible(ItemBase *);
	virtual void collectFemaleConnectees(PaletteItem *);
	virtual void findConnectorsUnder(ItemBase * item);

	bool currentlyInfoviewed(ItemBase *item);
	void updateInfoView();
	void resizeEvent(QResizeEvent *);

	void addViewLayersAux(const QList<ViewLayer::ViewLayerID> &layers, float startZ = 1.5);
	virtual void dealWithRatsnest(ConnectorItem * from, ConnectorItem * to, bool connect);
	virtual void checkAutorouted();
	class Wire * makeOneRatsnestWire(ConnectorItem * source, ConnectorItem * dest);
	void tempConnectWire(ItemBase * itemBase, ConnectorItem * from, ConnectorItem * to);
	void createJumperOrTrace(const QString & commandString, ViewGeometry::WireFlag, const QString & colorString);
	void rotateFlip(qreal degrees, Qt::Orientations orientation);
	void collectBusConnectorItems(QList<BusConnectorItem *> & busConnectorItems);
	virtual void disconnectFromFemale(ItemBase * item, QSet<ItemBase *> & savedItems, QSet <class VirtualWire *> & virtualWires, QUndoCommand * parentCommand);
	void cleanUpVirtualWires(QSet<class VirtualWire *> & virtualWires, QList<BusConnectorItem *> & busConnectorItems, QUndoCommand * parentCommand);
	void clearDragWireTempCommand();
	bool draggingWireEnd();
	void moveItems(QPoint globalPos);
	virtual void redrawRatsnest(QHash<long, ItemBase *> & newItems);
	virtual ViewLayer::ViewLayerID multiLayerGetViewLayerID(ModelPart * modelPart, QString & layerName);
	//void restoreDisconnectors();
	//void collectDisconnectors(ItemBase * item);
	//void dealWithVirtualDisconnections(ConnectorItem * src, ConnectorItem * dest);

protected:
	static bool lessThan(int a, int b);
	static bool greaterThan(int a, int b);


signals:
	void itemAddedSignal(ModelPart *, const ViewGeometry &, long id);
	void itemDeletedSignal(long id);
	void clearSelectionSignal();
	void itemSelectedSignal(long id, bool state);
	void tooltipAppliedToItem(long id, const QString &text);
	void wireDisconnectedSignal(long fromID, QString fromConnectorID);
	void wireConnectedSignal(long fromID,  QString fromConnectorID, long toID, QString toConnectorID);
	void changeConnectionSignal(long fromID, QString fromConnectorID,
								long toID, QString toConnectorID,  bool connect,
								bool fromBusConnector, bool chain);
	void zoomChanged(qreal zoom);
	void zoomOutOfRange(qreal zoom);
	void zoomIn(int amountSteps);
	void zoomOut(int amountSteps);
	void initializeBusConnectorItemSignal(long busOwnerID, const QString & busID);
	void mergeBusesSignal(long bus1OwnerID, const QString & bus1ID, QPointF bus1Pos,
					long bus2OwnerID, const QString & bus2ID, QPointF bus2Pos, bool merge);
	void copyItemSignal(long itemID, QHash<ItemBase::ViewIdentifier, ViewGeometry *> &);
	void deleteItemSignal(long itemID, QUndoCommand * parentCommand);
	void findSketchWidgetSignal(ItemBase::ViewIdentifier, SketchWidget * &);
	void cleanUpWiresSignal();
	void selectionChangedSignal();

	void swapped(long itemId, ModelPart *with);
	void resizeSignal();
	void routingStatusSignal(int netCount, int netRoutedCount, int connectorsLeftToRoute, int jumpers);
	void deletingSignal(SketchWidget *, QUndoCommand * parentCommand);
	void addingSignal(SketchWidget *, QUndoCommand * parentCommand);
	void rotatingFlippingSignal(SketchWidget *, QUndoCommand * parentCommand);
	void movingSignal(SketchWidget *, QUndoCommand * parentCommand);
	void changingConnectionSignal(SketchWidget *, QUndoCommand * parentCommand);
	void selectAllItemsSignal(bool state, bool doEmit);

protected slots:
	void sketchWidget_itemAdded(ModelPart *, const ViewGeometry &, long id);
	void sketchWidget_itemDeleted(long id);
	void sketchWidget_clearSelection();
	void sketchWidget_itemSelected(long id, bool state);
	void sketchWidget_tooltipAppliedToItem(long id, const QString& text);
	void scene_selectionChanged();
	void wire_wireChanged(class Wire*, QLineF oldLine, QLineF newLine, QPointF oldPos, QPointF newPos, ConnectorItem * from, ConnectorItem * to);
	void wire_wireSplit(class Wire*, QPointF newPos, QPointF oldPos, QLineF oldLine);
	void wire_wireJoin(class Wire*, ConnectorItem * clickedConnectorItem);
	void toggleLayerVisibility(QAction *);
	void sketchWidget_wireConnected(long fromID, QString fromConnectorID, long toID, QString toConnectorID);
	void sketchWidget_wireDisconnected(long fromID, QString fromConnectorID);
	void sketchWidget_changeConnection(long fromID, QString fromConnectorID, long toID, QString toConnectorID, bool connect, bool fromBusConnector, bool chain);
	void navigatorScrollChange(double x, double y);
	void restartPasteCount();
	void item_connectionChanged(ConnectorItem * from, ConnectorItem * to, bool connect);
	void sketchWidget_initializeBusConnectorItem(long busOwnerID, const QString & busID);
	void sketchWidget_mergeBuses(long bus1OwnerID, const QString & bus1ID, QPointF bus1Pos,
					long bus2OwnerID, const QString & bus2ID, QPointF bus2Pos, bool merge);
	void sketchWidget_copyItem(long itemID, QHash<ItemBase::ViewIdentifier, ViewGeometry *> &);
	void sketchWidget_deleteItem(long itemID, QUndoCommand * parentCommand);
	void dragIsDoneSlot(class ItemDrag *);
	void statusMessage(QString message, int timeout = 0);
	void sketchWidget_cleanUpWires();
	void updateInfoViewSlot();
	void spaceBarIsPressedSlot(bool);
	void autoScrollTimeout();

public slots:
	void swapSelected(const QString &moduleId);
	void swapSelected(PaletteItem* other);
	void swapSelected(ModelPart* other);
	void swap(PaletteItem* from, ModelPart *to);
	void swap(long itemId, const QString &moduleID, bool doEmit=false);
	void swap(long itemId, ModelPart *modelPart, bool doEmit=false);
	void changeWireColor(const QString &wireTitle, long wireId,
		const QString& oldColor, const QString newColor,
		qreal oldOpacity, qreal newOpacity);
 	void selectAllItems(bool state, bool doEmit);

protected:
	qreal m_scaleValue;
	int m_maxScaleValue;
	int m_minScaleValue;
	PaletteModel* m_paletteModel;
	ReferenceModel* m_refModel;
	SketchModel * m_sketchModel;
	ItemBase::ViewIdentifier m_viewIdentifier;
	class WaitPushUndoStack * m_undoStack;
	class SelectItemCommand * m_holdingSelectItemCommand;
	class SelectItemCommand * m_tempDragWireCommand;
	LayerHash m_viewLayers;
	QHash<ViewLayer::ViewLayerID, bool> m_viewLayerVisibility;
	Wire * m_connectorDragWire;
	ConnectorItem * m_connectorDragConnector;
	bool m_droppingWire;
	QPointF m_droppingOffset;
	ItemBase * m_droppingItem;
	int m_moveEventCount;
	QHash<ConnectorItem *, ConnectorItem *> m_needToConnectItems;
	QList<QGraphicsItem *> m_lastSelected;
	ViewLayer::ViewLayerID m_wireViewLayerID;
	ViewLayer::ViewLayerID m_partViewLayerID;
	ViewLayer::ViewLayerID m_rulerViewLayerID;
	ViewLayer::ViewLayerID m_connectorViewLayerID;
	QList<QGraphicsItem *> m_temporaries;
	bool m_chainDrag;
	QPointF m_mousePressScenePos;
	QTimer m_autoScrollTimer;
	volatile int m_autoScrollX;
	volatile int m_autoScrollY;
	QPoint m_globalPos;

	PaletteItem *m_lastPaletteItemSelected;

	int m_pasteCount;

	// Part Menu
	QMenu *m_itemMenu;

	bool m_infoViewOnHover;

	QSet<ItemBase *> m_savedItems;	
	QHash<Wire *, ConnectorItem *> m_savedWires;
	QList<ItemBase *> m_additionalSavedItems;
	bool m_ignoreSelectionChangeEvents;
	bool m_dealWithRatsNestEnabled;
	QHash<ConnectorItem *, ConnectorItem *> m_disconnectors;
};

#endif
