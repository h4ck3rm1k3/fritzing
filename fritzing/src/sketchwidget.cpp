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




#include <QtGui>
#include <QGraphicsScene>
#include <QPoint>
#include <QMatrix>
#include <QtAlgorithms>
#include <QPen>
#include <QColor>
#include <QRubberBand>
#include <QLine>
#include <QHash>
#include <QBrush>
#include <QGraphicsItem>
#include <QMainWindow>
#include <QApplication>

#include "paletteitem.h"
#include "wire.h"
#include "commands.h"
#include "modelpart.h"
#include "debugdialog.h"
#include "layerkinpaletteitem.h"
#include "sketchwidget.h"
#include "bettertriggeraction.h"
#include "connectoritem.h"
#include "bus.h"
#include "virtualwire.h"
#include "itemdrag.h"
#include "layerattributes.h"
#include "waitpushundostack.h"
#include "zoomcombobox.h"
#include "autorouter1.h"

SketchWidget::SketchWidget(ItemBase::ViewIdentifier viewIdentifier, QWidget *parent, int size, int minSize)
    : InfoGraphicsView(parent)
{
	m_dealWithRatsNestEnabled = true;
	m_ignoreSelectionChangeEvents = false;
	m_droppingItem = NULL;
	m_chainDrag = false;
	m_connectorDragWire = NULL;
	m_tempDragWireCommand = m_holdingSelectItemCommand = NULL;
	m_viewIdentifier = viewIdentifier;
	m_scaleValue = 100;
	m_maxScaleValue = 2000;
	m_minScaleValue = 1;
	setAlignment(Qt::AlignLeft | Qt::AlignTop);
	setDragMode(QGraphicsView::RubberBandDrag);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setAcceptDrops(true);
	setRenderHint(QPainter::Antialiasing, true);
	//setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	//setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	//setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	//setTransformationAnchor(QGraphicsView::NoAnchor);
    QGraphicsScene* scene = new QGraphicsScene(this);
    this->setScene(scene);

    //this->scene()->setSceneRect(0,0, 1000, 1000);

    // Setting the scene rect here seems to mean it never resizes when the user drags an object
    // outside the sceneRect bounds.  So catch some signal and do the resize manually?
    // this->scene()->setSceneRect(0, 0, 500, 500);

    // if the sceneRect isn't set, the view seems to grow and scroll gracefully as new items are added
    // however, it doesn't shrink if items are removed.

    // a bit of a hack so that, when there is no scenerect set,
    // the first item dropped into the scene doesn't leap to the top left corner
    // as the scene resizes to fit the new item
   	QGraphicsLineItem * item = new QGraphicsLineItem();
    item->setLine(1,1,1,1);
    this->scene()->addItem(item);
    item->setVisible(false);

    connect(this->scene(), SIGNAL(selectionChanged()), this, SLOT(scene_selectionChanged()));

    connect(QApplication::clipboard(),SIGNAL(changed(QClipboard::Mode)),this,SLOT(restartPasteCount()));
    restartPasteCount(); // the first time

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resize(size, size);
    setMinimumSize(minSize, minSize);


	//QGraphicsItemGroup * group = new QGraphicsItemGroup();
	//group->setZValue(10);
	//this->scene()->addItem(group);
	//
	//QPen pen(QBrush(QColor(255, 0, 0)), 5);
	//
    //item = new QGraphicsLineItem();
    //item->setLine(300,300,500,500);
    //this->scene()->addItem(item);
    //item->setZValue(0.5);
	//item->setFlags(QGraphicsItem::ItemIsSelectable  | QGraphicsItem::ItemIsMovable );
	//item->setPen(pen);
   	//group->addToGroup(item);
    //
    //item = new QGraphicsLineItem();
    //item->setLine(500,300,300,500);
    //this->scene()->addItem(item);
    //item->setZValue(10.5);
	//item->setFlags(QGraphicsItem::ItemIsSelectable  | QGraphicsItem::ItemIsMovable );
	//item->setPen(pen);
    //group->addToGroup(item);

    m_lastPaletteItemSelected = NULL;

#ifdef QT_NO_DEBUG
    m_infoViewOnHover = false;
#endif
}

void SketchWidget::restartPasteCount() {
	m_pasteCount = 1;
}

QUndoStack* SketchWidget::undoStack() {
	return m_undoStack;
}

void SketchWidget::setUndoStack(WaitPushUndoStack * undoStack) {
	m_undoStack = undoStack;
}

ItemBase* SketchWidget::loadFromModel(ModelPart *modelPart, const ViewGeometry& viewGeometry){
	// assumes modelPart has already been added to the sketch
	// or you're in big trouble when you delete the item
	return addItemAux(modelPart, viewGeometry, ItemBase::getNextID(), NULL, true);
}

void SketchWidget::loadFromModel() {
	ModelPart* root = m_sketchModel->root();
	QHash<long, ItemBase *> newItems;
	QHash<ItemBase *, QDomElement *> itemDoms;
	m_dealWithRatsNestEnabled = false;
	m_ignoreSelectionChangeEvents = true;

	QString viewName = ItemBase::viewIdentifierXmlName(m_viewIdentifier);

	// first make the parts
	QList<ModelPart *> badModelParts;
	foreach (QObject * object, root->children()) {
		ModelPart* mp = qobject_cast<ModelPart *>(object);
		if (mp == NULL) continue;

		QDomElement instance = mp->instanceDomElement();
		if (instance.isNull()) continue;

		QDomElement views = instance.firstChildElement("views");
		if (views.isNull()) continue;

		QDomElement view = views.firstChildElement(viewName);
		if (view.isNull()) continue;

		QDomElement geometry = view.firstChildElement("geometry");
		if (geometry.isNull()) continue;

		ViewGeometry viewGeometry(geometry);

		// hack 2008/11/25 to deal with earlier versions of saved files
		if (viewGeometry.getVirtual()) {
			badModelParts.append(mp);
			continue;
		}

		// use a function of the model index to ensure the same parts have the same ID across views
		ItemBase * item = addItemAux(mp, viewGeometry, ItemBase::getNextID(mp->modelIndex()), NULL, true);
		if (item != NULL) {
			PaletteItem * paletteItem = dynamic_cast<PaletteItem *>(item);
			if (paletteItem) {
				// wires don't have transforms
				paletteItem->setTransforms();
			}
			else {
				Wire * wire = dynamic_cast<Wire *>(item);
				if (wire != NULL) {
					QDomElement extras = view.firstChildElement("wireExtras");
					wire->setExtras(extras);
				}
			}

			// use the modelIndex from mp, not from the newly created item, because we're mapping from the modelIndex in the xml file
			newItems.insert(mp->modelIndex(), item);
			itemDoms.insert(item, new QDomElement(view));
		}
	}

	foreach (ModelPart * mp, badModelParts) {
		m_sketchModel->removeModelPart(mp);
	}


	foreach (ItemBase * itemBase, itemDoms.keys()) {
		QDomElement * dom = itemDoms.value(itemBase);
		itemBase->restoreConnections(*dom,  newItems);
		delete dom;
	}

	foreach (ItemBase * item, newItems) {
		if (item->sticky()) {
			stickyScoop(item, NULL);
		}
	}

	updateRatsnestStatus();

	m_dealWithRatsNestEnabled = true;
	redrawRatsnest(newItems);
	checkAutorouted();
	this->scene()->clearSelection();
	cleanUpWires(false);
	m_ignoreSelectionChangeEvents = false;
}

void SketchWidget::checkAutorouted() {
}

void SketchWidget::redrawRatsnest(QHash<long, ItemBase *> & newItems) {
	Q_UNUSED(newItems);
}

ItemBase * SketchWidget::addItem(const QString & moduleID, BaseCommand::CrossViewType crossViewType, const ViewGeometry & viewGeometry, long id) {
	if (m_paletteModel == NULL) return NULL;

	ItemBase * itemBase = NULL;
	ModelPart * modelPart = m_paletteModel->retrieveModelPart(moduleID);
	if (modelPart != NULL) {
		QApplication::setOverrideCursor(Qt::WaitCursor);
		statusMessage(tr("loading part"));
		itemBase = addItem(modelPart, crossViewType, viewGeometry, id);
		statusMessage(tr("done loading"), 2000);
		QApplication::restoreOverrideCursor();
	}

	return itemBase;
}

ItemBase * SketchWidget::addItem(ModelPart * modelPart, BaseCommand::CrossViewType crossViewType, const ViewGeometry & viewGeometry, long id, PaletteItem* partsEditorPaletteItem) {
	modelPart = m_sketchModel->addModelPart(m_sketchModel->root(), modelPart);
	ItemBase * newItem = addItemAux(modelPart, viewGeometry, id, partsEditorPaletteItem, true);
	if (crossViewType == BaseCommand::CrossView) {
		emit itemAddedSignal(modelPart, viewGeometry, id);
	}

	return newItem;
}

ItemBase * SketchWidget::addItemAux(ModelPart * modelPart, const ViewGeometry & viewGeometry, long id, PaletteItem* partsEditorPaletteItem, bool doConnectors )
{
	Q_UNUSED(partsEditorPaletteItem);

	if (doConnectors) {
		modelPart->initConnectors();    // is a no-op if connectors already in place
	}

	if (modelPart->itemType() == ModelPart::Wire ) {
		bool virtualWire = viewGeometry.getVirtual();
		Wire * wire = NULL;
		if (virtualWire) {
			wire = new VirtualWire(modelPart, m_viewIdentifier, viewGeometry, id, m_itemMenu);
             			wire->setUp(getWireViewLayerID(viewGeometry), m_viewLayers);

			// prevents virtual wires from flashing up on screen
			wire->setVisible(false);
		}
		else {
			wire = new Wire(modelPart, m_viewIdentifier, viewGeometry, id, m_itemMenu);
			wire->setUp(getWireViewLayerID(viewGeometry), m_viewLayers);
		}

		bool succeeded = connect(wire, SIGNAL(wireChangedSignal(Wire*, QLineF, QLineF, QPointF, QPointF, ConnectorItem *, ConnectorItem *)	),
				this, SLOT(wire_wireChanged(Wire*, QLineF, QLineF, QPointF, QPointF, ConnectorItem *, ConnectorItem *)),
				Qt::DirectConnection);		// DirectConnection means call the slot directly like a subroutine, without waiting for a thread or queue
		succeeded = succeeded && connect(wire, SIGNAL(wireSplitSignal(Wire*, QPointF, QPointF, QLineF)),
				this, SLOT(wire_wireSplit(Wire*, QPointF, QPointF, QLineF)));
		succeeded = succeeded && connect(wire, SIGNAL(wireJoinSignal(Wire*, ConnectorItem *)),
				this, SLOT(wire_wireJoin(Wire*, ConnectorItem*)));
		if (!succeeded) {
			DebugDialog::debug("wire signal connect failed");
		}

		addToScene(wire, wire->viewLayerID());
		DebugDialog::debug(QString("adding wire %1 %2 %3")
			.arg(wire->id())
			.arg(m_viewIdentifier)
			.arg(viewGeometry.flagsAsInt())
			);

		checkNewSticky(wire);
		return wire;
	}
	else {
    	PaletteItem* paletteItem = new PaletteItem(modelPart, m_viewIdentifier, viewGeometry, id, m_itemMenu);
		DebugDialog::debug(QString("adding part %1 %2 %3").arg(id).arg(paletteItem->title()).arg(m_viewIdentifier) );
    	ItemBase * itemBase = addPartItem(modelPart, paletteItem, doConnectors);
		setNewPartVisible(itemBase);
		return itemBase;
	}

}

void SketchWidget::cleanUpWire(Wire * wire, QList<Wire *> & wires) {
	Q_UNUSED(wire);
	Q_UNUSED(wires);
}

void SketchWidget::setNewPartVisible(ItemBase * itemBase) {
	Q_UNUSED(itemBase);
	// defaults to visible, so do nothing
}

void SketchWidget::checkSticky(ItemBase * item, QUndoCommand * parentCommand) {
	if (item->sticky()) {
		foreach (ItemBase * s, item->sticking().keys()) {
			ViewGeometry viewGeometry = s->getViewGeometry();
			s->saveGeometry();
			new MoveItemCommand(this, s->id(), viewGeometry, s->getViewGeometry(), parentCommand);
		}

		stickyScoop(item, parentCommand);
	}
	else {
		ItemBase * stickyOne = overSticky(item);
		ItemBase * wasStickyOne = item->stuckTo();
		if (stickyOne != wasStickyOne) {
			if (wasStickyOne != NULL) {
				new StickyCommand(this, wasStickyOne->id(), item->id(), false, parentCommand);
			}
			if (stickyOne != NULL) {
				new StickyCommand(this, stickyOne->id(), item->id(), true, parentCommand);
			}
		}
	}
}

void SketchWidget::checkNewSticky(ItemBase * itemBase) {
	if (itemBase->sticky()) {
		stickyScoop(itemBase, NULL);
	}
	else {
		ItemBase * stickyOne = overSticky(itemBase);
		if (stickyOne != NULL) {
			// would prefer to handle this via command object, but it's tricky because an item dropped in a given view
			// would only need to stick in a different view
			stickyOne->addSticky(itemBase, true);
			itemBase->addSticky(stickyOne, true);
		}
	}
}

PaletteItem* SketchWidget::addPartItem(ModelPart * modelPart, PaletteItem * paletteItem, bool doConnectors) {

	ViewLayer::ViewLayerID viewLayerID = getViewLayerID(modelPart);

	if (paletteItem->renderImage(modelPart, m_viewIdentifier, m_viewLayers, viewLayerID, true, doConnectors)) {
		addToScene(paletteItem, paletteItem->viewLayerID());
		paletteItem->loadLayerKin(m_viewLayers);
		for (int i = 0; i < paletteItem->layerKin().count(); i++) {
			LayerKinPaletteItem * lkpi = paletteItem->layerKin()[i];
			this->scene()->addItem(lkpi);
			lkpi->setHidden(!layerIsVisible(lkpi->viewLayerID()));
		}

		connect(paletteItem, SIGNAL(connectionChangedSignal(ConnectorItem *, ConnectorItem *, bool)	),
				this, SLOT(item_connectionChanged(ConnectorItem *, ConnectorItem *, bool)),
				Qt::DirectConnection);   // DirectConnection means call the slot directly like a subroutine, without waiting for a thread or queue

		checkNewSticky(paletteItem);
		return paletteItem;
	}
	else {
		// nobody falls through to here now?

		QMessageBox::information(dynamic_cast<QMainWindow *>(this->window()), QObject::tr("Fritzing"),
								 QObject::tr("The file %1 is not a Fritzing file (1).").arg(modelPart->modelPartStuff()->path()) );


		DebugDialog::debug(QString("addPartItem renderImage failed %1").arg(modelPart->moduleID()) );

		//paletteItem->modelPart()->removeViewItem(paletteItem);
		//delete paletteItem;
		//return NULL;
		scene()->addItem(paletteItem);
		//paletteItem->setVisible(false);
		return paletteItem;
	}
}

void SketchWidget::addToScene(ItemBase * item, ViewLayer::ViewLayerID viewLayerID) {
	scene()->addItem(item);
 	item->setSelected(true);
 	item->setHidden(!layerIsVisible(viewLayerID));
	if (!item->getVirtual()) {
		item->setFlag(QGraphicsItem::ItemIsMovable, true);
	}
}

ItemBase * SketchWidget::findItem(long id) {
	// TODO:  replace scene()->items()

	QList<QGraphicsItem *> items = this->scene()->items();
	for (int i = 0; i < items.size(); i++) {
		ItemBase* base = ItemBase::extractItemBase(items[i]);
		if (base == NULL) continue;

		if (base->id() == id) {
			return base;
		}
	}

	return NULL;
}

void SketchWidget::deleteItem(long id, bool deleteModelPart, bool doEmit) {
	DebugDialog::debug(tr("delete item %1 %2").arg(id).arg(doEmit) );
	ItemBase * pitem = findItem(id);
	if (pitem != NULL) {
		deleteItem(pitem, deleteModelPart, doEmit);
	}
}

void SketchWidget::deleteItem(ItemBase * itemBase, bool deleteModelPart, bool doEmit)
{
	if (m_infoView != NULL) {
		m_infoView->unregisterCurrentItemIf(itemBase->id());
	}
	m_lastSelected.removeOne(itemBase);

	long id = itemBase->id();
	itemBase->removeLayerKin();
	this->scene()->removeItem(itemBase);
	DebugDialog::debug(tr("delete item %1 %2").arg(id).arg(itemBase->title()) );

	if (deleteModelPart) {
		ModelPart * modelPart = itemBase->modelPart();
		m_sketchModel->removeModelPart(modelPart);
		delete modelPart;
	}

	delete itemBase;

	if (doEmit) {
		emit itemDeletedSignal(id);
	}

}

void SketchWidget::deleteItem() {
	cutDeleteAux("Delete");
}

void SketchWidget::cutDeleteAux(QString undoStackMessage) {
    // get sitems first, before calling stackSelectionState
    // because selectedItems will return an empty list
	const QList<QGraphicsItem *> sitems = scene()->selectedItems();

	QList<ItemBase *> deletedItems;

	for (int i = 0; i < sitems.size(); i++) {

		ItemBase *itemBase = ItemBase::extractTopLevelItemBase(sitems[i]);
		if (itemBase == NULL) continue;
		if (itemBase->modelPart() == NULL) continue;

		if (itemBase->itemType() == ModelPart::Wire) {
			Wire * wire = dynamic_cast<Wire *>(itemBase);
			if (wire->getRatsnest()) {
				// shouldn't be here, but check anyway
				continue;
			}
		}

		deletedItems.append(itemBase);
	}

	if (deletedItems.count() <= 0) {
		return;
	}

	QString string;

	if (deletedItems.count() == 1) {
		string = tr("%1 %2").arg(undoStackMessage).arg(deletedItems[0]->modelPart()->title());
	}
	else {
		string = tr("%1 %2 items").arg(undoStackMessage).arg(QString::number(deletedItems.count()));
	}

	QUndoCommand * parentCommand = new QUndoCommand(string);
	new CleanUpWiresCommand(this, false, parentCommand);
	parentCommand->setText(string);

	emit deletingSignal(this, parentCommand);

    stackSelectionState(false, parentCommand);

	// the theory is that we only need to disconnect virtual wires at this point
	// and not regular connectors
	// since in figuring out how to manage busConnections
	// all parts are connected to busConnections only by virtual wires

	foreach (ItemBase * itemBase, deletedItems) {
		QMultiHash<ConnectorItem *, ConnectorItem *>  connectorHash;
		itemBase->collectConnectors(connectorHash, this->scene());

		// now prepare to disconnect all the deleted item's connectors
		foreach (ConnectorItem * fromConnectorItem,  connectorHash.uniqueKeys()) {
			foreach (ConnectorItem * toConnectorItem, connectorHash.values(fromConnectorItem)) {
				extendChangeConnectionCommand(fromConnectorItem, toConnectorItem,
											  false, false, parentCommand);
				fromConnectorItem->tempRemove(toConnectorItem);
				toConnectorItem->tempRemove(fromConnectorItem);
			}
		}
   	}

	for (int i = 0; i < deletedItems.count(); i++) {
		ItemBase * itemBase = deletedItems[i];
		makeDeleteItemCommand(itemBase, parentCommand);
		emit deleteItemSignal(itemBase->id(), parentCommand);			// let the other views add the command
	}


	new CleanUpWiresCommand(this, true, parentCommand);
   	m_undoStack->push(parentCommand);


}

void SketchWidget::extendChangeConnectionCommand(long fromID, const QString & fromConnectorID,
												 long toID, const QString & toConnectorID,
												 bool connect, bool seekLayerKin, QUndoCommand * parentCommand)
{
	ItemBase * fromItem = findItem(fromID);
	if (fromItem == NULL) {
		return;  // for now
	}

	ItemBase * toItem = findItem(toID);
	if (toItem == NULL) {
		return;		// for now
	}

	ConnectorItem * fromConnectorItem = findConnectorItem(fromItem, fromConnectorID, seekLayerKin);
	if (fromConnectorItem == NULL) return; // for now

	ConnectorItem * toConnectorItem = findConnectorItem(toItem, toConnectorID, seekLayerKin);
	if (toConnectorItem == NULL) return; // for now

	extendChangeConnectionCommand(fromConnectorItem, toConnectorItem, connect, seekLayerKin, parentCommand);
}

void SketchWidget::extendChangeConnectionCommand(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem,
												 bool connect, bool seekLayerKin, QUndoCommand * parentCommand)
{
	// cases:
	//		delete
	//		paste
	//		drop (wire)
	//		drop (part)
	//		move (part)
	//		move (wire)
	//		drag wire end
	//		drag out new wire

	ItemBase * fromItem = fromConnectorItem->attachedTo();
	if (fromItem == NULL) {
		return;  // for now
	}

	ItemBase * toItem = toConnectorItem->attachedTo();
	if (toItem == NULL) {
		return;		// for now
	}

	emit changingConnectionSignal(this, parentCommand);

	new ChangeConnectionCommand(this, BaseCommand::CrossView,
								fromItem->id(), fromConnectorItem->connectorStuffID(),
								toItem->id(), toConnectorItem->connectorStuffID(),
								connect, seekLayerKin, false, parentCommand);
	if (connect) {
	}
	else {
	}
}


long SketchWidget::createWire(ConnectorItem * from, ConnectorItem * to, ViewGeometry::WireFlags wireFlags,
							  bool addItNow, BaseCommand::CrossViewType crossViewType, QUndoCommand * parentCommand)
{
	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	QPointF fromPos = from->sceneAdjustedTerminalPoint();
	viewGeometry.setLoc(fromPos);
	QPointF toPos = to->sceneAdjustedTerminalPoint();
	QLineF line(0, 0, toPos.x() - fromPos.x(), toPos.y() - fromPos.y());
	viewGeometry.setLine(line);
	viewGeometry.setWireFlags(wireFlags);

	DebugDialog::debug(QString("creating virtual wire %11: %1, flags: %6, from %7 %8, to %9 %10, frompos: %2 %3, topos: %4 %5")
		.arg(newID)
		.arg(fromPos.x()).arg(fromPos.y())
		.arg(toPos.x()).arg(toPos.y())
		.arg(wireFlags)
		.arg(from->attachedToTitle()).arg(from->connectorStuffID())
		.arg(to->attachedToTitle()).arg(to->connectorStuffID())
		.arg(m_viewIdentifier)
		);

	new AddItemCommand(this, crossViewType, Wire::moduleIDName, viewGeometry, newID, parentCommand, false);
	new ChangeConnectionCommand(this, crossViewType, from->attachedToID(), from->connectorStuffID(),
			newID, "connector0", true, true, false, parentCommand);
	new ChangeConnectionCommand(this, crossViewType, to->attachedToID(), to->connectorStuffID(),
			newID, "connector1", true, true, false, parentCommand);

	if (addItNow) {
		ItemBase * newItemBase = addItemAux(m_paletteModel->retrieveModelPart(Wire::moduleIDName), viewGeometry, newID, NULL, true);
		if (newItemBase) {
			tempConnectWire(newItemBase, from, to);
			m_temporaries.append(newItemBase);
		}
	}

	return newID;
}


void SketchWidget::moveItem(long id, ViewGeometry & viewGeometry) {
	ItemBase * pitem = findItem(id);
	if (pitem != NULL) {
		if (pitem != NULL) {
			pitem->moveItem(viewGeometry);
		}
	}
}

void SketchWidget::rotateItem(long id, qreal degrees) {
	DebugDialog::debug(QObject::tr("rotating %1 %2").arg(id).arg(degrees) );

	if (!isVisible()) return;

	ItemBase * pitem = findItem(id);
	if (pitem != NULL) {
		PaletteItem * paletteItem = dynamic_cast<PaletteItem *>(pitem);
		if (paletteItem != NULL) {
			paletteItem->rotateItemAnd(degrees);
		}
	}

}

void SketchWidget::flipItem(long id, Qt::Orientations orientation) {
	DebugDialog::debug(QObject::tr("fliping %1 %2").arg(id).arg(orientation) );

	if (!isVisible()) return;

	 ItemBase * pitem = findItem(id);
	if (pitem != NULL) {
		PaletteItem * paletteItem = dynamic_cast<PaletteItem *>(pitem);
		if (paletteItem != NULL) {
			paletteItem->flipItemAnd(orientation);
		}
	}
}


void SketchWidget::changeWire(long fromID, QLineF line, QPointF pos, bool useLine)
{
	ItemBase * fromItem = findItem(fromID);
	if (fromItem == NULL) return;

	Wire* wire = dynamic_cast<Wire *>(fromItem);
	if (wire == NULL) return;

	wire->setLineAnd(line, pos, useLine);
}

void SketchWidget::selectItem(long id, bool state, bool updateInfoView) {
	this->clearHoldingSelectItem();
	ItemBase * item = findItem(id);
	if (item != NULL) {
		item->setSelected(state);
		if(updateInfoView) {
			// show something in the info view, even if it's not selected
			Wire *wire = dynamic_cast<Wire*>(item);
			if(!wire) {
				InfoGraphicsView::viewItemInfo(item);
			} else {
				InfoGraphicsView::viewItemInfo(wire);
			}
		}
		emit itemSelectedSignal(id, state);
	}

	PaletteItem *pitem = dynamic_cast<PaletteItem*>(item);
	if(pitem) m_lastPaletteItemSelected = pitem;
}

void SketchWidget::selectDeselectAllCommand(bool state) {
	this->clearHoldingSelectItem();

	SelectItemCommand * cmd = stackSelectionState(false, NULL);
	cmd->setText(state ? tr("Select All") : tr("Deselect"));
	cmd->setSelectItemType( state ? SelectItemCommand::SelectAll : SelectItemCommand::DeselectAll );

	m_undoStack->push(cmd);

}

void SketchWidget::selectAllItems(bool state, bool doEmit) {
	foreach (QGraphicsItem * item, this->scene()->items()) {
		item->setSelected(state);
	}

	if (doEmit) {
		emit selectAllItemsSignal(state, false);
	}
}

void SketchWidget::cut() {
	copy();
	cutDeleteAux("Cut");
}

void SketchWidget::duplicate() {
	copy();
	pasteDuplicateAux(tr("Duplicate"));
}

void SketchWidget::copy() {
	QList<ItemBase *> bases;

	// sort them in z-order so the copies also appear in the same order
	sortSelectedByZ(bases);

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << bases.size();

 	for (int i = 0; i < bases.size(); ++i) {
 		ItemBase *base = bases.at(i);
 		ViewGeometry * viewGeometry = new ViewGeometry(base->getViewGeometry());
		QHash<ItemBase::ViewIdentifier, ViewGeometry * > geometryHash;
		geometryHash.insert(m_viewIdentifier, viewGeometry);
		emit copyItemSignal(base->id(), geometryHash);			// let the other views add their geometry
 		dataStream << base->modelPart()->moduleID() << (qint64) base->id();
		QList <ItemBase::ViewIdentifier> keys = geometryHash.keys();
		dataStream << (qint32) keys.count();
		for (int j = 0; j < keys.count(); j++) {
			ViewGeometry * vg = geometryHash.value(keys[j]);
			dataStream << (qint32) keys[j] << vg->loc() << vg->line() << vg->transform();
		}
		for (int j = 0; j < keys.count(); j++) {
			ViewGeometry * vg = geometryHash[keys[j]];
			delete vg;
		}
	}

	QMultiHash<ConnectorItem *, ConnectorItem *> allConnectorHash;

	// now save the connector info
	foreach (ItemBase * base, bases) {
		QMultiHash<ConnectorItem *, ConnectorItem *> connectorHash;
		base->collectConnectors(connectorHash, scene());
		QMultiHash<ConnectorItem *, ConnectorItem *> disconnectorHash;

		// first get the connectors from each part and remove the ones pointing outside the copied set of parts
		foreach (ConnectorItem * fromConnectorItem, connectorHash.uniqueKeys()) {
			DebugDialog::debug(QString("testing from: %1 %2 %3").arg(fromConnectorItem->attachedToID())
				.arg(fromConnectorItem->attachedToTitle())
				.arg(fromConnectorItem->connectorStuffID()) );
			foreach (ConnectorItem * toConnectorItem, connectorHash.values(fromConnectorItem)) {
				DebugDialog::debug(QString("testing to: %1 %2 %3").arg(toConnectorItem->attachedToID())
					.arg(toConnectorItem->attachedToTitle())
					.arg(toConnectorItem->connectorStuffID()) );
				ItemBase * toBase = toConnectorItem->attachedTo()->layerKinChief();
				if (!bases.contains(toBase)) {
					// don't copy external connection--but don't delete during walkthrough
					disconnectorHash.insert(fromConnectorItem, toConnectorItem);
					DebugDialog::debug("deleting");
				}
			}
		}

		// delete now so that connectorHash isn't modified as we're walking through it  
		foreach (ConnectorItem * fromConnectorItem, disconnectorHash.uniqueKeys()) {
			foreach (ConnectorItem * toConnectorItem, disconnectorHash.values(fromConnectorItem)) {
				connectorHash.remove(fromConnectorItem, toConnectorItem);
			}
		}

		foreach (ConnectorItem * fromConnectorItem, connectorHash.uniqueKeys()) {
			foreach (ConnectorItem * toConnectorItem, connectorHash.values(fromConnectorItem)) {
				allConnectorHash.insert(fromConnectorItem, toConnectorItem);
			}
		}
    }

	// now shove the connection info into the dataStream
	dataStream << (int) allConnectorHash.count();
	DebugDialog::debug(QString("copying %1").arg(allConnectorHash.count()) );
	foreach (ConnectorItem * fromConnectorItem,  allConnectorHash.uniqueKeys()) {
		foreach (ConnectorItem * toConnectorItem, allConnectorHash.values(fromConnectorItem)) {
			dataStream << (qint64) fromConnectorItem->attachedTo()->layerKinChief()->id();
			dataStream << fromConnectorItem->connectorStuffID();
			dataStream << (qint64) toConnectorItem->attachedTo()->layerKinChief()->id();
			dataStream << toConnectorItem->connectorStuffID();

			DebugDialog::debug(QString("copying %1 %2 %3 %4").arg(fromConnectorItem->attachedTo()->layerKinChief()->id())
			.arg(fromConnectorItem->connectorStuffID())
			.arg( toConnectorItem->attachedTo()->layerKinChief()->id())
			.arg(toConnectorItem->connectorStuffID()) );
		}
	}

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dnditemsdata", itemData);
	QClipboard *clipboard = QApplication::clipboard();
	if (clipboard == NULL) {
		// shouldn't happen
		delete mimeData;
		return;
	}

	clipboard->setMimeData(mimeData, QClipboard::Clipboard);
}
void SketchWidget::paste() {
	pasteDuplicateAux(tr("Paste"));
}

void SketchWidget::pasteDuplicateAux(QString undoStackMessage) {
	clearHoldingSelectItem();

	QClipboard *clipboard = QApplication::clipboard();
	if (clipboard == NULL) {
		// shouldn't happen
		return;
	}

	const QMimeData* mimeData = clipboard->mimeData(QClipboard::Clipboard);
	if (mimeData == NULL) return;

   	if (!mimeData->hasFormat("application/x-dnditemsdata")) return;

    QByteArray itemData = mimeData->data("application/x-dnditemsdata");
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);
    int size;
    dataStream >> size;
	qint64 dataStreamPos = dataStream.device()->pos();					// remember where we are so we can reset back

	QString messageStr;
	if (size == 1) {
		QString moduleID;
		dataStream >> moduleID;
		bool reset = dataStream.device()->seek(dataStreamPos);			// reset the datastream so the next loop can start reading from there
		if (!reset) {
			// this shouldn't happen
			DebugDialog::debug("reset datastream didn't happen, bailing out");
			return;
		}

		ModelPart * modelPart = m_paletteModel->retrieveModelPart(moduleID);
		messageStr = tr("%1 %2").arg(undoStackMessage).arg(modelPart->title());
	}
	else {
		messageStr = tr("%1 %2 items").arg(undoStackMessage).arg(QString::number(size));
	}

	QUndoCommand * parentCommand = new QUndoCommand(messageStr);
	new CleanUpWiresCommand(this, false, parentCommand);

    stackSelectionState(false, parentCommand);

	emit addingSignal(this, parentCommand);

	QHash<long, long> mapIDs;
    for (int i = 0; i < size; i++) {
		QString moduleID;
		qint64 itemID;
		ModelPart* modelPart = NULL;
		qint32 geometryCount;

		dataStream >> moduleID >> itemID >> geometryCount;
		long newItemID = ItemBase::getNextID();
		mapIDs.insert(itemID, newItemID);
		modelPart = m_paletteModel->retrieveModelPart(moduleID);
		for (int j = 0; j < geometryCount; j++) {
			QTransform transform;
			QPointF loc;
			QLineF line;
    		ViewGeometry viewGeometry;
			qint32 viewIdentifier;

			dataStream >> viewIdentifier >> loc >> line >> transform;
    		viewGeometry.setLoc(loc);
    		viewGeometry.setLine(line);
			viewGeometry.setTransform(transform);
    		viewGeometry.offset(20*m_pasteCount, 20*m_pasteCount);

			SketchWidget * sketchWidget = NULL;
			emit findSketchWidgetSignal((ItemBase::ViewIdentifier) viewIdentifier, sketchWidget);
			if (sketchWidget != NULL) {
				new AddItemCommand(sketchWidget, BaseCommand::SingleView, modelPart->moduleID(), viewGeometry, newItemID, parentCommand, false);
			}
		}
   	}
    m_pasteCount++;				// used for offsetting paste items, not a count of how many items are pasted


	// now deal with interconnections between the copied parts
	for (int i = 0; i < size; i++) {
		int connectionCount;
		dataStream >> connectionCount;
		DebugDialog::debug(QString("pasting connections %1").arg(connectionCount) );
		for (int j = 0; j < connectionCount; j++) {
			qint64 fromID;
			QString fromConnectorID;
			QString toConnectorID;
			qint64 toID;
			dataStream >> fromID >> fromConnectorID  >> toID >> toConnectorID;
			DebugDialog::debug(tr("pasting %1 %2 %3 %4").arg(fromID).arg(fromConnectorID).arg(toID).arg(toConnectorID) );
			fromID = mapIDs.value(fromID);
			toID = mapIDs.value(toID);
			new ChangeConnectionCommand(this, BaseCommand::CrossView,
										fromID, fromConnectorID,
										toID, toConnectorID,
										true, true, false, parentCommand);

			//extendChangeConnectionCommand(fromID, fromConnectorID,
			//							  toID, toConnectorID,
			//							  true, true, parentCommand);
		}
	}


	foreach (long oid, mapIDs.keys()) {
		DebugDialog::debug(QString("map from: %1 to: %2").arg(oid).arg(mapIDs.value(oid)) );
	}

	clearTemporaries();

	new CleanUpWiresCommand(this, true, parentCommand);
   	m_undoStack->push(parentCommand);
}


void SketchWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if (dragEnterEventAux(event)) {
		event->acceptProposedAction();
	}
	else {
		// subclass seems to call acceptProposedAction so don't invoke it
		//QGraphicsView::dragEnterEvent(event);
	}
}

bool SketchWidget::dragEnterEventAux(QDragEnterEvent *event) {
	if (!event->mimeData()->hasFormat("application/x-dnditemdata")) return false;
	if (event->source() == this) return false;

	m_droppingWire = false;
    QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);

    QString moduleID;
    QPointF offset;
    dataStream >> moduleID >> offset;

	ModelPart * modelPart = m_paletteModel->retrieveModelPart(moduleID);
	if (modelPart ==  NULL) return false;

	if (!canDropModelPart(modelPart)) return false;

	m_droppingWire = (modelPart->itemType() == ModelPart::Wire);
	m_droppingOffset = offset;

	if (ItemDrag::_cache().contains(this)) {
		m_droppingItem->setVisible(true);
	}
	else {
		ViewGeometry viewGeometry;
		QPointF p = QPointF(this->mapToScene(event->pos())) - offset;
		viewGeometry.setLoc(p);

		long fromID = ItemBase::getNextID();

		// create temporary item
		// don't need connectors for breadboard
		// could live without them for arduino as well
		// TODO: how to specify which parts don't need connectors during drag and drop from palette?
		m_droppingItem = addItemAux(modelPart, viewGeometry, fromID, NULL, modelPart->itemType() != ModelPart::Breadboard);

		ItemDrag::_cache().insert(this, m_droppingItem);
		//m_droppingItem->setCacheMode(QGraphicsItem::ItemCoordinateCache);
		connect(ItemDrag::_itemDrag(), SIGNAL(dragIsDoneSignal(ItemDrag *)), this, SLOT(dragIsDoneSlot(ItemDrag *)));
	}
	//ItemDrag::_setPixmapVisible(false);


// make sure relevant layer is visible
	ViewLayer::ViewLayerID viewLayerID;
	if (m_droppingWire) {
		viewLayerID = getWireViewLayerID(m_droppingItem->getViewGeometry());
	}
	else if(modelPart->tags().contains("ruler",Qt::CaseInsensitive)) {
		viewLayerID = getRulerViewLayerID();
	}
	else {
		viewLayerID = getPartViewLayerID();
	}

	ensureLayerVisible(viewLayerID);
	return true;
}

bool SketchWidget::canDropModelPart(ModelPart * modelPart) {
	Q_UNUSED(modelPart);
	return true;
}

void SketchWidget::dragLeaveEvent(QDragLeaveEvent * event) {
	if (m_droppingItem != NULL) {
		m_droppingItem->setVisible(false);
		//ItemDrag::_setPixmapVisible(true);
	}

	QGraphicsView::dragLeaveEvent(event);
}

void SketchWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata") && event->source() != this) {
    	dragMoveHighlightConnector(event);

        event->acceptProposedAction();
        return;
    }

	QGraphicsView::dragMoveEvent(event);
}

void SketchWidget::dragMoveHighlightConnector(QDragMoveEvent *event) {
	if (m_droppingItem == NULL) return;
	QPointF loc = this->mapToScene(event->pos()) - m_droppingOffset;
	m_droppingItem->setItemPos(loc);
	m_droppingItem->findConnectorsUnder();
}


void SketchWidget::dropEvent(QDropEvent *event)
{
	clearHoldingSelectItem();

	if (m_droppingItem == NULL) return;

    if (event->mimeData()->hasFormat("application/x-dnditemdata") && event->source() != this) {

    	ModelPart * modelPart = m_droppingItem->modelPart();
    	if (modelPart == NULL) return;
    	if (modelPart->modelPartStuff() == NULL) return;

		QUndoCommand* parentCommand = new QUndoCommand(tr("Add %1").arg(modelPart->title()));
		new CleanUpWiresCommand(this, false, parentCommand);
    	stackSelectionState(false, parentCommand);

		emit addingSignal(this, parentCommand);

		m_droppingItem->saveGeometry();
    	ViewGeometry viewGeometry = m_droppingItem->getViewGeometry();

		long fromID = m_droppingItem->id();

		BaseCommand::CrossViewType crossViewType = BaseCommand::CrossView;
		if (modelPart->properties().values().contains(QString("ruler"))) { // TODO Mariano: add case-insensitiveness
			// rulers are local to a particular view
			crossViewType = BaseCommand::SingleView;
		}
		new AddItemCommand(this, crossViewType, modelPart->moduleID(), viewGeometry, fromID, parentCommand);

		bool gotConnector = false;
		foreach (QGraphicsItem * childItem, m_droppingItem->childItems()) {
			ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
			if (connectorItem == NULL) continue;

			ConnectorItem * to = connectorItem->overConnectorItem();
			if (to != NULL) {
				to->connectorHover(to->attachedTo(), false);
				connectorItem->setOverConnectorItem(NULL);   // clean up
				extendChangeConnectionCommand(connectorItem, to, true, false, parentCommand);
				gotConnector = true;
			}
		}

		clearTemporaries();

		killDroppingItem();

		new CleanUpWiresCommand(this, true, parentCommand);
        m_undoStack->waitPush(parentCommand, 10);

        event->acceptProposedAction();
		DebugDialog::debug("after drop event");
    }
	else {
		QGraphicsView::dropEvent(event);
	}

}

void SketchWidget::viewItemInfo(long id) {
	ItemBase *item = findItem(id);
	if (item == NULL) return;

	m_lastPaletteItemSelected = dynamic_cast<PaletteItem*>(item);
	InfoGraphicsView::viewItemInfo(item);
}

SelectItemCommand* SketchWidget::stackSelectionState(bool pushIt, QUndoCommand * parentCommand) {

	// if pushIt assumes m_undoStack->beginMacro has previously been called

	DebugDialog::debug(QString("stack selection state %1 %2").arg(pushIt).arg((long) parentCommand));
	SelectItemCommand* selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
	const QList<QGraphicsItem *> sitems = scene()->selectedItems();
 	for (int i = 0; i < sitems.size(); ++i) {
 		ItemBase * base = ItemBase::extractTopLevelItemBase(sitems.at(i));
 		if (base == NULL) continue;

 		 selectItemCommand->addUndo(base->id());
    }

    if (pushIt) {
     	m_undoStack->push(selectItemCommand);
    }

    return selectItemCommand;
}

void SketchWidget::mousePressEvent(QMouseEvent *event) {

	//setRenderHint(QPainter::Antialiasing, false);

	clearHoldingSelectItem();
	m_savedItems.clear();
	m_savedWires.clear();
	m_needToConnectItems.clear();
	m_moveEventCount = 0;
	m_holdingSelectItemCommand = stackSelectionState(false, NULL);
	m_mousePressScenePos = mapToScene(event->pos());

	QGraphicsItem* item = this->itemAt(event->pos());

	QGraphicsView::mousePressEvent(event);
	if (item == NULL) {
		return;
	}

	QSet<Wire *> wires;
	foreach (QGraphicsItem * gitem,  this->scene()->selectedItems ()) {
		ItemBase *itemBase = ItemBase::extractItemBase(gitem);
		if (itemBase == NULL) continue;

		if (itemBase->itemType() == ModelPart::Wire) {
			if (itemBase->isVisible()) {
				wires.insert(dynamic_cast<Wire *>(itemBase));
			}
			continue;
		}

		ItemBase * chief = itemBase->layerKinChief();
		m_savedItems.insert(chief);
		if (chief->sticky()) {
			foreach(ItemBase * sitemBase, chief->sticking().keys()) {
				if (sitemBase->isVisible()) {
					if (sitemBase->itemType() == ModelPart::Wire) {
						wires.insert(dynamic_cast<Wire *>(sitemBase));
					}
					else {
						m_savedItems.insert(sitemBase);
					}
				}
			}
		}

		PaletteItem * paletteItem = dynamic_cast<PaletteItem *>(chief);
		collectFemaleConnectees(paletteItem);
		paletteItem->collectWireConnectees(wires);
	}

	/*
	m_disconnectors.clear();
	if (m_viewIdentifier == ItemBase::BreadboardView) {
		foreach (ItemBase * item, m_savedItems) {
			collectDisconnectors(item);
		}

		foreach (ConnectorItem * connectorItem, m_disconnectors.keys()) {
			connectorItem->connectorHover(NULL, true);
			m_disconnectors.value(connectorItem)->connectorHover(NULL, true);
		}
	}
*/

	foreach (Wire * wire, wires) {
		if (m_savedItems.contains(wire)) continue;

		wire->connectsWithin(m_savedItems, m_savedWires);
	}

	foreach (ItemBase * itemBase, m_savedItems) {
		itemBase->saveGeometry();
	}

	foreach (Wire * wire, wires) {
		wire->saveGeometry();
	}

	m_autoScrollX = m_autoScrollY = 0;
	connect(&m_autoScrollTimer, SIGNAL(timeout()), this, SLOT(autoScrollTimeout()));

	// do something with wires--chained, wires within, wires without
	// don't forget about checking connections-to-be

}

void SketchWidget::collectFemaleConnectees(PaletteItem * paletteItem) {
	Q_UNUSED(paletteItem);
}

bool SketchWidget::draggingWireEnd() {
	if (m_connectorDragWire != NULL) return true;

	Wire * wire = dynamic_cast<Wire *>( scene()->mouseGrabberItem());
	if (wire == NULL) return false;

	return wire->draggingEnd();
}

void SketchWidget::mouseMoveEvent(QMouseEvent *event) {
	// if its just dragging a wire end do default
	// otherwise handle all move action here

	if (m_savedItems.count() > 0) {
		if ((event->buttons() & Qt::LeftButton) && !draggingWireEnd()) {
			m_globalPos = event->globalPos();
			moveItems(event->globalPos());
		}
	}

	m_moveEventCount++;
	QGraphicsView::mouseMoveEvent(event);
}

void SketchWidget::moveItems(QPoint globalPos) {

	QRect r = rect();
	QPoint q = mapFromGlobal(globalPos);

	if (verticalScrollBar()->isVisible()) {
		r.setWidth(width() - verticalScrollBar()->width());
	}

	if (horizontalScrollBar()->isVisible()) {
		r.setHeight(height() - horizontalScrollBar()->height());
	}

	if (!r.contains(q)) {
		m_autoScrollX = m_autoScrollY = 0;
		return;
	}

	//DebugDialog::debug(QString("move items %1").arg(QTime::currentTime().msec()) );

	r.adjust(16,16,-16,-16);						// these should be set someplace
	bool autoScroll = !r.contains(q);
	if (autoScroll) {
		int dx = 0, dy = 0;
		if (q.x() > r.right()) {
			dx = q.x() - r.right();
		}
		else if (q.x() < r.left()) {
			dx = q.x() - r.left();
		}
		if (q.y() > r.bottom()) {
			dy = q.y() - r.bottom();
		}
		else if (q.y() < r.top()) {
			dy = q.y() - r.top();
		}

		int div = 3;
		if (dx != 0) {
			m_autoScrollX = (dx + ((dx > 0) ? div : -div)) / (div + 1);		// ((m_autoScrollX > 0) ? 1 : -1)
		}
		if (dy != 0) {
			m_autoScrollY = (dy + ((dy > 0) ? div : -div)) / (div + 1);		// ((m_autoScrollY > 0) ? 1 : -1)
		}

		if (!m_autoScrollTimer.isActive()) {
			m_autoScrollTimer.start(10);
		}

	}
	else {
		m_autoScrollX = m_autoScrollY = 0;
	}

	QPointF scenePos = mapToScene(q);

/*
	DebugDialog::debug(QString("scroll 1 sx:%1 sy:%2 sbx:%3 sby:%4 qx:%5 qy:%6")
		.arg(scenePos.x()).arg(scenePos.y())
		.arg(m_mousePressScenePos.x()).arg(m_mousePressScenePos.y())
		.arg(q.x()).arg(q.y())
		);
*/
	foreach (ItemBase * item, m_savedItems) {
       QPointF currentParentPos = item->mapToParent(item->mapFromScene(scenePos));
       QPointF buttonDownParentPos = item->mapToParent(item->mapFromScene(m_mousePressScenePos));
       item->setPos(item->getViewGeometry().loc() + currentParentPos - buttonDownParentPos);

	   findConnectorsUnder(item);

/*
	   DebugDialog::debug(QString("scroll 2 lx:%1 ly:%2 cpx:%3 cpy:%4 qx:%5 qy:%6 px:%7 py:%8")
		.arg(item->getViewGeometry().loc().x()).arg(item->getViewGeometry().loc().y())
		.arg(currentParentPos.x()).arg(currentParentPos.y())
		.arg(buttonDownParentPos.x()).arg(buttonDownParentPos.y())
		.arg(item->pos().x()).arg(item->pos().y())
		);
*/

	}

	foreach (Wire * wire, m_savedWires.keys()) {
		wire->simpleConnectedMoved(m_savedWires.value(wire));
	}

	//DebugDialog::debug(QString("done move items %1").arg(QTime::currentTime().msec()) );

}


void SketchWidget::findConnectorsUnder(ItemBase * item) {
	Q_UNUSED(item);
}

void SketchWidget::mouseReleaseEvent(QMouseEvent *event) {
	//setRenderHint(QPainter::Antialiasing, true);

	m_autoScrollTimer.stop();
	disconnect(&m_autoScrollTimer, SIGNAL(timeout()), this, SLOT(autoScrollTimeout()));
	//restoreDisconnectors();
	QGraphicsView::mouseReleaseEvent(event);

	if (m_connectorDragWire != NULL) {
		m_connectorDragWire->ungrabMouse();

		// remove again (may not have been removed earlier)
		if (m_connectorDragWire->scene() != NULL) {
			this->scene()->removeItem(m_connectorDragWire);
			m_infoView->unregisterCurrentItem();

		}

		DebugDialog::debug("deleting connector drag wire");
		delete m_connectorDragWire;
		m_connectorDragWire = NULL;
		m_savedItems.clear();
		return;
	}

	if ((m_savedItems.size() <= 0) || !checkMoved()) {
		if (this->m_holdingSelectItemCommand != NULL) {
			SelectItemCommand* tempCommand = m_holdingSelectItemCommand;
			m_holdingSelectItemCommand = NULL;
			//DebugDialog::debug("scene changed push select");
			m_undoStack->push(tempCommand);
		}
	}
	m_savedItems.clear();
}

bool SketchWidget::checkMoved()
{
	if (m_moveEventCount == 0) {
		return false;
	}

	int moveCount = m_savedItems.count();
	if (moveCount <= 0) {
		return false;
	}

	ItemBase * saveBase = NULL;
	foreach (ItemBase * item, m_savedItems) {
		saveBase = item;
		break;
	}

	clearHoldingSelectItem();

	QString moveString;
	QString viewName = ItemBase::viewIdentifierName(m_viewIdentifier);

	if (moveCount == 1) {
		moveString = tr("Move %2 (%1)").arg(viewName).arg(saveBase->modelPart()->title());
	}
	else {
		moveString = tr("Move %2 items (%1)").arg(viewName).arg(QString::number(moveCount));
	}

	QUndoCommand * parentCommand = new QUndoCommand(moveString);
	new CleanUpWiresCommand(this, false, parentCommand);

	bool hasBoard = false;

	foreach (ItemBase * item, m_savedItems) {
		if (item == NULL) continue;

		ViewGeometry viewGeometry(item->getViewGeometry());
		item->saveGeometry();

		new MoveItemCommand(this, item->id(), viewGeometry, item->getViewGeometry(), parentCommand);

		if (item->itemType() == ModelPart::Breadboard) {
			hasBoard = true;
			continue;
		}

		// TODO: boardtypes and breadboard types are always sticky
		if (item->itemType() == ModelPart::Board) {
			hasBoard = true;
		}

		checkSticky(item, parentCommand);

		if (item->itemType() == ModelPart::Wire) continue;

		disconnectFromFemale(item, m_savedItems, parentCommand);
	}

	if (!hasBoard) {
		// TODO: make a cleaner distinction if moving muliple parts (remember, this is for removing routing)
		emit movingSignal(this, parentCommand);
	}

	QList<ConnectorItem *> keys = m_needToConnectItems.keys();
	for (int i = 0; i < keys.count(); i++) {
		ConnectorItem * from = keys[i];
		if (from == NULL) continue;

		ConnectorItem * to = m_needToConnectItems[from];
		if (to == NULL) continue;

		extendChangeConnectionCommand(from, to, true, false, parentCommand);
	}

	clearTemporaries();

	m_needToConnectItems.clear();
	new CleanUpWiresCommand(this, true, parentCommand);
	m_undoStack->push(parentCommand);

	return true;
}

void SketchWidget::relativeZoom(qreal step) {
	qreal tempSize = m_scaleValue + step;
	if (tempSize < m_minScaleValue) {
		m_scaleValue = m_minScaleValue;
		emit zoomOutOfRange(m_scaleValue);
		return;
	}
	if (tempSize > m_maxScaleValue) {
		m_scaleValue = m_maxScaleValue;
		emit zoomOutOfRange(m_scaleValue);
		return;
	}
	qreal tempScaleValue = tempSize/100;

	m_scaleValue = tempSize;

	QMatrix matrix;
	//matrix.translate((width() / 2) - mousePosition.x(), (height() / 2) -  mousePosition.y());
	//qreal scale = qPow(qreal(2), (m_scaleValue-50) / qreal(50));
	matrix.scale(tempScaleValue, tempScaleValue);
	//matrix.translate(mousePosition.x() - (width() / 2), mousePosition.y() - (height() / 2));
	this->setMatrix(matrix);

	emit zoomChanged(m_scaleValue);
}

void SketchWidget::absoluteZoom(qreal percent) {
	relativeZoom(percent-m_scaleValue);
}

qreal SketchWidget::currentZoom() {
	return m_scaleValue;
}

void SketchWidget::wheelEvent(QWheelEvent* event) {
	QPointF mousePosition = event->pos();
	qreal delta = ((qreal)event->delta() / 120) * ZoomComboBox::ZoomStep;
	if (delta == 0) return;

	// Scroll zooming throw the combobox options
	/*if(delta < 0) {
		emit zoomOut(-1*delta);
	} else {
		emit zoomIn(delta);
	}*/

	// Scroll zooming relative to the current size
	relativeZoom(delta);

	//this->verticalScrollBar()->setValue(verticalScrollBar()->value() + 3);
	//this->horizontalScrollBar()->setValue(horizontalScrollBar()->value() + 3);


	//to do: center zoom around mouse location



	//QPointF pos = event->pos();
	//QPointF spos = this->mapToScene((int) pos.x(), (int) pos.y());


	//DebugDialog::debug(QObject::tr("translate %1 %2").arg(spos.x()).arg(spos.y()) );

}


void SketchWidget::setPaletteModel(PaletteModel * paletteModel) {
	m_paletteModel = paletteModel;
}

void SketchWidget::setRefModel(ReferenceModel *refModel) {
	m_refModel = refModel;
}

void SketchWidget::setSketchModel(SketchModel * sketchModel) {
	m_sketchModel = sketchModel;
}

void SketchWidget::sketchWidget_itemAdded(ModelPart * modelPart, const ViewGeometry & viewGeometry, long id) {
	addItemAux(modelPart, viewGeometry, id, NULL, true);
}

void SketchWidget::sketchWidget_itemDeleted(long id) {
	ItemBase * pitem = findItem(id);
	if (pitem != NULL) {
		DebugDialog::debug(QString("really deleting %1 %2").arg(id).arg(m_viewIdentifier) );
		pitem->removeLayerKin();
		this->scene()->removeItem(pitem);
		delete pitem;
	}
}

void SketchWidget::scene_selectionChanged() {
	if (m_ignoreSelectionChangeEvents) {
		return;
	}

	DebugDialog::debug("selection changed");
	// TODO: this can be dangerous if an item is on m_lastSelected and the item is deleted without being deselected first.

	// hack to make up for missing selection state updates
	foreach (QGraphicsItem * item, m_lastSelected) {
		item->update();
	}

	m_lastSelected = scene()->selectedItems();
	emit selectionChangedSignal();

	// hack to make up for missing selection state updates
	foreach (QGraphicsItem * item, m_lastSelected) {
		item->update();
	}

	if (m_holdingSelectItemCommand != NULL) {
		DebugDialog::debug("got holding command");

		int selCount = 0;
		ItemBase* saveBase = NULL;
		QString selString;
		m_holdingSelectItemCommand->clearRedo();
		const QList<QGraphicsItem *> sitems = scene()->selectedItems();
		foreach (QGraphicsItem * item, scene()->selectedItems()) {
	 		ItemBase * base = ItemBase::extractItemBase(item);
	 		if (base == NULL) continue;

			saveBase = base;
	 		m_holdingSelectItemCommand->addRedo(base->layerKinChief()->id());
	 		selCount++;
	    }
		if (selCount == 1) {
			selString = tr("Select %1").arg(saveBase->modelPart()->title());
		}
		else {
			selString = tr("Select %1 items").arg(QString::number(selCount));
		}
		m_holdingSelectItemCommand->setText(selString);
	}
}

void SketchWidget::clearHoldingSelectItem() {
	DebugDialog::debug("clear holding");
	if (m_holdingSelectItemCommand != NULL) {
		delete m_holdingSelectItemCommand;
		m_holdingSelectItemCommand = NULL;
	}
}

void SketchWidget::clearSelection() {
	this->scene()->clearSelection();
	emit clearSelectionSignal();
}

void SketchWidget::sketchWidget_clearSelection() {
	this->scene()->clearSelection();
}

void SketchWidget::sketchWidget_itemSelected(long id, bool state) {
	ItemBase * item = findItem(id);
	if (item != NULL) {
		item->setSelected(state);
	}

	PaletteItem *pitem = dynamic_cast<PaletteItem*>(item);
	if(pitem) m_lastPaletteItemSelected = pitem;
}

void SketchWidget::sketchWidget_tooltipAppliedToItem(long id, const QString& text) {
	ItemBase * pitem = findItem(id);
	if (pitem != NULL) {
		pitem->setInstanceTitleAndTooltip(text);
	}
}


void SketchWidget::group() {
	const QList<QGraphicsItem *> sitems = scene()->selectedItems();
	if (sitems.size() < 2) return;
}

void SketchWidget::wire_wireChanged(Wire* wire, QLineF oldLine, QLineF newLine, QPointF oldPos, QPointF newPos, ConnectorItem * from, ConnectorItem * to) {
	this->clearHoldingSelectItem();
	this->m_moveEventCount = 0;  // clear this so an extra MoveItemCommand isn't posted

	// TODO: make sure all these pointers to pointers to pointers aren't null...

	if (wire == this->m_connectorDragWire) {
		dragWireChanged(wire, from, to);
		return;
	}

	clearDragWireTempCommand();
	QUndoCommand * parentCommand = new QUndoCommand();
	new CleanUpWiresCommand(this, false, parentCommand);

	long fromID = wire->id();

	QString fromConnectorID;
	if (from != NULL) {
		fromConnectorID = from->connectorStuffID();
	}

	long toID = -1;
	QString toConnectorID;
	if (to != NULL) {
		toID = to->attachedToID();
		toConnectorID = to->connectorStuffID();
	}

	new ChangeWireCommand(this, fromID, oldLine, newLine, oldPos, newPos, true);

	checkSticky(wire, parentCommand);

	QList<ConnectorItem *> former = from->connectedToItems();

	QString prefix;
	QString suffix;
	if (to == NULL) {
		if (former.count() > 0  && !from->chained()) {
			prefix = tr("Disconnect");
			// the suffix is a little tricky to determine
			// it might be multiple disconnects, or might be disconnecting a virtual wire, in which case, the
			// title needs to come from the virtual wire's other connection's attachedTo()

			// suffix = tr("from %1").arg(former->attachedToTitle());
		}
		else {
			prefix = tr("Change");
		}
	}
	else {
		prefix = tr("Connect");
		suffix = tr("to %1").arg(to->attachedTo()->modelPart()->title());
	}

	parentCommand->setText(QObject::tr("%1 %2 %3").arg(prefix).arg(wire->modelPart()->title()).arg(suffix) );

	if (from->chained()) {
		foreach (ConnectorItem * toConnectorItem, from->connectedToItems()) {
			Wire * toWire = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
			if (toWire == NULL) continue;

			ViewGeometry vg = toWire->getViewGeometry();
			toWire->saveGeometry();
			ViewGeometry vg2 = toWire->getViewGeometry();
			new ChangeWireCommand(this, toWire->id(), vg.line(), vg2.line(), vg.loc(), vg2.loc(), true, parentCommand);
		}
	}
	else {
		if (former.count() > 0) {
			QList<ConnectorItem *> connectorItems;
			connectorItems.append(from);
			ConnectorItem::collectEqualPotential(connectorItems);
			QSet<VirtualWire *> virtualWires;

			foreach (ConnectorItem * formerConnectorItem, former) {
				if (formerConnectorItem->attachedTo()->getVirtual()) {
					virtualWires.insert(dynamic_cast<VirtualWire *>(formerConnectorItem->attachedTo()));
				}
				else {
					// temp remove here, virtual wires handle temp removes
					extendChangeConnectionCommand(from, formerConnectorItem, false, false, parentCommand);
					from->tempRemove(formerConnectorItem);
					formerConnectorItem->tempRemove(from);
				}

			}

		}
		if (to != NULL) {
			extendChangeConnectionCommand(from, to, true, false, parentCommand);
		}
	}


	clearTemporaries();

	new CleanUpWiresCommand(this, true, parentCommand);
	m_undoStack->push(parentCommand);
}

void SketchWidget::dragWireChanged(Wire* wire, ConnectorItem * from, ConnectorItem * to)
{
	QUndoCommand * parentCommand = new QUndoCommand();

	SelectItemCommand * selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
	if (m_tempDragWireCommand != NULL) {
		selectItemCommand->copyUndo(m_tempDragWireCommand);
		clearDragWireTempCommand();
	}

	new CleanUpWiresCommand(this, false, parentCommand);

	m_connectorDragWire->saveGeometry();
	long fromID = wire->id();

	DebugDialog::debug(tr("m_connectorDragConnector:%1 %4 from:%2 to:%3")
					   .arg(m_connectorDragConnector->connectorStuffID())
					   .arg(from->connectorStuffID())
					   .arg((to == NULL) ? "null" : to->connectorStuffID())
					   .arg(m_connectorDragConnector->attachedTo()->modelPart()->title()) );

	parentCommand->setText(tr("Create and connect wire"));

	// create a new wire with the same id as the temporary wire
	new AddItemCommand(this, BaseCommand::CrossView, m_connectorDragWire->modelPart()->moduleID(), m_connectorDragWire->getViewGeometry(), fromID, parentCommand);


	ConnectorItem * anchor = wire->otherConnector(from);
	if (anchor != NULL) {
		extendChangeConnectionCommand(anchor, m_connectorDragConnector, true, false, parentCommand);
	}
	if (to != NULL) {
		// since both wire connections are being newly created, set up the anchor connection temporarily
		// the other connection is created temporarily in extendChangeConnectionCommand
		m_connectorDragConnector->tempConnectTo(anchor);
		anchor->tempConnectTo(m_connectorDragConnector);
		extendChangeConnectionCommand(from, to, true, false, parentCommand);
		m_connectorDragConnector->tempRemove(anchor);
		anchor->tempRemove(m_connectorDragConnector);

	}

	clearTemporaries();

	// remove the temporary one
	this->scene()->removeItem(m_connectorDragWire);

	new CleanUpWiresCommand(this, true, parentCommand);
	m_undoStack->push(parentCommand);

}

void SketchWidget::addViewLayer(ViewLayer * viewLayer) {
	m_viewLayers.insert(viewLayer->viewLayerID(), viewLayer);
	BetterTriggerViewLayerAction* action = new BetterTriggerViewLayerAction(QObject::tr("%1 Layer").arg(viewLayer->displayName()), viewLayer, this);
	action->setCheckable(true);
	action->setChecked(viewLayer->visible());
    connect(action, SIGNAL(betterTriggered(QAction*)), this, SLOT(toggleLayerVisibility(QAction*)));
    viewLayer->setAction(action);
}

void SketchWidget::updateLayerMenu(QMenu * layerMenu, QAction * showAllAction, QAction * hideAllAction) {
	QList<ViewLayer::ViewLayerID>keys = m_viewLayers.keys();

	// make sure they're in ascending order when inserting into the menu
	qSort(keys.begin(), keys.end());

	for (int i = 0; i < keys.count(); i++) {
		ViewLayer * viewLayer = m_viewLayers.value(keys[i]);
    	if (viewLayer != NULL) {
			layerMenu->addAction(viewLayer->action());
		}
	}
	// TODO: Fix this function, to update the show/hide all actions
	updateAllLayersActions(showAllAction, hideAllAction);
}

void SketchWidget::setAllLayersVisible(bool visible) {
	QList<ViewLayer::ViewLayerID>keys = m_viewLayers.keys();

	for (int i = 0; i < keys.count(); i++) {
		ViewLayer * viewLayer = m_viewLayers.value(keys[i]);
		if (viewLayer != NULL) {
			setLayerVisible(viewLayer, visible);
		}
	}
}

void SketchWidget::updateAllLayersActions(QAction * showAllAction, QAction * hideAllAction) {
	QList<ViewLayer::ViewLayerID>keys = m_viewLayers.keys();

	if(keys.count()>0) {
		ViewLayer *prev = m_viewLayers.value(keys[0]);
		if (prev == NULL) {
			// jrc: I think prev == NULL is actually a side effect from an earlier bug
			// but I haven't figured out the cause yet
			// at any rate, when this bug occurs, keys[0] is some big negative number that looks like an
			// uninitialized or scrambled layerID
			DebugDialog::debug(QString("updateAllLayersActions keys[0] failed %1").arg(keys[0]) );
			showAllAction->setEnabled(false);
			hideAllAction->setEnabled(false);
			return;
		}
		bool sameState = prev->action()->isChecked();
		bool checked = prev->action()->isChecked();
		//DebugDialog::debug(tr("Layer: %1 is %2").arg(prev->action()->text()).arg(prev->action()->isChecked()));
		for (int i = 1; i < keys.count(); i++) {
			ViewLayer *viewLayer = m_viewLayers.value(keys[i]);
			//DebugDialog::debug(tr("Layer: %1 is %2").arg(viewLayer->action()->text()).arg(viewLayer->action()->isChecked()));
			if (viewLayer != NULL) {
				if(prev != NULL && prev->action()->isChecked() != viewLayer->action()->isChecked() ) {
					// if the actions aren't all checked or unchecked I don't bother about the "checked" variable
					sameState = false;
					break;
				} else {
					sameState = true;
					checked = viewLayer->action()->isChecked();
				}
				prev = viewLayer;
			}
		}

		//DebugDialog::debug(tr("sameState: %1").arg(sameState));
		//DebugDialog::debug(tr("checked: %1").arg(checked));
		if(sameState) {
			if(checked) {
				showAllAction->setEnabled(false);
				hideAllAction->setEnabled(true);
			} else {
				showAllAction->setEnabled(true);
				hideAllAction->setEnabled(false);
			}
		} else {
			showAllAction->setEnabled(true);
			hideAllAction->setEnabled(true);
		}
	} else {
		showAllAction->setEnabled(false);
		hideAllAction->setEnabled(false);
	}
}

ItemCount SketchWidget::calcItemCount() {
	ItemCount itemCount;

	// TODO: replace scene()->items()
	QList<QGraphicsItem *> items = scene()->items();
	QList<QGraphicsItem *> selItems = scene()->selectedItems();

	itemCount.selCount = 0;
	itemCount.selHFlipable = itemCount.selVFlipable = itemCount.selRotatable = 0;
	itemCount.itemsCount = 0;

	for (int i = 0; i < selItems.count(); i++) {
		ItemBase * itemBase = ItemBase::extractTopLevelItemBase(selItems[i]);
		if (itemBase != NULL) {
			itemCount.selCount++;
			// can't rotate a wire
			if (dynamic_cast<PaletteItemBase *>(itemBase) != NULL) {

				// TODO: allow breadboard and ardiuno to rotate
				bool rotatable = true;
				if (itemBase->itemType() == ModelPart::Breadboard || itemBase->itemType() == ModelPart::Board) {
					rotatable = false;
				}

				if (rotatable) {
					itemCount.selRotatable++;
				}
				if (itemBase->canFlipHorizontal()) {
					itemCount.selHFlipable++;
				}
				if (itemBase->canFlipVertical()) {
					itemCount.selVFlipable++;
				}
			}
		}
	}

	if (itemCount.selCount != itemCount.selRotatable) {
		itemCount.selRotatable = 0;
	}
	if (itemCount.selCount != itemCount.selVFlipable) {
		itemCount.selVFlipable = 0;
	}
	if (itemCount.selCount != itemCount.selHFlipable) {
		itemCount.selHFlipable = 0;
	}
	if (itemCount.selCount > 0) {
		for (int i = 0; i < items.count(); i++) {
			if (ItemBase::extractTopLevelItemBase(items[i]) != NULL) {
				itemCount.itemsCount++;
			}
		}
	}

	return itemCount;
}

bool SketchWidget::layerIsVisible(ViewLayer::ViewLayerID viewLayerID) {
	ViewLayer * viewLayer = m_viewLayers[viewLayerID];
	if (viewLayer == NULL) return false;

	return viewLayer->visible();
}

void SketchWidget::setLayerVisible(ViewLayer::ViewLayerID viewLayerID, bool vis) {
	ViewLayer * viewLayer = m_viewLayers[viewLayerID];
	if (viewLayer) {
		setLayerVisible(viewLayer, vis);
	}
}

void SketchWidget::toggleLayerVisibility(QAction * action) {
	BetterTriggerViewLayerAction * btvla = dynamic_cast<BetterTriggerViewLayerAction *>(action);
	if (btvla == NULL) return;

	ViewLayer * viewLayer = btvla->viewLayer();
	if (viewLayer == NULL) return;

	setLayerVisible(viewLayer, !viewLayer->visible());
}

void SketchWidget::setLayerVisible(ViewLayer * viewLayer, bool visible) {
	viewLayer->setVisible(visible);

	// TODO: replace scene()->items()
	const QList<QGraphicsItem *> items = scene()->items();
	for (int i = 0; i < items.size(); i++) {
		// want all items, not just topLevel
		ItemBase * itemBase = ItemBase::extractItemBase(items[i]);
		if (itemBase == NULL) continue;

		if (itemBase->viewLayerID() == viewLayer->viewLayerID()) {
			itemBase->setHidden(!visible);
			DebugDialog::debug(QObject::tr("setting visible %1").arg(viewLayer->visible()) );
		}
	}
}

void SketchWidget::sendToBack() {
	QList<ItemBase *> bases;
	if (!startZChange(bases)) return;

	QString text = QObject::tr("Bring forward");
	continueZChangeMax(bases, bases.size() - 1, -1, greaterThan, -1, text);
}

void SketchWidget::sendBackward() {

	QList<ItemBase *> bases;
	if (!startZChange(bases)) return;

	QString text = QObject::tr("Send backward");
	continueZChange(bases, 0, bases.size(), lessThan, 1, text);
}

void SketchWidget::bringForward() {
	QList<ItemBase *> bases;
	if (!startZChange(bases)) return;

	QString text = QObject::tr("Bring forward");
	continueZChange(bases, bases.size() - 1, -1, greaterThan, -1, text);

}

void SketchWidget::bringToFront() {
	QList<ItemBase *> bases;
	if (!startZChange(bases)) return;

	QString text = QObject::tr("Bring to front");
	continueZChangeMax(bases, 0, bases.size(), lessThan, 1, text);

}

void SketchWidget::fitInWindow() {
	QRectF itemsRect = this->scene()->itemsBoundingRect();
	QRectF viewRect = rect();

	//fitInView(itemsRect.x(), itemsRect.y(), itemsRect.width(), itemsRect.height(), Qt::KeepAspectRatio);

	qreal wRelation = (viewRect.width()-this->verticalScrollBar()->width())  / itemsRect.width();
	qreal hRelation = (viewRect.height()-this->horizontalScrollBar()->height()) / itemsRect.height();

	DebugDialog::debug(tr("scen rect: w%1 h%2").arg(itemsRect.width()).arg(itemsRect.height()));
	DebugDialog::debug(tr("view rect: w%1 h%2").arg(viewRect.width()).arg(viewRect.height()));
	DebugDialog::debug(tr("relations (v/s): w%1 h%2").arg(wRelation).arg(hRelation));

	if(wRelation < hRelation) {
		m_scaleValue = (wRelation * 100);
	} else {
		m_scaleValue = (hRelation * 100);
	}

	// Actually the zoom hasn't changed...
	emit zoomChanged(--m_scaleValue);

	this->centerOn(itemsRect.center());
}

bool SketchWidget::startZChange(QList<ItemBase *> & bases) {
	int selCount = scene()->selectedItems().count();
	if (selCount <= 0) return false;

	const QList<QGraphicsItem *> items = scene()->items();
	if (items.count() <= selCount) return false;

	sortAnyByZ(items, bases);

	return true;
}

void SketchWidget::continueZChange(QList<ItemBase *> & bases, int start, int end, bool (*test)(int current, int start), int inc, const QString & text) {

	bool moved = false;
	int last = bases.size();
	for (int i = start; test(i, end); i += inc) {
		ItemBase * base = bases[i];

		if (!base->getViewGeometry().selected()) continue;

		int j = i - inc;
		if (j >= 0 && j < last && bases[j]->viewLayerID() == base->viewLayerID()) {
			bases.move(i, j);
			moved = true;
		}
	}

	if (!moved) {
		return;
	}

	continueZChangeAux(bases, text);
}

void SketchWidget::continueZChangeMax(QList<ItemBase *> & bases, int start, int end, bool (*test)(int current, int start), int inc, const QString & text) {

	QHash<ItemBase *, ItemBase *> marked;
	bool moved = false;
	int last = bases.size();
	for (int i = start; test(i, end); i += inc) {
		ItemBase * base = bases[i];
		if (!base->getViewGeometry().selected()) continue;
		if (marked[base] != NULL) continue;

		marked.insert(base, base);

		int dest = -1;
		for (int j = i + inc; j >= 0 && j < last && bases[j]->viewLayerID() == base->viewLayerID(); j += inc) {
			dest = j;
		}

		if (dest >= 0) {
			moved = true;
			bases.move(i, dest);
			DebugDialog::debug(QObject::tr("moving %1 to %2").arg(i).arg(dest));
			i -= inc;	// because we just modified the list and would miss the next item
		}
	}

	if (!moved) {
		return;
	}

	continueZChangeAux(bases, text);
}


void SketchWidget::continueZChangeAux(QList<ItemBase *> & bases, const QString & text) {

	ChangeZCommand * changeZCommand = new ChangeZCommand(this );

	ViewLayer::ViewLayerID lastViewLayerID = ViewLayer::UnknownLayer;
	qreal z = 0;
	for (int i = 0; i < bases.size(); i++) {
		qreal oldZ = bases[i]->getViewGeometry().z();
		if (bases[i]->viewLayerID() != lastViewLayerID) {
			lastViewLayerID = bases[i]->viewLayerID();
			z = floor(oldZ);
		}
		else {
			z += ViewLayer::getZIncrement();
		}


		if (oldZ == z) continue;

		// optimize this by only adding z's that must change
		// rather than changing all of them
		changeZCommand->addTriplet(bases[i]->id(), oldZ, z);
	}

	changeZCommand->setText(text);
	m_undoStack->push(changeZCommand);
}

void SketchWidget::sortSelectedByZ(QList<ItemBase *> & bases) {

	const QList<QGraphicsItem *> items = scene()->selectedItems();
	if (items.size() <= 0) return;

	QList<QGraphicsItem *> tlBases;
	for (int i = 0; i < items.count(); i++) {
		ItemBase * itemBase =  ItemBase::extractTopLevelItemBase(items[i]);
		if (itemBase->getVirtual()) continue;

		if (itemBase != NULL) {
			tlBases.append(itemBase);
		}
	}

	sortAnyByZ(tlBases, bases);
}


void SketchWidget::sortAnyByZ(const QList<QGraphicsItem *> & items, QList<ItemBase *> & bases) {
	for (int i = 0; i < items.size(); i++) {
		ItemBase * base = ItemBase::extractItemBase(items[i]);
		if (base != NULL) {
			bases.append(base);
			base->saveGeometry();
		}
	}

    // order by z
    qSort(bases.begin(), bases.end(), ItemBase::zLessThan);
}

bool SketchWidget::lessThan(int a, int b) {
	return a < b;
}

bool SketchWidget::greaterThan(int a, int b) {
	return a > b;
}

void SketchWidget::changeZ(QHash<long, RealPair * > triplets, qreal (*pairAccessor)(RealPair *) ) {

	// TODO: replace scene->items
	const QList<QGraphicsItem *> items = scene()->items();
	for (int i = 0; i < items.size(); i++) {
		// want all items, not just topLevel
		ItemBase * itemBase = ItemBase::extractItemBase(items[i]);
		if (itemBase == NULL) continue;

		RealPair * pair = triplets[itemBase->id()];
		if (pair == NULL) continue;

		qreal newZ = pairAccessor(pair);
		DebugDialog::debug(QObject::tr("change z %1 %2").arg(itemBase->id()).arg(newZ));
		items[i]->setZValue(newZ);

	}
}

ViewLayer::ViewLayerID SketchWidget::getWireViewLayerID(const ViewGeometry & viewGeometry) {
	if (viewGeometry.getJumper()) {

		return ViewLayer::Jumperwires;
	}

	if (viewGeometry.getTrace()) {
		return ViewLayer::Copper0;
	}

	return m_wireViewLayerID;
}

ViewLayer::ViewLayerID SketchWidget::getRulerViewLayerID() {
	return m_rulerViewLayerID;
}

ViewLayer::ViewLayerID SketchWidget::getPartViewLayerID() {
	return m_partViewLayerID;
}

ViewLayer::ViewLayerID SketchWidget::getConnectorViewLayerID() {
	return m_connectorViewLayerID;
}


void SketchWidget::mousePressConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {

	ModelPart * wireModel = m_paletteModel->retrieveModelPart(Wire::moduleIDName);
	if (wireModel == NULL) return;

	m_tempDragWireCommand = m_holdingSelectItemCommand;
	m_holdingSelectItemCommand = NULL;
	clearHoldingSelectItem();

	// make sure wire layer is visible
	ViewLayer::ViewLayerID viewLayerID = getWireViewLayerID(connectorItem->attachedTo()->getViewGeometry());
	ViewLayer * viewLayer = m_viewLayers.value(viewLayerID);
	if (viewLayer != NULL && !viewLayer->visible()) {
		setLayerVisible(viewLayer, true);
	}

   	ViewGeometry viewGeometry;
   	QPointF p = QPointF(connectorItem->mapToScene(event->pos()));
   	viewGeometry.setLoc(p);
	viewGeometry.setLine(QLineF(0,0,0,0));

	// create a temporary wire for the user to drag
	m_connectorDragConnector = connectorItem;
	m_connectorDragWire = dynamic_cast<Wire *>(addItemAux(wireModel, viewGeometry, ItemBase::getNextID(), NULL, true));
	DebugDialog::debug("creating connector drag wire");
	if (m_connectorDragWire == NULL) {
		clearDragWireTempCommand();
		return;
	}

	// give connector item the mouse, so wire doesn't get mouse moved events
	m_connectorDragWire->grabMouse();
	m_connectorDragWire->initDragEnd(m_connectorDragWire->connector0());
}


void SketchWidget::rotateX(qreal degrees) {
	rotateFlip(degrees, 0);
}


void SketchWidget::flip(Qt::Orientations orientation) {
	rotateFlip(0, orientation);
}

void SketchWidget::rotateFlip(qreal degrees, Qt::Orientations orientation)
{
	if (!this->isVisible()) return;

	clearHoldingSelectItem();

	QList <QGraphicsItem *> items = scene()->selectedItems();
	QList <PaletteItem *> targets;

	for (int i = 0; i < items.size(); i++) {
		// can't rotate wires or layerkin (layerkin rotated indirectly)
		PaletteItem *item = dynamic_cast<PaletteItem *>(items[i]);
		if (item == NULL) continue;

		targets.append(item);
	}

	if (targets.count() <= 0) {
		return;
	}

	QString string = tr("%3 %2 (%1)")
			.arg(ItemBase::viewIdentifierName(m_viewIdentifier))
			.arg((targets.count() == 1) ? targets[0]->modelPart()->title() : QString::number(targets.count()) + " items" )
			.arg((degrees != 0) ? tr("Rotate") : tr("Flip"));

	QUndoCommand * parentCommand = new QUndoCommand(string);
	new CleanUpWiresCommand(this, false, parentCommand);

	emit rotatingFlippingSignal(this, parentCommand);		// eventually, don't send signal when rotating board

	QSet<ItemBase *> emptyList;			// emptylist is only used for a move command
	foreach (PaletteItem * item, targets) {
		disconnectFromFemale(item, emptyList, parentCommand);

		if (item->sticky()) {
			//TODO: apply transformation to stuck items
		}
		// TODO: if item has female connectors, then apply transform to connected items

		if (degrees != 0) {
			new RotateItemCommand(this, item->id(), degrees, parentCommand);
		}
		else {
			new FlipItemCommand(this, item->id(), orientation, parentCommand);
		}
	}

	clearTemporaries();

	new CleanUpWiresCommand(this, true, parentCommand);
	m_undoStack->push(parentCommand);

}

ConnectorItem * SketchWidget::findConnectorItem(ItemBase * itemBase, const QString & connectorID, bool seekLayerKin) {

	ConnectorItem * connectorItem = itemBase->findConnectorItemNamed(connectorID);
	if (connectorItem != NULL) return connectorItem;

	seekLayerKin = true;
	if (seekLayerKin) {
		PaletteItem * pitem = dynamic_cast<PaletteItem *>(itemBase);
		if (pitem == NULL) return NULL;

		QList<class LayerKinPaletteItem *> layerKin = pitem->layerKin();
		for (int j = 0; j < layerKin.count(); j++) {
			connectorItem = layerKin[j]->findConnectorItemNamed(connectorID);
			if (connectorItem != NULL) return connectorItem;
		}

		return NULL;
	}

	return NULL;
}

PaletteItem * SketchWidget::getSelectedPart(){
	QList <QGraphicsItem *> items= scene()->selectedItems();
	PaletteItem *item = NULL;

	// dynamic cast returns null in cases where non-PaletteItems (i.e. wires and layerKin palette items) are selected
	for(int i=0; i < items.count(); i++){
		PaletteItem *temp = dynamic_cast<PaletteItem *>(items[i]);
		if (temp == NULL) continue;

		if (item != NULL) return NULL;  // there are multiple items selected
		item = temp;
	}

	return item;
}

void SketchWidget::setBackground(QColor color) {
	scene()->setBackgroundBrush(QBrush(color));
}

const QColor& SketchWidget::background() {
	return scene()->backgroundBrush().color();
}

void SketchWidget::setItemMenu(QMenu* itemMenu){
	m_itemMenu = itemMenu;
}

void SketchWidget::sketchWidget_wireConnected(long fromID, QString fromConnectorID, long toID, QString toConnectorID) {
	ItemBase * fromItem = findItem(fromID);
	if (fromItem == NULL) return;

	Wire* wire = dynamic_cast<Wire *>(fromItem);
	if (wire == NULL) return;

	ConnectorItem * fromConnectorItem = findConnectorItem(fromItem, fromConnectorID, false);
	if (fromConnectorItem == NULL) {
		// shouldn't be here
		return;
	}

	ItemBase * toItem = findItem(toID);
	if (toItem == NULL) {
		// this was a disconnect
		return;
	}

	ConnectorItem * toConnectorItem = findConnectorItem(toItem, toConnectorID, false);
	if (toConnectorItem == NULL) {
		// shouldn't really be here
		return;
	}

	QPointF p1(0,0), p2, pos;

	if (fromConnectorItem == wire->connector0()) {
		pos = toConnectorItem->sceneAdjustedTerminalPoint();
		ConnectorItem * toConnector1 = wire->otherConnector(fromConnectorItem)->firstConnectedToIsh();
		if (toConnector1 == NULL) {
			p2 = wire->otherConnector(fromConnectorItem)->mapToScene(wire->otherConnector(fromConnectorItem)->pos()) - pos;
		}
		else {
			p2 = toConnector1->sceneAdjustedTerminalPoint();
		}
	}
	else {
		pos = wire->pos();
		ConnectorItem * toConnector0 = wire->otherConnector(fromConnectorItem)->firstConnectedToIsh();
		if (toConnector0 == NULL) {
			pos = wire->pos();
		}
		else {
			pos = toConnector0->sceneAdjustedTerminalPoint();
		}
		p2 = toConnectorItem->sceneAdjustedTerminalPoint() - pos;
	}
	wire->setLineAnd(QLineF(p1, p2), pos, true);

	// here's the connect (model has been previously updated)
	fromConnectorItem->connectTo(toConnectorItem);
	toConnectorItem->connectTo(fromConnectorItem);

	this->update();

}

void SketchWidget::sketchWidget_wireDisconnected(long fromID, QString fromConnectorID) {
	DebugDialog::debug(tr("got wire disconnected"));
	ItemBase * fromItem = findItem(fromID);
	if (fromItem == NULL) return;

	Wire* wire = dynamic_cast<Wire *>(fromItem);
	if (wire == NULL) return;

	ConnectorItem * fromConnectorItem = findConnectorItem(fromItem, fromConnectorID, false);
	if (fromConnectorItem == NULL) {
		// shouldn't be here
		return;
	}

	ConnectorItem * toConnectorItem = fromConnectorItem->firstConnectedToIsh();
	if (toConnectorItem != NULL) {
		fromConnectorItem->removeConnection(toConnectorItem, true);
		toConnectorItem->removeConnection(fromConnectorItem, true);
	}
}

void SketchWidget::changeConnection(long fromID, const QString & fromConnectorID,
									long toID, const QString & toConnectorID,
									bool connect, bool doEmit, bool seekLayerKin,
									bool chain)
{
	changeConnectionAux(fromID, fromConnectorID, toID, toConnectorID, connect, seekLayerKin, chain);
	if (doEmit) {
		emit changeConnectionSignal(fromID, fromConnectorID, toID, toConnectorID, connect, chain);
	}
}

void SketchWidget::changeConnectionAux(long fromID, const QString & fromConnectorID,
									long toID, const QString & toConnectorID,
									bool connect, bool seekLayerKin, bool chain)
{
	DebugDialog::debug(QObject::tr("changeConnection: from %1 %2; to %3 %4 con:%5 v:%6")
				.arg(fromID).arg(fromConnectorID)
				.arg(toID).arg(toConnectorID)
				.arg(connect).arg(m_viewIdentifier) );

	ItemBase * fromItem = findItem(fromID);
	if (fromItem == NULL) {
		DebugDialog::debug("change connection exit 1");
		return;			// for now
	}

	ConnectorItem * fromConnectorItem = findConnectorItem(fromItem, fromConnectorID, seekLayerKin);
	if (fromConnectorItem == NULL) {
		// shouldn't be here
		DebugDialog::debug("change connection exit 2");
		return;
	}

	ItemBase * toItem = findItem(toID);
	if (toItem == NULL) {
		DebugDialog::debug("change connection exit 3");
		return;
	}

	ConnectorItem * toConnectorItem = findConnectorItem(toItem, toConnectorID, seekLayerKin);
	if (toConnectorItem == NULL) {
		// shouldn't be here
		DebugDialog::debug("change connection exit 4");
		return;
	}


	if (connect) {
		fromConnectorItem->connector()->connectTo(toConnectorItem->connector());
		fromConnectorItem->connectTo(toConnectorItem);
		toConnectorItem->connectTo(fromConnectorItem);
		fromItem->setChained(fromConnectorItem, chain);
		toItem->setChained(toConnectorItem, chain);
	}
	else {
		fromConnectorItem->connector()->disconnectFrom(toConnectorItem->connector());
		fromConnectorItem->removeConnection(toConnectorItem, true);
		toConnectorItem->removeConnection(fromConnectorItem, true);
	}


	fromConnectorItem->attachedTo()->updateConnections(fromConnectorItem);
	toConnectorItem->attachedTo()->updateConnections(toConnectorItem);

	if (m_dealWithRatsNestEnabled) {
		dealWithRatsnest(fromConnectorItem, toConnectorItem, connect);
	}
}

void SketchWidget::dealWithRatsnest(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, bool connect) {
	Q_UNUSED(fromConnectorItem);
	Q_UNUSED(toConnectorItem);
	Q_UNUSED(connect);
}

Wire * SketchWidget::makeOneRatsnestWire(ConnectorItem * source, ConnectorItem * dest) {
	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	QPointF fromPos = source->sceneAdjustedTerminalPoint();
	viewGeometry.setLoc(fromPos);
	QPointF toPos = dest->sceneAdjustedTerminalPoint();
	QLineF line(0, 0, toPos.x() - fromPos.x(), toPos.y() - fromPos.y());
	viewGeometry.setLine(line);
	viewGeometry.setWireFlags(ViewGeometry::RatsnestFlag | ViewGeometry::VirtualFlag);

	/*
	 DebugDialog::debug(QString("creating ratsnest %10: %1, from %6 %7, to %8 %9, frompos: %2 %3, topos: %4 %5")
	 .arg(newID)
	 .arg(fromPos.x()).arg(fromPos.y())
	 .arg(toPos.x()).arg(toPos.y())
	 .arg(source->attachedToTitle()).arg(source->connectorStuffID())
	 .arg(dest->attachedToTitle()).arg(dest->connectorStuffID())
	 .arg(m_viewIdentifier)
	 );
	 */

	ItemBase * newItemBase = addItemAux(m_paletteModel->retrieveModelPart(Wire::moduleIDName), viewGeometry, newID, NULL, true);
	tempConnectWire(newItemBase, source, dest);
	return  dynamic_cast<Wire *>(newItemBase);
}


void SketchWidget::tempConnectWire(ItemBase * itemBase, ConnectorItem * from, ConnectorItem * to) {
	Wire * wire = dynamic_cast<Wire *>(itemBase);
	if (wire == NULL) return;

	ConnectorItem * connector0 = wire->connector0();
	from->tempConnectTo(connector0);
	connector0->tempConnectTo(from);
	ConnectorItem * connector1 = wire->connector1();
	to->tempConnectTo(connector1);
	connector1->tempConnectTo(to);
}

void SketchWidget::sketchWidget_changeConnection(long fromID, QString fromConnectorID,
												 long toID, QString toConnectorID,
												 bool connect, bool chain) 
{
	changeConnection(fromID, fromConnectorID,
					 toID, toConnectorID,
					 connect, false, true, chain);
}

void SketchWidget::navigatorScrollChange(double x, double y) {
	QScrollBar * h = this->horizontalScrollBar();
	QScrollBar * v = this->verticalScrollBar();
	int xmin = h->minimum();
   	int xmax = h->maximum();
   	int ymin = v->minimum();
   	int ymax = v->maximum();

   	h->setValue((int) ((xmax - xmin) * x) + xmin);
   	v->setValue((int) ((ymax - ymin) * y) + ymin);
}

void SketchWidget::item_connectionChanged(ConnectorItem * from, ConnectorItem * to, bool connect)
{
	if (connect) {
		this->m_needToConnectItems.insert(from, to);
		DebugDialog::debug(QString("need to connect %1 %2 %3, %4 %5 %6")
				.arg(from->attachedToID())
				.arg(from->attachedToTitle())
				.arg(from->connectorStuffID())
				.arg(to->attachedToID())
				.arg(to->attachedToTitle())
				.arg(to->connectorStuffID()) );
	}
	else {
		//this->m_needToDisconnectItems.insert(from, to);
	}
}

void SketchWidget::keyPressEvent ( QKeyEvent * event ) {
	QGraphicsView::keyPressEvent(event);
}



void SketchWidget::sketchWidget_copyItem(long itemID, QHash<ItemBase::ViewIdentifier, ViewGeometry *> & geometryHash) {
	ItemBase * itemBase = findItem(itemID);
	if (itemBase == NULL) {
		// this shouldn't happen
		return;
	}

	ViewGeometry * vg = new ViewGeometry(itemBase->getViewGeometry());
	geometryHash.insert(m_viewIdentifier, vg);
}

void SketchWidget::sketchWidget_deleteItem(long itemID, QUndoCommand * parentCommand) {
	ItemBase * itemBase = findItem(itemID);
	if (itemBase == NULL) return;

	makeDeleteItemCommand(itemBase, parentCommand);
}

void SketchWidget::makeDeleteItemCommand(ItemBase * itemBase, QUndoCommand * parentCommand) {
	if (itemBase->itemType() == ModelPart::Wire) {
		Wire * wire = dynamic_cast<Wire *>(itemBase);
		new WireWidthChangeCommand(this, wire->id(), wire->width(), wire->width(), parentCommand);
		new WireColorChangeCommand(this, wire->id(), wire->colorString(), wire->colorString(), wire->opacity(), wire->opacity(), parentCommand);
	}
	new DeleteItemCommand(this, BaseCommand::SingleView, itemBase->modelPart()->moduleID(), itemBase->getViewGeometry(), itemBase->id(), parentCommand);
}

ItemBase::ViewIdentifier SketchWidget::viewIdentifier() {
	return m_viewIdentifier;
}


void SketchWidget::setViewLayerIDs(ViewLayer::ViewLayerID part, ViewLayer::ViewLayerID wire, ViewLayer::ViewLayerID connector, ViewLayer::ViewLayerID ruler) {
	m_partViewLayerID = part;
	m_wireViewLayerID = wire;
	m_connectorViewLayerID = connector;
	m_rulerViewLayerID = ruler;
}

void SketchWidget::dragIsDoneSlot(ItemDrag * itemDrag) {
	disconnect(itemDrag, SIGNAL(dragIsDoneSignal(ItemDrag *)), this, SLOT(dragIsDoneSlot(ItemDrag *)));
	killDroppingItem();			// 		// drag is done, but nothing dropped here: remove the temporary item
}


void SketchWidget::clearTemporaries() {
	for (int i = 0; i < m_temporaries.count(); i++) {
		QGraphicsItem * item = m_temporaries[i];
		scene()->removeItem(item);
		delete item;
	}

	m_temporaries.clear();
}

void SketchWidget::killDroppingItem() {
	// was only a temporary placeholder, get rid of it now
	if (m_droppingItem != NULL) {
		m_droppingItem->removeLayerKin();
		this->scene()->removeItem(m_droppingItem);
		delete m_droppingItem;
		m_droppingItem = NULL;
	}
}

ViewLayer::ViewLayerID SketchWidget::getViewLayerID(ModelPart * modelPart) {

	QDomElement layers = LayerAttributes::getSvgElementLayers(modelPart->modelPartStuff()->domDocument(), m_viewIdentifier);
	if (layers.isNull()) return ViewLayer::UnknownLayer;

	QDomElement layer = layers.firstChildElement("layer");
	if (layer.isNull()) return ViewLayer::UnknownLayer;

	QString layerName = layer.attribute("layerId");
	int layerCount = 0;
	while (!layer.isNull()) {
		if (++layerCount > 1) break;

		layer = layer.nextSiblingElement("layer");
	}

	if (layerCount == 1) {
		return ViewLayer::viewLayerIDFromXmlString(layerName);
	}

	return multiLayerGetViewLayerID(modelPart, layerName);
}

ViewLayer::ViewLayerID SketchWidget::multiLayerGetViewLayerID(ModelPart * modelPart, QString & layerName) {
	Q_UNUSED(modelPart);
	return ViewLayer::viewLayerIDFromXmlString(layerName);
}

ItemBase * SketchWidget::overSticky(ItemBase * itemBase) {
	foreach (QGraphicsItem * childItem, itemBase->childItems()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (connectorItem == NULL) continue;

		QPointF p = connectorItem->sceneAdjustedTerminalPoint();
		foreach (QGraphicsItem * item,  this->scene()->items(p)) {
			ItemBase * s = dynamic_cast<ItemBase *>(item);
			if (s == NULL) continue;

			if (s == connectorItem->attachedTo()) continue;
			if (!s->sticky()) continue;

			return s;
		}
	}

	return NULL;
}


void SketchWidget::stickem(long stickTargetID, long stickSourceID, bool stick) {
	ItemBase * stickTarget = findItem(stickTargetID);
	if (stickTarget == NULL) return;

	ItemBase * stickSource = findItem(stickSourceID);
	if (stickSource == NULL) return;

	stickTarget->addSticky(stickSource, stick);
	stickSource->addSticky(stickTarget, stick);
}

void SketchWidget::setChainDrag(bool chainDrag) {
	m_chainDrag = chainDrag;
}

void SketchWidget::stickyScoop(ItemBase * stickyOne, QUndoCommand * parentCommand) {
	foreach (QGraphicsItem * item, scene()->collidingItems(stickyOne)) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;
		if (itemBase->sticky()) continue;
		if (stickyOne->alreadySticking(itemBase)) continue;
		if (!stickyOne->stickyEnabled(itemBase)) continue;

		bool gotOne = false;
		foreach (QGraphicsItem * childItem, itemBase->childItems()) {
			ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
			if (connectorItem == NULL) continue;

			QPointF p = connectorItem->sceneAdjustedTerminalPoint();
			if (stickyOne->contains(stickyOne->mapFromScene(p))) {
				gotOne = true;
				break;
			}
		}
		if (!gotOne) continue;

		if (parentCommand == NULL) {
			stickyOne->addSticky(itemBase, true);
			itemBase->addSticky(stickyOne, true);
		}
		else {
			new StickyCommand(this, stickyOne->id(), itemBase->id(), true, parentCommand);
		}

	}
}

void SketchWidget::wire_wireSplit(Wire* wire, QPointF newPos, QPointF oldPos, QLineF oldLine) {
	if (!m_chainDrag) return;  // can't split a wire in some views (for now)

	this->clearHoldingSelectItem();
	this->m_moveEventCount = 0;  // clear this so an extra MoveItemCommand isn't posted

	QUndoCommand * parentCommand = new QUndoCommand();
	new CleanUpWiresCommand(this, false, parentCommand);
	parentCommand->setText(QObject::tr("Split Wire") );

	long fromID = wire->id();

	QLineF newLine(oldLine.p1(), newPos - oldPos);
	new ChangeWireCommand(this, fromID, oldLine, newLine, oldPos, oldPos, true, parentCommand);

	long newID = ItemBase::getNextID();
	ViewGeometry vg(wire->getViewGeometry());
	vg.setLoc(newPos);
	QLineF newLine2(QPointF(0,0), oldLine.p2() + oldPos - newPos);
	vg.setLine(newLine2);

	new AddItemCommand(this, BaseCommand::SingleView, Wire::moduleIDName, vg, newID, parentCommand);
	new WireColorChangeCommand(this, newID, wire->colorString(), wire->colorString(), wire->opacity(), wire->opacity(), parentCommand);
	new WireWidthChangeCommand(this, newID, wire->width(), wire->width(), parentCommand);

	// disconnect from original wire and reconnect to new wire
	ConnectorItem * connector1 = wire->connector1();
	foreach (ConnectorItem * toConnectorItem, connector1->connectedToItems()) {
		new ChangeConnectionCommand(this, BaseCommand::SingleView, toConnectorItem->attachedToID(), toConnectorItem->connectorStuffID(),
			wire->id(), connector1->connectorStuffID(),
			false, true, toConnectorItem->chained(), parentCommand);
		new ChangeConnectionCommand(this, BaseCommand::SingleView, toConnectorItem->attachedToID(), toConnectorItem->connectorStuffID(),
			newID, connector1->connectorStuffID(),
			true, true, toConnectorItem->chained(), parentCommand);
	}

	// connect the two wires
	new ChangeConnectionCommand(this, BaseCommand::SingleView, wire->id(), connector1->connectorStuffID(),
			newID, "connector0", true, true, true, parentCommand);


	//checkSticky(wire, parentCommand);

	new CleanUpWiresCommand(this, true, parentCommand);
	m_undoStack->push(parentCommand);
}

void SketchWidget::wire_wireJoin(Wire* wire, ConnectorItem * clickedConnectorItem) {
	if (!m_chainDrag) return;  // can't join a wire in some views (for now)

	this->clearHoldingSelectItem();
	this->m_moveEventCount = 0;  // clear this so an extra MoveItemCommand isn't posted

	QUndoCommand * parentCommand = new QUndoCommand();
	parentCommand->setText(QObject::tr("Join Wire") );

	// assumes there is one and only one item connected
	ConnectorItem * toConnectorItem = clickedConnectorItem->connectedToItems()[0];
	if (toConnectorItem == NULL) return;

	Wire * toWire = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
	if (toWire == NULL) return;

	if (wire->id() > toWire->id()) {
		// delete the wire with the higher id
		// so we can keep the three views in sync
		// i.e. the original wire has the lowest id in the chain
		Wire * wtemp = toWire;
		toWire = wire;
		wire = wtemp;
		ConnectorItem * ctemp = toConnectorItem;
		toConnectorItem = clickedConnectorItem;
		clickedConnectorItem = ctemp;
	}

	ConnectorItem * otherConnector = toWire->otherConnector(toConnectorItem);

	// disconnect the wires
	new ChangeConnectionCommand(this, BaseCommand::SingleView, wire->id(), clickedConnectorItem->connectorStuffID(),
			toWire->id(), toConnectorItem->connectorStuffID(), false, true, true, parentCommand);

	// disconnect everyone from the other end of the wire being deleted, and reconnect to the remaining wire
	foreach (ConnectorItem * otherToConnectorItem, otherConnector->connectedToItems()) {
		new ChangeConnectionCommand(this, BaseCommand::SingleView, otherToConnectorItem->attachedToID(), otherToConnectorItem->connectorStuffID(),
			toWire->id(), otherConnector->connectorStuffID(),
			false, true, otherToConnectorItem->chained(), parentCommand);
		new ChangeConnectionCommand(this, BaseCommand::SingleView, otherToConnectorItem->attachedToID(), otherToConnectorItem->connectorStuffID(),
			wire->id(), clickedConnectorItem->connectorStuffID(),
			true, true, otherToConnectorItem->chained(), parentCommand);
	}

	toWire->saveGeometry();
	makeDeleteItemCommand(toWire, parentCommand);

	QLineF newLine;
	QPointF newPos;
	if (otherConnector == toWire->connector1()) {
		newPos = wire->pos();
		newLine = QLineF(QPointF(0,0), toWire->pos() - wire->pos() + toWire->line().p2());
	}
	else {
		newPos = toWire->pos();
		newLine = QLineF(QPointF(0,0), wire->pos() - toWire->pos() + wire->line().p2());
	}
	new ChangeWireCommand(this, wire->id(), wire->line(), newLine, wire->pos(), newPos, true, parentCommand);


	//checkSticky(wire, parentCommand);

	m_undoStack->push(parentCommand);
}

void SketchWidget::hoverEnterItem(QGraphicsSceneHoverEvent * event, ItemBase * item) {
	if(m_infoViewOnHover || currentlyInfoviewed(item)) {
		InfoGraphicsView::hoverEnterItem(event, item);
	}

	if (!this->m_chainDrag) return;

	Wire * wire = dynamic_cast<Wire *>(item);
	if (wire == NULL) return;

	statusMessage(tr("Shift-click to add a bend point to the wire"));
}

void SketchWidget::statusMessage(QString message, int timeout) {
	QMainWindow * mainWindow = dynamic_cast<QMainWindow *>(window());
	if (mainWindow == NULL) return;

	mainWindow->statusBar()->showMessage(message, timeout);
}

void SketchWidget::hoverLeaveItem(QGraphicsSceneHoverEvent * event, ItemBase * item){
	if(m_infoViewOnHover) {
		InfoGraphicsView::hoverLeaveItem(event, item);
	}

	if (!this->m_chainDrag) return;

	Wire * wire = dynamic_cast<Wire *>(item);
	if (wire == NULL) return;;

	statusMessage(tr(""));
}

void SketchWidget::hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item) {
	Q_UNUSED(event)
	if(m_infoViewOnHover || currentlyInfoviewed(item->attachedTo())) {
		viewConnectorItemInfo(item);
	}

	if (!this->m_chainDrag) return;
	if (!item->chained()) return;

	statusMessage(tr("Shift-click to delete this bend point"));
}

void SketchWidget::hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item){
	ItemBase* attachedTo = item->attachedTo();
	if(attachedTo) {
		if(m_infoViewOnHover || currentlyInfoviewed(attachedTo)) {
			InfoGraphicsView::hoverLeaveConnectorItem(event, item);
			if(attachedTo->collidesWithItem(item)) {
				hoverEnterItem(event,attachedTo);
			}
		}
		attachedTo->hoverLeaveConnectorItem(event, item);
	}

	if (!this->m_chainDrag) return;
	if (!item->chained()) return;

	statusMessage(tr(""));
}

bool SketchWidget::currentlyInfoviewed(ItemBase *item) {
	if(m_infoView) {
		ItemBase * currInfoView = m_infoView->currentItem();
		return !currInfoView || item == currInfoView;//->cu selItems.size()==1 && selItems[0] == item;
	}
	return false;
}

void SketchWidget::cleanUpWires(bool doEmit) {
	cleanUpWiresAux();

	if (doEmit) {
		emit cleanUpWiresSignal();
	}
}

void SketchWidget::sketchWidget_cleanUpWires() {
	cleanUpWiresAux();
}

void SketchWidget::cleanUpWiresAux() {
	QList<Wire *> wires;
	QList<QGraphicsItem *>items = scene()->items();
	// TODO: get rid of scene()->items()
	foreach (QGraphicsItem * item, items) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		if (wires.contains(wire)) continue;			// already processed

		cleanUpWire(wire, wires);
	}

	updateRatsnestStatus();
}

void SketchWidget::tempDisconnectWire(ConnectorItem * fromConnectorItem, QMultiHash<ConnectorItem *, ConnectorItem *> & connectionState) {
	foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
		connectionState.insert(fromConnectorItem, toConnectorItem);
		fromConnectorItem->tempRemove(toConnectorItem);
		toConnectorItem->tempRemove(fromConnectorItem);
		//DebugDialog::debug(QString("temp dis %1 %2 %3 %4")
			//.arg(fromConnectorItem->attachedToID())
			//.arg(fromConnectorItem->connectorStuffID())
			//.arg(toConnectorItem->attachedToID())
			//.arg(toConnectorItem->connectorStuffID()) );
	}
}

void SketchWidget::setItemTooltip(long id, const QString &newTooltip) {
	ItemBase * pitem = findItem(id);
	if (pitem != NULL) {
		pitem->setInstanceTitleAndTooltip(newTooltip);
		emit tooltipAppliedToItem(id, newTooltip);
	}
}

void SketchWidget::setInfoViewOnHover(bool infoViewOnHover) {
	m_infoViewOnHover = infoViewOnHover;
}

void SketchWidget::updateInfoView() {
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(updateInfoViewSlot()));
	timer->setSingleShot(true);
	timer->start(50);
}

void SketchWidget::updateInfoViewSlot() {
	InfoGraphicsView::viewItemInfo(m_lastPaletteItemSelected);
}

void SketchWidget::swapSelected(ModelPart* other) {
	swapSelected(other->moduleID());
}

void SketchWidget::swapSelected(const QString &moduleID) {
	if(moduleID != ___emptyString___) {
		if(m_lastPaletteItemSelected) {
			QUndoCommand* parentCommand = new QUndoCommand(tr("Swapped %1 with module %2").arg(m_lastPaletteItemSelected->instanceTitle()).arg(moduleID));
			new SwapCommand(
					this,
					m_lastPaletteItemSelected->id(),
					m_lastPaletteItemSelected->modelPart()->moduleID(),
					moduleID,
					parentCommand);
			m_undoStack->push(parentCommand);
		}
	} else {
		QMessageBox::information(
			this,
			tr("Sorry!"),
			tr(
			 "No part with those characteristics.\n"
			 "Where're working to avoid this message, and only let you choose between properties that do exist")
		);
	}
}

void SketchWidget::swapSelected(PaletteItem* other) {
	swapSelected(other->modelPart());
}

void SketchWidget::swap(PaletteItem* from, ModelPart *to) {
	if(from && to) {
		from->swap(to, m_viewLayers);
		updateInfoView();
	}
}

void SketchWidget::swap(long itemId, const QString &moduleID, bool doEmit) {
	swap(itemId, m_refModel->retrieveModelPart(moduleID), doEmit);
}

void SketchWidget::swap(long itemId, ModelPart *to, bool doEmit) {
	PaletteItem *from = dynamic_cast<PaletteItem*>(findItem(itemId));
	if(from && to) {
		swap(from,to);

		// let's make sure that the icon pixmap will be available for the infoview
		LayerAttributes layerAttributes;
		from->setUpImage(from->modelPart(), ItemBase::IconView, ViewLayer::Icon, layerAttributes);

		if(doEmit) {
			emit swapped(itemId, to);
		}
	}
}

void SketchWidget::changeWireColor(const QString &wireTitle, long wireId,
								   const QString& oldColor, const QString newColor,
								   qreal oldOpacity, qreal newOpacity) {
	QUndoCommand* parentCommand = new QUndoCommand(
		tr("Wire %1 color changed from %2 to %3")
			.arg(wireTitle)
			.arg(oldColor)
			.arg(newColor)
	);
	new WireColorChangeCommand(
			this,
			wireId,
			oldColor,
			newColor,
			oldOpacity,
			newOpacity,
			parentCommand);
	m_undoStack->push(parentCommand);
}

void SketchWidget::changeWireColor(long wireId, const QString& color, qreal opacity) {
	ItemBase *item = findItem(wireId);
	if(Wire* wire = dynamic_cast<Wire*>(item)) {
		wire->setColorString(color, opacity);
	}
}

void SketchWidget::changeWireWidth(long wireId, int width) {
	ItemBase *item = findItem(wireId);
	if(Wire* wire = dynamic_cast<Wire*>(item)) {
		wire->setWidth(width);
	}
}

PaletteModel * SketchWidget::paletteModel() {
	return m_paletteModel;
}

bool SketchWidget::swappingEnabled() {
	return m_refModel->swapEnabled();
}

void SketchWidget::resizeEvent(QResizeEvent * event) {
	InfoGraphicsView::resizeEvent(event);
	emit resizeSignal();
}

void SketchWidget::addBreadboardViewLayers() {
	setViewLayerIDs(ViewLayer::Breadboard, ViewLayer::BreadboardWire, ViewLayer::Breadboard, ViewLayer::BreadboardRuler);

	QList<ViewLayer::ViewLayerID> layers;
	layers << ViewLayer::BreadboardBreadboard << ViewLayer::Breadboard
		<< ViewLayer::BreadboardWire << ViewLayer::BreadboardRuler;

	addViewLayersAux(layers);
}

void SketchWidget::addSchematicViewLayers() {
	setViewLayerIDs(ViewLayer::Schematic, ViewLayer::SchematicWire, ViewLayer::Schematic, ViewLayer::SchematicRuler);

	QList<ViewLayer::ViewLayerID> layers;
	layers << ViewLayer::Schematic << ViewLayer::SchematicWire << ViewLayer::SchematicRuler;

	addViewLayersAux(layers);
}

void SketchWidget::addPcbViewLayers() {
	setViewLayerIDs(ViewLayer::Silkscreen, ViewLayer::Ratsnest, ViewLayer::Copper0, ViewLayer::PcbRuler);

	QList<ViewLayer::ViewLayerID> layers;
	layers << ViewLayer::Board << ViewLayer::Copper1 << ViewLayer::Copper0 << ViewLayer::Keepout
		<< ViewLayer::Vias << ViewLayer::Soldermask << ViewLayer::Silkscreen << ViewLayer::Outline
		<< ViewLayer::Jumperwires << ViewLayer::Ratsnest << ViewLayer::PcbRuler;

	addViewLayersAux(layers);
}



void SketchWidget::addViewLayers() {
}

void SketchWidget::addViewLayersAux(const QList<ViewLayer::ViewLayerID> &layers, float startZ) {
	float z = startZ;
	foreach(ViewLayer::ViewLayerID vlId, layers) {
		addViewLayer(new ViewLayer(vlId, true, z));
		z+=1;
	}
}

void SketchWidget::setIgnoreSelectionChangeEvents(bool ignore) {
	m_ignoreSelectionChangeEvents = ignore;
}

void SketchWidget::createJumper() {
	QString commandString = "Create jumper wire";
	QString colorString = "jumper";
	createJumperOrTrace(commandString, ViewGeometry::JumperFlag, colorString);
	ensureLayerVisible(ViewLayer::Jumperwires);
}

void SketchWidget::createTrace() {
	QString commandString = tr("Create trace wire");
	QString colorString = "trace";
	createJumperOrTrace(commandString, ViewGeometry::TraceFlag, colorString);
	ensureLayerVisible(ViewLayer::Copper0);
}

void SketchWidget::createJumperOrTrace(const QString & commandString, ViewGeometry::WireFlag flag, const QString & colorString)
{
	QList<QGraphicsItem *> items = scene()->selectedItems();
	if (items.count() != 1) return;

	Wire * wire = dynamic_cast<Wire *>(items[0]);
	if (wire == NULL) return;

	if (!wire->getRatsnest()) return;

	QList<ConnectorItem *> ends;
	Wire * jumperOrTrace = wire->findJumperOrTraced(ViewGeometry::JumperFlag | ViewGeometry::TraceFlag, ends);
	QUndoCommand * parentCommand = new QUndoCommand(commandString);
	if (jumperOrTrace != NULL) {
		new WireFlagChangeCommand(this, wire->id(), wire->wireFlags(), wire->wireFlags() | ViewGeometry::RoutedFlag, parentCommand);
		new WireColorChangeCommand(this, wire->id(), wire->colorString(), wire->colorString(), wire->opacity(), .35, parentCommand);
	}
	else {
		long newID = createWire(ends[0], ends[1], flag, false, BaseCommand::SingleView, parentCommand);
		new WireColorChangeCommand(this, newID, colorString, colorString, 1.0, 1.0, parentCommand);
		new WireWidthChangeCommand(this, newID, 3, 3, parentCommand);
		new WireColorChangeCommand(this, wire->id(), wire->colorString(), wire->colorString(), wire->opacity(), 0.35, parentCommand);
		new WireFlagChangeCommand(this, wire->id(), wire->wireFlags(), wire->wireFlags() | ViewGeometry::RoutedFlag, parentCommand);
	}
	m_undoStack->push(parentCommand);
}

void SketchWidget::hideConnectors(bool hide) {
	foreach (QGraphicsItem * item, scene()->items()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;
		if (!itemBase->isVisible()) continue;

		foreach (QGraphicsItem * childItem, itemBase->childItems()) {
			ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
			if (connectorItem == NULL) continue;

			connectorItem->setVisible(!hide);
		}
	}
}

void SketchWidget::saveLayerVisibility()
{
	m_viewLayerVisibility.clear();
	foreach (ViewLayer::ViewLayerID viewLayerID, m_viewLayers.keys()) {
		ViewLayer * viewLayer = m_viewLayers.value(viewLayerID);
		if (viewLayer == NULL) continue;

		m_viewLayerVisibility.insert(viewLayerID, viewLayer->visible());
	}
}

void SketchWidget::restoreLayerVisibility()
{
	foreach (ViewLayer::ViewLayerID viewLayerID, m_viewLayerVisibility.keys()) {
		setLayerVisible(m_viewLayers.value(viewLayerID),  m_viewLayerVisibility.value(viewLayerID));
	}
}

bool SketchWidget::ratsAllRouted() {
	foreach (QGraphicsItem * item, scene()->items()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		if (!wire->getRatsnest()) continue;

		if (!wire->getRouted()) return false;
	}

	return true;
}

void SketchWidget::changeWireFlags(long wireId, ViewGeometry::WireFlags wireFlags)
{
	ItemBase *item = findItem(wireId);
	if(Wire* wire = dynamic_cast<Wire*>(item)) {
		wire->setWireFlags(wireFlags);
	}
}

void SketchWidget::disconnectFromFemale(ItemBase * item, QSet<ItemBase *> & savedItems, QUndoCommand * parentCommand)
{
	Q_UNUSED(item);
	Q_UNUSED(savedItems);
	Q_UNUSED(parentCommand);
}

void SketchWidget::spaceBarIsPressedSlot(bool isPressed) {
	if (isPressed) {
		setDragMode(QGraphicsView::ScrollHandDrag);
		setCursor(Qt::OpenHandCursor);
	}
	else {
		setDragMode(QGraphicsView::RubberBandDrag);
		setCursor(Qt::ArrowCursor);
	}
}

void SketchWidget::updateRatsnestStatus() {
}


void SketchWidget::ensureLayerVisible(ViewLayer::ViewLayerID viewLayerID)
{
	ViewLayer * viewLayer = m_viewLayers.value(viewLayerID, NULL);
	if (viewLayer == NULL) return;

	if (!viewLayer->visible()) {
		setLayerVisible(viewLayer, true);
	}
}

void SketchWidget::clearRouting(QUndoCommand * parentCommand) {
	Q_UNUSED(parentCommand);
	Autorouter1::clearTraces(this, true);
	updateRatsnestStatus();
}

void SketchWidget::clearDragWireTempCommand()
{
	if (m_tempDragWireCommand) {
		delete m_tempDragWireCommand;
		m_tempDragWireCommand = NULL;
	}
}

void SketchWidget::autoScrollTimeout()
{
	if (m_autoScrollX == 0 && m_autoScrollY == 0 ) return;

	//DebugDialog::debug(QString("scrolling dx:%1 dy%2").arg(m_autoScrollX).arg(m_autoScrollY) );

	if (m_autoScrollX != 0) {
		QScrollBar * h = horizontalScrollBar();
		h->setValue(m_autoScrollX + h->value());
	}
	if (m_autoScrollY != 0) {
		QScrollBar * v = verticalScrollBar();
		v->setValue(m_autoScrollY + v->value());
	}

	moveItems(m_globalPos);
}

/*

void SketchWidget::collectDisconnectors(ItemBase * item) {
	if (item->itemType() == ModelPart::Breadboard || item->itemType() == ModelPart::Board) return;

	foreach (QGraphicsItem * childItem, item->childItems()) {
		ConnectorItem * fromConnectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (fromConnectorItem == NULL) return;
		if (fromConnectorItem->connectorType() != Connector::Male) continue;

		ConnectorItem * disconnectee = NULL;
		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
			if (toConnectorItem->connectorType() == Connector::Female && !m_savedItems.contains(toConnectorItem->attachedTo())) {
				m_disconnectors.insert(fromConnectorItem, toConnectorItem);
				fromConnectorItem->tempConnectTo(toConnectorItem);
				toConnectorItem->tempConnectTo(fromConnectorItem);
				disconnectee = toConnectorItem;
				break;
			}
		}

		if (disconnectee) {
			// deal with bus connections
			dealWithVirtualDisconnections(disconnectee, fromConnectorItem);
			dealWithVirtualDisconnections(fromConnectorItem, disconnectee);
		}
	}
}

void SketchWidget::dealWithVirtualDisconnections(ConnectorItem * source, ConnectorItem * dest)
{
	Bus* bus = source->bus();
	if (bus == NULL) return;

	foreach (ConnectorItem * toConnectorItem, dest->connectedToItems()) {
		if (toConnectorItem->attachedTo()->getVirtual()) {
			Wire * wire = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
			ConnectorItem * otherEnd = wire->otherConnector(toConnectorItem);
			foreach (ConnectorItem * otherConnectorToItem, otherEnd->connectedToItems()) {
				if (otherConnectorToItem->bus() == bus) {
					m_disconnectors.insert(dest, source);
					dest->tempConnectTo(toConnectorItem);
					source->tempConnectTo(dest);
					break;
				}
			}
		}
	}
}

void SketchWidget::restoreDisconnectors() {
	foreach (ConnectorItem * from, m_disconnectors.keys()) {
		ConnectorItem * to = m_disconnectors.value(from);
		from->tempConnectTo(to);
		to->tempConnectTo(from);
	}
	m_disconnectors.clear();
}

*/

const QString &SketchWidget::selectedModuleID() {
	if(m_lastPaletteItemSelected) {
		return m_lastPaletteItemSelected->modelPart()->moduleID();
	}
	return ___emptyString___;
}
