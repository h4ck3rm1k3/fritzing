/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2009 Fachhochschule Potsdam - http://fh-potsdam.de

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty ofro
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
#include <QMultiHash>
#include <QBrush>
#include <QGraphicsItem>
#include <QMainWindow>
#include <QApplication>
#include <QDomElement>
#include <QSettings>

#include "../items/paletteitem.h"
#include "../items/logoitem.h"
#include "../items/ruler.h"
#include "../items/symbolpaletteitem.h"
#include "../items/wire.h"
#include "../commands.h"
#include "../model/modelpart.h"
#include "../debugdialog.h"
#include "../items/layerkinpaletteitem.h"
#include "sketchwidget.h"
#include "../connectors/connectoritem.h"
#include "../connectors/bus.h"
#include "../items/jumperitem.h"
#include "../items/virtualwire.h"
#include "../items/tracewire.h"
#include "../itemdrag.h"
#include "../layerattributes.h"
#include "../waitpushundostack.h"
#include "../utils/zoomcombobox.h"
#include "../autoroute/autorouter1.h"
#include "../fgraphicsscene.h"
#include "../version/version.h"
#include "../labels/partlabel.h"
#include "../labels/note.h"
#include "../group/groupitem.h"
#include "../svg/svgfilesplitter.h"
#include "../svg/svgflattener.h"
#include "../help/sketchmainhelp.h"
#include "../infoview/htmlinfoview.h"
#include "../items/resizableboard.h"
#include "../utils/graphicsutils.h"
#include "../utils/textutils.h"
#include "../fsvgrenderer.h"
#include "../items/resistor.h"
#include "../items/mysterypart.h"
#include "../items/pinheader.h"
#include "../items/dip.h"
#include "../items/groundplane.h"
#include "../items/moduleidnames.h"

QHash<ViewIdentifierClass::ViewIdentifier,QColor> SketchWidget::m_bgcolors;

SketchWidget::SketchWidget(ViewIdentifierClass::ViewIdentifier viewIdentifier, QWidget *parent, int size, int minSize)
    : InfoGraphicsView(parent)
{
	m_alignToGrid = false;
	m_movingByMouse = m_movingByArrow = false;
	m_statusConnectState = StatusConnectNotTried;
	m_dragBendpointWire = NULL;
	m_lastHoverEnterItem = NULL;
	m_lastHoverEnterConnectorItem = NULL;
	m_resizingBoard = NULL;
	m_resizingJumperItem = NULL;
	m_fixedToCenterItem = NULL;
	m_spaceBarWasPressed = m_spaceBarIsPressed = false;
	m_current = false;
	m_ignoreSelectionChangeEvents = 0;
	m_droppingItem = NULL;
	m_chainDrag = false;
	m_bendpointWire = m_connectorDragWire = NULL;
	m_tempDragWireCommand = m_holdingSelectItemCommand = NULL;
	m_viewIdentifier = viewIdentifier;
	//setAlignment(Qt::AlignLeft | Qt::AlignTop);
	setDragMode(QGraphicsView::RubberBandDrag);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setAcceptDrops(true);
	setRenderHint(QPainter::Antialiasing, true);

	//setCacheMode(QGraphicsView::CacheBackground);
	//setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	//setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	//setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	//setTransformationAnchor(QGraphicsView::NoAnchor);
    FGraphicsScene* scene = new FGraphicsScene(this);
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
    item->setLine(0, 0, rect().width(), rect().height());
    this->scene()->addItem(item);
    item->setVisible(false);

	connect(this->scene(), SIGNAL(selectionChanged()), this, SLOT(scene_selectionChanged()));

    connect(QApplication::clipboard(),SIGNAL(changed(QClipboard::Mode)),this,SLOT(restartPasteCount()));
    restartPasteCount(); // the first time

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resize(size, size);
    setMinimumSize(minSize, minSize);

    setLastPaletteItemSelected(NULL);

    m_infoViewOnHover = true;

    connect(this, SIGNAL(resizeSignal()), this, SLOT(ensureFixedItemsPositions()));
    connect(this, SIGNAL(wheelSignal()),  this, SLOT(ensureFixedItemsPositions()));
    connect(
    	horizontalScrollBar(), SIGNAL(valueChanged(int)),
    	this, SLOT(ensureFixedItemsPositions())
    );
    connect(
    	verticalScrollBar(), SIGNAL(valueChanged(int)),
    	this, SLOT(ensureFixedItemsPositions())
    );


	QTimer::singleShot(400, this, SLOT(ensureFixedItemsPositions()));
}

SketchWidget::~SketchWidget() {
	foreach (ViewLayer * viewLayer, m_viewLayers.values()) {
		if (viewLayer == NULL) continue;

		delete viewLayer;
	}
	m_viewLayers.clear();
}


void SketchWidget::ensureFixedItemsPositions() {

	//DebugDialog::debug("ensure fixed items positions");

	ensureFixedToBottomLeftItems();
	ensureFixedToCenterItems();
	ensureFixedToTopLeftItems();
	ensureFixedToTopRightItems();
	ensureFixedToBottomRightItems();

	scene()->update(sceneRect());
}

void SketchWidget::restartPasteCount() {
	m_pasteCount = 1;
}

WaitPushUndoStack* SketchWidget::undoStack() {
	return m_undoStack;
}

void SketchWidget::setUndoStack(WaitPushUndoStack * undoStack) {
	m_undoStack = undoStack;
}

ItemBase* SketchWidget::loadFromModel(ModelPart *modelPart, const ViewGeometry& viewGeometry){
	// assumes modelPart has already been added to the sketch
	// or you're in big trouble when you delete the item
	return addItemAux(modelPart, viewGeometry, ItemBase::getNextID(), -1, NULL, NULL, true, m_viewIdentifier);
}

void SketchWidget::loadFromModel(QList<ModelPart *> & modelParts, BaseCommand::CrossViewType crossViewType, QUndoCommand * parentCommand, bool doRatsnest, bool offsetPaste) {
	clearHoldingSelectItem();

	if (parentCommand) {
		SelectItemCommand * selectItemCommand = stackSelectionState(false, parentCommand);
		selectItemCommand->setSelectItemType(SelectItemCommand::DeselectAll);
		selectItemCommand->setCrossViewType(crossViewType);
	}

	QHash<long, ItemBase *> newItems;
	setIgnoreSelectionChangeEvents(true);

	QString viewName = ViewIdentifierClass::viewIdentifierXmlName(m_viewIdentifier);
	QMultiMap<qreal, ItemBase *> zmap;

	// make parts
	foreach (ModelPart * mp, modelParts) {
		QDomElement instance = mp->instanceDomElement();
		if (instance.isNull()) continue;

		QDomElement views = instance.firstChildElement("views");
		if (views.isNull()) continue;

		QDomElement view = views.firstChildElement(viewName);
		if (view.isNull()) continue;

		QDomElement geometry = view.firstChildElement("geometry");
		if (geometry.isNull()) continue;;
		ViewGeometry viewGeometry(geometry);

		QDomElement labelGeometry = view.firstChildElement("titleGeometry");

		// use a function of the model index to ensure the same parts have the same ID across views
		long newID = ItemBase::getNextID(mp->modelIndex());
		if (parentCommand == NULL) {
			ItemBase * item = addItemAux(mp, viewGeometry, newID, -1, NULL, NULL, true, m_viewIdentifier);
			if (item != NULL) {
				zmap.insert(viewGeometry.z() - qFloor(viewGeometry.z()), item);   
				bool gotOne = false;
				if (!gotOne) {
					PaletteItem * paletteItem = dynamic_cast<PaletteItem *>(item);
					if (paletteItem != NULL) {
						// wires don't have transforms
						paletteItem->setTransforms();
						gotOne = true;
					}
				}
				if (!gotOne) {
					Wire * wire = dynamic_cast<Wire *>(item);
					if (wire != NULL) {
						QDomElement extras = view.firstChildElement("wireExtras");
						wire->setExtras(extras, this);
						gotOne = true;
					}
				}
				if (!gotOne) {
					Note * note = dynamic_cast<Note *>(item);
					if (note != NULL) {
						note->setText(mp->instanceText(), true);
						gotOne = true;
					}
				}
				if (!gotOne) {
					GroupItem * groupItem = dynamic_cast<GroupItem *>(item);
					if (groupItem != NULL) {
						groupItem->setTransforms();
						gotOne = true;
					}
				}

				// use the modelIndex from mp, not from the newly created item, because we're mapping from the modelIndex in the xml file
				newItems.insert(mp->modelIndex(), item);
				item->restorePartLabel(labelGeometry, getLabelViewLayerID());
			}
		}
		else {
			// offset pasted items so we can differentiate them from the originals
			if (offsetPaste) {
				viewGeometry.offset(20*m_pasteCount, 20*m_pasteCount);
			}
			AddItemCommand * addItemCommand = newAddItemCommand(crossViewType, mp->moduleID(), viewGeometry, newID, false, mp->modelIndex(), mp->originalModelIndex(), parentCommand);
			if (mp->itemType() == ModelPart::ResizableBoard) {
				bool ok;
				qreal w = mp->prop("width").toDouble(&ok);
				if (ok) {
					qreal h = mp->prop("height").toDouble(&ok);
					if (ok) {
						new ResizeBoardCommand(this, newID, w, h, w, h, parentCommand);
					}
				}
			}
			else if (mp->itemType() == ModelPart::Note) {
				ChangeNoteTextCommand * changeNoteTextCommand = new ChangeNoteTextCommand(this, newID, mp->instanceText(), mp->instanceText(), viewGeometry.rect().size(), viewGeometry.rect().size(), parentCommand);
				changeNoteTextCommand->setFirstTime(false);
			}
			else if (mp->itemType() == ModelPart::Ruler) {
				QString w = mp->prop("width").toString();
				QString w2 = w;
				w.chop(2);
				int units = w2.endsWith("cm") ? 0 : 1;
				new ResizeBoardCommand(this, newID, w.toDouble(), units, w.toDouble(), units, parentCommand);
				mp->setProp("width", "");		// ResizeBoardCommand won't execute if the width property is already set
			}

			if (!labelGeometry.isNull()) {
				QDomElement clone = labelGeometry.cloneNode(true).toElement();
				bool ok;
				qreal x = clone.attribute("x").toDouble(&ok);
				if (ok) {
					x += (20 * m_pasteCount);
					clone.setAttribute("x", QString::number(x));
				}
				qreal y = clone.attribute("y").toDouble(&ok);
				if (ok) {
					y += (20 * m_pasteCount);
					clone.setAttribute("y", QString::number(y));
				}
				new RestoreLabelCommand(this, newID, clone, parentCommand);
			}

			new CheckStickyCommand(this, crossViewType, newID, false, parentCommand);
			if (mp->moduleID() == ModuleIDNames::wireModuleIDName) {
				addWireExtras(newID, view, parentCommand);
			}
			else if (mp->itemType() == ModelPart::Module) {
				RestoreIndexesCommand * restoreIndexesCommand = new RestoreIndexesCommand(this, newID, NULL, true, parentCommand);
				addItemCommand->addRestoreIndexesCommand(restoreIndexesCommand);
			}
		}
	}

	if (zmap.count() > 0) {
		qreal z = 0.5;
		foreach (ItemBase * itemBase, zmap.values()) {
			itemBase->slamZ(z);
			z += ViewLayer::getZIncrement();
		}
		foreach (ViewLayer * viewLayer, m_viewLayers) {
			if (viewLayer != NULL) viewLayer->resetNextZ(z);
		}
	}

	QStringList alreadyConnected;

	// now restore connections
	foreach (ModelPart * mp, modelParts) {
		QDomElement instance = mp->instanceDomElement();
		if (instance.isNull()) continue;

		QDomElement views = instance.firstChildElement("views");
		if (views.isNull()) continue;

		QDomElement view = views.firstChildElement(viewName);
		if (view.isNull()) continue;

		QDomElement connectors = view.firstChildElement("connectors");
		if (connectors.isNull()) continue;

		QDomElement connector = connectors.firstChildElement("connector");
		while (!connector.isNull()) {
			// TODO: make sure layerkin are searched for connectors
			QString fromConnectorID = connector.attribute("connectorId");
			QDomElement connects = connector.firstChildElement("connects");
			if (!connects.isNull()) {
				QDomElement connect = connects.firstChildElement("connect");
				while (!connect.isNull()) {
					handleConnect(connect, mp, fromConnectorID, alreadyConnected, newItems, doRatsnest, parentCommand);
					connect = connect.nextSiblingElement("connect");
				}
			}

			connector = connector.nextSiblingElement("connector");
		}
	}

	if (parentCommand == NULL) {
		foreach (ItemBase * item, newItems) {
			if (item->sticky()) {
				stickyScoop(item, false, NULL);
			}
		}

		m_pasteCount = 0;
		this->scene()->clearSelection();
		if (doRatsnest) {
			cleanUpWires(false, NULL, false);
		}
	}
	else {
		if (offsetPaste) {
			// m_pasteCount used for offsetting paste items, not a count of how many items are pasted
			m_pasteCount++;
		}
		//emit ratsnestChangeSignal(this, parentCommand);
		if (doRatsnest) {
			new CleanUpWiresCommand(this, false, parentCommand);
		}
	}

	setIgnoreSelectionChangeEvents(false);
}

void SketchWidget::handleConnect(QDomElement & connect, ModelPart * mp, const QString & fromConnectorID, QStringList & alreadyConnected, QHash<long, ItemBase *> & newItems, bool doRatsnest, QUndoCommand * parentCommand)
{
	bool ok;
	QHash<long, ItemBase *> otherNewItems;
	long modelIndex = connect.attribute("modelIndex").toLong(&ok);
	QString toConnectorID = connect.attribute("connectorId");
	if (ok) {
		QString already = ((mp->modelIndex() <= modelIndex) ? QString("%1.%2.%3.%4") : QString("%3.%4.%1.%2"))
							.arg(mp->modelIndex()).arg(fromConnectorID).arg(modelIndex).arg(toConnectorID);
		if (alreadyConnected.contains(already)) return;

		alreadyConnected.append(already);
	}
	else {
		// connected inside a module
		QDomElement p = connect.firstChildElement("mp");
		if (p.isNull()) return;

		modelIndex = p.attribute("i").toLong(&ok);
		if (!ok) return;

		QList<long> indexes;
		indexes.append(modelIndex);
		QString s = QString::number(modelIndex);
		while (true) {
			p = p.firstChildElement("mp");
			if (p.isNull()) break;

			long originalModelIndex = p.attribute("i").toLong(&ok);
			if (!ok) return;

			indexes.append(originalModelIndex);
			s.append(".");
			s.append(QString::number(originalModelIndex));
		}

		QString already = ((mp->modelIndex() <= modelIndex) ? QString("%1.%2.%3.%4") : QString("%3.%4.%1.%2"))
					.arg(mp->modelIndex()).arg(fromConnectorID).arg(s).arg(toConnectorID);
		if (alreadyConnected.contains(already)) return;

		alreadyConnected.append(already);

		if (parentCommand == NULL) {
			// walk down the tree of original model indexes until you find the part that you actually connect to
			ItemBase * toBase = newItems.value(modelIndex, NULL);
			if (toBase == NULL) return;

			toBase = findModulePart(toBase, indexes);
			if (toBase == NULL) return;

			modelIndex = toBase->modelPart()->modelIndex();
			otherNewItems.insert(modelIndex, toBase);
		}
		else {
			new ModuleChangeConnectionCommand(this, BaseCommand::SingleView,
											ItemBase::getNextID(mp->modelIndex()), fromConnectorID,
											indexes, toConnectorID, doRatsnest && doRatsnestOnCopy(),
											true, true, parentCommand);

			return;
		}
	}

	if (parentCommand == NULL) {
		ItemBase * fromBase = newItems.value(mp->modelIndex(), NULL);
		ItemBase * toBase = newItems.value(modelIndex, NULL);
		if (toBase == NULL) {
			toBase = otherNewItems.value(modelIndex, NULL);
		}
		if (fromBase == NULL || toBase == NULL) return;

		// TODO: make sure layerkin are searched for connectors
		ConnectorItem * fromConnectorItem = fromBase->findConnectorItemNamed(fromConnectorID);
		ConnectorItem * toConnectorItem = toBase->findConnectorItemNamed(toConnectorID);
		if (fromConnectorItem == NULL || toConnectorItem == NULL) return;

		fromConnectorItem->connectTo(toConnectorItem);
		toConnectorItem->connectTo(fromConnectorItem);
		fromConnectorItem->connector()->connectTo(toConnectorItem->connector());
		if (fromConnectorItem->attachedToItemType() == ModelPart::Wire && toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			fromConnectorItem->setHidden(false);
			toConnectorItem->setHidden(false);
		}
		return;
	}

	new ChangeConnectionCommand(this, BaseCommand::SingleView,
								ItemBase::getNextID(mp->modelIndex()), fromConnectorID,
								ItemBase::getNextID(modelIndex), toConnectorID,
								true, true, parentCommand);
	if (doRatsnest && doRatsnestOnCopy()) {
		new RatsnestCommand(this, BaseCommand::SingleView,
									ItemBase::getNextID(mp->modelIndex()), fromConnectorID,
									ItemBase::getNextID(modelIndex), toConnectorID,
									true, true, parentCommand);
	}
}

void SketchWidget::addWireExtras(long newID, QDomElement & view, QUndoCommand * parentCommand)
{
	QDomElement extras = view.firstChildElement("wireExtras");
	if (extras.isNull()) return;

	bool ok;
	int w = extras.attribute("width").toInt(&ok);
	if (ok) {
		new WireWidthChangeCommand(this, newID, w, w, parentCommand);
	}

	QString colorString = extras.attribute("color");
	if (!colorString.isEmpty()) {
		qreal op = extras.attribute("opacity").toDouble(&ok);
		if (!ok) {
			op = 1.0;
		}
		new WireColorChangeCommand(this, newID, colorString, colorString, op, op, parentCommand);
	}
}

ItemBase * SketchWidget::addItem(const QString & moduleID, BaseCommand::CrossViewType crossViewType, const ViewGeometry & viewGeometry, long id, long modelIndex, long originalModelIndex, AddDeleteItemCommand * originatingCommand) {
	if (m_paletteModel == NULL) return NULL;

	ItemBase * itemBase = NULL;
	ModelPart * modelPart = m_paletteModel->retrieveModelPart(moduleID);
	if (modelPart != NULL) {
		QApplication::setOverrideCursor(Qt::WaitCursor);
		statusMessage(tr("loading part"));
		itemBase = addItem(modelPart, crossViewType, viewGeometry, id, modelIndex, originalModelIndex, originatingCommand, NULL);
		if (itemBase != NULL && originalModelIndex > 0) {
			itemBase->modelPart()->setOriginalModelIndex(originalModelIndex);
			// DebugDialog::debug(QString("additem original model index %1 %2").arg(originalModelIndex).arg((long) modelPart, 0, 16));

		}
		statusMessage(tr("done loading"), 2000);
		QApplication::restoreOverrideCursor();
	}

	return itemBase;
}

ItemBase * SketchWidget::makeModule(ModelPart * modelPart, long originalModelIndex, QList<ModelPart *> & modelParts, const ViewGeometry & viewGeometry, long id)
{
	modelPart->setModelIndexFromMultiplied(id);
	if (originalModelIndex > 0) {
		modelPart->setOriginalModelIndex(originalModelIndex);
		// DebugDialog::debug(QString("group original model index %1 %2").arg(originalModelIndex).arg((long) modelPart, 0, 16));
	}
	if (modelParts.count() <= 0) {
		foreach (QObject * object, modelPart->children()) {
			ModelPart * cmp = dynamic_cast<ModelPart *>(object);
			if (cmp != NULL) {
				modelParts.append(cmp);
			}
		}
	}

	DebugDialog::debug(QString("mp:%1, omi:%2, view:%3, id:%4").arg(modelPart->modelIndex()).arg(originalModelIndex).arg(m_viewIdentifier).arg(id));

	bool doExternals = false;
	QHash<QList<long> *, QString > externalConnectors;
	if (modelParts.count() <= 0) {
		doExternals = modelPart->parent() == m_sketchModel->root();  // only need external connectors for top level modules (not for modules-in-modules)
		if (!m_sketchModel->paste(m_paletteModel, modelPart->modelPartShared()->path(), modelParts, doExternals ? &externalConnectors : NULL)) {
			return NULL;
		}

		foreach (ModelPart * sub, modelParts) {
			sub->setParent(modelPart);
		}
	}

	loadFromModel(modelParts, BaseCommand::SingleView, NULL, false, false);

	QHash<long, ModelPart *> newItems;
	QList<long> ids;
	foreach (ModelPart * mp, modelParts) {
		ItemBase * item = mp->viewItem(scene());
		if (item) {
			if (doExternals) {
				newItems.insert(mp->modelIndex(), mp);
			}
			ids.append(item->id());
		}
	}

	group(modelPart->moduleID(), id, ids, viewGeometry, false);
	ItemBase * toBase = modelPart->viewItem(scene());

	if (doExternals) {
		if (toBase) {
			foreach (QList<long> * indexes, externalConnectors.keys()) {
				QString connectorID = externalConnectors.value(indexes, "");
				if (connectorID.isEmpty()) continue;

				ModelPart * first = newItems.value(indexes->at(0));
				if (first == NULL) continue;

				ItemBase * target = first->viewItem(scene());
				if (target == NULL) continue;

				target = findModulePart(target, *indexes);
				if (target == NULL) continue;

				ConnectorItem * connectorItem = findConnectorItem(target, connectorID, true);
				if (connectorItem == NULL) continue;

				connectorItem->connector()->setExternal(true);

				// must call setIgnoreAncestorFlag here, since group() is called previously,
				// externals weren't set up
				// and group() calls setIgnoreAncestorFlagIfExternal
				connectorItem->setIgnoreAncestorFlag(true);
			}
		}

		foreach (QList<long> * l, externalConnectors.keys()) {
			delete l;
		}
	}

	if (toBase) {
		if (modelPart->parent() == m_sketchModel->root()) {
			dynamic_cast<GroupItem *>(toBase)->collectExternalConnectorItems();
		}
	}

	return toBase;
}

ItemBase * SketchWidget::addItem(ModelPart * modelPart, BaseCommand::CrossViewType crossViewType, const ViewGeometry & viewGeometry, long id, long modelIndex, long originalModelIndex, AddDeleteItemCommand * originatingCommand, PaletteItem* partsEditorPaletteItem) {

	ModelPart * mp = NULL;
	if (modelIndex >= 0) {
		// used only with Paste, so far--this assures that parts created across views will share the same ModelPart
		mp = m_sketchModel->findModelPart(modelPart->moduleID(), id);
	}
	if (mp == NULL) {
		modelPart = m_sketchModel->addModelPart(m_sketchModel->root(), modelPart);
	}
	else {
		modelPart = mp;
	}
	if (modelPart == NULL) return NULL;

	ItemBase * newItem = addItemAux(modelPart, viewGeometry, id, originalModelIndex, originatingCommand, partsEditorPaletteItem, true, m_viewIdentifier);
	if (crossViewType == BaseCommand::CrossView) {
		emit itemAddedSignal(modelPart, viewGeometry, id, originatingCommand ? originatingCommand->dropOrigin() : NULL);
	}

	return newItem;
}

ItemBase * SketchWidget::addItemAux(ModelPart * modelPart, const ViewGeometry & viewGeometry, long id, long originalModelIndex, AddDeleteItemCommand * originatingCommand, PaletteItem* partsEditorPaletteItem, bool doConnectors, ViewIdentifierClass::ViewIdentifier viewIdentifier)
{
	Q_UNUSED(partsEditorPaletteItem);
	if (viewIdentifier == ViewIdentifierClass::UnknownView) {
		viewIdentifier = m_viewIdentifier;
	}

	if (doConnectors) {
		modelPart->initConnectors();    // is a no-op if connectors already in place
	}

	PaletteItem* paletteItem = NULL;
	switch (modelPart->itemType()) {
		case ModelPart::Module:
			if (doConnectors || originatingCommand || originalModelIndex > 0)
			{
				QList<ModelPart *>  modelParts;
				return makeModule(modelPart, originalModelIndex, modelParts, viewGeometry, id);
			}
			break;					// module at drag-and-drop time falls through and a fake paletteItem is created for dragging
		case ModelPart::Wire:
		{
			bool virtualWire = viewGeometry.getVirtual();
			Wire * wire = NULL;
			if (virtualWire) {
				VirtualWire * vw = new VirtualWire(modelPart, viewIdentifier, viewGeometry, id, m_wireMenu);
				setClipEnds(vw, true);
				wire = vw;
             	wire->setUp(getWireViewLayerID(viewGeometry), m_viewLayers, this);
				
				// prevents virtual wires from flashing up on screen
			}
			else {
				if (viewGeometry.getTrace()) {
					TraceWire * traceWire = new TraceWire(modelPart, viewIdentifier, viewGeometry, id, m_wireMenu);
					setClipEnds(traceWire, true);
					wire = traceWire;
				}
				else {
					wire = new Wire(modelPart, viewIdentifier, viewGeometry, id, m_wireMenu);
					if (!wire->hasAnyFlag(ViewGeometry::RatsnestFlag)) {
						wire->setNormal(true);
					}
				}
				wire->setUp(getWireViewLayerID(viewGeometry), m_viewLayers, this);
			}

			setWireVisible(wire);

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
			DebugDialog::debug(QString("adding wire %1 %2 %3 %4 %5")
				.arg(wire->id())
				.arg(viewIdentifier)
				.arg(viewGeometry.flagsAsInt())
				.arg((long) wire, 0, 16)
				.arg((long) static_cast<QGraphicsItem *>(wire), 0, 16)
				);


			return wire;
		}
		case ModelPart::Note:
		{
			Note * note = new Note(modelPart, viewIdentifier, viewGeometry, id, NULL);
			note->setViewLayerID(getNoteViewLayerID(), m_viewLayers);
			note->setZValue(note->z());
			note->setVisible(true);
			addToScene(note, getNoteViewLayerID());
			return note;
		}
		case ModelPart::CopperFill:
			paletteItem = new GroundPlane(modelPart, viewIdentifier, viewGeometry, id, m_itemMenu);
			break;
		case ModelPart::Jumper:
			paletteItem = new JumperItem(modelPart, viewIdentifier, viewGeometry, id, m_itemMenu);
			break;
		case ModelPart::ResizableBoard:
			paletteItem = new ResizableBoard(modelPart, viewIdentifier, viewGeometry, id, m_itemMenu);
			break;
		case ModelPart::Logo:
			paletteItem = new LogoItem(modelPart, viewIdentifier, viewGeometry, id, m_itemMenu);
			break;
		case ModelPart::Ruler:
			paletteItem = new Ruler(modelPart, viewIdentifier, viewGeometry, id, m_itemMenu);
			break;
		case ModelPart::Symbol:
			paletteItem = new SymbolPaletteItem(modelPart, viewIdentifier, viewGeometry, id, m_itemMenu);
			break;
		default:
			{
				QString family = modelPart->properties().value("family", "");
				if (modelPart->moduleID().compare(ModuleIDNames::resistorModuleIDName) == 0) {
					paletteItem = new Resistor(modelPart, viewIdentifier, viewGeometry, id, m_itemMenu, true);
				}
				else if (family.compare("mystery part", Qt::CaseInsensitive) == 0) {
					paletteItem = new MysteryPart(modelPart, viewIdentifier, viewGeometry, id, m_itemMenu, true);
				}
				else if (family.compare("pin header", Qt::CaseInsensitive) == 0) {
					paletteItem = new PinHeader(modelPart, viewIdentifier, viewGeometry, id, m_itemMenu, true);
				}
				else if (family.compare("generic IC", Qt::CaseInsensitive) == 0) {
					paletteItem = new Dip(modelPart, viewIdentifier, viewGeometry, id, m_itemMenu, true);
				}
				else {
					paletteItem = new PaletteItem(modelPart, viewIdentifier, viewGeometry, id, m_itemMenu);
				}
			}
			break;
	}

	bool ok;
	ItemBase * itemBase = addPartItem(modelPart, paletteItem, doConnectors, ok, viewIdentifier);
	DebugDialog::debug(QString("adding part %1 %2 %4 %5 %3")
		.arg(id)
		.arg(paletteItem->title())
		.arg(viewIdentifier)
		.arg((long) itemBase, 0, 16)
		.arg((long) static_cast<QGraphicsItem *>(itemBase), 0, 16));
	setNewPartVisible(itemBase);
	return itemBase;
}


void SketchWidget::setNewPartVisible(ItemBase * itemBase) {
	Q_UNUSED(itemBase);
	// defaults to visible, so do nothing
}

void SketchWidget::checkSticky(long id, bool doEmit, bool checkCurrent, CheckStickyCommand * checkStickyCommand)
{
	ItemBase * itemBase = findItem(id);
	if (itemBase == NULL) return;

	if (itemBase->sticky()) {
		stickyScoop(itemBase, checkCurrent, checkStickyCommand);
	}
	else {
		ItemBase * stickyOne = overSticky(itemBase);
		ItemBase * wasStickyOne = itemBase->stickingTo();
		if (stickyOne != wasStickyOne) {
			if (wasStickyOne != NULL) {
				wasStickyOne->addSticky(itemBase, false);
				itemBase->addSticky(wasStickyOne, false);
				if (checkStickyCommand) {
					checkStickyCommand->stick(this, wasStickyOne->id(), itemBase->id(), false);
				}
			}
			if (stickyOne != NULL) {
				stickyOne->addSticky(itemBase, true);
				itemBase->addSticky(stickyOne, true);
				if (checkStickyCommand) {
					checkStickyCommand->stick(this, stickyOne->id(), itemBase->id(), true);
				}
			}
		}
	}

	if (doEmit) {
		checkStickySignal(id, false, false, checkStickyCommand);
	}
}

PaletteItem* SketchWidget::addPartItem(ModelPart * modelPart, PaletteItem * paletteItem, bool doConnectors, bool & ok, ViewIdentifierClass::ViewIdentifier viewIdentifier) {

	ok = false;
	ViewLayer::ViewLayerID viewLayerID = getViewLayerID(modelPart, viewIdentifier);

	// render it, only if the layer is defined in the fzp file
	// if the view is not defined in the part file, without this condition
	// fritzing crashes
	if(viewLayerID != ViewLayer::UnknownLayer) {
		if (paletteItem->renderImage(modelPart, viewIdentifier, m_viewLayers, viewLayerID, doConnectors)) {
			addToScene(paletteItem, paletteItem->viewLayerID());
			paletteItem->loadLayerKin(m_viewLayers);
			foreach (ItemBase * lkpi, paletteItem->layerKin()) {
				this->scene()->addItem(lkpi);
				lkpi->setHidden(!layerIsVisible(lkpi->viewLayerID()));
			}

			ok = true;
			return paletteItem;
		}
		else {
			// nobody falls through to here now?

			QMessageBox::information(dynamic_cast<QMainWindow *>(this->window()), QObject::tr("Fritzing"),
									 QObject::tr("The file %1 is not a Fritzing file (1).").arg(modelPart->modelPartShared()->path()) );


			DebugDialog::debug(QString("addPartItem renderImage failed %1").arg(modelPart->moduleID()) );

			//paletteItem->modelPart()->removeViewItem(paletteItem);
			//delete paletteItem;
			//return NULL;
			scene()->addItem(paletteItem);
			//paletteItem->setVisible(false);
			return paletteItem;
		}
	} else {
		return paletteItem;
	}
}

void SketchWidget::addToScene(ItemBase * item, ViewLayer::ViewLayerID viewLayerID) {
	scene()->addItem(item);
 	item->setSelected(true);
 	item->setHidden(!layerIsVisible(viewLayerID));
}

ItemBase * SketchWidget::findItem(long id) {
	// TODO:  replace scene()->items()

	QList<QGraphicsItem *> items = this->scene()->items();
	for (int i = 0; i < items.size(); i++) {
		ItemBase* base = dynamic_cast<ItemBase *>(items[i]);
		if (base == NULL) continue;

		if (base->id() == id) {
			return base;
		}
	}

	return NULL;
}

void SketchWidget::deleteItem(long id, bool deleteModelPart, bool doEmit, bool later, RestoreIndexesCommand * restore) {
	ItemBase * pitem = findItem(id);
	DebugDialog::debug(QString("delete item (1) %1 %2 %3 %4").arg(id).arg(doEmit).arg(m_viewIdentifier).arg((long) pitem, 0, 16) );
	if (pitem != NULL) {
		//if (pitem->itemType() == ModelPart::Module) {
		//	m_sketchModel->walk(m_sketchModel->root(),0);
		//}
		if (restore != NULL) {
			if (restore->modelPartTiny() == NULL) {
				ModelPartTiny * mpt = m_sketchModel->makeTiny(pitem->modelPart());
				restore->setModelPartTiny(mpt);
			}
		}
		deleteItem(pitem, deleteModelPart, doEmit, later);
	}
	else {
		if (doEmit) {
			emit itemDeletedSignal(id);
		}
	}
}

void SketchWidget::deleteItem(ItemBase * itemBase, bool deleteModelPart, bool doEmit, bool later)
{
	long id = itemBase->id();
	DebugDialog::debug(QString("delete item (2) %1 %2 %3 %4").arg(id).arg(itemBase->title()).arg(m_viewIdentifier).arg((long) itemBase, 0, 16) );

	if (m_infoView != NULL) {
		m_infoView->unregisterCurrentItemIf(itemBase->id());
	}
	if (itemBase == this->m_lastPaletteItemSelected) {
		setLastPaletteItemSelected(NULL);
	}
	m_lastSelected.removeOne(itemBase);
	removeIfFixedPos(itemBase);


	if (deleteModelPart) {
		ModelPart * modelPart = itemBase->modelPart();
		if (modelPart != NULL) {
			m_sketchModel->removeModelPart(modelPart);
			delete modelPart;
		}
	}

	itemBase->removeLayerKin();
	this->scene()->removeItem(itemBase);

	if (later) {
		itemBase->deleteLater();
	}
	else {
		delete itemBase;
	}

	if (doEmit) {
		emit itemDeletedSignal(id);
	}

}

void SketchWidget::deleteItem() {
	cutDeleteAux("Delete");
}

void SketchWidget::cutDeleteAux(QString undoStackMessage) {

	//DebugDialog::debug("before delete");
	//m_sketchModel->walk(m_sketchModel->root(), 0);

    // get sitems first, before calling stackSelectionState
    // because selectedItems will return an empty list
	const QList<QGraphicsItem *> sitems = scene()->selectedItems();

	QSet<ItemBase *> deletedItems;

	foreach (QGraphicsItem * sitem, sitems) {
		if (!canDeleteItem(sitem)) continue;

		// canDeleteItem insures dynamic_cast<ItemBase *>(sitem)->layerKinChief() won't break
		deletedItems.insert(dynamic_cast<ItemBase *>(sitem)->layerKinChief());
	}

	if (deletedItems.count() <= 0) {
		return;
	}

	deleteAux(deletedItems, undoStackMessage);
}

void SketchWidget::deleteAux(QSet<ItemBase *> & deletedItems, QString undoStackMessage) 
{
	QString string;

	if (deletedItems.count() == 1) {
		ItemBase * firstItem = *(deletedItems.begin());
		string = tr("%1 %2").arg(undoStackMessage).arg(firstItem->title());
	}
	else {
		string = tr("%1 %2 items").arg(undoStackMessage).arg(QString::number(deletedItems.count()));
	}

	QUndoCommand * parentCommand = new QUndoCommand(string);
	parentCommand->setText(string);

    stackSelectionState(false, parentCommand);

	bool skipMe = deleteMiddle(deletedItems, parentCommand);

	foreach (ItemBase * itemBase, deletedItems) {
		if (itemBase->itemType() == ModelPart::Module) {
			// TODO: do these need reviewDeletedConnections?
			ConnectorPairHash externalConnectors;
			collectModuleExternalConnectors(itemBase, itemBase, externalConnectors);
			foreach (ConnectorItem * fromConnectorItem, externalConnectors.uniqueKeys()) {
				foreach (ConnectorItem * toConnectorItem, externalConnectors.values(fromConnectorItem)) {
					extendChangeConnectionCommand(fromConnectorItem, toConnectorItem, false, true, parentCommand);
				}
			}
		}
	}

	foreach (ItemBase * itemBase, deletedItems) {
		Note * note = dynamic_cast<Note *>(itemBase);
		if (note != NULL) {
			new ChangeNoteTextCommand(this, note->id(), note->text(), note->text(), QSizeF(), QSizeF(), parentCommand);
		}
		else {
			new ChangeLabelTextCommand(this, itemBase->id(), itemBase->instanceTitle(), itemBase->instanceTitle(), parentCommand);
		}
	}


	emit ratsnestChangeSignal(this, parentCommand);

	new CleanUpWiresCommand(this, skipMe, parentCommand);

	foreach (ItemBase * itemBase, deletedItems) {
		if (itemBase->itemType() == ModelPart::Module) {
			ModelPartTiny * mpt = m_sketchModel->makeTiny(itemBase->modelPart());
			new RestoreIndexesCommand(this, itemBase->id(), mpt, false, parentCommand);
		}
	}

	// actual delete commands must come last for undo to work properly
	foreach (ItemBase * itemBase, deletedItems) {
		BaseCommand::CrossViewType crossView = BaseCommand::CrossView;
		if (itemBase->getVirtual() || (dynamic_cast<TraceWire *>(itemBase) != NULL)) {
			crossView = BaseCommand::SingleView;
		}
		makeDeleteItemCommand(itemBase, crossView, parentCommand);
	}
   	m_undoStack->push(parentCommand);
}

bool SketchWidget::deleteMiddle(QSet<ItemBase *> & deletedItems, QUndoCommand * parentCommand) {
	QHash<ItemBase *, QMultiHash<ConnectorItem *, ConnectorItem *> * > deletedConnections;

	foreach (ItemBase * itemBase, deletedItems) {
		ConnectorPairHash * connectorHash = new ConnectorPairHash;
		itemBase->collectConnectors(*connectorHash, this->scene());
		deletedConnections.insert(itemBase, connectorHash);
	}

	bool skipMe = reviewDeletedConnections(deletedItems, deletedConnections, parentCommand);

	foreach ( ConnectorPairHash * connectorHash, deletedConnections.values())
	{
		// now prepare to disconnect all the deleted item's connectors
		foreach (ConnectorItem * fromConnectorItem,  connectorHash->uniqueKeys()) {
			foreach (ConnectorItem * toConnectorItem, connectorHash->values(fromConnectorItem)) {
				extendChangeConnectionCommand(fromConnectorItem, toConnectorItem,
											  false, true, parentCommand);
				fromConnectorItem->tempRemove(toConnectorItem, false);
				toConnectorItem->tempRemove(fromConnectorItem, false);
			}
		}
   	}

	foreach (ConnectorPairHash * connectorHash, deletedConnections.values())
	{
		delete connectorHash;
	}

	return skipMe;
}


bool SketchWidget::reviewDeletedConnections(QSet<ItemBase *> & deletedItems, QHash<ItemBase *, ConnectorPairHash * > & deletedConnections, QUndoCommand * parentCommand) {
	Q_UNUSED(parentCommand);
	Q_UNUSED(deletedConnections);
	Q_UNUSED(deletedItems);

	return false;
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

	new ChangeConnectionCommand(this, BaseCommand::CrossView,
								fromItem->id(), fromConnectorItem->connectorSharedID(),
								toItem->id(), toConnectorItem->connectorSharedID(),
								connect, seekLayerKin, parentCommand);
	new RatsnestCommand(this, BaseCommand::CrossView,
								fromItem->id(), fromConnectorItem->connectorSharedID(),
								toItem->id(), toConnectorItem->connectorSharedID(),
								connect, seekLayerKin, parentCommand);
}


long SketchWidget::createWire(ConnectorItem * from, ConnectorItem * to, ViewGeometry::WireFlags wireFlags,
							  bool addItNow, bool doRatsnest,
							  BaseCommand::CrossViewType crossViewType, QUndoCommand * parentCommand)
{
	long newID = ItemBase::getNextID();
	ViewGeometry viewGeometry;
	QPointF fromPos = from->sceneAdjustedTerminalPoint(NULL);
	viewGeometry.setLoc(fromPos);
	QPointF toPos = to->sceneAdjustedTerminalPoint(NULL);
	QLineF line(0, 0, toPos.x() - fromPos.x(), toPos.y() - fromPos.y());
	viewGeometry.setLine(line);
	viewGeometry.setWireFlags(wireFlags);

	DebugDialog::debug(QString("creating virtual wire %11: %1, flags: %6, from %7 %8, to %9 %10, frompos: %2 %3, topos: %4 %5")
		.arg(newID)
		.arg(fromPos.x()).arg(fromPos.y())
		.arg(toPos.x()).arg(toPos.y())
		.arg(wireFlags)
		.arg(from->attachedToTitle()).arg(from->connectorSharedID())
		.arg(to->attachedToTitle()).arg(to->connectorSharedID())
		.arg(m_viewIdentifier)
		);

	new AddItemCommand(this, crossViewType, ModuleIDNames::wireModuleIDName, viewGeometry, newID, false, -1, -1, parentCommand);
	new CheckStickyCommand(this, crossViewType, newID, false, parentCommand);
	new ChangeConnectionCommand(this, crossViewType, from->attachedToID(), from->connectorSharedID(),
			newID, "connector0", true, true, parentCommand);
	new ChangeConnectionCommand(this, crossViewType, to->attachedToID(), to->connectorSharedID(),
			newID, "connector1", true, true, parentCommand);
	if (doRatsnest) {
		new RatsnestCommand(this, crossViewType, from->attachedToID(), from->connectorSharedID(),
				newID, "connector0", true, true, parentCommand);
		new RatsnestCommand(this, crossViewType, to->attachedToID(), to->connectorSharedID(),
				newID, "connector1", true, true, parentCommand);
	}

	if (addItNow) {
		ItemBase * newItemBase = addItemAux(m_paletteModel->retrieveModelPart(ModuleIDNames::wireModuleIDName), viewGeometry, newID, -1, NULL, NULL, true, m_viewIdentifier);
		if (newItemBase) {
			tempConnectWire(dynamic_cast<Wire *>(newItemBase), from, to);
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
	//DebugDialog::debug(QString("rotating %1 %2").arg(id).arg(degrees) );

	if (!isVisible()) return;

	ItemBase * pitem = findItem(id);
	if (pitem != NULL) {
		pitem->rotateItem(degrees);
	}

}
void SketchWidget::transformItem(long id, const QMatrix & matrix) {
	ItemBase * pitem = findItem(id);
	if (pitem != NULL) {
		pitem->transformItem2(matrix);
	}
}

void SketchWidget::flipItem(long id, Qt::Orientations orientation) {
	DebugDialog::debug(QString("fliping %1 %2").arg(id).arg(orientation) );

	if (!isVisible()) return;

	 ItemBase * pitem = findItem(id);
	if (pitem != NULL) {
		pitem->flipItem(orientation);
	}
}


void SketchWidget::changeWire(long fromID, QLineF line, QPointF pos, bool useLine)
{
	DebugDialog::debug(QString("change wire %1; %2,%3,%4,%5; %6,%7; %8")
			.arg(fromID)
			.arg(line.x1())
			.arg(line.y1())
			.arg(line.x2())
			.arg(line.y2())
			.arg(pos.x())
			.arg(pos.y())
			.arg(useLine) );
	ItemBase * fromItem = findItem(fromID);
	if (fromItem == NULL) return;

	Wire* wire = dynamic_cast<Wire *>(fromItem);
	if (wire == NULL) return;

	wire->setLineAnd(line, pos, useLine);
	wire->updateConnections(wire->connector0());
	wire->updateConnections(wire->connector1());
}

void SketchWidget::selectItem(long id, bool state, bool updateInfoView, bool doEmit) {
	this->clearHoldingSelectItem();
	ItemBase * item = findItem(id);
	if (item != NULL) {
		item->setSelected(state);
		if(updateInfoView) {
			// show something in the info view, even if it's not selected
			InfoGraphicsView::viewItemInfo(item);
		}
		if (doEmit) {
			emit itemSelectedSignal(id, state);
		}
	}

	PaletteItem *pitem = dynamic_cast<PaletteItem*>(item);
	if(pitem) {
		setLastPaletteItemSelected(pitem);
	}
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

void SketchWidget::copy() {
	QList<ItemBase *> bases;

	// sort them in z-order so the copies also appear in the same order
	sortSelectedByZ(bases);

    QByteArray itemData;
	QXmlStreamWriter streamWriter(&itemData);

	QList<long> modelIndexes;
	streamWriter.writeStartElement("module");
	streamWriter.writeAttribute("fritzingVersion", Version::versionString());
	streamWriter.writeStartElement("instances");
	foreach (ItemBase * base, bases) {
		base->modelPart()->saveInstances(streamWriter, false);
		modelIndexes.append(base->modelPart()->modelIndex());
	}
	streamWriter.writeEndElement();
	streamWriter.writeEndElement();

	// only preserve connections for copied items that connect to each other
	QByteArray newItemData = removeOutsideConnections(itemData, modelIndexes);

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dnditemsdata", newItemData);
	mimeData->setData("text/plain", newItemData);

	QClipboard *clipboard = QApplication::clipboard();
	if (clipboard == NULL) {
		// shouldn't happen
		delete mimeData;
		return;
	}

	clipboard->setMimeData(mimeData, QClipboard::Clipboard);
}

QByteArray SketchWidget::removeOutsideConnections(const QByteArray & itemData, QList<long> & modelIndexes) {
	// now have to remove each connection that points to a part outside of the set of parts being copied

	QDomDocument domDocument;
	QString errorStr;
	int errorLine;
	int errorColumn;
	bool result = domDocument.setContent(itemData, &errorStr, &errorLine, &errorColumn);
	if (!result) return ___emptyByteArray___;

	QDomElement root = domDocument.documentElement();
   	if (root.isNull()) {
   		return ___emptyByteArray___;
	}

	QDomElement instances = root.firstChildElement("instances");
	if (instances.isNull()) return ___emptyByteArray___;

	QDomElement instance = instances.firstChildElement("instance");
	while (!instance.isNull()) {
		QDomElement views = instance.firstChildElement("views");
		if (!views.isNull()) {
			QDomElement view = views.firstChildElement();
			while (!view.isNull()) {
				QDomElement connectors = view.firstChildElement("connectors");
				if (!connectors.isNull()) {
					QDomElement connector = connectors.firstChildElement("connector");
					while (!connector.isNull()) {
						QDomElement connects = connector.firstChildElement("connects");
						if (!connects.isNull()) {
							QDomElement connect = connects.firstChildElement("connect");
							QList<QDomElement> toDelete;
							while (!connect.isNull()) {
								long modelIndex = connect.attribute("modelIndex").toLong();
								if (!modelIndexes.contains(modelIndex)) {
									toDelete.append(connect);
								}

								connect = connect.nextSiblingElement("connect");
							}

							foreach (QDomElement connect, toDelete) {
								QDomNode removed = connects.removeChild(connect);
								if (removed.isNull()) {
									DebugDialog::debug("shit");
								}
							}
						}
						connector = connector.nextSiblingElement("connector");
					}
				}

				view = view.nextSiblingElement();
			}
		}

		instance = instance.nextSiblingElement("instance");
	}

	return domDocument.toByteArray();
}


void SketchWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if (dragEnterEventAux(event)) {
		setupAutoscroll(false);
		event->acceptProposedAction();
	}
	else {
		// subclass seems to call acceptProposedAction so don't invoke it
		// QGraphicsView::dragEnterEvent(event);
		event->ignore();
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
	} else {
		ViewGeometry viewGeometry;
		QPointF p = QPointF(this->mapToScene(event->pos())) - offset;
		viewGeometry.setLoc(p);

		long fromID = ItemBase::getNextID();

		// create temporary item
		// don't need connectors for breadboard
		// TODO: how to specify which parts don't need connectors during drag and drop from palette?

		bool doConnectors = true;
		switch (modelPart->itemType()) {
			case ModelPart::Breadboard:
			case ModelPart::Board:
			case ModelPart::ResizableBoard:
			case ModelPart::Logo:
			case ModelPart::Ruler:
			case ModelPart::Module:
			case ModelPart::Symbol:
			case ModelPart::Jumper:
			case ModelPart::CopperFill:
			case ModelPart::Unknown:
				doConnectors = false;
				break;
			default:
				doConnectors = true;
				break;
		}

		m_droppingItem = addItemAux(modelPart, viewGeometry, fromID, -1, NULL, NULL, doConnectors, m_viewIdentifier);

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
	turnOffAutoscroll();

	if (m_droppingItem != NULL) {
		m_droppingItem->setVisible(false);
		//ItemDrag::_setPixmapVisible(true);
	}

	QGraphicsView::dragLeaveEvent(event);
}

void SketchWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata") && event->source() != this) {
    	dragMoveHighlightConnector(event->pos());
        event->acceptProposedAction();
        return;
    }

	QGraphicsView::dragMoveEvent(event);
}

void SketchWidget::dragMoveHighlightConnector(QPoint eventPos) {
	if (m_droppingItem == NULL) return;

	m_globalPos = this->mapToGlobal(eventPos);
	checkAutoscroll(m_globalPos);

	QPointF loc = this->mapToScene(eventPos) - m_droppingOffset;
	m_droppingItem->setItemPos(loc);
	m_droppingItem->findConnectorsUnder();

}


void SketchWidget::dropEvent(QDropEvent *event)
{
	turnOffAutoscroll();
	clearHoldingSelectItem();

	if (m_droppingItem == NULL) return;

    if (event->mimeData()->hasFormat("application/x-dnditemdata") && event->source() != this) {

    	ModelPart * modelPart = m_droppingItem->modelPart();
    	if (modelPart == NULL) return;
    	if (modelPart->modelPartShared() == NULL) return;

		QUndoCommand* parentCommand = new QUndoCommand(tr("Add %1").arg(m_droppingItem->title()));
    	stackSelectionState(false, parentCommand);

		m_droppingItem->saveGeometry();
    	ViewGeometry viewGeometry = m_droppingItem->getViewGeometry();

		long fromID = m_droppingItem->id();

		BaseCommand::CrossViewType crossViewType = BaseCommand::CrossView;
		switch (modelPart->itemType()) {
			case ModelPart::Ruler:
			case ModelPart::Logo:
				// rulers and logos are local to a particular view
				crossViewType = BaseCommand::SingleView;
				break;
			default:
				break;				
		}
		AddItemCommand * addItemCommand = newAddItemCommand(crossViewType, modelPart->moduleID(), viewGeometry, fromID, true, -1, -1, parentCommand);
		addItemCommand->setDropOrigin(this);
		
		new CheckStickyCommand(this, crossViewType, fromID, false, parentCommand);

		if (modelPart->itemType() == ModelPart::Module) {
			RestoreIndexesCommand * restoreIndexesCommand = new RestoreIndexesCommand(this, fromID, NULL, true, parentCommand);
			addItemCommand->addRestoreIndexesCommand(restoreIndexesCommand);
		}

		SelectItemCommand * selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
		selectItemCommand->addRedo(fromID);

		new ShowLabelFirstTimeCommand(this, crossViewType, fromID, true, true, parentCommand);

		if (modelPart->itemType() == ModelPart::Wire && !m_lastColorSelected.isEmpty()) {
			new WireColorChangeCommand(this, fromID, m_lastColorSelected, m_lastColorSelected, 1.0, 1.0, parentCommand);
		}

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
			connectorItem->clearConnectorHover();
		}

		clearTemporaries();

		killDroppingItem();

		emit ratsnestChangeSignal(this, parentCommand);
		new CleanUpWiresCommand(this, false, parentCommand);
        m_undoStack->waitPush(parentCommand, 10);


        event->acceptProposedAction();
		DebugDialog::debug("after drop event");

		emit dropSignal(event->pos());
    }
	else {
		QGraphicsView::dropEvent(event);
	}


}

SelectItemCommand* SketchWidget::stackSelectionState(bool pushIt, QUndoCommand * parentCommand) {

	// if pushIt assumes m_undoStack->beginMacro has previously been called

	//DebugDialog::debug(QString("stacking"));

	// DebugDialog::debug(QString("stack selection state %1 %2").arg(pushIt).arg((long) parentCommand));
	SelectItemCommand* selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
	const QList<QGraphicsItem *> sitems = scene()->selectedItems();
 	for (int i = 0; i < sitems.size(); ++i) {
 		ItemBase * base = ItemBase::extractTopLevelItemBase(sitems.at(i));
 		if (base == NULL) continue;

 		 selectItemCommand->addUndo(base->id());
		 //DebugDialog::debug(QString("\tstacking %1").arg(base->id()));
    }

	selectItemCommand->setText(tr("Selection"));

    if (pushIt) {
     	m_undoStack->push(selectItemCommand);
    }

    return selectItemCommand;
}

bool SketchWidget::moveByArrow(int dx, int dy, QKeyEvent * event) {
	DebugDialog::debug(QString("move by arrow %1").arg(event->isAutoRepeat()));
	if (!event->isAutoRepeat()) {
		m_dragBendpointWire = NULL;
		clearHoldingSelectItem();
		m_savedItems.clear();
		m_savedWires.clear();
		m_moveEventCount = 0;
		m_arrowTotalX = m_arrowTotalY = 0;
		prepMove(NULL);
		if (m_savedItems.count() == 0) return false;

		m_mousePressScenePos = this->mapToScene(this->rect().center());
		m_movingByArrow = true;
	}
	else {
		DebugDialog::debug("hello");
	}

	m_moveEventCount++;

	if (event->modifiers() & Qt::ShiftModifier) {
		dx *= 10;
		dy *= 10;
	}

	if (m_alignToGrid) {
		dx *= gridSizeInches() * FSvgRenderer::printerScale();
		dy *= gridSizeInches() * FSvgRenderer::printerScale();
	}

	m_arrowTotalX += dx;
	m_arrowTotalY += dy;

	QPoint globalPos = mapFromScene(m_mousePressScenePos + QPoint(m_arrowTotalX, m_arrowTotalY));
	globalPos = mapToGlobal(globalPos);
	moveItems(globalPos, false);
	return true;
}


void SketchWidget::mousePressEvent(QMouseEvent *event) {

	if (m_movingByArrow) return;

	m_movingByMouse = true;

	m_dragBendpointWire = NULL;
	m_spaceBarWasPressed = m_spaceBarIsPressed;
	if (m_spaceBarIsPressed) {
		InfoGraphicsView::mousePressEvent(event);
		return;
	}

	//setRenderHint(QPainter::Antialiasing, false);

	clearHoldingSelectItem();
	m_savedItems.clear();
	m_savedWires.clear();
	m_moveEventCount = 0;
	m_holdingSelectItemCommand = stackSelectionState(false, NULL);
	m_mousePressScenePos = mapToScene(event->pos());
	m_mousePressGlobalPos = event->globalPos();

	QList<QGraphicsItem *> items = this->items(event->pos());
	QGraphicsItem* wasItem = NULL;
	foreach (QGraphicsItem * gitem, items) {
		if (gitem->acceptedMouseButtons() != Qt::NoButton) {
			wasItem = gitem;
			break;
		}
	}

	QGraphicsView::mousePressEvent(event);

	items = this->items(event->pos());
	QGraphicsItem* item = NULL;
	foreach (QGraphicsItem * gitem, items) {
		if (gitem->acceptedMouseButtons() != Qt::NoButton) {
			item = gitem;
			break;
		}
	}

	if (item != wasItem) {
		// if the item was deleted during mousePressEvent
		// for example, by shift-clicking a connectorItem
		return;
	}

	if (item == NULL) {
		if (items.length() == 1) {
			// if we unambiguously click on a partlabel whose owner is unselected, go ahead and activate it
			PartLabel * partLabel =  dynamic_cast<PartLabel *>(items[0]);
			if (partLabel != NULL) {
				partLabel->owner()->setSelected(true);
				return;
			}
		}

		clickBackground(event);
		if (m_infoView != NULL) {
			m_infoView->viewItemInfo(this, NULL, true);
		}
		return;
	}

	PartLabel * partLabel =  dynamic_cast<PartLabel *>(item);
	if (partLabel != NULL) {
		InfoGraphicsView::viewItemInfo(partLabel->owner());
		setLastPaletteItemSelectedIf(partLabel->owner());
		return;
	}

	// Note's child items (at the moment) are the resize grip and the text editor
	Note * note = dynamic_cast<Note *>(item->parentItem());
	if (note != NULL)  {
		return;
	}

	ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
	if (itemBase) {
		InfoGraphicsView::viewItemInfo(itemBase);
		setLastPaletteItemSelectedIf(itemBase);
	}

	// board's child items (at the moment) are the resize grips
	m_resizingBoard = dynamic_cast<ResizableBoard *>(item->parentItem());
	if (m_resizingBoard != NULL) {
		m_resizingBoard->saveParams();
		return;
	}

	Wire * wire = dynamic_cast<Wire *>(item);
	if ((event->button() == Qt::LeftButton) && (wire != NULL) && !wire->getRatsnest()) {
		if (canChainWire(wire) && wire->hasConnections() ) {
			if (event->modifiers() & Qt::AltModifier) {
				prepDragWire(wire);
				return;
			}
			else {
				m_dragBendpointWire = wire;
				m_dragBendpointPos = event->pos();
				return;	
			}
		}
	}

	JumperItem * jumperItem = dynamic_cast<JumperItem *>(item);
	if (jumperItem != NULL && jumperItem->inDrag()) {
		m_resizingJumperItem = jumperItem;
		m_resizingJumperItem->saveParams();
		return;
	}

	prepMove(itemBase ? itemBase : dynamic_cast<ItemBase *>(item->parentItem()));

	setupAutoscroll(true);
}

void SketchWidget::prepMove(ItemBase * originatingItem) {
	QSet<Wire *> wires;
	QList<ItemBase *> items;
	foreach (QGraphicsItem * gitem,  this->scene()->selectedItems()) {
		ItemBase *itemBase = dynamic_cast<ItemBase *>(gitem);
		if (itemBase == NULL) continue;

		items.append(itemBase);
	}

	for (int i = 0; i < items.count(); i++) {
		ItemBase * itemBase = items[i];
		if (itemBase->itemType() == ModelPart::Wire) {
			if (itemBase->isVisible()) {
				wires.insert(dynamic_cast<Wire *>(itemBase));
			}
			continue;
		}

		ItemBase * chief = itemBase->layerKinChief();
		m_savedItems.insert(chief);
		if (chief->sticky()) {
			foreach(ItemBase * sitemBase, chief->stickyList()) {
				if (sitemBase->isVisible()) {
					if (sitemBase->itemType() == ModelPart::Wire) {
						wires.insert(dynamic_cast<Wire *>(sitemBase));
					}
					else {
						m_savedItems.insert(sitemBase);
						if (!items.contains(sitemBase)) {
							items.append(sitemBase);
						}
					}
				}
			}
		}

		QSet<ItemBase *> set;
		collectFemaleConnectees(chief, set);
		foreach (ItemBase * sitemBase, set) {
			if (!items.contains(sitemBase)) {
				items.append(sitemBase);
			}
		}
		chief->collectWireConnectees(wires);
	}

	if (wires.count() > 0) {
		categorizeDragWires(wires);
	}

	m_alignmentItem = NULL;
	if (originatingItem) {
		bool gotOne = false;
		foreach (QGraphicsItem * childItem, originatingItem->childItems()) {
			ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
			if (connectorItem) {
				m_alignmentStartPoint = connectorItem->sceneAdjustedTerminalPoint(NULL);
				gotOne = true;
				break;
			}
		}
		if (!gotOne && canAlignToTopLeft(originatingItem)) {
			m_alignmentStartPoint = originatingItem->pos();
			gotOne = true;
		}
		if (gotOne) {
			m_alignmentItem = originatingItem;
		}
	}

	foreach (ItemBase * itemBase, m_savedItems) {
		itemBase->saveGeometry();
		if (m_alignmentItem == NULL) {
			foreach (QGraphicsItem * childItem, itemBase->childItems()) {
				ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
				if (connectorItem) {
					m_alignmentStartPoint = connectorItem->sceneAdjustedTerminalPoint(NULL);
					m_alignmentItem = itemBase;
					break;
				}
			}
		}
	}

	foreach (Wire * w, m_savedWires.keys()) {
		w->saveGeometry();
		if (m_alignmentItem == NULL) {
			m_alignmentItem = w;
			m_alignmentStartPoint = w->connector0()->sceneAdjustedTerminalPoint(NULL);
		}
	}

	if (m_alignmentItem == NULL) {
		foreach (ItemBase * itemBase, m_savedItems) {
			if (canAlignToTopLeft(itemBase)) {
				m_alignmentStartPoint = itemBase->pos();
				m_alignmentItem = itemBase;
				break;
			}
		}
	}
}

enum ConnectionStatus {
	IN,
	OUT,
	FREE,
	UNDETERMINED
};

struct ConnectionThing {
	Wire * wire;
	ConnectionStatus status[2];
};

void SketchWidget::categorizeDragWires(QSet<Wire *> & wires) 
{

	foreach (Wire * w, wires) {
		QList<Wire *> chainedWires;
		QList<ConnectorItem *> uniqueEnds;
		QList<ConnectorItem *> ends;
		w->collectChained(chainedWires, ends, uniqueEnds);
		foreach (Wire * ww, chainedWires) {
			wires.insert(ww);
		}
	}

	QList<ConnectionThing *> connectionThings;
	foreach (Wire * w, wires) {
		ConnectionThing * ct = new ConnectionThing;
		ct->wire = w;
		ct->status[0] = ct->status[1] = UNDETERMINED;
		connectionThings.append(ct);
	}

	int noChangeCount = 0;
	QList<ItemBase *> outWires;
	while (connectionThings.count() > 0) {
		ConnectionThing * ct = connectionThings.takeFirst();
		bool changed = false;

		QList<ConnectorItem *> from;
		from.append(ct->wire->connector0());
		from.append(ct->wire->connector1());
		for (int i = 0; i < 2; i++) {
			if (ct->status[i] != UNDETERMINED) continue;

			foreach (ConnectorItem * toConnectorItem, from.at(i)->connectedToItems()) {
				if (m_savedItems.contains(toConnectorItem->attachedTo())) {
					changed = true;
					ct->status[i] = IN;
					break;
				}

				if ((toConnectorItem->attachedToItemType() != ModelPart::Wire) || 
					outWires.contains(toConnectorItem->attachedTo())) 
				{
					changed = true;
					ct->status[i] = OUT;
					break;
				}
			}
			if (ct->status[i] != UNDETERMINED) continue;

			ItemBase * stickingTo = ct->wire->stickingTo();
			if (stickingTo != NULL) {
				QPointF p = from.at(i)->sceneAdjustedTerminalPoint(NULL);
				if (stickingTo->contains(stickingTo->mapFromScene(p))) {
					ct->status[i] = m_savedItems.contains(stickingTo) ? IN : OUT;
					changed = true;
				}
			}
			if (ct->status[i] != UNDETERMINED) continue;

			// if it's not connected at either end and not stuck
			if (from.at(i)->connectionsCount() == 0) {
				changed = true;
				ct->status[i] = FREE;
			}
		}

		if (ct->status[0] != UNDETERMINED && ct->status[1] != UNDETERMINED) {
			if (ct->status[0] == IN) {
				if (ct->status[1] == IN) {
					m_savedItems.insert(ct->wire);
				}
				else {
					// OUT == FREE in this case
					// attach the connector that stays IN
					m_savedWires.insert(ct->wire, ct->wire->connector0());
				}
			}
			else if (ct->status[0] == OUT) {
				if (ct->status[1] == IN) {
					// attach the connector that stays in
					m_savedWires.insert(ct->wire, ct->wire->connector1());
				}
				else {
					// don't drag this; both ends are connected OUT
					outWires.append(ct->wire);
				}
			}
			else /* ct->status[0] == FREE */ {  
				if (ct->status[1] == IN) {
					// attach the connector that stays IN
					m_savedWires.insert(ct->wire, ct->wire->connector1());
				}
				else if (ct->status[1] == FREE) {
					// both sides are free, so if the wire is selected, drag it
					if (ct->wire->isSelected()) {
						m_savedItems.insert(ct->wire);
					}
				}
				else {
					// don't drag this; both ends are connected OUT
					outWires.append(ct->wire);
				}
			}
			delete ct;
			noChangeCount = 0;
		}
		else {
			connectionThings.append(ct);
			if (changed) {
				noChangeCount = 0;
			}
			else {
				if (++noChangeCount > connectionThings.count()) {
					QList<ConnectionThing *> cts;
					foreach (ConnectionThing * ct, connectionThings) {
						// if one end is OUT and the other end is unaccounted for at this pass, then both ends are OUT
						if ((ct->status[0] == FREE || ct->status[0] == OUT) || 
							(ct->status[1] == FREE || ct->status[1] == OUT)) 
						{
							noChangeCount = 0;
							outWires.append(ct->wire);
							delete ct;
						}
						else {
							cts.append(ct);
						}
					}
					if (noChangeCount == 0) {
						// get ready for another pass, we got rid of some 
						connectionThings.clear();
						foreach (ConnectionThing * ct, cts) {
							connectionThings.append(ct);
						}
					}
					else {
						// we've elimated all OUT items so mark everybody IN
						foreach (ConnectionThing * ct, connectionThings) {
							m_savedItems.insert(ct->wire);
							delete ct;
						}						
						connectionThings.clear();
					}
				}
			}
		}
	}
}

void SketchWidget::clickBackground(QMouseEvent * event) 
{
	// in here if you clicked on the sketch itself,

	if (m_fixedToCenterItem && m_fixedToCenterItem->getVisible()) {
		QRectF r(m_fixedToCenterItemOffset, m_fixedToCenterItem->size());
		if (r.contains(event->pos())) {
			QMouseEvent newEvent(event->type(), event->pos() - m_fixedToCenterItemOffset,
				event->globalPos(), event->button(), event->buttons(), event->modifiers());
			if (m_fixedToCenterItem->forwardMousePressEvent(&newEvent)) {
				// update background
				setBackground(background());
			}
		}
	}
}

void SketchWidget::prepDragWire(Wire * wire) 
{
	bool drag = true;
	foreach (ConnectorItem * toConnectorItem, wire->connector0()->connectedToItems()) {
		if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			m_savedWires.insert(qobject_cast<Wire *>(toConnectorItem->attachedTo()), toConnectorItem);
		}
		else {
			drag = false;
			break;
		}
	}
	if (drag) {
		foreach (ConnectorItem * toConnectorItem, wire->connector1()->connectedToItems()) {
			if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
				m_savedWires.insert(qobject_cast<Wire *>(toConnectorItem->attachedTo()), toConnectorItem);
			}
			else {
				drag = false;
				break;
			}
		}
	}
	if (!drag) {
		m_savedWires.clear();
		return;
	}

	m_savedItems.clear();
	m_savedItems.insert(wire);
	wire->saveGeometry();
	foreach (Wire * w, m_savedWires.keys()) {
		w->saveGeometry();
	}
	setupAutoscroll(true);
}

void SketchWidget::prepDragBendpoint(Wire * wire, QPoint eventPos) 
{
	m_bendpointWire = wire;
	wire->saveGeometry();
	ViewGeometry vg = m_bendpointVG = wire->getViewGeometry();
	QPointF newPos = mapToScene(eventPos); 
	QPointF oldPos = wire->pos();
	QLineF oldLine = wire->line();
	//DebugDialog::debug(QString("oldpos"), oldPos);
	//DebugDialog::debug(QString("oldline p1"), oldLine.p1());
	//DebugDialog::debug(QString("oldline p2"), oldLine.p2());
	QLineF newLine(oldLine.p1(), newPos - oldPos);
	wire->setLine(newLine);
	vg.setLoc(newPos);
	QLineF newLine2(QPointF(0,0), oldLine.p2() + oldPos - newPos);
	vg.setLine(newLine2);
	long newID = ItemBase::getNextID();
	ConnectorItem * oldConnector1 = wire->connector1();
	m_connectorDragWire = dynamic_cast<Wire *>(addItemAux(wire->modelPart(), vg, newID, -1, NULL, NULL, true, m_viewIdentifier));
	ConnectorItem * newConnector1 = m_connectorDragWire->connector1();
	foreach (ConnectorItem * toConnectorItem, oldConnector1->connectedToItems()) {
		oldConnector1->tempRemove(toConnectorItem, false);
		toConnectorItem->tempRemove(oldConnector1, false);
		newConnector1->tempConnectTo(toConnectorItem, false);
		toConnectorItem->tempConnectTo(newConnector1, false);
	}
	oldConnector1->tempConnectTo(m_connectorDragWire->connector0(), false);
	m_connectorDragWire->connector0()->tempConnectTo(oldConnector1, false);
	m_connectorDragConnector = oldConnector1;
	m_connectorDragWire->initDragEnd(m_connectorDragWire->connector0(), newPos);
	m_connectorDragWire->grabMouse();
}


void SketchWidget::collectFemaleConnectees(ItemBase * itemBase, QSet<ItemBase *> & items) {
	Q_UNUSED(itemBase);
	Q_UNUSED(items);
}

bool SketchWidget::draggingWireEnd() {
	if (m_connectorDragWire != NULL) return true;

	QGraphicsItem * mouseGrabberItem = scene()->mouseGrabberItem();
	Wire * wire = dynamic_cast<Wire *>(mouseGrabberItem);
	if (wire == NULL) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(mouseGrabberItem);
		if (connectorItem == NULL) return false;
		if (connectorItem->attachedToItemType() != ModelPart::Wire) return false;

		wire = dynamic_cast<Wire *>(connectorItem->attachedTo());
	}

	return wire->draggingEnd();
}

void SketchWidget::mouseMoveEvent(QMouseEvent *event) {
	// if its just dragging a wire end do default
	// otherwise handle all move action here

	if (m_movingByArrow) return;

	if (m_dragBendpointWire != NULL) {
		prepDragBendpoint(m_dragBendpointWire, m_dragBendpointPos);
		m_dragBendpointWire = NULL;
		return;
	}

	if (m_spaceBarWasPressed) {
		InfoGraphicsView::mouseMoveEvent(event);
		return;
	}

	if (m_savedItems.count() > 0) {
		if ((event->buttons() & Qt::LeftButton) && !draggingWireEnd()) {
			m_globalPos = event->globalPos();
			if ((event->modifiers() & Qt::ShiftModifier) != 0) {
				QPointF p = GraphicsUtils::calcConstraint(m_mousePressGlobalPos, m_globalPos);
				m_globalPos.setX(p.x());
				m_globalPos.setY(p.y());
			}

			moveItems(m_globalPos, true);
		}
	}

	if (event->buttons() == Qt::NoButton) {
		if (m_fixedToCenterItem && m_fixedToCenterItem->getVisible()) {
			QSize size((int) m_fixedToCenterItem->size().width(), (int) m_fixedToCenterItem->size().height());
			QRect r(m_fixedToCenterItemOffset, size);
			bool within = r.contains(event->pos()) && (itemAt(event->pos()) == NULL);
			if (m_fixedToCenterItem->setMouseWithin(within)) {
				// seems to be the only way to force a redraw of the background here
				setBackground(background());
			}
		}
	}

	m_moveEventCount++;
	QGraphicsView::mouseMoveEvent(event);
}

void SketchWidget::moveItems(QPoint globalPos, bool checkAutoScroll)
{

	if (checkAutoScroll) {
		bool result = checkAutoscroll(globalPos);
		if (!result) return;
	}

	QPoint q = mapFromGlobal(globalPos);
	QPointF scenePos = mapToScene(q);

	if (m_alignToGrid && (m_alignmentItem != NULL)) {
		QPointF currentParentPos = m_alignmentItem->mapToParent(m_alignmentItem->mapFromScene(scenePos));
		QPointF buttonDownParentPos = m_alignmentItem->mapToParent(m_alignmentItem->mapFromScene(m_mousePressScenePos));
		QPointF newPos = m_alignmentStartPoint + currentParentPos - buttonDownParentPos;
		qreal ny = GraphicsUtils::getNearestOrdinate(newPos.y(), gridSizeInches() * FSvgRenderer::printerScale());
		qreal nx = GraphicsUtils::getNearestOrdinate(newPos.x(), gridSizeInches() * FSvgRenderer::printerScale());
		scenePos.setX(scenePos.x() + nx - newPos.x());
		scenePos.setY(scenePos.y() + ny - newPos.y());
	}

/*
	DebugDialog::debug(QString("scroll 1 sx:%1 sy:%2 sbx:%3 sby:%4 qx:%5 qy:%6")
		.arg(scenePos.x()).arg(scenePos.y())
		.arg(m_mousePressScenePos.x()).arg(m_mousePressScenePos.y())
		.arg(q.x()).arg(q.y())
		);
*/

	if (m_moveEventCount == 0) {
		// first time
		m_moveDisconnectedFromFemale.clear();
		foreach (ItemBase * item, m_savedItems) {
			if (item->itemType() == ModelPart::Wire) continue;

			//DebugDialog::debug(QString("disconnecting from female %1").arg(item->instanceTitle()));
			disconnectFromFemale(item, m_savedItems, m_moveDisconnectedFromFemale, false, NULL);
		}
	}

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

	if (m_movingByArrow) return;

	m_alignmentItem = NULL;
	m_movingByMouse = false;

	m_dragBendpointWire = NULL;
	ConnectorItem::clearEqualPotentialDisplay();

	if (m_spaceBarWasPressed) {
		InfoGraphicsView::mouseReleaseEvent(event);
		return;
	}

	if (m_resizingBoard != NULL) {
		resizeBoard();
		InfoGraphicsView::mouseReleaseEvent(event);
		return;
	}

	if (m_resizingJumperItem != NULL) {
		resizeJumperItem();
		InfoGraphicsView::mouseReleaseEvent(event);
		return;
	}

	turnOffAutoscroll();
	QGraphicsView::mouseReleaseEvent(event);

	if (m_connectorDragWire != NULL) {
		if (scene()->mouseGrabberItem() == m_connectorDragWire) {
			// probably already ungrabbed by the wire, but just in case
			m_connectorDragWire->ungrabMouse();
		}

		// remove again (may not have been removed earlier)
		if (m_connectorDragWire->scene() != NULL) {
			this->scene()->removeItem(m_connectorDragWire);
			//m_infoView->unregisterCurrentItem();
			updateInfoView();

		}

		if (m_bendpointWire) {
			// click on wire but no drag:  restore original state of wire
			foreach (ConnectorItem * toConnectorItem, m_connectorDragWire->connector1()->connectedToItems()) {
				m_connectorDragWire->connector1()->tempRemove(toConnectorItem, false);
				toConnectorItem->tempRemove(m_connectorDragWire->connector1(), false);
				m_bendpointWire->connector1()->tempConnectTo(toConnectorItem, false);
				toConnectorItem->tempConnectTo(m_bendpointWire->connector1(), false);
			}
			m_bendpointWire->connector1()->tempRemove(m_connectorDragWire->connector0(), false);
			m_connectorDragWire->connector0()->tempRemove(m_bendpointWire->connector1(), false);
			m_bendpointWire->setLine(m_bendpointVG.line());			
		}

		DebugDialog::debug("deleting connector drag wire");
		delete m_connectorDragWire;

		m_bendpointWire = m_connectorDragWire = NULL;
		m_savedItems.clear();
		m_savedWires.clear();
		return;
	}

	if ((m_savedItems.size() <= 0) || !checkMoved()) {
		if (this->m_holdingSelectItemCommand != NULL) {
			if (m_holdingSelectItemCommand->updated()) {
				SelectItemCommand* tempCommand = m_holdingSelectItemCommand;
				m_holdingSelectItemCommand = NULL;
				//DebugDialog::debug(QString("scene changed push select %1").arg(scene()->selectedItems().count()));
				m_undoStack->push(tempCommand);
			}
			else {
				clearHoldingSelectItem();
			}
		}
	}
	m_savedItems.clear();
	m_savedWires.clear();
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
	QString viewName = ViewIdentifierClass::viewIdentifierName(m_viewIdentifier);

	if (moveCount == 1) {
		moveString = tr("Move %2 (%1)").arg(viewName).arg(saveBase->title());
	}
	else {
		moveString = tr("Move %2 items (%1)").arg(viewName).arg(QString::number(moveCount));
	}

	QUndoCommand * parentCommand = new QUndoCommand(moveString);

	bool hasBoard = false;
	bool doEmit = false;

	foreach (ItemBase * item, m_savedItems) {
		if (item == NULL) continue;

		ViewGeometry viewGeometry(item->getViewGeometry());
		item->saveGeometry();

		new MoveItemCommand(this, item->id(), viewGeometry, item->getViewGeometry(), parentCommand);
		new CheckStickyCommand(this, BaseCommand::SingleView, item->id(), false, parentCommand);

		if (item->itemType() == ModelPart::Breadboard) {
			hasBoard = true;
			continue;
		}

		// TODO: boardtypes and breadboard types are always sticky
		if (item->itemType() == ModelPart::Board || item->itemType() == ModelPart::ResizableBoard) {
			hasBoard = true;
		}
	}

	foreach (ItemBase * item, m_savedWires.keys()) {
		if (item == NULL) continue;

		ViewGeometry viewGeometry(item->getViewGeometry());
		item->saveGeometry();
		ViewGeometry newViewGeometry(item->getViewGeometry());

		new ChangeWireCommand(this, item->id(), viewGeometry.line(), newViewGeometry.line(), viewGeometry.loc(), newViewGeometry.loc(), true, parentCommand);
		new CheckStickyCommand(this, BaseCommand::SingleView, item->id(), false, parentCommand);
	}

	foreach (ConnectorItem * fromConnectorItem, m_moveDisconnectedFromFemale.uniqueKeys()) {
		foreach (ConnectorItem * toConnectorItem, m_moveDisconnectedFromFemale.values(fromConnectorItem)) {
			extendChangeConnectionCommand(fromConnectorItem, toConnectorItem, false, true, parentCommand);
			doEmit = true;
		}
	}

	foreach (ItemBase * item, m_savedItems) {
		if (item->itemType() == ModelPart::Wire) continue;

		QList<ConnectorItem *> connectorItems;
		item->collectConnectors(connectorItems);
		foreach (ConnectorItem * fromConnectorItem, connectorItems) {
			ConnectorItem * toConnectorItem = fromConnectorItem->overConnectorItem();
			if (toConnectorItem != NULL) {
				toConnectorItem->connectorHover(item, false);
				fromConnectorItem->setOverConnectorItem(NULL);   // clean up
				extendChangeConnectionCommand(fromConnectorItem, toConnectorItem, true, true, parentCommand);
			}
			fromConnectorItem->clearConnectorHover();
		}
	}

	clearTemporaries();

	if (doEmit) {
		emit ratsnestChangeSignal(this, parentCommand);
	}
	else if (!hasBoard) {
		// TODO: hasboard: make a cleaner distinction if moving muliple parts (remember, this is for removing routing)
		emit movingSignal(this, parentCommand);
	}

	new CleanUpWiresCommand(this, false, parentCommand);
	m_undoStack->push(parentCommand);

	return true;
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

void SketchWidget::sketchWidget_itemAdded(ModelPart * modelPart, const ViewGeometry & viewGeometry, long id, SketchWidget * dropOrigin) {
	if (dropOrigin != NULL && dropOrigin != this) {
		// offset the part 
		QPointF from = dropOrigin->mapToScene(QPoint(0, 0));
		QPointF to = this->mapToScene(QPoint(0, 0));
		QPointF dp = viewGeometry.loc() - from;
		ViewGeometry vg(viewGeometry);
		vg.setLoc(to + dp);
		addItemAux(modelPart, vg, id, -1, NULL, NULL, true, m_viewIdentifier);
	}
	else {
		addItemAux(modelPart, viewGeometry, id, -1, NULL, NULL, true, m_viewIdentifier);
	}
}

void SketchWidget::sketchWidget_itemDeleted(long id) {
	ItemBase * pitem = findItem(id);
	if (pitem != NULL) {
		deleteItem(pitem, false, false, false);
	}
}

void SketchWidget::scene_selectionChanged() {
	if (m_ignoreSelectionChangeEvents > 0) {
		return;
	}

	// DebugDialog::debug("selection changed");
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
		//DebugDialog::debug("update holding command");

		int selCount = 0;
		ItemBase* saveBase = NULL;
		QString selString;
		m_holdingSelectItemCommand->clearRedo();
		const QList<QGraphicsItem *> sitems = scene()->selectedItems();
		foreach (QGraphicsItem * item, scene()->selectedItems()) {
	 		ItemBase * base = dynamic_cast<ItemBase *>(item);
	 		if (base == NULL) continue;

			saveBase = base;
	 		m_holdingSelectItemCommand->addRedo(base->layerKinChief()->id());
	 		selCount++;
	    }
		if (selCount == 1) {
			selString = tr("Select %1").arg(saveBase->title());
		}
		else {
			selString = tr("Select %1 items").arg(QString::number(selCount));
		}
		m_holdingSelectItemCommand->setText(selString);
		m_holdingSelectItemCommand->setUpdated(true);
	}
}

void SketchWidget::clearHoldingSelectItem() {
	// DebugDialog::debug("clear holding");
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
	//DebugDialog::debug(QString("got item selected signal %1 %2 %3 %4").arg(id).arg(state).arg(item != NULL).arg(m_viewIdentifier));
	if (item != NULL) {
		item->setSelected(state);
	}

	PaletteItem *pitem = dynamic_cast<PaletteItem*>(item);
	if(pitem) {
		setLastPaletteItemSelected(pitem);
	}
}

void SketchWidget::group(const QString & moduleID, long itemID, QList<long> & itemIDs, const ViewGeometry & viewGeometry, bool doEmit)
{
	ModelPart * modelPart = m_sketchModel->findModelPart(moduleID, itemID);
	if (modelPart == NULL) {
		return;
	}

	QList<ItemBase *> itemBases;
	foreach (long id, itemIDs) {
		ItemBase * itemBase = findItem(id);
		if (itemBase == NULL) return;

		itemBases.append(itemBase);
	}

	if (itemBases.count() < 1) return;

	GroupItem * groupItem = new GroupItem(modelPart, m_viewIdentifier, viewGeometry, itemID, NULL);
	scene()->addItem(groupItem);

	// sort by z
    qSort(itemBases.begin(), itemBases.end(), ItemBase::zLessThan);
	foreach (ItemBase * itemBase, itemBases) {
		groupItem->addToGroup(itemBase);
	}
	groupItem->doneAdding(m_viewLayers, defaultConnectorLayer(m_viewIdentifier));
	//groupItem->setSelected(true);

	groupItem->setPos(viewGeometry.loc());

	if (doEmit) {
		emit groupSignal(moduleID, itemID, itemIDs, viewGeometry, false);
	}
}

ModelPart * SketchWidget::group(ModelPart * modelPart) {
	const QList<QGraphicsItem *> sitems = scene()->selectedItems();
	if (sitems.size() < 2) return NULL;

	QList<ItemBase *> itemBases;
	foreach (QGraphicsItem * item, sitems) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;

		itemBase = itemBase->layerKinChief();
		if (itemBases.contains(itemBase)) continue;

		itemBases.append(itemBase);
	}

	if (itemBases.count() < 2) return NULL;

	if (modelPart == NULL) {
		modelPart = m_paletteModel->retrieveModelPart(ModuleIDNames::groupModuleIDName);
	}
	if (modelPart == NULL) return NULL;

	modelPart = m_sketchModel->addModelPart(m_sketchModel->root(), modelPart);
	ViewGeometry vg;
	vg.setLoc(itemBases[0]->pos());		// temporary position
	GroupItem * groupItem = new GroupItem(modelPart, m_viewIdentifier, vg, ItemBase::getNextID(), NULL);
	scene()->addItem(groupItem);

	// sort by z
    qSort(itemBases.begin(), itemBases.end(), ItemBase::zLessThan);
	foreach (ItemBase * itemBase, itemBases) {
		groupItem->addToGroup(itemBase);
	}
	groupItem->doneAdding(m_viewLayers, defaultConnectorLayer(m_viewIdentifier));
	groupItem->setSelected(true);

	return modelPart;
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
	if ((to != NULL) && from->connectedToItems().contains(to)) {
		// there's no change: the wire was dragged back to its original connection
		from->attachedTo()->updateConnections(to);
		return;
	}

	QUndoCommand * parentCommand = new QUndoCommand();

	long fromID = wire->id();

	QString fromConnectorID;
	if (from != NULL) {
		fromConnectorID = from->connectorSharedID();
	}

	long toID = -1;
	QString toConnectorID;
	if (to != NULL) {
		toID = to->attachedToID();
		toConnectorID = to->connectorSharedID();
	}

	new ChangeWireCommand(this, fromID, oldLine, newLine, oldPos, newPos, true, parentCommand);
	new CheckStickyCommand(this, BaseCommand::SingleView, fromID, false, parentCommand);

	bool chained = false;
	foreach (ConnectorItem * toConnectorItem, from->connectedToItems()) {
		Wire * toWire = dynamic_cast<Wire *>(toConnectorItem->attachedTo());
		if (toWire == NULL) continue;

		ViewGeometry vg = toWire->getViewGeometry();
		QLineF nl = toWire->line();
		QPointF np = toWire->pos();
		new ChangeWireCommand(this, toWire->id(), vg.line(), nl, vg.loc(), np, true, parentCommand);
		new CheckStickyCommand(this, BaseCommand::SingleView, toWire->id(), false, parentCommand);
		chained = true;
	}

	QList< QPointer<ConnectorItem> > former = from->connectedToItems();

	QString prefix;
	QString suffix;
	if (to == NULL) {
		if (former.count() > 0  && !chained) {
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
		suffix = tr("to %1").arg(to->attachedTo()->title());
	}

	parentCommand->setText(QObject::tr("%1 %2 %3").arg(prefix).arg(wire->title()).arg(suffix) );

	bool doEmit = false;
	if (!chained) {
		if (former.count() > 0) {
			QList<ConnectorItem *> connectorItems;
			connectorItems.append(from);
			ConnectorItem::collectEqualPotential(connectorItems);

			foreach (ConnectorItem * formerConnectorItem, former) {
				extendChangeConnectionCommand(from, formerConnectorItem, false, false, parentCommand);
				doEmit = true;
				from->tempRemove(formerConnectorItem, false);
				formerConnectorItem->tempRemove(from, false);
			}

		}
		if (to != NULL) {
			doEmit = true;
			extendChangeConnectionCommand(from, to, true, false, parentCommand);
		}
	}

	if (doEmit) {
		emit ratsnestChangeSignal(this, parentCommand);
	}

	clearTemporaries();

	new CleanUpWiresCommand(this, false, parentCommand);
	m_undoStack->push(parentCommand);
}

void SketchWidget::dragWireChanged(Wire* wire, ConnectorItem * fromOnWire, ConnectorItem * to)
{
	BaseCommand::CrossViewType crossViewType = BaseCommand::CrossView;
	if (m_bendpointWire) {
		crossViewType = BaseCommand::SingleView;
	}
	else {
		m_connectorDragConnector->tempRemove(m_connectorDragWire->connector1(), false);
		m_connectorDragWire->connector1()->tempRemove(m_connectorDragConnector, false);

		// if to and from are the same connector, you can't draw a wire to yourself 
		// or to == NULL and it's pcb or schematic view, bail out
		if ((m_connectorDragConnector == to) || !canCreateWire(wire, fromOnWire, to)) {
			clearDragWireTempCommand();
			this->scene()->removeItem(m_connectorDragWire);
			return;
		}
	}

	QUndoCommand * parentCommand = new QUndoCommand();
	parentCommand->setText(tr("Create and connect wire"));

	SelectItemCommand * selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
	if (m_tempDragWireCommand != NULL) {
		selectItemCommand->copyUndo(m_tempDragWireCommand);
		clearDragWireTempCommand();
	}

	m_connectorDragWire->saveGeometry();
	bool doEmit = false;
	if ((m_bendpointWire == NULL) && modifyNewWireConnections(wire, fromOnWire, m_connectorDragConnector, to, parentCommand)) {
	}
	else {
		long fromID = wire->id();

		DebugDialog::debug(QString("m_connectorDragConnector:%1 %4 from:%2 to:%3")
						   .arg(m_connectorDragConnector->connectorSharedID())
						   .arg(fromOnWire->connectorSharedID())
						   .arg((to == NULL) ? "null" : to->connectorSharedID())
						   .arg(m_connectorDragConnector->attachedTo()->title()) );


		// create a new wire with the same id as the temporary wire
		new AddItemCommand(this, crossViewType, m_connectorDragWire->modelPart()->moduleID(), m_connectorDragWire->getViewGeometry(), fromID, true, -1, -1, parentCommand);
		new CheckStickyCommand(this, crossViewType, fromID, false, parentCommand);
		SelectItemCommand * selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
		selectItemCommand->addRedo(fromID);

		if (m_bendpointWire == NULL) {
			ConnectorItem * anchor = wire->otherConnector(fromOnWire);
			if (anchor != NULL) {
				extendChangeConnectionCommand(anchor, m_connectorDragConnector, true, false, parentCommand);
				doEmit = true;
			}
			if (to != NULL) {
				extendChangeConnectionCommand(fromOnWire, to, true, false, parentCommand);
				doEmit = true;
			}
			if (!this->m_lastColorSelected.isEmpty()) {
				new WireColorChangeCommand(this, wire->id(), m_lastColorSelected, m_lastColorSelected, wire->opacity(), wire->opacity(), parentCommand);
			}
		}
		else {
			new WireColorChangeCommand(this, wire->id(), m_bendpointWire->colorString(), m_bendpointWire->colorString(), m_bendpointWire->opacity(), m_bendpointWire->opacity(), parentCommand);
			new WireWidthChangeCommand(this, wire->id(), m_bendpointWire->width(), m_bendpointWire->width(), parentCommand);
		}
	}

	if (m_bendpointWire) {
		new ChangeWireCommand(this, m_bendpointWire->id(), m_bendpointVG.line(), m_bendpointWire->line(), m_bendpointVG.loc(), m_bendpointWire->pos(), true, parentCommand);		
		foreach (ConnectorItem * toConnectorItem, wire->connector1()->connectedToItems()) {
			toConnectorItem->tempRemove(wire->connector1(), false);
			wire->connector1()->tempRemove(toConnectorItem, false);
			new ChangeConnectionCommand(this, BaseCommand::SingleView,
										m_bendpointWire->id(), m_connectorDragConnector->connectorSharedID(),
										toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
										false, true, parentCommand);
			new ChangeConnectionCommand(this, BaseCommand::SingleView,
										wire->id(), wire->connector1()->connectorSharedID(),
										toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
										true, true, parentCommand);
		}

		m_connectorDragConnector->tempRemove(wire->connector0(), false);
		wire->connector0()->tempRemove(m_connectorDragConnector, false);
		new ChangeConnectionCommand(this, BaseCommand::SingleView,
										m_connectorDragConnector->attachedToID(), m_connectorDragConnector->connectorSharedID(),
										wire->id(), wire->connector0()->connectorSharedID(),
										true, true, parentCommand);

		SelectItemCommand * selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
		selectItemCommand->addRedo(m_bendpointWire->id());

		m_bendpointWire = NULL;			// signal that we're done

	}

	if (doEmit) {
		emit ratsnestChangeSignal(this, parentCommand);
	}

	clearTemporaries();

	// remove the temporary wire
	this->scene()->removeItem(m_connectorDragWire);

	new CleanUpWiresCommand(this, false, parentCommand);
	m_undoStack->push(parentCommand);

}

void SketchWidget::addViewLayer(ViewLayer * viewLayer) {
	ViewLayer * oldViewLayer = m_viewLayers.value(viewLayer->viewLayerID(), NULL);
	if (oldViewLayer) {
		delete oldViewLayer;
	}

	m_viewLayers.insert(viewLayer->viewLayerID(), viewLayer);
	QAction* action = new QAction(QObject::tr("%1 Layer").arg(viewLayer->displayName()), this);
	action->setData(QVariant::fromValue<ViewLayer *>(viewLayer));
	action->setCheckable(true);
	action->setChecked(viewLayer->visible());
    connect(action, SIGNAL(triggered()), this, SLOT(toggleLayerVisibility()));
    viewLayer->setAction(action);
}

void SketchWidget::updateLayerMenu(QMenu * layerMenu, QAction * showAllAction, QAction * hideAllAction, QAction * alignAction) {
	alignAction->setChecked(m_alignToGrid);
	
	QList<ViewLayer::ViewLayerID>keys = m_viewLayers.keys();

	// make sure they're in ascending order when inserting into the menu
	qSort(keys.begin(), keys.end());

	for (int i = 0; i < keys.count(); i++) {
		ViewLayer * viewLayer = m_viewLayers.value(keys[i]);
    	if (viewLayer != NULL) {
			if (viewLayer->parentLayer()) continue;
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
		//DebugDialog::debug(QString("Layer: %1 is %2").arg(prev->action()->text()).arg(prev->action()->isChecked()));
		for (int i = 1; i < keys.count(); i++) {
			ViewLayer *viewLayer = m_viewLayers.value(keys[i]);
			//DebugDialog::debug(QString("Layer: %1 is %2").arg(viewLayer->action()->text()).arg(viewLayer->action()->isChecked()));
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

		//DebugDialog::debug(QString("sameState: %1").arg(sameState));
		//DebugDialog::debug(QString("checked: %1").arg(checked));
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

	itemCount.visLabelCount = itemCount.hasLabelCount = 0;
	itemCount.selCount = 0;
	itemCount.selHFlipable = itemCount.selVFlipable = itemCount.selRotatable = 0;
	itemCount.itemsCount = 0;
	itemCount.obsoleteCount = 0;

	for (int i = 0; i < selItems.count(); i++) {
		ItemBase * itemBase = ItemBase::extractTopLevelItemBase(selItems[i]);
		if (itemBase != NULL) {
			itemCount.selCount++;

			if (itemBase->hasPartLabel()) {
				itemCount.hasLabelCount++;
				if (itemBase->isPartLabelVisible()) {
					itemCount.visLabelCount++;
				}
			}

			if (itemBase->canFlipHorizontal()) {
				itemCount.selHFlipable++;
			}

			if (itemBase->canFlipVertical()) {
				itemCount.selVFlipable++;
			}

			if (itemBase->isObsolete()) {
				itemCount.obsoleteCount++;
			}

			bool rotatable = rotationAllowed(itemBase);
			if (rotatable) {
				itemCount.selRotatable++;
			}
		}
	}

	if (itemCount.selCount != itemCount.selRotatable) {
		// if you can't rotate them all, then you can't rotate any
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
	ViewLayer * viewLayer = m_viewLayers.value(viewLayerID);
	if (viewLayer == NULL) return false;

	return viewLayer->visible();
}

void SketchWidget::setLayerVisible(ViewLayer::ViewLayerID viewLayerID, bool vis) {
	ViewLayer * viewLayer = m_viewLayers.value(viewLayerID);
	if (viewLayer) {
		setLayerVisible(viewLayer, vis);
	}
}

void SketchWidget::toggleLayerVisibility() {
	QAction * action = qobject_cast<QAction *>(sender());
	if (action == NULL) return;

	ViewLayer * viewLayer = action->data().value<ViewLayer *>();
	if (viewLayer == NULL) return;

	setLayerVisible(viewLayer, !viewLayer->visible());
}

void SketchWidget::setLayerVisible(ViewLayer * viewLayer, bool visible) {

	QList<ViewLayer::ViewLayerID> viewLayerIDs;
	viewLayerIDs.append(viewLayer->viewLayerID());

	viewLayer->setVisible(visible);
	foreach (ViewLayer * childLayer, viewLayer->childLayers()) {
		childLayer->setVisible(visible);
		viewLayerIDs.append(childLayer->viewLayerID());
	}

	// TODO: replace scene()->items()
	foreach (QGraphicsItem * item, scene()->items()) {
		// want all items, not just topLevel
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase && (viewLayerIDs.contains(itemBase->viewLayerID()))) {
			itemBase->setHidden(!visible);
			//DebugDialog::debug(QString("setting visible %1").arg(viewLayer->visible()));
		}

		PartLabel * partLabel = dynamic_cast<PartLabel *>(item);
		if (partLabel && (viewLayerIDs.contains(partLabel->viewLayerID()))) {
			partLabel->setHidden(!visible);
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

	DebugDialog::debug(QString("scen rect: w%1 h%2").arg(itemsRect.width()).arg(itemsRect.height()));
	DebugDialog::debug(QString("view rect: w%1 h%2").arg(viewRect.width()).arg(viewRect.height()));
	DebugDialog::debug(QString("relations (v/s): w%1 h%2").arg(wRelation).arg(hRelation));

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
			DebugDialog::debug(QString("moving %1 to %2").arg(i).arg(dest));
			i -= inc;	// because we just modified the list and would miss the next item
		}
	}

	if (!moved) {
		return;
	}

	continueZChangeAux(bases, text);
}


void SketchWidget::continueZChangeAux(QList<ItemBase *> & bases, const QString & text) {

	ChangeZCommand * changeZCommand = new ChangeZCommand(this, NULL);

	ViewLayer::ViewLayerID lastViewLayerID = ViewLayer::UnknownLayer;
	qreal z = 0;
	for (int i = 0; i < bases.size(); i++) {
		qreal oldZ = bases[i]->getViewGeometry().z();
		if (bases[i]->viewLayerID() != lastViewLayerID) {
			lastViewLayerID = bases[i]->viewLayerID();
			z = qFloor(oldZ);
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
		if (itemBase == NULL) continue;
		if (itemBase->getVirtual()) continue;

		if (itemBase != NULL) {
			tlBases.append(itemBase);
		}
	}

	sortAnyByZ(tlBases, bases);
}


void SketchWidget::sortAnyByZ(const QList<QGraphicsItem *> & items, QList<ItemBase *> & bases) {
	for (int i = 0; i < items.size(); i++) {
		ItemBase * base = dynamic_cast<ItemBase *>(items[i]);
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
		ItemBase * itemBase = dynamic_cast<ItemBase *>(items[i]);
		if (itemBase == NULL) continue;

		RealPair * pair = triplets[itemBase->id()];
		if (pair == NULL) continue;

		qreal newZ = pairAccessor(pair);
		DebugDialog::debug(QString("change z %1 %2").arg(itemBase->id()).arg(newZ));
		items[i]->setZValue(newZ);

	}
}

ViewLayer::ViewLayerID SketchWidget::getDragWireViewLayerID() {
	return m_wireViewLayerID;
}

ViewLayer::ViewLayerID SketchWidget::getWireViewLayerID(const ViewGeometry & viewGeometry) {
	Q_UNUSED(viewGeometry);
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

ViewLayer::ViewLayerID SketchWidget::getLabelViewLayerID() {
	return m_labelViewLayerID;
}

ViewLayer::ViewLayerID SketchWidget::getNoteViewLayerID() {
	return m_noteViewLayerID;
}


void SketchWidget::mousePressConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {

	ModelPart * wireModel = m_paletteModel->retrieveModelPart(ModuleIDNames::wireModuleIDName);
	if (wireModel == NULL) return;

	m_tempDragWireCommand = m_holdingSelectItemCommand;
	m_holdingSelectItemCommand = NULL;
	clearHoldingSelectItem();

	// make sure wire layer is visible
	ViewLayer::ViewLayerID viewLayerID = getDragWireViewLayerID();
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
	m_connectorDragWire = dynamic_cast<Wire *>(addItemAux(wireModel, viewGeometry, ItemBase::getNextID(), -1, NULL, NULL, true, m_viewIdentifier));
	DebugDialog::debug("creating connector drag wire");
	if (m_connectorDragWire == NULL) {
		clearDragWireTempCommand();
		return;
	}

	// give connector item the mouse, so wire doesn't get mouse moved events
	m_connectorDragWire->setVisible(true);
	m_connectorDragWire->grabMouse();
	m_connectorDragWire->initDragEnd(m_connectorDragWire->connector0(), event->scenePos());
	m_connectorDragConnector->tempConnectTo(m_connectorDragWire->connector1(), false);
	m_connectorDragWire->connector1()->tempConnectTo(m_connectorDragConnector, false);
	if (!m_lastColorSelected.isEmpty()) {
		m_connectorDragWire->setColorString(m_lastColorSelected, m_connectorDragWire->opacity());
	}
}

void SketchWidget::rotateX(qreal degrees) 
{
	clearHoldingSelectItem();
	m_savedItems.clear();
	m_savedWires.clear();
	prepMove(NULL);

	QRectF itemsBoundingRect;
	// want the bounding rect of the original selected items, not all the items that are secondarily being rotated
	foreach (QGraphicsItem * item, scene()->selectedItems()) {
		itemsBoundingRect |= (item->transform() * QTransform().translate(item->x(), item->y()))
                            .mapRect(item->boundingRect() | item->childrenBoundingRect());
	}

	QPointF center = itemsBoundingRect.center();

	QTransform rotation;
	rotation.rotate(degrees);

	QString string = tr("Rotate %2 (%1)")
			.arg(ViewIdentifierClass::viewIdentifierName(m_viewIdentifier))
			.arg((m_savedItems.count() == 1) ? m_savedItems.values()[0]->title() : QString::number(m_savedItems.count() + m_savedWires.count()) + " items" );
	QUndoCommand * parentCommand = new QUndoCommand(string);

	foreach (ItemBase * item, m_savedItems) {
		if (!rotationAllowed(item)) {
			continue;
		}

		ViewGeometry vg1 = item->getViewGeometry();
		ViewGeometry vg2(vg1);
		if (item->itemType() != ModelPart::Wire) {
			if (item->modelPart()->moduleID().compare(ModuleIDNames::rectangleModuleIDName) == 0) {
				// because these boards don't actually rotate yet
				QRectF r = item->boundingRect();
				//QPointF test(center.x() - (r.height() / 2.0), center.y() - (r.width() / 2.0));
				QPointF p0 = item->pos();
				if (degrees == 90) {
					p0 += r.bottomLeft();
				}
				else if (degrees == -90) {
					p0 += r.topRight();
				}
				else if (degrees == 180) {
					p0 += r.bottomRight();
				}
				else {
					// we're screwed: only multiples of 90 for now.
				}
				QPointF d0 = p0 - center;
				QPointF d0t = rotation.map(d0);
				vg2.setLoc(d0t + center);
			}
			else {
				QPointF dp = center - item->pos();
				QTransform tp = QTransform().translate(-dp.x(), -dp.y()) * rotation * QTransform().translate(dp.x(), dp.y());
				QPointF dc = item->boundingRect().center();
				QTransform tc = QTransform().translate(-dc.x(), -dc.y()) * rotation * QTransform().translate(dc.x(), dc.y());
				QPointF mp = tp.map(QPointF(0,0));
				QPointF mc = tc.map(QPointF(0,0));
				vg2.setLoc(vg1.loc() + mp - mc);
			}
			new MoveItemCommand(this, item->id(), vg1, vg1, parentCommand);
			new RotateItemCommand(this, item->id(), degrees, parentCommand);
			new MoveItemCommand(this, item->id(), vg2, vg2, parentCommand);
		}
		else {
			Wire * wire = qobject_cast<Wire *>(item);
			QPointF p0 = wire->connector0()->sceneAdjustedTerminalPoint(NULL);
			QPointF d0 = p0 - center;
			QPointF d0t = rotation.map(d0);

			QPointF p1 = wire->connector1()->sceneAdjustedTerminalPoint(NULL);
			QPointF d1 = p1 - center;
			QPointF d1t = rotation.map(d1);

			new ChangeWireCommand(this, wire->id(), vg1.line(), QLineF(QPointF(0,0), d1t - d0t), vg1.loc(), d0t + center, true, parentCommand);
		}
	}
	
	foreach (Wire * wire, m_savedWires.keys()) {
		ViewGeometry vg1 = wire->getViewGeometry();
		ViewGeometry vg2(vg1);

		ConnectorItem * rotater = m_savedWires.value(wire);
		QPointF p0 = rotater->sceneAdjustedTerminalPoint(NULL);
		QPointF d0 = p0 - center;
		QPointF d0t = rotation.map(d0);

		QPointF p1 = wire->otherConnector(rotater)->sceneAdjustedTerminalPoint(NULL);
		if (rotater == wire->connector0()) {
			new ChangeWireCommand(this, wire->id(), vg1.line(), QLineF(QPointF(0,0), p1 - (d0t + center)), vg1.loc(), d0t + center, true, parentCommand);
		}
		else {
			new ChangeWireCommand(this, wire->id(), vg1.line(), QLineF(QPointF(0,0), d0t + center - p1), vg1.loc(), vg1.loc(), true, parentCommand);
		}
	}

	m_undoStack->push(parentCommand);

	

	//rotateFlip(degrees, 0);
}


void SketchWidget::flip(Qt::Orientations orientation) {
	rotateFlip(0, orientation);
}

void SketchWidget::rotateFlip(qreal degrees, Qt::Orientations orientation)
{
	if (!this->isVisible()) return;

	clearHoldingSelectItem();

	QList <QGraphicsItem *> items = scene()->selectedItems();
	QList <ItemBase *> targets;

	for (int i = 0; i < items.size(); i++) {
		// can't rotate wires or layerkin (layerkin rotated indirectly)
		ItemBase *itemBase = ItemBase::extractTopLevelItemBase(items[i]);
		if (itemBase == NULL) continue;

		switch (itemBase->itemType()) {
			case ModelPart::Wire:
			case ModelPart::Note:
			case ModelPart::CopperFill:
			case ModelPart::Unknown:
				continue;
			case ModelPart::Board:
			case ModelPart::ResizableBoard:
			case ModelPart::Breadboard:
				if (orientation != 0) {
					// can't flip
					continue;
				}

			default:
				if (!rotationAllowed(itemBase)) {
					continue;
				}
				break;
		}

		targets.append(itemBase);
	}

	if (targets.count() <= 0) {
		return;
	}

	QString string = tr("%3 %2 (%1)")
			.arg(ViewIdentifierClass::viewIdentifierName(m_viewIdentifier))
			.arg((targets.count() == 1) ? targets[0]->title() : QString::number(targets.count()) + " items" )
			.arg((degrees != 0) ? tr("Rotate") : tr("Flip"));

	QUndoCommand * parentCommand = new QUndoCommand(string);

	bool doEmit = false;
	QSet<ItemBase *> emptyList;			// emptylist is only used for a move command
	ConnectorPairHash connectorHash;
	foreach (ItemBase * item, targets) {
		if (disconnectFromFemale(item, emptyList, connectorHash, true, parentCommand)) doEmit = true;

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
	if (doEmit) {
		emit ratsnestChangeSignal(this, parentCommand);
	}
	else {
		emit rotatingFlippingSignal(this, parentCommand);		// eventually, don't send signal when rotating board
	}

	new CleanUpWiresCommand(this, false, parentCommand);
	m_undoStack->push(parentCommand);

}

ConnectorItem * SketchWidget::findConnectorItem(ItemBase * itemBase, const QString & connectorID, bool seekLayerKin) {

	ConnectorItem * connectorItem = itemBase->findConnectorItemNamed(connectorID);
	if (connectorItem != NULL) return connectorItem;

	seekLayerKin = true;
	if (seekLayerKin) {
		PaletteItem * pitem = dynamic_cast<PaletteItem *>(itemBase);
		if (pitem == NULL) return NULL;

		foreach (ItemBase * lkpi, pitem->layerKin()) {
			connectorItem = lkpi->findConnectorItemNamed(connectorID);
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
	/*QBrush brush(color);
	brush.setTexture(QPixmap("/home/merun/workspace/fritzing_trunk/phoenix/resources/images/schematic_grid_tile.png"));
	scene()->setBackgroundBrush(brush);*/
	scene()->setBackgroundBrush(QBrush(color));
}

const QColor& SketchWidget::background() {
	return scene()->backgroundBrush().color();
}

void SketchWidget::setItemMenu(QMenu* itemMenu){
	m_itemMenu = itemMenu;
}

void SketchWidget::setWireMenu(QMenu* wireMenu){
	m_wireMenu = wireMenu;
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

	ConnectorItem * other = wire->otherConnector(fromConnectorItem);
	if (fromConnectorItem == wire->connector0()) {
		pos = toConnectorItem->sceneAdjustedTerminalPoint(fromConnectorItem);
		ConnectorItem * toConnector1 = other->firstConnectedToIsh();
		if (toConnector1 == NULL) {
			p2 = other->mapToScene(other->pos()) - pos;
		}
		else {
			p2 = toConnector1->sceneAdjustedTerminalPoint(other);
		}
	}
	else {
		pos = wire->pos();
		ConnectorItem * toConnector0 = other->firstConnectedToIsh();
		if (toConnector0 == NULL) {
			pos = wire->pos();
		}
		else {
			pos = toConnector0->sceneAdjustedTerminalPoint(other);
		}
		p2 = toConnectorItem->sceneAdjustedTerminalPoint(fromConnectorItem) - pos;
	}
	wire->setLineAnd(QLineF(p1, p2), pos, true);

	// here's the connect (model has been previously updated)
	fromConnectorItem->connectTo(toConnectorItem);
	toConnectorItem->connectTo(fromConnectorItem);

	this->update();

}

void SketchWidget::sketchWidget_wireDisconnected(long fromID, QString fromConnectorID) {
	DebugDialog::debug(QString("got wire disconnected"));
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
									bool connect, bool doEmit, bool seekLayerKin, bool updateConnections)
{
	changeConnectionAux(fromID, fromConnectorID, toID, toConnectorID, connect, seekLayerKin, updateConnections);

	// TODO: make a global NET data structure and use this to transfer connection/disconnection between views
	// the data structure is parts (via part id) and connections (via ?)
	// how the connection is instantiated (ratsnest, real wire, trace, etc) is view dependent

	// TODO: not sure this works for bendpoints with multiple connections

	if (doEmit) {
		fromID = findWire(fromID);
		toID = findWire(toID);
		emit changeConnectionSignal(fromID, fromConnectorID, toID, toConnectorID, connect, updateConnections);
	}
}

void SketchWidget::moduleChangeConnection(long fromID, const QString & fromConnectorID,
									QList<long> & toIDs, const QString & toConnectorID, bool doRatsnest,
									bool connect, bool doEmit, bool seekLayerKin, bool updateConnections)
{
	// figure out what part we're actually pointing at, then call changeConnection
	long id = ItemBase::getNextID(toIDs[0]);
	ItemBase * superBase = findItem(id);
	if (superBase == NULL) {
		return;
	}

	ItemBase * toBase = findModulePart(superBase, toIDs);
	if (toBase == NULL) {
		return;
	}

	changeConnection(fromID, fromConnectorID, toBase->id(), toConnectorID, connect, doEmit, seekLayerKin, updateConnections);

	if (doRatsnest) {
		DebugDialog::debug("moduleChangeConnection: doRatsnest not yet implemented");
	}
}

void SketchWidget::changeConnectionAux(long fromID, const QString & fromConnectorID,
									long toID, const QString & toConnectorID,
									bool connect, bool seekLayerKin, bool updateConnections)
{
	DebugDialog::debug(QString("changeConnection: from %1 %2; to %3 %4 con:%5 v:%6")
				.arg(fromID).arg(fromConnectorID)
				.arg(toID).arg(toConnectorID)
				.arg(connect).arg(m_viewIdentifier) );

	ItemBase * fromItem = findItem(fromID);
	if (fromItem == NULL) {
		DebugDialog::debug(QString("change connection exit 1 %1").arg(fromID));
		return;			// for now
	}

	ConnectorItem * fromConnectorItem = findConnectorItem(fromItem, fromConnectorID, seekLayerKin);
	if (fromConnectorItem == NULL) {
		// shouldn't be here
		DebugDialog::debug(QString("change connection exit 2 %1 %2").arg(fromID).arg(fromConnectorID));
		return;
	}

	ItemBase * toItem = findItem(toID);
	if (toItem == NULL) {
		DebugDialog::debug(QString("change connection exit 3 %1").arg(toID));
		return;
	}

	ConnectorItem * toConnectorItem = findConnectorItem(toItem, toConnectorID, seekLayerKin);
	if (toConnectorItem == NULL) {
		// shouldn't be here
		DebugDialog::debug(QString("change connection exit 4 %1 %2").arg(toID).arg(toConnectorID));
		return;
	}

	if (connect) {
		fromConnectorItem->connector()->connectTo(toConnectorItem->connector());
		fromConnectorItem->connectTo(toConnectorItem);
		toConnectorItem->connectTo(fromConnectorItem);
	}
	else {

		fromConnectorItem->connector()->disconnectFrom(toConnectorItem->connector());
		fromConnectorItem->removeConnection(toConnectorItem, true);
		toConnectorItem->removeConnection(fromConnectorItem, true);
	}

	chainVisible(fromConnectorItem, toConnectorItem, connect);

	if (updateConnections) {
		fromConnectorItem->attachedTo()->updateConnections(fromConnectorItem);
		toConnectorItem->attachedTo()->updateConnections(toConnectorItem);
	}
}

void SketchWidget::tempConnectWire(Wire * wire, ConnectorItem * from, ConnectorItem * to) {
	ConnectorItem * connector0 = wire->connector0();
	from->tempConnectTo(connector0, false);
	connector0->tempConnectTo(from, false);

	ConnectorItem * connector1 = wire->connector1();
	to->tempConnectTo(connector1, false);
	connector1->tempConnectTo(to, false);
}

void SketchWidget::sketchWidget_changeConnection(long fromID, QString fromConnectorID,
												 long toID, QString toConnectorID,
												 bool connect, bool updateConnections)
{
	changeConnection(fromID, fromConnectorID,
					 toID, toConnectorID,
					 connect, false, true, updateConnections);
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

void SketchWidget::keyReleaseEvent(QKeyEvent * event) {
	//DebugDialog::debug(QString("key release event %1").arg(event->isAutoRepeat()));
	if (m_movingByArrow) {
		// strange logic when doing autorepeat
		// each autorepeat sends both a keyPressEvent and a keyReleaseEvent
		// in keyPressEvents, the first event has autorepeat = false
		// but in keyReleaseEvents, the last event has autorepeat = false
		if (!event->isAutoRepeat()) {
			m_movingByArrow = false;
			if (checkMoved()) {
				m_savedItems.clear();
				m_savedWires.clear();
			}
		}
	}
	else {
		QGraphicsView::keyReleaseEvent(event);
	}
}

void SketchWidget::keyPressEvent ( QKeyEvent * event ) {
	if ((m_inFocus.length() == 0) && !m_movingByMouse) {
		int dx = 0, dy = 0;
		switch (event->key()) {
			case Qt::Key_Up:
				dy = -1;
				break;
			case Qt::Key_Down:
				dy = 1;
				break;
			case Qt::Key_Left:
				dx = -1;
				break;
			case Qt::Key_Right:
				dx = 1;
				break;
			default:
				break;
		}
		if (dx != 0 || dy != 0) {
			DebugDialog::debug("arrow key event");
			if (moveByArrow(dx, dy, event)) {
				return;
			}
		}
	}

	QGraphicsView::keyPressEvent(event);
}



void SketchWidget::sketchWidget_copyItem(long itemID, QHash<ViewIdentifierClass::ViewIdentifier, ViewGeometry *> & geometryHash) {
	ItemBase * itemBase = findItem(itemID);
	if (itemBase == NULL) {
		// this shouldn't happen
		return;
	}

	ViewGeometry * vg = new ViewGeometry(itemBase->getViewGeometry());
	geometryHash.insert(m_viewIdentifier, vg);
}

void SketchWidget::makeDeleteItemCommand(ItemBase * itemBase, BaseCommand::CrossViewType crossView, QUndoCommand * parentCommand) {
	// TODO: handle this with virtual functions in the itemBase

	if (itemBase->isPartLabelVisible()) {
		ShowLabelCommand * slc = new ShowLabelCommand(this, parentCommand);
		slc->add(itemBase->id(), true, true);
	}

	switch (itemBase->itemType()) {
		case ModelPart::Wire:
			{
			Wire * wire = dynamic_cast<Wire *>(itemBase);
			new WireWidthChangeCommand(this, wire->id(), wire->width(), wire->width(), parentCommand);
			new WireColorChangeCommand(this, wire->id(), wire->colorString(), wire->colorString(), wire->opacity(), wire->opacity(), parentCommand);
			}
			break;
		
		// TODO: LogoItem and Ruler

		case ModelPart::ResizableBoard:
			{
			ResizableBoard * rb = dynamic_cast<ResizableBoard *>(itemBase);
			rb->saveParams();
			QPointF p;
			QSizeF sz;
			rb->getParams(p, sz);
			new ResizeBoardCommand(this, rb->id(), sz.width(), sz.height(), sz.width(), sz.height(), parentCommand);
			}
			break;
		case ModelPart::Jumper:
			{
			JumperItem * jumper = dynamic_cast<JumperItem *>(itemBase);
			jumper->saveParams();
			QPointF p;
			QPointF c0, c1;
			jumper->getParams(p, c0, c1);
			new ResizeJumperItemCommand(this, jumper->id(), p, c0, c1, p, c0, c1, parentCommand);
			}
			break;
		case ModelPart::CopperFill:
			{
			GroundPlane * groundPlane = dynamic_cast<GroundPlane *>(itemBase);
			new SetPropCommand(this, itemBase->id(), "svg", groundPlane->svg(), groundPlane->svg(), parentCommand);
			}
			break;
		default:
			break;
	}

	// TODO: does this need to be generalized to the whole set of modelpart props?
	MysteryPart * mysteryPart = dynamic_cast<MysteryPart *>(itemBase);
	if (mysteryPart != NULL) {
		new SetPropCommand(this, itemBase->id(), "chip label", mysteryPart->chipLabel(), mysteryPart->chipLabel(), parentCommand);
		new SetPropCommand(this, itemBase->id(), "spacing", mysteryPart->spacing(), mysteryPart->spacing(), parentCommand);
	}

	PinHeader * pinHeader = dynamic_cast<PinHeader *>(itemBase);
	if (pinHeader != NULL) {
		new SetPropCommand(this, itemBase->id(), "form", pinHeader->form(), pinHeader->form(), parentCommand);
	}

	Resistor * resistor =  dynamic_cast<Resistor *>(itemBase);
	if (resistor != NULL) {
		new SetResistanceCommand(this, itemBase->id(), resistor->resistance(), resistor->resistance(), resistor->pinSpacing(), resistor->pinSpacing(), parentCommand);
	}

	rememberSticky(itemBase->id(), parentCommand);
	if (crossView == BaseCommand::CrossView) {
		emit rememberStickySignal(itemBase->id(), parentCommand);
	}

	new DeleteItemCommand(this, crossView, itemBase->modelPart()->moduleID(), itemBase->getViewGeometry(), itemBase->id(), itemBase->modelPart()->modelIndex(), itemBase->modelPart()->originalModelIndex(), parentCommand);
}

void SketchWidget::rememberSticky(long id, QUndoCommand * parentCommand) {
	ItemBase * itemBase = findItem(id);
	if (itemBase == NULL) return;

	QList<ItemBase *> stickyList = itemBase->stickyList();
	if (stickyList.count() <= 0) return;

	CheckStickyCommand * checkStickyCommand = new CheckStickyCommand(this, BaseCommand::SingleView, itemBase->id(), false, parentCommand);
	if (itemBase->sticky()) {
		foreach (ItemBase * sticking, stickyList) {
			checkStickyCommand->stick(this, itemBase->id(), sticking->id(), false);
		}
	}
	else if (itemBase->stickingTo() != NULL) {
		checkStickyCommand->stick(this, itemBase->stickingTo()->id(), itemBase->id(), false);
	}
}

ViewIdentifierClass::ViewIdentifier SketchWidget::viewIdentifier() {
	return m_viewIdentifier;
}


void SketchWidget::setViewLayerIDs(ViewLayer::ViewLayerID part, ViewLayer::ViewLayerID wire, ViewLayer::ViewLayerID connector, ViewLayer::ViewLayerID ruler, ViewLayer::ViewLayerID label, ViewLayer::ViewLayerID note) {
	m_partViewLayerID = part;
	m_wireViewLayerID = wire;
	m_connectorViewLayerID = connector;
	m_rulerViewLayerID = ruler;
	m_labelViewLayerID = label;
	m_noteViewLayerID = note;
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

ViewLayer::ViewLayerID SketchWidget::getViewLayerID(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier) {

	QDomElement layers = LayerAttributes::getSvgElementLayers(modelPart->modelPartShared()->domDocument(), viewIdentifier);
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

	return multiLayerGetViewLayerID(modelPart, viewIdentifier, layers, layerName);
}

ViewLayer::ViewLayerID SketchWidget::multiLayerGetViewLayerID(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, QDomElement & layers, QString & layerName) {
	Q_UNUSED(modelPart);
	Q_UNUSED(layers);
	Q_UNUSED(viewIdentifier);

	return ViewLayer::viewLayerIDFromXmlString(layerName);
}

ItemBase * SketchWidget::overSticky(ItemBase * itemBase) {
	if (!itemBase->stickyEnabled()) return NULL;

	if (itemBase->itemType() != ModelPart::Module) {
		foreach (QGraphicsItem * item, scene()->collidingItems(itemBase)) {
			ItemBase * s = dynamic_cast<ItemBase *>(item);
			if (s == NULL) continue;
			if (s == itemBase) continue;
			if (!s->sticky()) continue;

			return s->layerKinChief();
		}

		return NULL;
	}

	// if it's a module, look at connectors inside the module

	foreach (QGraphicsItem * childItem, itemBase->childItems()) {
		ItemBase * subItemBase = dynamic_cast<ItemBase *>(childItem);
		if (subItemBase != NULL) {
			ItemBase * result = overSticky(subItemBase);
			if (result != NULL) {
				return result;
			}
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

void SketchWidget::stickyScoop(ItemBase * stickyOne, bool checkCurrent, CheckStickyCommand * checkStickyCommand) {
	// TODO: use the shape rather than the rect
	// need to find the best layerkin to use in that case
	//foreach (QGraphicsItem * item, scene()->collidingItems(stickyOne)) {

	QList<ItemBase *> added;
	QList<ItemBase *> already;
	QPolygonF poly = stickyOne->mapToScene(stickyOne->boundingRect());
	foreach (QGraphicsItem * item, scene()->items(poly)) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;

		itemBase = itemBase->layerKinChief();

		// check whether it's a module
		ItemBase * parent = itemBase;
		while (true) {
			ItemBase * temp = dynamic_cast<ItemBase *>(parent->parentItem());
			if (temp == NULL) break;

			parent = temp;
		}

		itemBase = parent;

		if (!itemBase->stickyEnabled()) continue;
		if (added.contains(itemBase)) continue;
		if (itemBase->sticky()) continue;
		if (stickyOne->alreadySticking(itemBase)) {
			already.append(itemBase);
			continue;
		}

		stickyOne->addSticky(itemBase, true);
		itemBase->addSticky(stickyOne, true);
		if (checkStickyCommand) {
			checkStickyCommand->stick(this, stickyOne->id(), itemBase->id(), true);
		}

		added.append(itemBase);
	}

	if (checkCurrent) {
		foreach (ItemBase * itemBase, stickyOne->stickyList()) {
			if (added.contains(itemBase)) continue;
			if (already.contains(itemBase)) continue;

			stickyOne->addSticky(itemBase, false);
			itemBase->addSticky(stickyOne, false);
			if (checkStickyCommand) {
				checkStickyCommand->stick(this, stickyOne->id(), itemBase->id(), false);
			}
		}
	}
}

void SketchWidget::wire_wireSplit(Wire* wire, QPointF newPos, QPointF oldPos, QLineF oldLine) {
	if (!canChainWire(wire)) return;

	this->clearHoldingSelectItem();
	this->m_moveEventCount = 0;  // clear this so an extra MoveItemCommand isn't posted

	QUndoCommand * parentCommand = new QUndoCommand();
	parentCommand->setText(QObject::tr("Split Wire") );

	long fromID = wire->id();

	QLineF newLine(oldLine.p1(), newPos - oldPos);

	long newID = ItemBase::getNextID();
	ViewGeometry vg(wire->getViewGeometry());
	vg.setLoc(newPos);
	QLineF newLine2(QPointF(0,0), oldLine.p2() + oldPos - newPos);
	vg.setLine(newLine2);

	BaseCommand::CrossViewType crossView = wireSplitCrossView();

	new AddItemCommand(this, crossView, ModuleIDNames::wireModuleIDName, vg, newID, true, -1, -1, parentCommand);
	new CheckStickyCommand(this, crossView, newID, false, parentCommand);
	new WireColorChangeCommand(this, newID, wire->colorString(), wire->colorString(), wire->opacity(), wire->opacity(), parentCommand);
	new WireWidthChangeCommand(this, newID, wire->width(), wire->width(), parentCommand);

	// disconnect from original wire and reconnect to new wire
	ConnectorItem * connector1 = wire->connector1();
	foreach (ConnectorItem * toConnectorItem, connector1->connectedToItems()) {
		new ChangeConnectionCommand(this, crossView, toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
			wire->id(), connector1->connectorSharedID(),
			false, true, parentCommand);
		new ChangeConnectionCommand(this, crossView, toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
			newID, connector1->connectorSharedID(),
			true, true, parentCommand);
	}

	new ChangeWireCommand(this, fromID, oldLine, newLine, oldPos, oldPos, true, parentCommand);

	// connect the two wires
	new ChangeConnectionCommand(this, crossView, wire->id(), connector1->connectorSharedID(),
			newID, "connector0", true, true, parentCommand);

	SelectItemCommand * selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
	selectItemCommand->addRedo(newID);

	new CleanUpWiresCommand(this, false, parentCommand);

	m_undoStack->push(parentCommand);
}

void SketchWidget::wire_wireJoin(Wire* wire, ConnectorItem * clickedConnectorItem) {
	// if (!canChainWire(wire)) return;  // can't join a wire in some views (for now)

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

	BaseCommand::CrossViewType crossView = BaseCommand::CrossView; // wireSplitCrossView();

	// disconnect the wires
	new ChangeConnectionCommand(this, crossView, wire->id(), clickedConnectorItem->connectorSharedID(),
			toWire->id(), toConnectorItem->connectorSharedID(), false, true, parentCommand);

	// disconnect everyone from the other end of the wire being deleted, and reconnect to the remaining wire
	foreach (ConnectorItem * otherToConnectorItem, otherConnector->connectedToItems()) {
		new ChangeConnectionCommand(this, crossView, otherToConnectorItem->attachedToID(), otherToConnectorItem->connectorSharedID(),
			toWire->id(), otherConnector->connectorSharedID(),
			false, true, parentCommand);
		new ChangeConnectionCommand(this, crossView, otherToConnectorItem->attachedToID(), otherToConnectorItem->connectorSharedID(),
			wire->id(), clickedConnectorItem->connectorSharedID(),
			true, true, parentCommand);
	}

	toWire->saveGeometry();
	makeDeleteItemCommand(toWire, crossView, parentCommand);

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

	m_undoStack->push(parentCommand);
}

void SketchWidget::hoverEnterItem(QGraphicsSceneHoverEvent * event, ItemBase * item) {
	if(m_infoViewOnHover || currentlyInfoviewed(item)) {
		InfoGraphicsView::hoverEnterItem(event, item);
	}

	Wire * wire = dynamic_cast<Wire *>(item);
	if (wire != NULL) {
		if (canChainWire(wire)) {	
			bool segment = wire->connector0()->chained() && wire->connector1()->chained();
			bool disconnected = wire->connector0()->connectionsCount() == 0 &&  wire->connector1()->connectionsCount() == 0;
			statusMessage(QString("%1 to add a bendpoint %2")
				.arg(disconnected ? tr("Double-click") : tr("Drag or double-click"))
				.arg(segment ? tr("or alt-drag to move the segment") : tr("")));
			m_lastHoverEnterItem = item;
		}
	}
}

void SketchWidget::statusMessage(QString message, int timeout) 
{
	// TODO: eventually do the connecting from the window not from the sketch
	QMainWindow * mainWindow = dynamic_cast<QMainWindow *>(window());
	if (mainWindow == NULL) return;

	if (m_statusConnectState == StatusConnectNotTried) {
		bool result = connect(this, SIGNAL(statusMessageSignal(QString, int)),
							  mainWindow, SLOT(statusMessage(QString, int)));
		m_statusConnectState = (result) ? StatusConnectSucceeded : StatusConnectFailed;
	}

	if (m_statusConnectState == StatusConnectFailed) {
		QStatusBar * sb = mainWindow->statusBar();
		if (sb != NULL) {
			sb->showMessage(message, timeout);
		}
	}
	else {
		emit statusMessageSignal(message, timeout);
	}
}

void SketchWidget::hoverLeaveItem(QGraphicsSceneHoverEvent * event, ItemBase * item){
	m_lastHoverEnterItem = NULL;

	if(m_infoViewOnHover) {
		InfoGraphicsView::hoverLeaveItem(event, item);
	}

	if (canChainWire(dynamic_cast<Wire *>(item))) {
		statusMessage(QString());
	}
}

void SketchWidget::hoverEnterConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item) {
	if(m_infoViewOnHover || currentlyInfoviewed(item->attachedTo())) {
		viewConnectorItemInfo(item);
	}

	if (item->attachedToItemType() == ModelPart::Wire) {
		if (!this->m_chainDrag) return;
		if (!item->chained()) return;

		m_lastHoverEnterConnectorItem = item;
		QString msg = hoverEnterWireConnectorMessage(event, item);
		statusMessage(msg);
	}
	else {
		QString msg = hoverEnterPartConnectorMessage(event, item);
		statusMessage(msg);
	}

}
const QString & SketchWidget::hoverEnterWireConnectorMessage(QGraphicsSceneHoverEvent * event, ConnectorItem * item)
{
	Q_UNUSED(event);
	Q_UNUSED(item);

	static QString message = tr("Double-click to delete this bend point");
	return message;
}


const QString & SketchWidget::hoverEnterPartConnectorMessage(QGraphicsSceneHoverEvent * event, ConnectorItem * item)
{
	Q_UNUSED(event);
	Q_UNUSED(item);

	return ___emptyString___;
}

void SketchWidget::hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * event, ConnectorItem * item) {
	m_lastHoverEnterConnectorItem = NULL;

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

	if (attachedTo->itemType() == ModelPart::Wire) {
		if (!this->m_chainDrag) return;
		if (!item->chained()) return;

		statusMessage(QString());
	}
	else {
		statusMessage(QString());
	}
}

bool SketchWidget::currentlyInfoviewed(ItemBase *item) {
	if(m_infoView) {
		ItemBase * currInfoView = m_infoView->currentItem();
		return !currInfoView || item == currInfoView;//->cu selItems.size()==1 && selItems[0] == item;
	}
	return false;
}

void SketchWidget::cleanUpWires(bool doEmit, CleanUpWiresCommand * command, bool skipMe) {
	if (!skipMe) {
		RoutingStatus routingStatus;
		updateRatsnestStatus(command, NULL, routingStatus);
	}

	if (doEmit) {
		emit cleanUpWiresSignal(command);
	}
}

void SketchWidget::sketchWidget_cleanUpWires(CleanUpWiresCommand * command) {
	RoutingStatus routingStatus;
	updateRatsnestStatus(command, NULL, routingStatus);
}

void SketchWidget::noteChanged(ItemBase * item, const QString &oldText, const QString & newText, QSizeF oldSize, QSizeF newSize) {
	ChangeNoteTextCommand * command = new ChangeNoteTextCommand(this, item->id(), oldText, newText, oldSize, newSize, NULL);
	command->setText(tr("Change %note to '%2'").arg(newText));
	m_undoStack->push(command);
}

void SketchWidget::partLabelChanged(ItemBase * pitem,const QString & oldText, const QString &newText) {
	// partLabelChanged triggered from inline editing the label

	if (!m_current) {
		// all three views get the partLabelChanged call, but only need to act on this once
		return;
	}

	//if (currentlyInfoviewed(pitem))  {
		// TODO: just change the affected item in the info view
		//InfoGraphicsView::viewItemInfo(pitem);
	//}

	partLabelChangedAux(pitem, oldText, newText);
}

void SketchWidget::partLabelChangedAux(ItemBase * pitem,const QString & oldText, const QString &newText)
{
	if (pitem == NULL) return;

	ChangeLabelTextCommand * command = new ChangeLabelTextCommand(this, pitem->id(), oldText, newText, NULL);
	command->setText(tr("Change %1 label to '%2'").arg(pitem->title()).arg(newText));
	m_undoStack->push(command);
}

void SketchWidget::setInfoViewOnHover(bool infoViewOnHover) {
	m_infoViewOnHover = infoViewOnHover;
}

void SketchWidget::updateInfoView() {
	QTimer::singleShot(50, this,  SLOT(updateInfoViewSlot()));
}

void SketchWidget::updateInfoViewSlot() {
	foreach (QGraphicsItem * item, scene()->selectedItems())
	{
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;

		ItemBase * chief = itemBase->layerKinChief();
		InfoGraphicsView::viewItemInfo(chief);
		return;
	}

	InfoGraphicsView::viewItemInfo(m_lastPaletteItemSelected);
}

long SketchWidget::setUpSwap(long itemID, long newModelIndex, const QString & newModuleID, bool master, QUndoCommand * parentCommand)
{
	ItemBase * itemBase = findItem(itemID);
	if (itemBase == NULL) return 0;

	long newID = ItemBase::getNextID(newModelIndex);

	ViewGeometry vg = itemBase->getViewGeometry();
	QTransform oldTransform = vg.transform();
	bool needsTransform = false;
	if (!oldTransform.isIdentity()) {
		// restore identity transform
		vg.setTransform(QTransform());
		needsTransform = true;
	}

	newAddItemCommand(BaseCommand::SingleView, newModuleID, vg, newID, true, newModelIndex, -1, parentCommand);

	if (needsTransform) {
		QMatrix m;
		m.setMatrix(oldTransform.m11(), oldTransform.m12(), oldTransform.m21(), oldTransform.m22(), 0, 0);
		new TransformItemCommand(this, newID, m, m, parentCommand);
	}

	setUpSwapReconnect(itemBase, newID, newModuleID, master, parentCommand);
	new CheckStickyCommand(this, BaseCommand::SingleView, newID, false, parentCommand);
	if (itemBase->isPartLabelVisible()) {
		ShowLabelCommand * slc = new ShowLabelCommand(this, parentCommand);
		slc->add(newID, true, true);
	}

	if (master) {		
		SelectItemCommand * selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
		selectItemCommand->addRedo(newID);
		selectItemCommand->addUndo(itemBase->id());
		new ChangeLabelTextCommand(this, itemBase->id(), itemBase->instanceTitle(), itemBase->instanceTitle(), parentCommand);
		new ChangeLabelTextCommand(this, newID, itemBase->instanceTitle(), itemBase->instanceTitle(), parentCommand);
			
		MysteryPart * mysteryPart = dynamic_cast<MysteryPart *>(itemBase);
		if (mysteryPart != NULL) {
			new SetPropCommand(this, newID, "chip label", mysteryPart->chipLabel(), mysteryPart->chipLabel(), parentCommand);
			new SetPropCommand(this, newID, "spacing", mysteryPart->spacing(), mysteryPart->spacing(), parentCommand);
		}
	
		PinHeader * pinHeader = dynamic_cast<PinHeader *>(itemBase);
		if (pinHeader != NULL) {
			new SetPropCommand(this, newID, "form", pinHeader->form(), pinHeader->form(), parentCommand);
		}
	
		makeDeleteItemCommand(itemBase, BaseCommand::CrossView, parentCommand);
		selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
		selectItemCommand->addRedo(newID);  // to make sure new item is selected so it appears in the info view
		new CleanUpWiresCommand(this, false, parentCommand);
	}

	return newID;
}

void SketchWidget::setUpSwapReconnect(ItemBase* itemBase, long newID, const QString & newModuleID, bool master, QUndoCommand * parentCommand)
{
	ConnectorPairHash connectorHash;
	itemBase->collectConnectors(connectorHash, this->scene());

	ModelPart * newModelPart = m_refModel->retrieveModelPart(newModuleID);
	if (newModelPart == NULL) return;

	newModelPart->initConnectors();			//  make sure the connectors are set up
	QHash<QString, Connector *> newConnectors = newModelPart->connectors();

	foreach (ConnectorItem * fromConnectorItem, connectorHash.uniqueKeys()) {
		QList<Connector *> candidates;
		Connector * newConnector = NULL;
		QString fromName = fromConnectorItem->connectorSharedName();
		foreach (Connector * connector, newConnectors.values()) {
			if (fromName.compare(connector->connectorSharedName(), Qt::CaseInsensitive) == 0) {
				candidates.append(connector);
			}
		}

		bool swappedGenderFlag = false;
		if (candidates.count() > 0) {
			// TODO: check distances (for capacitors, for example)
			//			use preloadSlowParts code to set up new connectors (which layers?)
			//			skip breadboards, skip gender changes
			//			only look at connectors with direct m/f connections
			//			only care about single parts,
			newConnector = candidates[0];
			if (candidates.count() > 1) {
				foreach (Connector * connector, candidates) {
					// this gets an exact match, if there is one
					if (fromConnectorItem->connectorSharedID().compare(connector->connectorSharedID(), Qt::CaseInsensitive) == 0) {					
						newConnector = connector;
						break;
					}
				}
			}
			swappedGenderFlag = swappedGender(fromConnectorItem, newConnector);
		}
	
		foreach (ConnectorItem * toConnectorItem, connectorHash.values(fromConnectorItem)) {
			// delete connection to part being swapped out
			new ChangeConnectionCommand(this, BaseCommand::SingleView,
										fromConnectorItem->attachedToID(), fromConnectorItem->connectorSharedID(),
										toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
										false, true, parentCommand);

			bool cleanup = false;
			if (swappedGenderFlag) {
				cleanup = toConnectorItem->connectorType() == newConnector->connectorType();
			}
			if ((newConnector == NULL) || cleanup) {
					// clean up after disconnect
					new RatsnestCommand(this, BaseCommand::SingleView,
										fromConnectorItem->attachedToID(), fromConnectorItem->connectorSharedID(),
										toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
										false, true, parentCommand);
			}
			else {
				// reconnect directly without cleaning up
				new ChangeConnectionCommand(this, BaseCommand::SingleView,
											newID, newConnector->connectorSharedID(),
											toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
											true, true, parentCommand);
			}

			if (cleanup && master) {
				long wireID = ItemBase::getNextID();
				ViewGeometry vg;
				new AddItemCommand(this, BaseCommand::CrossView, ModuleIDNames::wireModuleIDName, vg, wireID, false, -1, -1, parentCommand);
				new CheckStickyCommand(this, BaseCommand::CrossView, wireID, false, parentCommand);
				new ChangeConnectionCommand(this, BaseCommand::CrossView, newID, newConnector->connectorSharedID(),
						wireID, "connector0", true, true, parentCommand);
				new ChangeConnectionCommand(this, BaseCommand::CrossView, toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
						wireID, "connector1", true, true, parentCommand);
				new RatsnestCommand(this, BaseCommand::CrossView, newID, newConnector->connectorSharedID(),
						wireID, "connector0", true, true, parentCommand);
				new RatsnestCommand(this, BaseCommand::CrossView, toConnectorItem->attachedToID(), toConnectorItem->connectorSharedID(),
						wireID, "connector1", true, true, parentCommand);
			}
		}
	}
}

void SketchWidget::changeWireColor(const QString newColor)
{
	m_lastColorSelected = newColor;
	QList <Wire *> wires;
	foreach (QGraphicsItem * item, scene()->selectedItems()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;

		wires.append(wire);
	}

	if (wires.count() <= 0) return;

	QString commandString;
	if (wires.count() == 1) {
		commandString = tr("Change %1 color from %2 to %3")
			.arg(wires[0]->instanceTitle())
			.arg(wires[0]->colorString())
			.arg(newColor);
	}
	else {
		commandString = tr("Change color of %1 wires to %2")
			.arg(wires.count())
			.arg(newColor);
	}

	QUndoCommand* parentCommand = new QUndoCommand(commandString);
	foreach (Wire * wire, wires) {
		QList<Wire *> subWires;
		wire->collectWires(subWires);

		foreach (Wire * subWire, subWires) {
			new WireColorChangeCommand(
					this,
					subWire->id(),
					subWire->colorString(),
					newColor,
					subWire->opacity(),
					subWire->opacity(),
					parentCommand);
		}
	}

	m_undoStack->push(parentCommand);
}

void SketchWidget::changeWireWidthMils(const QString newWidthStr)
{
	bool ok = false;
	qreal newWidth = newWidthStr.toDouble(&ok);
	if (!ok) return;

	qreal trueWidth = FSvgRenderer::printerScale() * newWidth / 1000.0;

	QList <Wire *> wires;
	foreach (QGraphicsItem * item, scene()->selectedItems()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;
		if (!wire->getTrace()) continue;

		wires.append(wire);
	}

	if (wires.count() <= 0) return;

	QString commandString;
	if (wires.count() == 1) {
		commandString = tr("Change %1 width from %2 to %3")
			.arg(wires[0]->instanceTitle())
			.arg((int) wires[0]->mils())
			.arg(newWidth);
	}
	else {
		commandString = tr("Change width of %1 wires to %2")
			.arg(wires.count())
			.arg(newWidth);
	}

	QUndoCommand* parentCommand = new QUndoCommand(commandString);
	foreach (Wire * wire, wires) {
		QList<Wire *> subWires;
		wire->collectWires(subWires);

		foreach (Wire * subWire, subWires) {
			new WireWidthChangeCommand(
					this,
					subWire->id(),
					subWire->width(),
					trueWidth,
					parentCommand);
		}
	}

	m_undoStack->push(parentCommand);
}

void SketchWidget::changeWireColor(long wireId, const QString& color, qreal opacity) {
	ItemBase *item = findItem(wireId);
	Wire* wire = dynamic_cast<Wire*>(item);
	if (wire) {
		wire->setColorString(color, opacity);
		updateInfoView();
	}
}

void SketchWidget::changeWireWidth(long wireId, qreal width) {
	ItemBase *item = findItem(wireId);
	Wire* wire = dynamic_cast<Wire*>(item);
	if (wire) {
		wire->setWireWidth(width, this);
		updateInfoView();
	}
}

PaletteModel * SketchWidget::paletteModel() {
	return m_paletteModel;
}

bool SketchWidget::swappingEnabled(ItemBase * itemBase) {
	if (itemBase == NULL) {
		return m_refModel->swapEnabled();
	}

	return (m_refModel->swapEnabled() && itemBase->isSwappable());
}

void SketchWidget::resizeEvent(QResizeEvent * event) {
	InfoGraphicsView::resizeEvent(event);
	emit resizeSignal();
}

void SketchWidget::addBreadboardViewLayers() {
	setViewLayerIDs(ViewLayer::Breadboard, ViewLayer::BreadboardWire, ViewLayer::Breadboard, ViewLayer::BreadboardRuler, ViewLayer::BreadboardLabel, ViewLayer::BreadboardNote);

	QList<ViewLayer::ViewLayerID> layers;
	layers << ViewLayer::BreadboardBreadboard << ViewLayer::Breadboard
		<< ViewLayer::BreadboardWire << ViewLayer::BreadboardLabel << ViewLayer::BreadboardNote << ViewLayer::BreadboardRuler;

	addViewLayersAux(layers);
}

void SketchWidget::addSchematicViewLayers() {
	setViewLayerIDs(ViewLayer::Schematic, ViewLayer::SchematicTrace, ViewLayer::Schematic, ViewLayer::SchematicRuler, ViewLayer::SchematicLabel, ViewLayer::SchematicNote);

	QList<ViewLayer::ViewLayerID> layers;
	layers << ViewLayer::Schematic << ViewLayer::SchematicWire << ViewLayer::SchematicTrace << ViewLayer::SchematicLabel << ViewLayer::SchematicNote <<  ViewLayer::SchematicRuler;

	addViewLayersAux(layers);
}

void SketchWidget::addPcbViewLayers() {
	setViewLayerIDs(ViewLayer::Silkscreen, ViewLayer::Copper0Trace, ViewLayer::Copper0, ViewLayer::PcbRuler, ViewLayer::SilkscreenLabel, ViewLayer::PcbNote);

	QList<ViewLayer::ViewLayerID> layers;
	layers << ViewLayer::Board << ViewLayer::GroundPlane << ViewLayer::Copper1 << ViewLayer::Copper0 << ViewLayer::Ratsnest << ViewLayer::Copper0Trace
		/* << ViewLayer::Keepout */ << ViewLayer::Vias /* << ViewLayer::Soldermask */  
		<< ViewLayer::Silkscreen << ViewLayer::SilkscreenLabel /* << ViewLayer::Outline */
		<< ViewLayer::Jumperwires << ViewLayer::PcbNote << ViewLayer::PcbRuler;

	addViewLayersAux(layers);

	ViewLayer * silkscreen = m_viewLayers.value(ViewLayer::Silkscreen);
	ViewLayer * silkscreenLabel = m_viewLayers.value(ViewLayer::SilkscreenLabel);
	if (silkscreen && silkscreenLabel) {
		//silkscreenLabel->setParentLayer(silkscreen);
	}
	ViewLayer * copper0 = m_viewLayers.value(ViewLayer::Copper0);
	ViewLayer * copper0Trace = m_viewLayers.value(ViewLayer::Copper0Trace);
	if (copper0 && copper0Trace) {
		copper0Trace->setParentLayer(copper0);
	}
	ViewLayer * groundPlane = m_viewLayers.value(ViewLayer::GroundPlane);
	if (copper0 && groundPlane) {
		groundPlane->setParentLayer(copper0);
	}
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
	if (ignore) {
		m_ignoreSelectionChangeEvents++;
	}
	else {
		m_ignoreSelectionChangeEvents--;
	}
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

void SketchWidget::changeWireFlags(long wireId, ViewGeometry::WireFlags wireFlags)
{
	ItemBase *item = findItem(wireId);
	if(Wire* wire = dynamic_cast<Wire*>(item)) {
		wire->setWireFlags(wireFlags);
	}
}

bool SketchWidget::disconnectFromFemale(ItemBase * item, QSet<ItemBase *> & savedItems, ConnectorPairHash & connectorHash, bool doCommand, QUndoCommand * parentCommand)
{
	// schematic and pcb view connections are always via wires so this is a no-op.  breadboard view has it's own version.

	Q_UNUSED(item);
	Q_UNUSED(savedItems);
	Q_UNUSED(parentCommand);
	Q_UNUSED(connectorHash);
	Q_UNUSED(doCommand);
	return false;
}

void SketchWidget::spaceBarIsPressedSlot(bool isPressed) {
	m_spaceBarIsPressed = isPressed;
	if (isPressed) {
		setDragMode(QGraphicsView::ScrollHandDrag);
		//setInteractive(false);
		setCursor(Qt::OpenHandCursor);
	}
	else {
		setDragMode(QGraphicsView::RubberBandDrag);
		//setInteractive(true);
		setCursor(Qt::ArrowCursor);
	}
}

void SketchWidget::updateRatsnestStatus(CleanUpWiresCommand * command, QUndoCommand * undoCommand, RoutingStatus &) {
	Q_UNUSED(command);
	Q_UNUSED(undoCommand);
}


void SketchWidget::ensureLayerVisible(ViewLayer::ViewLayerID viewLayerID)
{
	ViewLayer * viewLayer = m_viewLayers.value(viewLayerID, NULL);
	if (viewLayer == NULL) return;

	if (!viewLayer->visible()) {
		setLayerVisible(viewLayer, true);
	}
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
	//DebugDialog::debug(QString("scrolling dx:%1 dy:%2").arg(m_autoScrollX).arg(m_autoScrollY) );

	if (m_autoScrollX == 0 && m_autoScrollY == 0 ) return;


	if (m_autoScrollX != 0) {
		QScrollBar * h = horizontalScrollBar();
		h->setValue(m_autoScrollX + h->value());
	}
	if (m_autoScrollY != 0) {
		QScrollBar * v = verticalScrollBar();
		v->setValue(m_autoScrollY + v->value());
	}
}

void SketchWidget::dragAutoScrollTimeout()
{
	autoScrollTimeout();
	dragMoveHighlightConnector(mapFromGlobal(m_globalPos));
}


void SketchWidget::moveAutoScrollTimeout()
{
	autoScrollTimeout();
	moveItems(m_globalPos, true);
}

const QString &SketchWidget::selectedModuleID() {
	if(m_lastPaletteItemSelected) {
		return m_lastPaletteItemSelected->modelPart()->moduleID();
	}
	return ___emptyString___;
}

BaseCommand::CrossViewType SketchWidget::wireSplitCrossView()
{
	return BaseCommand::SingleView;
}

bool SketchWidget::canDeleteItem(QGraphicsItem * item)
{
	ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
	if (itemBase == NULL) return false;

	ItemBase * chief = itemBase->layerKinChief();
	if (chief == NULL) return false;

	return true;
}

bool SketchWidget::canCopyItem(QGraphicsItem * item)
{
	ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
	if (itemBase == NULL) return false;

	ItemBase * chief = itemBase->layerKinChief();
	if (chief == NULL) return false;

	return true;
}

bool SketchWidget::canChainMultiple() {
	return false;
}

bool SketchWidget::canChainWire(Wire * wire) {
	if (!this->m_chainDrag) return false;
	if (wire == NULL) return false;

	return true;
}

bool SketchWidget::canCreateWire(Wire * dragWire, ConnectorItem * from, ConnectorItem * to)
{
	Q_UNUSED(dragWire);
	Q_UNUSED(from);
	Q_UNUSED(to);
	return true;
}


bool SketchWidget::modifyNewWireConnections(Wire * dragWire, ConnectorItem * fromOnWire, ConnectorItem * from, ConnectorItem * to, QUndoCommand * parentCommand)
{
	Q_UNUSED(dragWire);
	Q_UNUSED(fromOnWire);
	Q_UNUSED(from);
	Q_UNUSED(to);
	Q_UNUSED(parentCommand);
	return false;
}

void SketchWidget::setupAutoscroll(bool moving) {
	m_autoScrollX = m_autoScrollY = 0;
	connect(&m_autoScrollTimer, SIGNAL(timeout()), this,
		moving ? SLOT(moveAutoScrollTimeout()) : SLOT(dragAutoScrollTimeout()));
	//DebugDialog::debug("set up autoscroll");
}

void SketchWidget::turnOffAutoscroll() {
	m_autoScrollTimer.stop();
	disconnect(&m_autoScrollTimer, SIGNAL(timeout()), this, SLOT(autoScrollTimeout()));
	//DebugDialog::debug("turn off autoscroll");

}

bool SketchWidget::checkAutoscroll(QPoint globalPos)
{
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
		return false;
	}

	//DebugDialog::debug(QString("check autoscroll %1, %2 %3").arg(QTime::currentTime().msec()).arg(q.x()).arg(q.y()) );

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
			//DebugDialog::debug("starting autoscroll timer");
			m_autoScrollTimer.start(10);
		}

	}
	else {
		m_autoScrollX = m_autoScrollY = 0;
	}

	//DebugDialog::debug(QString("autoscroll %1 %2").arg(m_autoScrollX).arg(m_autoScrollY) );
	return true;
}

void SketchWidget::dealWithRatsnest(long fromID, const QString & fromConnectorID,
											long toID, const QString & toConnectorID,
											bool connect, RatsnestCommand * ratsnestCommand, bool doEmit)
{
	if (doEmit) {
		fromID = findWire(fromID);
		toID = findWire(toID);
		emit dealWithRatsnestSignal(fromID, fromConnectorID, toID, toConnectorID, connect, ratsnestCommand);
	}
}


void SketchWidget::dealWithRatsnestSlot(long fromID, const QString & fromConnectorID,
													long toID, const QString & toConnectorID,
													bool connect, class RatsnestCommand * ratsnestCommand)
{
	dealWithRatsnest(fromID, fromConnectorID, toID, toConnectorID, connect, ratsnestCommand, false);
}

void SketchWidget::setWireVisible(Wire * wire) {
	Q_UNUSED(wire);
}

void SketchWidget::forwardRoutingStatus(const RoutingStatus & routingStatus) {

	emit routingStatusSignal(this, routingStatus);
}

void SketchWidget::addFixedToTopLeftItem(QGraphicsItem *item) {
	item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	if(!scene()->items().contains(item)) {
		scene()->addItem(item);
	}
	m_fixedToTopLeftItems << item;
	ensureFixedToTopLeft(item);
}

void SketchWidget::addFixedToTopRightItem(QGraphicsItem *item) {
	item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	if(!scene()->items().contains(item)) {
		scene()->addItem(item);
	}
	m_fixedToTopRightItems << item;
	ensureFixedToTopRight(item);
}

void SketchWidget::addFixedToBottomLeftItem(QGraphicsItem *item) {
	item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	if(!scene()->items().contains(item)) {
		scene()->addItem(item);
	}
	m_fixedToBottomLeftItems << item;
	ensureFixedToBottomLeft(item);
}

void SketchWidget::addFixedToBottomRightItem(QGraphicsItem *item) {
	item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	if(!scene()->items().contains(item)) {
		scene()->addItem(item);
	}
	m_fixedToBottomRightItems << item;
	ensureFixedToBottomRight(item);
}

void SketchWidget::addFixedToCenterItem(QGraphicsItem *item) {
	item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	if(!scene()->items().contains(item)) {
		scene()->addItem(item);
	}
	m_fixedToCenterItems << item;
	ensureFixedToCenter(item);
}

void SketchWidget::ensureFixedToTopLeftItems() {
	if(isVisible()) {
		QList<QGraphicsItem*> toRemove;

		foreach(QGraphicsItem* item, m_fixedToTopLeftItems) {
			if(scene()->items().contains(item)) {
				ensureFixedToTopLeft(item);
			} else {
				toRemove << item;
			}
		}

		foreach(QGraphicsItem* item, toRemove) {
			m_fixedToTopLeftItems.removeAll(item);
		}
	}
}

void SketchWidget::ensureFixedToTopLeft(QGraphicsItem* item) {
	item->setPos(mapToScene(0,0));
}

void SketchWidget::ensureFixedToTopRightItems() {
	if(isVisible()) {
		QList<QGraphicsItem*> toRemove;

		foreach(QGraphicsItem* item, m_fixedToTopRightItems) {
			if(scene()->items().contains(item)) {
				ensureFixedToTopRight(item);
			} else {
				toRemove << item;
			}
		}

		foreach(QGraphicsItem* item, toRemove) {
			m_fixedToTopRightItems.removeAll(item);
		}
	}
}

void SketchWidget::ensureFixedToTopRight(QGraphicsItem* item) {
	int x = (int) (width()-fixedItemWidth(item));
	int y = 0;

	item->setPos(mapToScene(x,y));
}

void SketchWidget::ensureFixedToBottomLeftItems() {
	if(isVisible()) {
		QList<QGraphicsItem*> toRemove;

		foreach(QGraphicsItem* item, m_fixedToBottomLeftItems) {
			if(scene()->items().contains(item)) {
				ensureFixedToBottomLeft(item);
			} else {
				toRemove << item;
			}
		}

		foreach(QGraphicsItem* item, toRemove) {
			m_fixedToBottomLeftItems.removeAll(item);
		}
	}
}

void SketchWidget::ensureFixedToBottomLeft(QGraphicsItem* item) {
	int x = 0;
	int y = (int) (height()-fixedItemHeight(item));

	item->setPos(mapToScene(x,y));
}

void SketchWidget::ensureFixedToBottomRightItems() {
	if(isVisible()) {
		QList<QGraphicsItem*> toRemove;

		foreach(QGraphicsItem* item, m_fixedToBottomRightItems) {
			if(scene()->items().contains(item)) {
				ensureFixedToBottomRight(item);
			} else {
				toRemove << item;
			}
		}

		foreach(QGraphicsItem* item, toRemove) {
			m_fixedToBottomRightItems.removeAll(item);
		}
	}
}

void SketchWidget::ensureFixedToBottomRight(QGraphicsItem* item) {
	int x = (int) (width()-fixedItemWidth(item));
	int y = (int) (height()-fixedItemHeight(item));

	item->setPos(mapToScene(x,y));
}

void SketchWidget::ensureFixedToCenterItems() {
	if(isVisible()) {
		QList<QGraphicsItem*> toRemove;

		foreach(QGraphicsItem* item, m_fixedToCenterItems) {
			if(scene()->items().contains(item)) {
				ensureFixedToCenter(item);
			} else {
				toRemove << item;
			}
		}

		foreach(QGraphicsItem* item, toRemove) {
			m_fixedToCenterItems.removeAll(item);
		}
	}
}

void SketchWidget::ensureFixedToCenter(QGraphicsItem* item) {
	qreal x = (width()-fixedItemWidth(item))/2;
	qreal y = (height()-fixedItemHeight(item))/2;

	QPointF pos = mapToScene(x,y);

	if(pos.x() < scene()->width() && pos.y() < scene()->height()) {
		item->setPos(pos);
	}
}

qreal SketchWidget::fixedItemWidth(QGraphicsItem* item) {
	QGraphicsProxyWidget* gWidget = dynamic_cast<QGraphicsProxyWidget*>(item);
	if(gWidget) {
		return gWidget->widget()->width();
	} else {
		return item->boundingRect().width();
	}
}

qreal SketchWidget::fixedItemHeight(QGraphicsItem* item) {
	QGraphicsProxyWidget* gWidget = dynamic_cast<QGraphicsProxyWidget*>(item);
	if(gWidget) {
		return gWidget->widget()->height();
	} else {
		return item->boundingRect().height();
	}
}

void SketchWidget::removeIfFixedPos(QGraphicsItem *item) {
	m_fixedToBottomLeftItems.removeAll(item);
	m_fixedToBottomRightItems.removeAll(item);
	m_fixedToCenterItems.removeAll(item);
	m_fixedToTopLeftItems.removeAll(item);
	m_fixedToTopRightItems.removeAll(item);
}

void SketchWidget::clearFixedItems() {
	m_fixedToBottomLeftItems.clear();
	m_fixedToBottomRightItems.clear();
	m_fixedToCenterItems.clear();
	m_fixedToTopLeftItems.clear();
	m_fixedToTopRightItems.clear();
}

void SketchWidget::chainVisible(ConnectorItem * fromConnectorItem, ConnectorItem * toConnectorItem, bool connect)
{
	Q_UNUSED(fromConnectorItem);
	Q_UNUSED(toConnectorItem);
	Q_UNUSED(connect);

}

bool SketchWidget::matchesLayer(ModelPart * modelPart) {
	QDomDocument * domDocument = modelPart->modelPartShared()->domDocument();
	if (domDocument->isNull()) return false;

	QDomElement views = domDocument->documentElement().firstChildElement("views");
	if(views.isNull()) return false;

	QDomElement view = views.firstChildElement(ViewIdentifierClass::viewIdentifierXmlName(m_viewIdentifier));
	if (view.isNull()) return false;

	QDomElement layers = view.firstChildElement("layers");
	if (layers.isNull()) return false;

	QDomElement layer = layers.firstChildElement("layer");
	while (!layer.isNull()) {
		QString layerName = layer.attribute("layerId");
		ViewLayer::ViewLayerID viewLayerID = ViewLayer::viewLayerIDFromXmlString(layerName);
		foreach (ViewLayer* viewLayer, m_viewLayers) {
			if (viewLayer == NULL) continue;

			if (viewLayer->viewLayerID() == viewLayerID) return true;
		}

		layer = layer.nextSiblingElement("layer");
	}

	return false;
}

bool SketchWidget::doRatsnestOnCopy()
{
	return false;
}

const QString & SketchWidget::viewName() {
	return m_viewName;
}

void SketchWidget::setNoteText(long itemID, const QString & newText) {
	ItemBase * itemBase = findItem(itemID);
	if (itemBase == NULL) return;

	Note * note = dynamic_cast<Note *>(itemBase);
	if (note == NULL) return;

	note->setText(newText, false);
}

void SketchWidget::setInstanceTitle(long itemID, const QString & newText, bool isUndoable, bool doEmit) {
	// isUndoable is true when setInstanceTitle is called from the infoview 
	ItemBase * itemBase = findItem(itemID);
	if (itemBase == NULL) return;

	QString oldText = itemBase->instanceTitle();
	if (!isUndoable) {
		itemBase->setInstanceTitle(newText);
		if (doEmit && currentlyInfoviewed(itemBase))  {
			// TODO: just change the affected item in the info view
			InfoGraphicsView::viewItemInfo(itemBase);
		}

		if (doEmit) {
			emit setInstanceTitleSignal(itemID, newText, isUndoable, false);
		}
	}
	else {
		partLabelChangedAux(itemBase, oldText, newText);
	}
}

void SketchWidget::showPartLabel(long itemID, bool showIt) {

	ItemBase * itemBase = findItem(itemID);
	if (itemBase != NULL) {
		itemBase->showPartLabel(showIt, m_viewLayers.value(getLabelViewLayerID()));
	}
}

void SketchWidget::collectParts(QList<ItemBase *> & partList) {
	foreach (QGraphicsItem * item, scene()->items()) {
		PaletteItem * pitem = dynamic_cast<PaletteItem *>(item);
		if (pitem == NULL) continue;
		if (pitem->itemType() == ModelPart::Symbol) continue;

		partList.append(pitem);
	}
}

void SketchWidget::movePartLabel(long itemID, QPointF newPos, QPointF newOffset)
{
	ItemBase * item = findItem(itemID);
	if (item == NULL) return;

	item->movePartLabel(newPos, newOffset);
}

void SketchWidget::setCurrent(bool current) {
	m_current = current;
}

void SketchWidget::partLabelMoved(ItemBase * itemBase, QPointF oldPos, QPointF oldOffset, QPointF newPos, QPointF newOffset)
{
	MoveLabelCommand * command = new MoveLabelCommand(this, itemBase->id(), oldPos, oldOffset, newPos, newOffset, NULL);
	command->setText(tr("Move label '%1'").arg(itemBase->title()));
	m_undoStack->push(command);
}


void SketchWidget::rotateFlipPartLabel(ItemBase * itemBase, qreal degrees, Qt::Orientations flipDirection) {
	RotateFlipLabelCommand * command = new RotateFlipLabelCommand(this, itemBase->id(), degrees, flipDirection, NULL);
	command->setText(tr("%1 label '%2'").arg((degrees != 0) ? tr("Rotate") : tr("Flip")).arg(itemBase->title()));
	m_undoStack->push(command);
}


void SketchWidget::rotateFlipPartLabel(long itemID, qreal degrees, Qt::Orientations flipDirection) {
	ItemBase * itemBase = findItem(itemID);
	if (itemBase == NULL) return;

	itemBase->doRotateFlipPartLabel(degrees, flipDirection);
}


void SketchWidget::showPartLabels(bool show)
{
	QList<ItemBase *> itemBases;
	foreach (QGraphicsItem * item, scene()->selectedItems()) {
		ItemBase * itemBase = ItemBase::extractTopLevelItemBase(item);
		if (itemBase == NULL) continue;

		if (itemBase->hasPartLabel()) {
			itemBases.append(itemBase);
		}
	}

	if (itemBases.count() <= 0) return;

	ShowLabelCommand * showLabelCommand = new ShowLabelCommand(this, NULL);
	QString text;
	if (show) {
		text = tr("show part label(s)", "", itemBases.count());
	}
	else {
		text = tr("hide part label(s)", "", itemBases.count());
	}
	showLabelCommand->setText(text);

	foreach (ItemBase * itemBase, itemBases) {
		showLabelCommand->add(itemBase->id(), itemBase->isPartLabelVisible(), show);
	}

	m_undoStack->push(showLabelCommand);
}

void SketchWidget::noteSizeChanged(ItemBase * itemBase, const QSizeF & oldSize, const QSizeF & newSize)
{
	ResizeNoteCommand * command = new ResizeNoteCommand(this, itemBase->id(), oldSize, newSize, NULL);
	command->setText(tr("Resize Note"));
	m_undoStack->push(command);
	clearHoldingSelectItem();
}

void SketchWidget::resizeNote(long itemID, const QSizeF & size)
{
	Note * note = dynamic_cast<Note *>(findItem(itemID));
	if (note == NULL) return;

	note->setSize(size);
}

QString SketchWidget::renderToSVG(qreal printerScale, const QList<ViewLayer::ViewLayerID> & partLayers, const QList<ViewLayer::ViewLayerID> & wireLayers, 
								  bool blackOnly, QSizeF & imageSize, ItemBase * offsetPart, qreal dpi, bool selectedItems, bool flatten)
{
	qreal width =  scene()->width();
	qreal height =  scene()->height();

	QList<ItemBase *> itemBases;
	QRectF itemsBoundingRect;
	foreach (QGraphicsItem * item, (selectedItems) ? scene()->selectedItems() : scene()->items()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;
		if (!itemBase->isVisible()) continue;
		if (itemBase->hidden()) continue;

		switch (itemBase->itemType()) {
			case ModelPart::Wire:
				if (!wireLayers.contains(itemBase->viewLayerID())) {
					continue;
				}
				break;
			case ModelPart::Module:
			case ModelPart::Unknown:
				continue;
			default:
				if (!partLayers.contains(itemBase->viewLayerID())) {
					continue;
				}
		}

		itemBases.append(itemBase);

		// TODO: make sure this does the right thing with grouped items
		itemsBoundingRect |= item->sceneBoundingRect();
	}

	width = itemsBoundingRect.width();
	height = itemsBoundingRect.height();
	QPointF offset = itemsBoundingRect.topLeft();

	if (offsetPart) {
		QPointF p = offsetPart->scenePos();
		QPointF dp = offset - p;
		width += dp.x();
		height += dp.y();
		offset = p;
	}

	imageSize.setWidth(width);
	imageSize.setHeight(height);

	QString outputSVG = TextUtils::makeSVGHeader(printerScale, dpi, width, height);

	QHash<QString, SvgFileSplitter *> svgHash;

	// put them in z order
	qSort(itemBases.begin(), itemBases.end(), ItemBase::zLessThan);

	foreach (ItemBase * itemBase, itemBases) {
		if (itemBase->itemType() != ModelPart::Wire) {
			foreach (ViewLayer::ViewLayerID viewLayerID, partLayers) {
				if (itemBase->viewLayerID() != viewLayerID) continue;

				QString itemSvg = itemBase->retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
				if (itemSvg.isEmpty()) continue;

				if (flatten) {
					QDomDocument domDocument;
					QString errorStr;
					int errorLine;
					int errorColumn;
					bool result = domDocument.setContent(itemSvg, &errorStr, &errorLine, &errorColumn);
					if (!result) continue;

					QDomElement root = domDocument.documentElement();
					SvgFlattener flattener;
					flattener.flattenChildren(root);
					SvgFileSplitter::fixStyleAttributeRecurse(root);
					itemSvg = domDocument.toString();
				}

				if (!itemBase->transform().isIdentity()) {
					QTransform transform = itemBase->transform();
					itemSvg = QString("<g transform=\"matrix(%1,%2,%3,%4,%5,%6)\" >")
						.arg(transform.m11())
						.arg(transform.m12())
						.arg(transform.m21())
						.arg(transform.m22())
						.arg(0.0)  // transform.dx()			// don't understand why but SVG doesn't like this transform
						.arg(0.0)  // transform.dy()			// maybe it's redundant--already dealt with in the translate used next?
						.append(itemSvg)
						.append("</g>");
				}

				QPointF loc = itemBase->scenePos() - offset;
				loc.setX(loc.x() * dpi / printerScale);
				loc.setY(loc.y() * dpi / printerScale);
				if (loc.x() != 0 || loc.y() != 0) {
					itemSvg = QString("<g transform=\"translate(%1,%2)\" >")
						.arg(loc.x())
						.arg(loc.y())
						.append(itemSvg)
						.append("</g>");
				}

				outputSVG.append(itemSvg);

				/*
				// TODO:  deal with rotations and flips
				QString shifted = splitter->shift(loc.x(), loc.y(), xmlName);
				outputSVG.append(shifted);
				splitter->shift(-loc.x(), -loc.y(), xmlName);
				*/
			}
		}
		else {
			foreach (ViewLayer::ViewLayerID viewLayerID, wireLayers) {
				Wire * wire = dynamic_cast<Wire *>(itemBase);
				if (wire == NULL) continue;
				if (wire->viewLayerID() != viewLayerID) continue;

				QLineF line = wire->getPaintLine();
				QPointF p1 = wire->scenePos() + line.p1() - offset;
				QPointF p2 = wire->scenePos() + line.p2() - offset;
				p1.setX(p1.x() * dpi / printerScale);
				p1.setY(p1.y() * dpi / printerScale);
				p2.setX(p2.x() * dpi / printerScale);
				p2.setY(p2.y() * dpi / printerScale);
				// TODO: use original colors, not just black
				QString stroke = (blackOnly) ? "black" : wire->hexString();
				QString lineString = QString("<line style=\"stroke-linecap: round\" stroke=\"%6\" x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" stroke-width=\"%5\" />")
								.arg(p1.x())
								.arg(p1.y())
								.arg(p2.x())
								.arg(p2.y())
								.arg(wire->width() * dpi / printerScale)
								.arg(stroke);
				outputSVG.append(lineString);
			}
		}
	}

	outputSVG += "</svg>";


	foreach (SvgFileSplitter * splitter, svgHash.values()) {
		delete splitter;
	}

	return outputSVG;

}

void SketchWidget::addFixedToCenterItem2(SketchMainHelp * item) {
	m_fixedToCenterItem = item;
}

void SketchWidget::drawBackground( QPainter * painter, const QRectF & rect )
{
	InfoGraphicsView::drawBackground(painter, rect);

	// always draw the widget in the same place in the window
	// no matter how the view is zoomed or scrolled

	if (m_fixedToCenterItem != NULL) {
		if (m_fixedToCenterItem->getVisible()) {
			QWidget * widget = m_fixedToCenterItem->widget();
			if (widget != NULL) {
				QSizeF helpsize = m_fixedToCenterItem->size();
				QRectF vp = painter->viewport();

				/*
				// add in scrollbar widths so image doesn't jump when scroll bars appear or disappear?
				if (verticalScrollBar()->isVisible()) {
					vp.setWidth(vp.width() + verticalScrollBar()->width());
				}
				if (horizontalScrollBar()->isVisible()) {
					vp.setHeight(vp.height() + horizontalScrollBar()->height());
				}
				*/

				m_fixedToCenterItemOffset.setX((int) ((vp.width() - helpsize.width()) / 2.0));
				m_fixedToCenterItemOffset.setY((int) ((vp.height() - helpsize.height()) / 2.0));
				painter->save();
				painter->setWindow(painter->viewport());
				painter->setTransform(QTransform());
				painter->drawPixmap(m_fixedToCenterItemOffset, m_fixedToCenterItem->getPixmap());
				//painter->fillRect(m_fixedToCenterItemOffset.x(), m_fixedToCenterItemOffset.y(), helpsize.width(), helpsize.height(), QBrush(QColor(Qt::blue)));
				painter->restore();
			}
		}
	}
}

void SketchWidget::pushCommand(QUndoCommand * command) {
	if (m_undoStack) {
		m_undoStack->push(command);
	}
}

ItemBase * SketchWidget::findModulePart(ItemBase * toBase, QList<long> & indexes)
{
	for (int i = 1; i < indexes.count(); i++) {
		long originalModelIndex = indexes[i];

		bool gotOne = false;
		foreach (QGraphicsItem * child, toBase->childItems()) {
			ItemBase * itemBase = dynamic_cast<ItemBase *>(child);
			if (itemBase == NULL) continue;

			if (itemBase->modelPart()->originalModelIndex() == originalModelIndex) {
				toBase = itemBase;
				gotOne = true;
				break;
			}
		}
		if (!gotOne) {
			return NULL;
		}
	}

	return toBase;
}

bool SketchWidget::spaceBarIsPressed() {
	return m_spaceBarIsPressed;
}

void SketchWidget::restoreIndexes(long id, ModelPartTiny * modelPartTiny, bool doEmit)
{
	ItemBase * itemBase = findItem(id);
	if (itemBase == NULL) return;

	restoreIndexes(itemBase->modelPart(), modelPartTiny, doEmit);
	//m_sketchModel->walk(m_sketchModel->root(), 0);
}

void SketchWidget::restoreIndexes(ModelPart * modelPart, ModelPartTiny * modelPartTiny, bool doEmit)
{
	modelPart->setModelIndex(modelPartTiny->m_index);
	modelPart->setOriginalModelIndex(modelPartTiny->m_originalIndex);
	ItemBase * itemBase = modelPart->viewItem(scene());
	if (itemBase != NULL) {
		itemBase->resetID();
	}
	for (int i = 0; i < modelPart->children().count(); i++) {
		QObject * object = modelPart->children().at(i);
		restoreIndexes(dynamic_cast<ModelPart *>(object), modelPartTiny->m_children.at(i), false);
	}

	if (doEmit) {
		emit restoreIndexesSignal(modelPart, modelPartTiny, false);
	}
}

void SketchWidget::collectModuleExternalConnectors(ItemBase * itemBase, ItemBase * parent, ConnectorPairHash & connectorHash)
{
	foreach (QGraphicsItem * item, itemBase->childItems()) {
		ConnectorItem * fromConnectorItem = dynamic_cast<ConnectorItem *>(item);
		if (fromConnectorItem) {
			foreach (ConnectorItem* toConnectorItem, fromConnectorItem->connectedToItems()) {
				if (parent->isAncestorOf(toConnectorItem)) continue;

				connectorHash.insert(fromConnectorItem, toConnectorItem);
			}
		}
		else {
			ItemBase * childBase = dynamic_cast<ItemBase *>(item);
			if (childBase == NULL) continue;

			collectModuleExternalConnectors(childBase, parent, connectorHash);
		}
	}
}


ViewLayer::ViewLayerID SketchWidget::defaultConnectorLayer(ViewIdentifierClass::ViewIdentifier viewId) {
	switch(viewId) {
		case ViewIdentifierClass::IconView: return ViewLayer::Icon;
		case ViewIdentifierClass::BreadboardView: return ViewLayer::Breadboard;
		case ViewIdentifierClass::SchematicView: return ViewLayer::Schematic;
		case ViewIdentifierClass::PCBView: return ViewLayer::Copper0;
		default: return ViewLayer::UnknownLayer;
	}
}

bool SketchWidget::swappedGender(ConnectorItem * connectorItem, Connector * newConnector) 
{
	return (((connectorItem->connectorType() == Connector::Male) && (newConnector->connectorType() == Connector::Female))  ||
			((connectorItem->connectorType() == Connector::Female) && (newConnector->connectorType() == Connector::Male)));
}

void SketchWidget::setLastPaletteItemSelected(PaletteItem * paletteItem)
{
	m_lastPaletteItemSelected = paletteItem;
	//DebugDialog::debug(QString("m_lastPaletteItemSelected:%1 %2").arg(paletteItem == NULL ? "NULL" : paletteItem->instanceTitle()).arg(m_viewIdentifier));
}

void SketchWidget::setLastPaletteItemSelectedIf(ItemBase * itemBase)
{
	PaletteItem * paletteItem = dynamic_cast<PaletteItem *>(itemBase);
	if (paletteItem == NULL) return;

	setLastPaletteItemSelected(paletteItem);
}

void SketchWidget::setSpacing(const QString & spacing) {
	PaletteItem * item = getSelectedPart();
	if (item == NULL) return;

	MysteryPart * mysteryPart = dynamic_cast<MysteryPart *>(item);
	if (mysteryPart == NULL) return;

	SetPropCommand * cmd = new SetPropCommand(this, item->id(), "spacing", mysteryPart->spacing(), spacing, NULL);
	cmd->setText(tr("Change pin spacing from %1 to %2").arg(mysteryPart->spacing()).arg(spacing));
	m_undoStack->push(cmd);
}

void SketchWidget::setForm(const QString & form) {
	PaletteItem * item = getSelectedPart();
	if (item == NULL) return;

	PinHeader * pinHeader = dynamic_cast<PinHeader *>(item);
	if (pinHeader == NULL) return;

	SetPropCommand * cmd = new SetPropCommand(this, item->id(), "form", pinHeader->form(), form, NULL);
	cmd->setText(tr("Change form from %1 to %2").arg(pinHeader->form()).arg(form));
	m_undoStack->push(cmd);
}

void SketchWidget::setResistance(QString resistance, QString pinSpacing)
{
	PaletteItem * item = getSelectedPart();
	if (item == NULL) return;

	ModelPart * modelPart = item->modelPart();

	if (modelPart->moduleID().compare(ModuleIDNames::resistorModuleIDName) != 0) return;

	Resistor * resistor = dynamic_cast<Resistor *>(item);
	if (resistor == NULL) return;

	if (resistance.isEmpty()) {
		resistance = resistor->resistance();
	}

	if (pinSpacing.isEmpty()) {
		pinSpacing = resistor->pinSpacing();
	}

	SetResistanceCommand * cmd = new SetResistanceCommand(this, item->id(), resistor->resistance(), resistance, resistor->pinSpacing(), pinSpacing, NULL);
	cmd->setText(tr("Change Resistance from %1 to %2").arg(resistor->resistance()).arg(resistance));
	m_undoStack->push(cmd);
}

void SketchWidget::setResistance(long itemID, QString resistance, QString pinSpacing, bool doEmit) {
	ItemBase * item = findItem(itemID);
	if (item == NULL) return;

	Resistor * ritem = dynamic_cast<Resistor *>(item);
	if (ritem == NULL) return;

	ritem->setResistance(resistance, pinSpacing, false);
	viewItemInfo(item);

	if (doEmit) {
		emit setResistanceSignal(itemID, resistance, pinSpacing, false);
	}
}

void SketchWidget::setChipLabel(QString label)
{
	PaletteItem * item = getSelectedPart();
	if (item == NULL) return;

	SetPropCommand * cmd = NULL;

	MysteryPart * mysteryPart = dynamic_cast<MysteryPart *>(item);
	if (mysteryPart != NULL) {
		cmd = new SetPropCommand(this, item->id(), "chip label", mysteryPart->chipLabel(), label, NULL);
		cmd->setText(tr("Change chip label from %1 to %2").arg(mysteryPart->chipLabel()).arg(label));
	}

	LogoItem * logoItem = dynamic_cast<LogoItem *>(item);
	if (logoItem != NULL) {
		cmd = new SetPropCommand(this, item->id(), "logo", logoItem->logo(), label, NULL);
		cmd->setText(tr("Change logo from %1 to %2").arg(logoItem->logo()).arg(label));
	}

	if (cmd == NULL) return;

	m_undoStack->push(cmd);
}

void SketchWidget::setProp(long itemID, const QString & prop, const QString & value, bool doEmit) {
	ItemBase * item = findItem(itemID);
	if (item == NULL) return;

	item->setProp(prop, value);
	viewItemInfo(item);

	if (doEmit) {
		emit setPropSignal(itemID, prop, value, false);
	}
}

// called from ResizeBoardCommand
void SketchWidget::resizeBoard(long itemID, qreal mmW, qreal mmH) {
	ItemBase * item = findItem(itemID);
	if (item == NULL) return;

	switch (item->itemType()) {
		case ModelPart::ResizableBoard:
			dynamic_cast<ResizableBoard *>(item)->resizeMM(mmW, mmH, m_viewLayers);
			return;

		case ModelPart::Logo:
			dynamic_cast<LogoItem *>(item)->resizeMM(mmW, mmH, m_viewLayers);
			return;

		case ModelPart::Ruler:
			dynamic_cast<Ruler *>(item)->resizeMM(mmW, mmH, m_viewLayers);
			return;
	}

}

void SketchWidget::resizeBoard(qreal mmW, qreal mmH, bool doEmit)
{
	Q_UNUSED(doEmit);
	Q_UNUSED(mmH);

	PaletteItem * item = getSelectedPart();
	if (item == NULL) {
		return InfoGraphicsView::resizeBoard(mmW, mmH, doEmit);
	}

	switch (item->itemType()) {
		case ModelPart::Ruler:
			break;
		default:
			return;
	}

	QString orig = item->modelPart()->prop("width").toString();
	QString temp = orig;
	temp.chop(2);
	qreal origw = temp.toDouble();
	qreal origh = orig.endsWith("cm") ? 0 : 1;
	QUndoCommand * parentCommand = new QUndoCommand(tr("Resize ruler to %1%2").arg(mmW).arg((mmH == 0) ? "cm" : "in"));
	new ResizeBoardCommand(this, item->id(), origw, origh, mmW, mmH, parentCommand);
	m_undoStack->push(parentCommand);
}


bool SketchWidget::rotationAllowed(ItemBase * itemBase) 
{
	// TODO: allow breadboard and ardiuno to rotate even when connected to something

	switch(itemBase->itemType()) {
		//case ModelPart::Wire:
		case ModelPart::Note:
		case ModelPart::Unknown:
		case ModelPart::Jumper:
		case ModelPart::CopperFill:
			return false;

		case ModelPart::Board:
		case ModelPart::ResizableBoard:
		case ModelPart::Breadboard:
		case ModelPart::Logo:
		case ModelPart::Ruler:
			//if (itemBase->sticky() && itemBase->stickyList().count() > 0) {
				//return false;
			//}
			break;
		default:
			break;
	}

	return allowFemaleRotation(itemBase);
}

bool SketchWidget::allowFemaleRotation(ItemBase * itemBase) {
	Q_UNUSED(itemBase);
	return true;
}

void SketchWidget::addBendpoint(ItemBase * lastHoverEnterItem, ConnectorItem * lastHoverEnterConnectorItem, QPointF lastLocation) {
	if (lastHoverEnterConnectorItem) {
		Wire * wire = dynamic_cast<Wire *>(lastHoverEnterConnectorItem->attachedTo());
		if (wire != NULL) {
			wire_wireJoin(wire, lastHoverEnterConnectorItem);
		}
	}
	else if (lastHoverEnterItem) {
		Wire * wire = dynamic_cast<Wire *>(lastHoverEnterItem);
		if (wire != NULL) {
			wire_wireSplit(wire, lastLocation, wire->pos(), wire->line());
		}
	}
}

ConnectorItem * SketchWidget::lastHoverEnterConnectorItem() {
	return m_lastHoverEnterConnectorItem;
}

ItemBase * SketchWidget::lastHoverEnterItem() {
	return m_lastHoverEnterItem;
}

LayerHash & SketchWidget::viewLayers() {
	return m_viewLayers;
}

void SketchWidget::setClipEnds(ClipableWire * vw, bool) {
	vw->setClipEnds(false);
}

void SketchWidget::createTrace() {
}

void SketchWidget::selectAllWires(ViewGeometry::WireFlag flag) 
{
	QList<Wire *> wires;
	foreach (QGraphicsItem * item, scene()->items()) {
		Wire * wire = dynamic_cast<Wire *>(item);
		if (wire == NULL) continue;

		if (wire->hasFlag(flag)) {
			if (wire->parentItem() != NULL) {
				// skip module wires
				continue;
			}

			wires.append(wire);
		}
	}

	if (wires.count() <= 0) {
		// TODO: tell user?
	}

	QString wireName;
	if (flag == ViewGeometry::JumperFlag) {
		wireName = QObject::tr("Jumper wires");
	}
	else if (flag == ViewGeometry::TraceFlag) {
		wireName = QObject::tr("Trace wires");
	}
	else if (flag == ViewGeometry::RatsnestFlag) {
		wireName = QObject::tr("Ratsnest wires");
	}
	QUndoCommand * parentCommand = new QUndoCommand(QObject::tr("Select all %1").arg(wireName));

	stackSelectionState(false, parentCommand);
	SelectItemCommand * selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
	foreach (Wire * wire, wires) {
		selectItemCommand->addRedo(wire->id());
	}

	scene()->clearSelection();
	m_undoStack->push(parentCommand);
}

void SketchWidget::tidyWires() {
}   

void SketchWidget::updateConnectors() {
	// update issue with 4.5.0?
	foreach (QGraphicsItem* item, scene()->items()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(item);
		if (connectorItem == NULL) continue;

		connectorItem->restoreColor(false, -1);
	}
}

const QString & SketchWidget::getShortName() {
	return m_shortName;
}

void SketchWidget::getBendpointWidths(Wire * wire, qreal width, qreal & bendpointWidth, qreal & bendpoint2Width) {
	int dwidth = (wire->getTrace() && (width > 1)) ? 4 : 3;
	bendpoint2Width = bendpointWidth = (width - dwidth);
}

const QColor & SketchWidget::standardBackground() {
	return m_standardBackgroundColor;
}

void SketchWidget::initBackgroundColor() {
	m_bgcolors[m_viewIdentifier] = m_standardBackgroundColor;
	setBackground(m_standardBackgroundColor);

	QSettings settings;
	QString colorName = settings.value(QString("%1BackgroundColor").arg(getShortName())).toString();
	if (colorName.isEmpty()) return;

	QColor color;
	color.setNamedColor(colorName);
	setBackground(color);
}

bool SketchWidget::includeSymbols() {
	return false;
}

void SketchWidget::disconnectAll() {

	// TODO: collect all wires from separate views

	QSet<ItemBase *> itemBases;
	foreach (QGraphicsItem * item, scene()->selectedItems()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;

		itemBase = itemBase->layerKinChief();
		if (itemBase == NULL) continue;

		itemBases.insert(itemBase);
	}

	QList<ConnectorItem *> connectorItems;
	foreach (ItemBase * itemBase, itemBases) {
		ConnectorItem * fromConnectorItem = itemBase->rightClickedConnector();
		if (fromConnectorItem == NULL) continue;

		bool gotOne = false;
		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
			if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
				gotOne = true;
			}
		}
		if (gotOne) {
			connectorItems.append(fromConnectorItem);
		}
	}

	if (connectorItems.count() <= 0) return;

	QString string;
	if (itemBases.count() == 1) {
		ItemBase * firstItem = *(itemBases.begin());
		string = tr("Disconnect all wires from %1").arg(firstItem->title());
	}
	else {
		string = tr("Disconnect all wires from %1 items").arg(QString::number(itemBases.count()));
	}

	QUndoCommand * parentCommand = new QUndoCommand(string);

	stackSelectionState(false, parentCommand);

	QHash<ItemBase *, SketchWidget *> itemsToDelete;
	disconnectAllSlot(connectorItems, itemsToDelete, parentCommand);
	emit disconnectAllSignal(connectorItems, itemsToDelete, parentCommand);

	emit ratsnestChangeSignal(this, parentCommand);
	new CleanUpWiresCommand(this, false, parentCommand);
	foreach (ItemBase * item, itemsToDelete.keys()) {
		itemsToDelete.value(item)->makeDeleteItemCommand(item, BaseCommand::SingleView, parentCommand);
	}
	m_undoStack->push(parentCommand);
}

void SketchWidget::disconnectAllSlot(QList<ConnectorItem *> connectorItems, QHash<ItemBase *, SketchWidget *> & itemsToDelete, QUndoCommand * parentCommand)
{
	QList<ConnectorItem *> myConnectorItems;
	foreach (ConnectorItem * ci, connectorItems) {
		ItemBase * itemBase = findItem(ci->attachedToID());
		if (itemBase == NULL) continue;

		ConnectorItem * fromConnectorItem = findConnectorItem(itemBase, ci->connectorSharedID(), true);
		if (fromConnectorItem == NULL) continue;

		myConnectorItems.append(fromConnectorItem);
	}

	QSet<ItemBase *> deleteItems;
	foreach (ConnectorItem * fromConnectorItem, myConnectorItems) {
		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
			if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
				Wire * wire = qobject_cast<Wire *>(toConnectorItem->attachedTo());
				if (!wire->getRatsnest()) {
					QList<Wire *> chained;
					QList<ConnectorItem *> ends;
					QList<ConnectorItem *> uniqueEnds;
					wire->collectChained(chained, ends, uniqueEnds);
					foreach (Wire * w, chained) {
						itemsToDelete.insert(w, this);
						deleteItems.insert(w);
					}
				}
			}
			else if (toConnectorItem->connectorType() == Connector::Female) {
				if (ignoreFemale()) {
					//fromConnectorItem->tempRemove(toConnectorItem, false);
					//toConnectorItem->tempRemove(fromConnectorItem, false);
					//extendChangeConnectionCommand(fromConnectorItem, toConnectorItem, false, true, parentCommand);
				}
				else {
					ItemBase * detachee = fromConnectorItem->attachedTo();
					QPointF newPos = calcNewLoc(detachee, toConnectorItem->attachedTo());
					// delete connections
					// add wires and connections for undisconnected connectors

					detachee->saveGeometry();
					ViewGeometry vg = detachee->getViewGeometry();
					vg.setLoc(newPos);
					new MoveItemCommand(this, detachee->id(), detachee->getViewGeometry(), vg, parentCommand);
					QSet<ItemBase *> tempItems;
					ConnectorPairHash connectorHash;
					disconnectFromFemale(detachee, tempItems, connectorHash, true, parentCommand);
					foreach (ConnectorItem * fConnectorItem, connectorHash.uniqueKeys()) {
						if (myConnectorItems.contains(fConnectorItem)) {
							// don't need to reconnect
							continue;
						}

						foreach (ConnectorItem * tConnectorItem, connectorHash.values(fConnectorItem)) {
							createWire(fConnectorItem, tConnectorItem, ViewGeometry::NoFlag, false, true, BaseCommand::CrossView, parentCommand);
						}
					}
				}
			}
		}
	}

	deleteMiddle(deleteItems, parentCommand);
}

bool SketchWidget::canDisconnectAll() {
	return true;
}

bool SketchWidget::ignoreFemale() {
	return true;
}

QPointF SketchWidget::calcNewLoc(ItemBase * moveBase, ItemBase * detachFrom)
{
	QRectF dr = detachFrom->boundingRect();
	dr.moveTopLeft(detachFrom->pos());

	QPointF pos = moveBase->pos();
	QRectF r = moveBase->boundingRect();
	pos.setX(pos.x() + (r.width() / 2.0));
	pos.setY(pos.y() + (r.height() / 2.0));
	qreal d[4];
	d[0] = qAbs(pos.y() - dr.top());
	d[1] = qAbs(pos.y() - dr.bottom());
	d[2] = qAbs(pos.x() - dr.left());
	d[3] = qAbs(pos.x() - dr.right());
	int ix = 0;
	for (int i = 1; i < 4; i++) {
		if (d[i] < d[ix]) {
			ix = i;
		}
	}
	QPointF newPos = moveBase->pos();
	switch (ix) {
		case 0:
			newPos.setY(dr.top() - r.height());
			break;
		case 1:
			newPos.setY(dr.bottom());
			break;
		case 2:
			newPos.setX(dr.left() - r.width());
			break;
		case 3:
			newPos.setX(dr.right());
			break;
	}
	return newPos;
}

long SketchWidget::findWire(long itemID) 
{
	ItemBase * item = findItem(itemID);
	if (item == NULL) return itemID;

	if (item->itemType() != ModelPart::Wire) return itemID;

	QList<Wire *> chained;
	QList<ConnectorItem *> ends;
	QList<ConnectorItem *> uniqueEnds;
	qobject_cast<Wire *>(item)->collectChained(chained, ends, uniqueEnds);
	if (chained.length() <= 1) return itemID;

	foreach (Wire * w, chained) {
		if (w->id() < itemID) {
			itemID = w->id();
		}
	}

	return itemID;
}

void SketchWidget::resizeBoard() {
	QSizeF oldSize;
	QPointF oldPos;
	m_resizingBoard->getParams(oldPos, oldSize);
	QSizeF newSize;
	QPointF newPos;
	m_resizingBoard->saveParams();
	m_resizingBoard->getParams(newPos, newSize);
	QUndoCommand * parentCommand = new QUndoCommand(tr("Resize board to %1 %2").arg(newSize.width()).arg(newSize.height()));
	new ResizeBoardCommand(this, m_resizingBoard->id(), oldSize.width(), oldSize.height(), newSize.width(), newSize.height(), parentCommand);
	if (oldPos != newPos) {
		m_resizingBoard->saveGeometry();
		ViewGeometry vg1 = m_resizingBoard->getViewGeometry();
		ViewGeometry vg2 = vg1;
		vg1.setLoc(oldPos);
		vg2.setLoc(newPos);
		new MoveItemCommand(this, m_resizingBoard->id(), vg1, vg2, parentCommand);
	}
	new CheckStickyCommand(this, BaseCommand::SingleView, m_resizingBoard->id(), true, parentCommand);
	m_undoStack->waitPush(parentCommand, 10);
	m_resizingBoard = NULL;
}

void SketchWidget::resizeJumperItem() {
	QPointF oldC0, oldC1;
	QPointF oldPos;
	m_resizingJumperItem->getParams(oldPos, oldC0, oldC1);
	QPointF newC0, newC1;
	QPointF newPos;
	m_resizingJumperItem->saveParams();
	m_resizingJumperItem->getParams(newPos, newC0, newC1);
	QUndoCommand * cmd = new ResizeJumperItemCommand(this, m_resizingJumperItem->id(), oldPos, oldC0, oldC1, newPos, newC0, newC1, NULL);
	cmd->setText("Resize Jumper");
	m_undoStack->waitPush(cmd, 10);
	m_resizingJumperItem = NULL;
}

void SketchWidget::resizeJumperItem(long itemID, QPointF pos, QPointF c0, QPointF c1) {
	ItemBase * item = findItem(itemID);
	if (item == NULL) return;

	if (item->itemType() != ModelPart::Jumper) return;

	dynamic_cast<JumperItem *>(item)->resize(pos, c0, c1);
}


int SketchWidget::selectAllItemType(ModelPart::ItemType itemType) 
{
	QSet<ItemBase *> itemBases;
	foreach (QGraphicsItem * item, scene()->items()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;
		if (itemBase->itemType() != itemType) continue;

		itemBases.insert(itemBase->layerKinChief());
	}

	return selectAllItems(itemBases, QObject::tr("Select all jumpers"));

}

int SketchWidget::selectAllObsolete() 
{
	QSet<ItemBase *> itemBases;
	foreach (QGraphicsItem * item, scene()->items()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;
		if (!itemBase->isObsolete()) continue;

		itemBases.insert(itemBase->layerKinChief());
	}

	return selectAllItems(itemBases, QObject::tr("Select outdated parts"));
}


int SketchWidget::selectAllItems(QSet<ItemBase *> & itemBases, const QString & msg) 
{
	if (itemBases.count() <= 0) {
		// TODO: tell user?
		return 0;
	}

	QUndoCommand * parentCommand = new QUndoCommand(msg);

	stackSelectionState(false, parentCommand);
	SelectItemCommand * selectItemCommand = new SelectItemCommand(this, SelectItemCommand::NormalSelect, parentCommand);
	foreach (ItemBase * itemBase, itemBases) {
		selectItemCommand->addRedo(itemBase->id());
	}

	scene()->clearSelection();
	m_undoStack->push(parentCommand);

	return itemBases.count();
}

AddItemCommand * SketchWidget::newAddItemCommand(BaseCommand::CrossViewType crossViewType, QString moduleID, ViewGeometry & viewGeometry, qint64 id, bool updateInfoView, long modelIndex, long originalModelIndex, QUndoCommand *parent)
{
	return new AddItemCommand(this, crossViewType, moduleID, viewGeometry, id, updateInfoView, modelIndex, originalModelIndex, parent);
}

bool SketchWidget::partLabelsVisible() {
	foreach (QGraphicsItem * item, scene()->items()) {
		ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
		if (itemBase == NULL) continue;

		if (itemBase->isPartLabelVisible()) return true;

	}

	return false;
}

void SketchWidget::showLabelFirstTime(long itemID, bool show, bool doEmit) {
	if (doEmit) {
		emit showLabelFirstTimeSignal(itemID, show, false);
	}
}

void SketchWidget::restorePartLabel(long itemID, QDomElement & element) {
	ItemBase * itemBase = findItem(itemID);
	if (itemBase == NULL) return;

	itemBase->restorePartLabel(element, getLabelViewLayerID());
}

void SketchWidget::loadLogoImage(long itemID, const QString & oldSvg, const QSizeF oldAspectRatio, const QString & oldFilename, const QString & newFilename, bool addName) {
	QUndoCommand * cmd = new LoadLogoImageCommand(this, itemID, oldSvg, oldAspectRatio, oldFilename, newFilename, addName, NULL);
	cmd->setText(tr("Change image from %1 to %2").arg(oldFilename).arg(newFilename));
	m_undoStack->push(cmd);
}

void SketchWidget::loadLogoImage(long itemID, const QString & oldSvg, const QSizeF oldAspectRatio, const QString & oldFilename) {
	ItemBase * itemBase = findItem(itemID);
	if (itemBase == NULL) return;

	LogoItem * logoItem = qobject_cast<LogoItem *>(itemBase);
	if (logoItem == NULL) return;

	logoItem->reloadImage(oldSvg, oldAspectRatio, oldFilename, false);
}

void SketchWidget::loadLogoImage(long itemID, const QString & newFilename, bool addName) {
	ItemBase * itemBase = findItem(itemID);
	if (itemBase == NULL) return;

	LogoItem * logoItem = qobject_cast<LogoItem *>(itemBase);
	if (logoItem == NULL) return;

	logoItem->loadImage(newFilename, addName);
}

void SketchWidget::paintEvent ( QPaintEvent * event ) {
    //DebugDialog::debug("sketch widget paint event");
    if (scene()) {
        ((FGraphicsScene *) scene())->setDisplayHandles(true);
    }
    QGraphicsView::paintEvent(event);
}

void SketchWidget::setNoteFocus(QGraphicsItem * item, bool inFocus) {
	if (inFocus) {
		m_inFocus.append(item);
	}
	else {
		m_inFocus.removeOne(item);
	}
}

double SketchWidget::gridSizeInches() {
	return 0.5;
}

void SketchWidget::alignToGrid(bool align) {
	m_alignToGrid = align;
}

bool SketchWidget::canAlignToTopLeft(ItemBase *) 
{
	return false;
}
