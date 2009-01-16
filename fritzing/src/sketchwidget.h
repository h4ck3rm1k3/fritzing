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

class SketchWidget : public InfoGraphicsView
{
	Q_OBJECT

public:
    SketchWidget(ItemBase::ViewIdentifier, QWidget *parent=0, int size=600, int minSize=400);

    QUndoStack* undoStack();
    ItemBase * addItem(ModelPart *, BaseCommand::CrossViewType, const ViewGeometry &, long id, long modelIndex, PaletteItem* item);
	ItemBase * addItem(const QString & moduleID, BaseCommand::CrossViewType, const ViewGeometry &, long id, long modelIndex);
    void deleteItem(long id, bool deleteModelPart, bool doEmit);
    void deleteItem(ItemBase *, bool deleteModelPart, bool doEmit);
    void moveItem(long id, ViewGeometry &);
    void rotateItem(long id, qreal degrees);
    void flipItem(long id, Qt::Orientations orientation);
    void selectItem(long id, bool state, bool updateInfoView, bool doEmit);
    void selectDeselectAllCommand(bool state);
    void changeWire(long fromID, QLineF line, QPointF pos, bool useLine);   //  const QString & fromConnectorID, long toID, const QString & toConnectorID, bool dragend
    void cut();
    void copy();
    void setPaletteModel(PaletteModel *);
    void setRefModel(ReferenceModel *refModel);
    void setSketchModel(SketchModel *);
    void setUndoStack(class WaitPushUndoStack *);
    void clearSelection();
    void loadFromModel(QList<ModelPart *> & modelParts, QUndoCommand * parentCommand);
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
 	void setWireMenu(QMenu*);
	void changeConnection(long fromID,
						  const QString & fromConnectorID,
						  long toID, const QString & toConnectorID,
						  bool connect, bool doEmit, bool seekLayerKin,
						  bool updateConnections);

 	ItemCount calcItemCount();

 	qreal currentZoom();
	ItemBase::ViewIdentifier viewIdentifier();
	void setViewLayerIDs(ViewLayer::ViewLayerID part, ViewLayer::ViewLayerID wire, ViewLayer::ViewLayerID connector, ViewLayer::ViewLayerID ruler, ViewLayer::ViewLayerID label);
	void stickem(long stickTargetID, long stickSourceID, bool stick);
	void stickyScoop(ItemBase * stickyOne, QUndoCommand * parentCommand);
	void checkNewSticky(ItemBase * itemBase);
	void checkSticky(ItemBase * item, QUndoCommand * parentCommand);
	void setChainDrag(bool);
	void hoverEnterItem(QGraphicsSceneHoverEvent * event, ItemBase * item);
	void hoverLeaveItem(QGraphicsSceneHoverEvent * event, ItemBase * item);
	void hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	void hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	void cleanUpWires(bool doEmit, class CleanUpWiresCommand *);

	void partLabelChanged(ItemBase *, const QString & oldText, const QString &newtext);

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
	void hideConnectors(bool hide);
	void saveLayerVisibility();
	void restoreLayerVisibility();
	virtual void updateRatsnestStatus(CleanUpWiresCommand*, QUndoCommand*);
	void ensureLayerVisible(ViewLayer::ViewLayerID);

	const QString &selectedModuleID();
	virtual bool canDeleteItem(QGraphicsItem * item);
	virtual bool canCopyItem(QGraphicsItem * item);
	const QString & viewName();
	void makeDeleteItemCommand(ItemBase * itemBase, QUndoCommand * parentCommand);
	virtual void dealWithRatsnest(long fromID, const QString & fromConnectorID,
								  long toID, const QString & toConnectorID,
								  bool connect, class RatsnestCommand *, bool doEmit);
	virtual void forwardRoutingStatusSignal(int netCount, int netRoutedCount, int connectorsLeftToRoute, int jumperCount);

	void addFixedToTopLeftItem(QGraphicsProxyWidget *item);
	void addFixedToTopRightItem(QGraphicsProxyWidget *item);
	void addFixedToBottomLeftItem(QGraphicsProxyWidget *item);
	void addFixedToCenterItem(QGraphicsProxyWidget *item);
	void addFixedToBottomRightItem(QGraphicsProxyWidget *item);

	void ensureFixedToTopLeftItems();
	void ensureFixedToTopRightItems();
	void ensureFixedToBottomLeftItems();
	void ensureFixedToBottomRightItems();
	void ensureFixedToCenterItems();

	void collectParts(QList<ItemBase *> & partList);

	void movePartLabel(long itemID, QPointF newPos, QPointF newOffset); 

	void updateInfoView();
	void setCurrent(bool current);
	void partLabelMoved(ItemBase *, QPointF oldPos, QPointF oldOffset, QPointF newPos, QPointF newOffset);

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
	ViewLayer::ViewLayerID getLabelViewLayerID();
	void dragMoveHighlightConnector(QPoint eventPos);

	void addToScene(ItemBase * item, ViewLayer::ViewLayerID viewLayerID);
	ConnectorItem * findConnectorItem(ItemBase * item, const QString & connectorID, bool seekLayerKin);
	void updateAllLayersActions(QAction * showAllAction, QAction * hideAllAction);
	bool checkMoved();

	long createWire(ConnectorItem * from, ConnectorItem * to, ViewGeometry::WireFlags, bool addItNow, bool doRatsnest, BaseCommand::CrossViewType, QUndoCommand * parentCommand);
	void changeConnectionAux(long fromID, const QString & fromConnectorID,
						  long toID, const QString & toConnectorID,
						  bool connect, bool seekLayerKin, bool updateConnections);


	void cutDeleteAux(QString undoStackMessage);
	void extendChangeConnectionCommand(long fromID, const QString & fromConnectorID,
									   long toID, const QString & toConnectorID,
									   bool connect, bool seekLayerKin, QUndoCommand * parent);
	void extendChangeConnectionCommand(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem,
										bool connect, bool seekLayerKin, QUndoCommand * parentCommand);


	void keyPressEvent ( QKeyEvent * event );
	void clearTemporaries();
	void dragWireChanged(class Wire* wire, ConnectorItem * from, ConnectorItem * to);
	void killDroppingItem();
	ViewLayer::ViewLayerID getViewLayerID(ModelPart *);
	ItemBase * overSticky(ItemBase *);
	void tempDisconnectWire(ConnectorItem * fromConnectorItem, ConnectorPairHash & connectionState);
	virtual void setNewPartVisible(ItemBase *);
	virtual void collectFemaleConnectees(PaletteItem *);
	virtual void findConnectorsUnder(ItemBase * item);

	bool currentlyInfoviewed(ItemBase *item);
	void resizeEvent(QResizeEvent *);

	void addViewLayersAux(const QList<ViewLayer::ViewLayerID> &layers, float startZ = 1.5);
	void tempConnectWire(Wire * wire, ConnectorItem * from, ConnectorItem * to);
	void rotateFlip(qreal degrees, Qt::Orientations orientation);
	virtual bool disconnectFromFemale(ItemBase * item, QSet<ItemBase *> & savedItems, ConnectorPairHash &, QUndoCommand * parentCommand);
	void clearDragWireTempCommand();
	bool draggingWireEnd();
	void moveItems(QPoint globalPos);
	virtual ViewLayer::ViewLayerID multiLayerGetViewLayerID(ModelPart * modelPart, QString & layerName);
	virtual BaseCommand::CrossViewType wireSplitCrossView();
	virtual void reviewDeletedConnections(QList<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash * > & deletedConnections, QUndoCommand * parentCommand);
	virtual bool canChainMultiple();
	virtual bool canChainWire(Wire *);
	virtual bool canCreateWire(Wire * dragWire, ConnectorItem * from, ConnectorItem * to);
	virtual bool modifyNewWireConnections(Wire * dragWire, ConnectorItem * fromOnWire, ConnectorItem * from, ConnectorItem * to, QUndoCommand * parentCommand);
	void setupAutoscroll(bool moving);
	void turnOffAutoscroll();
	bool checkAutoscroll(QPoint globalPos);
	virtual void setWireVisible(Wire *);
	virtual void chainVisible(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, bool connect);
	bool matchesLayer(ModelPart * modelPart);

	void ensureFixedToTopLeft(QGraphicsProxyWidget* item);
	void ensureFixedToTopRight(QGraphicsProxyWidget* item);
	void ensureFixedToBottomLeft(QGraphicsProxyWidget* item);
	void ensureFixedToBottomRight(QGraphicsProxyWidget* item);
	void ensureFixedToCenter(QGraphicsProxyWidget* item);
	void removeOutsideConnections(QByteArray & itemData, QList<long> & modelIndexes);
	void addWireExtras(long newID, QDomElement & view, QUndoCommand * parentCommand);
	virtual bool doRatsnestOnCopy();
	virtual const QString & hoverEnterConnectorMessage(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	virtual const QColor & getLabelTextColor();
	void partLabelChangedAux(ItemBase * pitem,const QString & oldText, const QString &newText);

protected:
	static bool lessThan(int a, int b);
	static bool greaterThan(int a, int b);


signals:
	void itemAddedSignal(ModelPart *, const ViewGeometry &, long id);
	void itemDeletedSignal(long id);
	void clearSelectionSignal();
	void itemSelectedSignal(long id, bool state);
	void partLabelChangedSignal(long id, const QString &text, bool isUndoable);
	void wireDisconnectedSignal(long fromID, QString fromConnectorID);
	void wireConnectedSignal(long fromID,  QString fromConnectorID, long toID, QString toConnectorID);
	void changeConnectionSignal(long fromID, QString fromConnectorID,
								long toID, QString toConnectorID,
								bool connect, bool updateConnections);
	void zoomChanged(qreal zoom);
	void zoomOutOfRange(qreal zoom);
	void zoomIn(int amountSteps);
	void zoomOut(int amountSteps);
	void copyItemSignal(long itemID, QHash<ItemBase::ViewIdentifier, ViewGeometry *> &);
	void deleteItemSignal(long itemID, QUndoCommand * parentCommand);
	void findSketchWidgetSignal(ItemBase::ViewIdentifier, SketchWidget * &);
	void cleanUpWiresSignal(CleanUpWiresCommand *);
	void selectionChangedSignal();

	void swapped(long itemId, ModelPart *with);
	void resizeSignal();
	void dropSignal(const QPoint &pos);
	void wheelSignal();
	void routingStatusSignal(int netCount, int netRoutedCount, int connectorsLeftToRoute, int jumpers);
	void ratsnestChangeSignal(SketchWidget *, QUndoCommand * parentCommand);
	void movingSignal(SketchWidget *, QUndoCommand * parentCommand);
	void rotatingFlippingSignal(SketchWidget *, QUndoCommand * parentCommand);
	void selectAllItemsSignal(bool state, bool doEmit);
	void dealWithRatsnestSignal(long fromID, const QString & fromConnectorID,
								long toID, const QString & toConnectorID,
								bool connect, class RatsnestCommand * ratsnestCommand);

protected slots:
	void sketchWidget_itemAdded(ModelPart *, const ViewGeometry &, long id);
	void sketchWidget_itemDeleted(long id);
	void sketchWidget_clearSelection();
	void sketchWidget_itemSelected(long id, bool state);
	void scene_selectionChanged();
	void wire_wireChanged(class Wire*, QLineF oldLine, QLineF newLine, QPointF oldPos, QPointF newPos, ConnectorItem * from, ConnectorItem * to);
	void wire_wireSplit(class Wire*, QPointF newPos, QPointF oldPos, QLineF oldLine);
	void wire_wireJoin(class Wire*, ConnectorItem * clickedConnectorItem);
	void toggleLayerVisibility(QAction *);
	void sketchWidget_wireConnected(long fromID, QString fromConnectorID, long toID, QString toConnectorID);
	void sketchWidget_wireDisconnected(long fromID, QString fromConnectorID);
	void sketchWidget_changeConnection(long fromID, QString fromConnectorID, long toID, QString toConnectorID, bool connect, bool updateConnections);
	void navigatorScrollChange(double x, double y);
	void restartPasteCount();
	void item_connectionChanged(ConnectorItem * from, ConnectorItem * to, bool connect);
	void sketchWidget_copyItem(long itemID, QHash<ItemBase::ViewIdentifier, ViewGeometry *> &);
	void sketchWidget_deleteItem(long itemID, QUndoCommand * parentCommand);
	void dragIsDoneSlot(class ItemDrag *);
	void statusMessage(QString message, int timeout = 0);
	void sketchWidget_cleanUpWires(CleanUpWiresCommand *);
	void updateInfoViewSlot();
	void spaceBarIsPressedSlot(bool);
	void autoScrollTimeout();
	void dragAutoScrollTimeout();
	void moveAutoScrollTimeout();
	void dealWithRatsnestSlot(long fromID, const QString & fromConnectorID,
							  long toID, const QString & toConnectorID,
							  bool connect, class RatsnestCommand * ratsnestCommand);

	void ensureFixedItemsPositions();

public slots:
	void swapSelected(const QString &moduleId);
	void swapSelected(PaletteItem* other);
	void swapSelected(ModelPart* other);
	void swap(PaletteItem* from, ModelPart *to);
	void swap(long itemId, const QString &moduleID, bool doEmit=false);
	void swap(long itemId, ModelPart *modelPart, bool doEmit=false);
	void changeWireColor(const QString newColor);
 	void selectAllItems(bool state, bool doEmit);
	void setInstanceTitle(long id, const QString & title, bool isUndoable);
	void showPartLabel(long id, bool showIt);

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
	ViewLayer::ViewLayerID m_labelViewLayerID;
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
	QMenu *m_wireMenu;

	bool m_infoViewOnHover;

	QSet<ItemBase *> m_savedItems;
	QHash<Wire *, ConnectorItem *> m_savedWires;
	QList<ItemBase *> m_additionalSavedItems;
	bool m_ignoreSelectionChangeEvents;
	bool m_current;

	QList<QGraphicsProxyWidget*> m_fixedToTopLeftItems;
	QList<QGraphicsProxyWidget*> m_fixedToTopRightItems;
	QList<QGraphicsProxyWidget*> m_fixedToBottomLeftItems;
	QList<QGraphicsProxyWidget*> m_fixedToBottomRightItems;
	QList<QGraphicsProxyWidget*> m_fixedToCenterItems;

protected:
	QString m_viewName;
};

#endif
