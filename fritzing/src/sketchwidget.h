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

#include "items/paletteitem.h"
#include "palettemodel.h"
#include "sketchmodel.h"
#include "viewgeometry.h"
#include "infographicsview.h"
#include "viewlayer.h"
#include "utils/misc.h"
#include "commands.h"

QT_BEGIN_NAMESPACE
class QDragEnterEvent;
class QDropEvent;
QT_END_NAMESPACE

struct ItemCount {
	int selCount;
	int labelCount;
	int itemsCount;
	int selRotatable;
	int selHFlipable;
	int selVFlipable;
	int noteCount;
};

class SketchWidget : public InfoGraphicsView
{
	Q_OBJECT

public:
    SketchWidget(ViewIdentifierClass::ViewIdentifier, QWidget *parent=0, int size=600, int minSize=400);
	~SketchWidget();

	void pushCommand(QUndoCommand *);
    class WaitPushUndoStack * undoStack();
    ItemBase * addItem(ModelPart *, BaseCommand::CrossViewType, const ViewGeometry &, long id, long modelIndex, long originalModelIndex, AddDeleteItemCommand * originatingCommand, PaletteItem* item);
	ItemBase * addItem(const QString & moduleID, BaseCommand::CrossViewType, const ViewGeometry &, long id, long modelIndex, long originalModelIndex, AddDeleteItemCommand * originatingCommand);
    void deleteItem(long id, bool deleteModelPart, bool doEmit, bool later, RestoreIndexesCommand * restoreIndexesCommand);
    void deleteItem(ItemBase *, bool deleteModelPart, bool doEmit, bool later);
    void moveItem(long id, ViewGeometry &);
    void rotateItem(long id, qreal degrees);
    void transformItem(long id, const QMatrix &);
    void flipItem(long id, Qt::Orientations orientation);
    void selectItem(long id, bool state, bool updateInfoView, bool doEmit);
    void selectDeselectAllCommand(bool state);
    void changeWire(long fromID, QLineF line, QPointF pos, bool useLine);   //  const QString & fromConnectorID, long toID, const QString & toConnectorID, bool dragend
    void cut();
    void copy();
    void setPaletteModel(PaletteModel *);
    void setRefModel(class ReferenceModel *refModel);
    void setSketchModel(SketchModel *);
    void setUndoStack(class WaitPushUndoStack *);
    void clearSelection();
	void loadFromModel(QList<ModelPart *> & modelParts, BaseCommand::CrossViewType, QUndoCommand * parentCommand, bool doRatsnest, bool offsetPaste);
    ItemBase* loadFromModel(ModelPart *, const ViewGeometry&);
    void changeZ(QHash<long, RealPair * >, qreal (*pairAccessor)(RealPair *) );
	void sendToBack();
	void sendBackward();
	void bringForward();
	void bringToFront();
	void fitInWindow();
	void rotateX(qreal degrees);
	void flip(Qt::Orientations orientation);
	void addBendpoint(ItemBase * lastHoverEnterItem, ConnectorItem * lastHoverEnterConnectorItem, QPointF lastLocation);

	ModelPart * group(ModelPart *);
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
	const QColor& standardBackground();
 	void setItemMenu(QMenu*);
 	void setWireMenu(QMenu*);
	void changeConnection(long fromID,
						  const QString & fromConnectorID,
						  long toID, const QString & toConnectorID,
						  bool connect, bool doEmit, bool seekLayerKin,
						  bool updateConnections);
	void moduleChangeConnection(long fromID,
						  const QString & fromConnectorID,
						  QList<long> & toIDs, const QString & toConnectorID, bool doRatsnest,
						  bool connect, bool doEmit, bool seekLayerKin,
						  bool updateConnections);

 	ItemCount calcItemCount();

	ViewIdentifierClass::ViewIdentifier viewIdentifier();
	void setViewLayerIDs(ViewLayer::ViewLayerID part, ViewLayer::ViewLayerID wire, ViewLayer::ViewLayerID connector, ViewLayer::ViewLayerID ruler, ViewLayer::ViewLayerID label, ViewLayer::ViewLayerID note);
	void stickem(long stickTargetID, long stickSourceID, bool stick);
	void stickyScoop(ItemBase * stickyOne, bool checkCurrent, CheckStickyCommand *);
	void setChainDrag(bool);
	void hoverEnterItem(QGraphicsSceneHoverEvent * event, ItemBase * item);
	void hoverLeaveItem(QGraphicsSceneHoverEvent * event, ItemBase * item);
	void hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	void hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	void cleanUpWires(bool doEmit, class CleanUpWiresCommand *, bool skipMe);

	void partLabelChanged(ItemBase *, const QString & oldText, const QString &newtext, QSizeF oldSize, QSizeF newSize, bool isLabel);

	void setInfoViewOnHover(bool infoViewOnHover);
	PaletteModel * paletteModel();
    virtual ItemBase * addItemAux(ModelPart *, const ViewGeometry &, long id, long originalModelIndex, AddDeleteItemCommand * originatingCommand, PaletteItem * paletteItem, bool doConnectors);

    bool swappingEnabled(ItemBase *);

	virtual void addViewLayers();
	void addPcbViewLayers();
	void addSchematicViewLayers();
	void addBreadboardViewLayers();

	void changeWireColor(long wireId, const QString& color, qreal opacity);
	void changeWireWidth(long wireId, qreal width);
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
	void makeDeleteItemCommand(ItemBase * itemBase, BaseCommand::CrossViewType, QUndoCommand * parentCommand);
	virtual void dealWithRatsnest(long fromID, const QString & fromConnectorID,
								  long toID, const QString & toConnectorID,
								  bool connect, class RatsnestCommand *, bool doEmit);
	virtual void forwardRoutingStatusSignal(int netCount, int netRoutedCount, int connectorsLeftToRoute, int jumperCount);

	void addFixedToTopLeftItem(QGraphicsItem *item);
	void addFixedToTopRightItem(QGraphicsItem *item);
	void addFixedToBottomLeftItem(QGraphicsItem *item);
	void addFixedToCenterItem(QGraphicsItem *item);
	void addFixedToBottomRightItem(QGraphicsItem *item);
	void addFixedToCenterItem2(class SketchMainHelp *item);

	void ensureFixedToTopLeftItems();
	void ensureFixedToTopRightItems();
	void ensureFixedToBottomLeftItems();
	void ensureFixedToBottomRightItems();
	void ensureFixedToCenterItems();

	void collectParts(QList<ItemBase *> & partList);

	void movePartLabel(long itemID, QPointF newPos, QPointF newOffset);

	void updateInfoView();
	virtual void setCurrent(bool current);
	void partLabelMoved(ItemBase *, QPointF oldPos, QPointF oldOffset, QPointF newPos, QPointF newOffset);
	void rotateFlipPartLabel(ItemBase *, qreal degrees, Qt::Orientations flipDirection);
	void rotateFlipPartLabel(long itemID, qreal degrees, Qt::Orientations flipDirection);
	void showPartLabels(bool show);
	void noteSizeChanged(ItemBase * itemBase, const QSizeF & oldSize, const QSizeF & newSize);
	void resizeNote(long itemID, const QSizeF & );
	class SelectItemCommand* stackSelectionState(bool pushIt, QUndoCommand * parentCommand);
	QString renderToSVG(qreal printerScale, const QList<ViewLayer::ViewLayerID> & partLayers, const QList<ViewLayer::ViewLayerID> & wireLayers, bool blackOnly, QSizeF & imageSize, ItemBase * offsetPart, qreal dpi, bool selectedItems);
	bool spaceBarIsPressed();
	void restoreIndexes(long id, ModelPartTiny *, bool doEmit);
	void setUpSwap(long itemID, long newModelIndex, const QString & newModuleID, bool doEmit, QUndoCommand * parentCommand);
	ConnectorItem * lastHoverEnterConnectorItem();
	ItemBase * lastHoverEnterItem();
	LayerHash & viewLayers();
	virtual void createTrace();
	void selectAllWires(ViewGeometry::WireFlag);
	virtual void tidyWires();
	void painterPathHack(long itemID, const QString & connectorID, QPainterPath &);
	void updateConnectors();
	const QString & getShortName();
	virtual void setClipEnds(class ClipableWire *, bool);
	void getBendpointWidths(class Wire *, qreal w, qreal & w1, qreal & w2);
	virtual bool includeSymbols();
	void disconnectAll();
	virtual bool canDisconnectAll();
	virtual bool ignoreFemale();
	virtual ViewLayer::ViewLayerID getWireViewLayerID(const ViewGeometry & viewGeometry);
	void setVoltage(long itemID, qreal voltage);

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
    PaletteItem* addPartItem(ModelPart * modelPart, PaletteItem * paletteItem, bool doConnectors, bool & ok);
	ItemBase * findItem(long id);
	void clearHoldingSelectItem();
	bool startZChange(QList<ItemBase *> & bases);
	void continueZChange(QList<ItemBase *> & bases, int start, int end, bool (*test)(int current, int start), int inc, const QString & text);
	void continueZChangeMax(QList<ItemBase *> & bases, int start, int end, bool (*test)(int current, int start), int inc, const QString & text);
	void continueZChangeAux(QList<ItemBase *> & bases, const QString & text);
	virtual ViewLayer::ViewLayerID getDragWireViewLayerID();
	ViewLayer::ViewLayerID getPartViewLayerID();
	ViewLayer::ViewLayerID getRulerViewLayerID();
	ViewLayer::ViewLayerID getConnectorViewLayerID();
	ViewLayer::ViewLayerID getLabelViewLayerID();
	ViewLayer::ViewLayerID getNoteViewLayerID();
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
	void deleteAux(QSet<ItemBase *> & deletedItems, QString undoStackMessage);
	bool deleteMiddle(QSet<ItemBase *> & deletedItems, QUndoCommand * parentCommand);
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
	virtual void setNewPartVisible(ItemBase *);
	virtual void collectFemaleConnectees(ItemBase *, QSet<ItemBase *> &);
	virtual void findConnectorsUnder(ItemBase * item);

	bool currentlyInfoviewed(ItemBase *item);
	void resizeEvent(QResizeEvent *);

	void addViewLayersAux(const QList<ViewLayer::ViewLayerID> &layers, float startZ = 1.5);
	void tempConnectWire(Wire * wire, ConnectorItem * from, ConnectorItem * to);
	void rotateFlip(qreal degrees, Qt::Orientations orientation);
	virtual bool disconnectFromFemale(ItemBase * item, QSet<ItemBase *> & savedItems, ConnectorPairHash &, bool doCommand, QUndoCommand * parentCommand);
	void clearDragWireTempCommand();
	bool draggingWireEnd();
	void moveItems(QPoint globalPos);
	virtual ViewLayer::ViewLayerID multiLayerGetViewLayerID(ModelPart * modelPart, QDomElement & layers, QString & layerName);
	virtual BaseCommand::CrossViewType wireSplitCrossView();
	virtual bool reviewDeletedConnections(QSet<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash * > & deletedConnections, QUndoCommand * parentCommand);
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

	void ensureFixedToTopLeft(QGraphicsItem* item);
	void ensureFixedToTopRight(QGraphicsItem* item);
	void ensureFixedToBottomLeft(QGraphicsItem* item);
	void ensureFixedToBottomRight(QGraphicsItem* item);
	void ensureFixedToCenter(QGraphicsItem* item);

	qreal fixedItemWidth(QGraphicsItem* item);
	qreal fixedItemHeight(QGraphicsItem* item);

	void removeIfFixedPos(QGraphicsItem *item);
	void clearFixedItems();

	QByteArray removeOutsideConnections(const QByteArray & itemData, QList<long> & modelIndexes);
	void addWireExtras(long newID, QDomElement & view, QUndoCommand * parentCommand);
	virtual bool doRatsnestOnCopy();
	virtual const QString & hoverEnterWireConnectorMessage(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	virtual const QString & hoverEnterPartConnectorMessage(QGraphicsSceneHoverEvent * event, ConnectorItem * item);
	void partLabelChangedAux(ItemBase * pitem,const QString & oldText, const QString &newText, QSizeF oldSize, QSizeF newSize, bool isLabel);
	void drawBackground( QPainter * painter, const QRectF & rect );
	void handleConnect(QDomElement & connect, ModelPart *, const QString & fromConnectorID, QStringList & alreadyConnected, QHash<long, ItemBase *> & newItems, bool doRatsnest, QUndoCommand * parentCommand);
	ItemBase * findModulePart(ItemBase * toBase, QList<long> & indexes);
	ItemBase * makeModule(ModelPart *, long originalModelIndex, QList<ModelPart *> & modelParts, const ViewGeometry &, long id); 
	void collectModuleExternalConnectors(ItemBase *, ItemBase * parent, ConnectorPairHash &);
	void setUpSwapReconnect(ItemBase* itemBase, long newID, const QString & newModuleID, bool master, QUndoCommand * parentCommand);
	bool swappedGender(ConnectorItem * originalConnectorItem, Connector * newConnector);
	void setLastPaletteItemSelected(PaletteItem * paletteItem);
	void setLastPaletteItemSelectedIf(ItemBase * itemBase);
	bool rotationAllowed(ItemBase *);
	virtual bool allowFemaleRotation(ItemBase *);
	void prepDragBendpoint(Wire *, QPoint eventPos);
	void prepDragWire(Wire *);
	void clickBackground(QMouseEvent *);
	void categorizeDragWires(QSet<Wire *> & wires);
	void prepMove();
	void initBackgroundColor();
	QPointF calcNewLoc(ItemBase * moveBase, ItemBase * detachFrom);
	long findWire(long itemID);
	void resizeBoard();
	void resizeJumperItem();

protected:
	static bool lessThan(int a, int b);
	static bool greaterThan(int a, int b);

signals:
	void itemAddedSignal(ModelPart *, const ViewGeometry &, long id, SketchWidget * dropOrigin);
	void itemDeletedSignal(long id);
	void clearSelectionSignal();
	void itemSelectedSignal(long id, bool state);
	void wireDisconnectedSignal(long fromID, QString fromConnectorID);
	void wireConnectedSignal(long fromID,  QString fromConnectorID, long toID, QString toConnectorID);
	void changeConnectionSignal(long fromID, QString fromConnectorID,
								long toID, QString toConnectorID,
								bool connect, bool updateConnections);
	void copyItemSignal(long itemID, QHash<ViewIdentifierClass::ViewIdentifier, ViewGeometry *> &);
	void cleanUpWiresSignal(CleanUpWiresCommand *);
	void selectionChangedSignal();

	void resizeSignal();
	void dropSignal(const QPoint &pos);
	void routingStatusSignal(SketchWidget *, int netCount, int netRoutedCount, int connectorsLeftToRoute, int jumpers);
	void ratsnestChangeSignal(SketchWidget *, QUndoCommand * parentCommand);
	void movingSignal(SketchWidget *, QUndoCommand * parentCommand);
	void rotatingFlippingSignal(SketchWidget *, QUndoCommand * parentCommand);
	void selectAllItemsSignal(bool state, bool doEmit);
	void dealWithRatsnestSignal(long fromID, const QString & fromConnectorID,
								long toID, const QString & toConnectorID,
								bool connect, class RatsnestCommand * ratsnestCommand);
	void groupSignal(const QString & moduleID, long itemID, QList<long> & itemIDs, const ViewGeometry &, bool doEmit);
	void restoreIndexesSignal(ModelPart *, ModelPartTiny *, bool doEmit);
	void checkStickySignal(long id, bool doEmit, bool checkCurrent, CheckStickyCommand *);
	void rememberStickySignal(long id, QUndoCommand * parentCommand);
	void disconnectAllSignal(QList<ConnectorItem *>, QHash<ItemBase *, SketchWidget *> & itemsToDelete, QUndoCommand * parentCommand);
	void setResistanceSignal(long itemID, QString resistance, QString pinSpacing, bool doEmit);
	void setChipLabelSignal(long itemID, QString label, bool doEmit);

protected slots:
	void sketchWidget_itemAdded(ModelPart *, const ViewGeometry &, long id, SketchWidget * dropOrigin);
	void sketchWidget_itemDeleted(long id);
	void sketchWidget_clearSelection();
	void sketchWidget_itemSelected(long id, bool state);
	void scene_selectionChanged();
	void wire_wireChanged(class Wire*, QLineF oldLine, QLineF newLine, QPointF oldPos, QPointF newPos, ConnectorItem * from, ConnectorItem * to);
	void wire_wireSplit(class Wire*, QPointF newPos, QPointF oldPos, QLineF oldLine);
	void wire_wireJoin(class Wire*, ConnectorItem * clickedConnectorItem);
	void toggleLayerVisibility();
	void sketchWidget_wireConnected(long fromID, QString fromConnectorID, long toID, QString toConnectorID);
	void sketchWidget_wireDisconnected(long fromID, QString fromConnectorID);
	void sketchWidget_changeConnection(long fromID, QString fromConnectorID, long toID, QString toConnectorID, bool connect, bool updateConnections);
	void navigatorScrollChange(double x, double y);
	void restartPasteCount();
	void sketchWidget_copyItem(long itemID, QHash<ViewIdentifierClass::ViewIdentifier, ViewGeometry *> &);
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
	void rememberSticky(long id, QUndoCommand * parentCommand);

public slots:
	void changeWireColor(const QString newColor);
	void changeWireWidthMils(const QString newWidth);
 	void selectAllItems(bool state, bool doEmit);
	void setInstanceTitle(long id, const QString & title, bool isLabel, bool isUndoable);
	void showPartLabel(long id, bool showIt);
	void group(const QString & moduleID, long itemID, QList<long> & itemIDs, const ViewGeometry &, bool doEmit);
	void restoreIndexes(ModelPart *, ModelPartTiny *, bool doEmit);
	void checkSticky(long id, bool doEmit, bool checkCurrent, CheckStickyCommand *);
	void resizeBoard(qreal w, qreal h);
	void resizeBoard(long id, qreal w, qreal h);
	void resizeJumperItem(long id, QPointF pos, QPointF c0, QPointF c1);
	void setVoltage(qreal voltage);
	void disconnectAllSlot(QList<ConnectorItem *>, QHash<ItemBase *, SketchWidget *> & itemsToDelete, QUndoCommand * parentCommand);
	void setResistance(long itemID, QString resistance, QString pinSpacing, bool doEmit);
	void setResistance(QString resistance, QString pinSpacing);
	void setChipLabel(long itemID, QString label, bool doEmit);
	void setChipLabel(QString label);

protected:
	QPointer<PaletteModel> m_paletteModel;
	QPointer<class ReferenceModel> m_refModel;
	QPointer<SketchModel> m_sketchModel;
	ViewIdentifierClass::ViewIdentifier m_viewIdentifier;
	class WaitPushUndoStack * m_undoStack;
	class SelectItemCommand * m_holdingSelectItemCommand;
	class SelectItemCommand * m_tempDragWireCommand;
	LayerHash m_viewLayers;
	QHash<ViewLayer::ViewLayerID, bool> m_viewLayerVisibility;
	QPointer<Wire> m_connectorDragWire;
	QPointer<Wire> m_bendpointWire;
	ViewGeometry m_bendpointVG;
	QPointer<ConnectorItem> m_connectorDragConnector;
	bool m_droppingWire;
	QPointF m_droppingOffset;
	QPointer<ItemBase> m_droppingItem;
	int m_moveEventCount;
	QList<QGraphicsItem *> m_lastSelected;
	ViewLayer::ViewLayerID m_wireViewLayerID;
	ViewLayer::ViewLayerID m_partViewLayerID;
	ViewLayer::ViewLayerID m_rulerViewLayerID;
	ViewLayer::ViewLayerID m_connectorViewLayerID;
	ViewLayer::ViewLayerID m_labelViewLayerID;
	ViewLayer::ViewLayerID m_noteViewLayerID;
	QList<QGraphicsItem *> m_temporaries;
	bool m_chainDrag;
	QPointF m_mousePressScenePos;
	QPointF m_mousePressGlobalPos;
	QTimer m_autoScrollTimer;
	volatile int m_autoScrollX;
	volatile int m_autoScrollY;
	QPoint m_globalPos;

	QPointer<PaletteItem> m_lastPaletteItemSelected;

	int m_pasteCount;

	// Part Menu
	QMenu *m_itemMenu;
	QMenu *m_wireMenu;

	bool m_infoViewOnHover;

	QSet<ItemBase *> m_savedItems;
	QHash<Wire *, ConnectorItem *> m_savedWires;
	QList<ItemBase *> m_additionalSavedItems;
	int m_ignoreSelectionChangeEvents;
	bool m_current;

	QList<QGraphicsItem*> m_fixedToTopLeftItems;
	QList<QGraphicsItem*> m_fixedToTopRightItems;
	QList<QGraphicsItem*> m_fixedToBottomLeftItems;
	QList<QGraphicsItem*> m_fixedToBottomRightItems;
	QList<QGraphicsItem*> m_fixedToCenterItems;
	class SketchMainHelp * m_fixedToCenterItem;
	QPoint m_fixedToCenterItemOffset;

	QString m_lastColorSelected;

	ConnectorPairHash m_moveDisconnectedFromFemale;
	bool m_spaceBarIsPressed;
	bool m_spaceBarWasPressed;

	QPointer<class ResizableBoard> m_resizingBoard;
	QPointer<class JumperItem> m_resizingJumperItem;
	QPointer<ConnectorItem> m_lastHoverEnterConnectorItem;
	QPointer<ItemBase> m_lastHoverEnterItem;
	QString m_shortName;
	Wire * m_dragBendpointWire;
	QPoint m_dragBendpointPos;
	QColor m_standardBackgroundColor;

protected:
	QString m_viewName;

public:
	static ViewLayer::ViewLayerID defaultConnectorLayer(ViewIdentifierClass::ViewIdentifier viewId);

protected:
	static QHash<ViewIdentifierClass::ViewIdentifier,QColor> m_bgcolors;
};

#endif
