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


#include "itembase.h"
#include "debugdialog.h"
#include "modelpart.h"
#include "connectoritem.h"
#include "infographicsview.h"
#include "busconnectoritem.h"
#include "connector.h"
#include "bus.h"

#include <QScrollBar>
#include <QTimer>

long ItemBase::nextID = 0;
QHash <ItemBase::ViewIdentifier, StringTriple * > ItemBase::names;
QString ItemBase::rulerModuleIDName = "RulerModuleID";
QString ItemBase::breadboardModuleIDName = "BreadboardModuleID";

// for autoscroll  TODO:  clean up these statics and put them somewhere clean
static QMap<QGraphicsItem *, QPointF> movingItemsInitialPositions;
static QTimer autoScrollTimer;
static volatile int autoScrollX = 0;
static volatile int autoScrollY = 0;
static QGraphicsView * autoScrollView = NULL;
bool _qt_movableAncestorIsSelected(const QGraphicsItem *item);
bool _qt_ancestorIgnoresTransformations(const QGraphicsItem *item);

ItemBase::ItemBase( ModelPart* modelPart, ItemBase::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, bool topLevel, QMenu * itemMenu )
	: GraphicsSvgLineItem()
{
	m_itemMenu = itemMenu;
	m_topLevel = topLevel;
	m_connectorHoverCount = 0;
	m_viewIdentifier = viewIdentifier;
	m_modelPart = modelPart;
	m_modelPart->addViewItem(this);
	setTooltip();
	m_id = id;
	m_hidden = false;
	m_sticky = false;
	m_canFlipHorizontal = m_canFlipVertical = false;

	setCursor(Qt::ArrowCursor);

   	m_viewGeometry.set(viewGeometry);
	setAcceptHoverEvents ( true );

}

ItemBase::~ItemBase() {
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (connectorItem == NULL) continue;

		foreach (ConnectorItem * toConnectorItem, connectorItem->connectedToItems()) {
			toConnectorItem->tempRemove(connectorItem);
		}
	}

	foreach (ItemBase * itemBase, m_stickyList.keys()) {
		itemBase->addSticky(this, false);
	}
	if (m_modelPart != NULL) {
		m_modelPart->removeViewItem(this);
	}
	foreach (BusConnectorItem * busConnectorItem, m_busConnectorItems) {
		if (busConnectorItem->attachedTo() == this) {
			delete busConnectorItem;
		}
	}
	m_busConnectorItems.clear();
}

void ItemBase::setTooltip() {
	if(m_modelPart->partInstanceStuff()) {
		QString title = m_modelPart->partInstanceStuff()->title();
		if(!title.isNull() && !title.isEmpty()) {
			setInstanceTitleTooltip(m_modelPart->partInstanceStuff()->title());
		} else {
			setDefaultTooltip();
		}
	} else {
		setDefaultTooltip();
	}
}

void ItemBase::removeTooltip() {
	this->setToolTip(___emptyString___);
}

bool ItemBase::zLessThan(ItemBase * & p1, ItemBase * & p2)
{
	return p1->z() < p2->z();
}

qint64 ItemBase::getNextID() {
	qint64 temp = nextID;
	nextID += 10;								// make sure we leave room for layerkin inbetween
	return temp;
}

qint64 ItemBase::getNextID(qint64 index) {
	qint64 temp = index * 10;						// make sure we leave room for layerkin inbetween
	if (nextID <= temp) {
		nextID = temp + 10;
	}
	return temp;
}


QSize ItemBase::size() {
	return m_size;
}

void ItemBase::setSize(QSize size) {
	m_size = size;
}

qint64 ItemBase::id() {
 	return m_id;
}

qreal ItemBase::z() {
	return getViewGeometry().z();
}

ModelPart * ItemBase::modelPart() {
	return m_modelPart;
}

void ItemBase::setModelPart(ModelPart * modelPart) {
	m_modelPart = modelPart;
}

ModelPartStuff * ItemBase::modelPartStuff() {
	if (m_modelPart == NULL) return NULL;

	return m_modelPart->modelPartStuff();
}

QString & ItemBase::viewIdentifierName(ItemBase::ViewIdentifier viewIdentifier) {
	Q_ASSERT(viewIdentifier >= 0);
	Q_ASSERT(viewIdentifier < ItemBase::ViewCount);
	return names[viewIdentifier]->second;
}

QString & ItemBase::viewIdentifierXmlName(ItemBase::ViewIdentifier viewIdentifier) {
	Q_ASSERT(viewIdentifier >= 0);
	Q_ASSERT(viewIdentifier < ItemBase::ViewCount);
	return names[viewIdentifier]->first;
}

QString & ItemBase::viewIdentifierNaturalName(ItemBase::ViewIdentifier viewIdentifier) {
	Q_ASSERT(viewIdentifier >= 0);
	Q_ASSERT(viewIdentifier < ItemBase::ViewCount);
	return names[viewIdentifier]->third;
}

void ItemBase::initNames() {
	if (names.count() == 0) {
		names.insert(ItemBase::IconView, new StringTriple("iconView", QObject::tr("icon view"), "icon"));
		names.insert(ItemBase::BreadboardView, new StringTriple("breadboardView", QObject::tr("breadboard view"), "breadboard"));
		names.insert(ItemBase::SchematicView, new StringTriple("schematicView", QObject::tr("schematic view"), "schematic"));
		names.insert(ItemBase::PCBView, new StringTriple("pcbView", QObject::tr("pcb view"), "pcb"));
	}
}

void ItemBase::saveInstance(QXmlStreamWriter & streamWriter) {
	streamWriter.writeStartElement(names[m_viewIdentifier]->first);
	streamWriter.writeAttribute("layer", ViewLayer::viewLayerXmlNameFromID(m_viewLayerID));
	this->saveGeometry();
	writeGeometry(streamWriter);

	bool saveConnectorItems = false;
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (connectorItem == NULL) continue;

		if (connectorItem->connectionsCount() > 0) {
			saveConnectorItems = true;
			break;
		}
	}

	if (saveConnectorItems) {
		streamWriter.writeStartElement("connectors");
		foreach (QGraphicsItem * childItem, childItems()) {
			ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
			if (connectorItem == NULL) continue;

			connectorItem->saveInstance(streamWriter);
		}
		streamWriter.writeEndElement();
	}

	bool saveBuses = false;
	foreach (BusConnectorItem * busConnectorItem, m_busConnectorItems) {
		if (busConnectorItem->connectionsCount() > 0) {
			saveBuses = true;
			break;
		}
	}

	if (saveBuses) {
		streamWriter.writeStartElement("buses");
		foreach (BusConnectorItem * busConnectorItem, m_busConnectorItems) {
			if (busConnectorItem->connectionsCount() <= 0) continue;

			busConnectorItem->saveInstance(streamWriter);
		}
		streamWriter.writeEndElement();
	}

	streamWriter.writeEndElement();
}

void ItemBase::writeGeometry(QXmlStreamWriter & streamWriter) {
	streamWriter.writeStartElement("geometry");
	streamWriter.writeAttribute("z", QString::number(z()));
	this->saveInstanceLocation(streamWriter);
	// do not write attributes here
	streamWriter.writeEndElement();
}

ViewGeometry & ItemBase::getViewGeometry() {
	return m_viewGeometry;
}

ItemBase::ViewIdentifier ItemBase::viewIdentifier() {
	return m_viewIdentifier;
}

QString & ItemBase::viewIdentifierName() {
	return ItemBase::viewIdentifierName(m_viewIdentifier);
}

ViewLayer::ViewLayerID ItemBase::viewLayerID() {
	return m_viewLayerID;
}

void ItemBase::setViewLayerID(const QString & layerName, const LayerHash & viewLayers) {
	//DebugDialog::debug(QObject::tr("using z %1").arg(layerName));
	setViewLayerID(ViewLayer::viewLayerIDFromXmlString(layerName), viewLayers);
}

void ItemBase::setViewLayerID(ViewLayer::ViewLayerID viewLayerID, const LayerHash & viewLayers) {
	m_viewLayerID = viewLayerID;
	if (this->z() < 0) {
   		ViewLayer * viewLayer = viewLayers.value(m_viewLayerID);
   		if (viewLayer != NULL) {
   			m_viewGeometry.setZ(viewLayer->nextZ());
  		}

  	}

    //DebugDialog::debug(QObject::tr("using z: %1 z:%2 lid:%3").arg(modelPart()->modelPartStuff()->title()).arg(m_viewGeometry.z()).arg(m_viewLayerID) );
}

void ItemBase::removeLayerKin() {
}

void ItemBase::hoverEnterConnectorItem(QGraphicsSceneHoverEvent * , ConnectorItem * ) {
	m_connectorHoverCount++;
	this->update();
}

void ItemBase::hoverLeaveConnectorItem(QGraphicsSceneHoverEvent * , ConnectorItem * ) {
	m_connectorHoverCount--;
	this->update();
}

void ItemBase::connectorHover(ConnectorItem *, ItemBase *, bool hovering) {
	if (hovering) {
		m_connectorHoverCount++;
		this->update();
	}
	else {
		m_connectorHoverCount--;
		this->update();
	}
}

void ItemBase::mousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *) {
}

bool ItemBase::acceptsMousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent *) {
	return true;
}

void ItemBase::connectionChange(ConnectorItem *) {
}

void ItemBase::connectedMoved(ConnectorItem * /* from */, ConnectorItem * /* to */) {
}

ItemBase * ItemBase::extractTopLevelItemBase(QGraphicsItem * item) {
	ItemBase * itemBase = dynamic_cast<ItemBase *>(item);
	if (itemBase == NULL) return NULL;

	if (itemBase->topLevel()) return itemBase;

	return NULL;
}

bool ItemBase::topLevel() {
	return m_topLevel;
}

ItemBase * ItemBase::extractItemBase(QGraphicsItem * item) {
	return dynamic_cast<ItemBase *>(item);
}

void ItemBase::setHidden(bool hide) {

	m_hidden = hide;
	setAcceptedMouseButtons(hide ? Qt::NoButton : Qt::LeftButton | Qt::MidButton | Qt::RightButton | Qt::XButton1 | Qt::XButton2);
	setAcceptHoverEvents(!hide);
	update();
	for (int i = 0; i < childItems().count(); i++) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItems()[i]);
		if (connectorItem == NULL) continue;

		connectorItem->setHidden(hide);
	}
}

bool ItemBase::hidden() {
	return m_hidden;
}

void ItemBase::collectConnectors(QMultiHash<ConnectorItem *, ConnectorItem *> & connectorHash, QGraphicsScene * scene) {
	Q_UNUSED(scene);

	ModelPart * modelPart = this->modelPart();
	if (modelPart == NULL) return;

	// collect all the connectorItem pairs

	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * fromConnectorItem = dynamic_cast<ConnectorItem *>( childItem );
		if (fromConnectorItem == NULL) continue;

		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
			connectorHash.insert(fromConnectorItem, toConnectorItem);
		}

	}

	// now add any bus items belonging to this part

	foreach (BusConnectorItem * busConnectorItem, m_busConnectorItems.values()) {
		foreach (ConnectorItem * toConnectorItem, busConnectorItem->connectedToItems()) {
			connectorHash.insert(busConnectorItem, toConnectorItem);
		}
	}
}

ConnectorItem * ItemBase::findConnectorItemNamed(const QString & connectorID)  {
	for (int i = 0; i < childItems().count(); i++) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItems()[i]);
		if (connectorItem == NULL) continue;

		if (connectorID.compare(connectorItem->connectorStuffID()) == 0) {
			return connectorItem;
		}
	}

	return NULL;
}

void ItemBase::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) {
	InfoGraphicsView * infoGraphicsView = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
	if (infoGraphicsView != NULL) {
		infoGraphicsView->hoverEnterItem(event, this);
	}
}

void ItemBase::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) {
	InfoGraphicsView * infoGraphicsView = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
	if (infoGraphicsView != NULL) {
		infoGraphicsView->hoverLeaveItem(event, this);
	}
}

void ItemBase::hoverMoveEvent ( QGraphicsSceneHoverEvent *  ) {
}

ConnectorItem * ItemBase::findConnectorUnder(ConnectorItem * connectorItemOver, ConnectorItem * lastUnderConnector, bool useTerminalPoint)
{

	QList<QGraphicsItem *> items = useTerminalPoint
		? this->scene()->items(connectorItemOver->sceneAdjustedTerminalPoint())
		: this->scene()->items(mapToScene(connectorItemOver->rect()));
	bool gotOne = false;
	// for the moment, take the topmost ConnectorItem that doesn't belong to me
	for (int i = 0; i < items.count(); i++) {
		ConnectorItem * connectorItemUnder = dynamic_cast<ConnectorItem *>(items[i]);
		if (connectorItemUnder == NULL) continue;
		if (connectorItemUnder->connector() == NULL) continue;			// for now; this is probably a busConnectorItem
		if (childItems().contains(connectorItemUnder)) continue;
		if (!connectorItemOver->connector()->connectionIsAllowed(connectorItemUnder->connector())) {
			break;
		}
		if (connectorItemUnder->connectedToItems().contains(connectorItemOver)) {
			break;		// already connected
		}

		//if (connector->attachedToItemType() == ModelPart::Wire) {
		// for the moment, wires can't connect to wires
		//continue;
		//}

		if (connectorItemUnder == lastUnderConnector) {
			// no change
			gotOne = true;
			break;
		}

		if (lastUnderConnector != NULL) {
			lastUnderConnector->connectorHover(this, false);
		}

		gotOne = true;
		lastUnderConnector = connectorItemUnder;
		lastUnderConnector->connectorHover(this, true);
		break;

		//DebugDialog::debug("rolled over a connector");
	}

	if (!gotOne) {
		if (lastUnderConnector != NULL) {
			lastUnderConnector->connectorHover(this, false);
			lastUnderConnector = NULL;
		}
	}

	return lastUnderConnector;
}


void ItemBase::sendConnectionChangedSignal(ConnectorItem * from, ConnectorItem * to, bool connect) {
	Q_UNUSED(from);
	Q_UNUSED(connect);
	Q_UNUSED(to);
}

void ItemBase::	updateConnections() {
}

void ItemBase::	updateConnections(ConnectorItem *) {
}

const QString & ItemBase::title() {
	if (m_modelPart == NULL) return ___emptyString___;

	return m_modelPart->title();
}

bool ItemBase::getVirtual() {
	return m_viewGeometry.getVirtual();
}

const QHash<QString, Bus *> & ItemBase::buses() {
	if (m_modelPart != NULL) return m_modelPart->buses();

	return Bus::___emptyBusList___;
}

void ItemBase::addBusConnectorItem(Bus * bus, BusConnectorItem * item) {
	m_busConnectorItems.insert(bus, item);
}

void ItemBase::removeBusConnectorItem(Bus * bus) {
	m_busConnectorItems.remove(bus);
}

BusConnectorItem * ItemBase::busConnectorItem(const QString & busID) {
	Bus * bus = m_modelPart->bus(busID);
	if (bus == NULL) return NULL;

	return m_busConnectorItems.value(bus);
}

ConnectorItem * ItemBase::busConnectorItemCast(const QString & busID) {

	return busConnectorItem(busID);
}


BusConnectorItem * ItemBase::busConnectorItem(Bus * bus) {
	if (bus == NULL) return NULL;

	return m_busConnectorItems.value(bus);
}

int ItemBase::itemType() {
	if (m_modelPart == NULL) return ModelPart::Unknown;

	return m_modelPart->itemType();
}

void ItemBase::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	if (m_connectorHoverCount > 0) {
		painter->save();
		painter->setOpacity(0.25);
		painter->fillPath(this->hoverShape(), QBrush(ConnectorItem::hoverPen.color()));
		painter->restore();
	}

	GraphicsSvgLineItem::paint(painter, option, widget);
}

void ItemBase::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	//scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	GraphicsSvgLineItem::mousePressEvent(event);
}

void ItemBase::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if (scene() && !event->buttons()) {
		autoScrollTimer.stop();
        movingItemsInitialPositions.clear();
		autoScrollView = NULL;
		disconnect(&autoScrollTimer, SIGNAL(timeout()), this, SLOT(autoScrollTimeout()));
	}

	// calling parent class so that multiple selection will work
	// haven't yet discovered any nasty side-effect
	GraphicsSvgLineItem::mouseReleaseEvent(event);
	//scene()->setItemIndexMethod(QGraphicsScene::BspTreeIndex);
	setCacheMode(QGraphicsItem::NoCache);

}

void ItemBase::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	//GraphicsSvgLineItem::mouseMoveEvent(event);
	//return;

	//	code mostly borrowed from QGraphicsItem::mouseMoveEvent,
//	but (soon) modified to add autoscroll

	bool doMove = (event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable);
	if (!doMove) {
		event->ignore();
		return;
	}

	    // Find the active view.
    QGraphicsView *view = 0;
	if (event->widget()) {
        view = qobject_cast<QGraphicsView *>(event->widget()->parentWidget());
	}

	autoScrollView = view;

	if (view == NULL) {
		event->ignore();
		return;
	}

	QGraphicsScene * _scene = scene();
    // Determine the list of items that need to be moved.
    QList<QGraphicsItem *> selectedItems;
    if (_scene) {
        selectedItems = _scene->selectedItems();
        if (movingItemsInitialPositions.isEmpty()) {
			foreach (QGraphicsItem *item, selectedItems) {
                movingItemsInitialPositions[item] = item->pos();
			}

			autoScrollX = autoScrollY = 0;
			connect(&autoScrollTimer, SIGNAL(timeout()), this, SLOT(autoScrollTimeout()));
        }
    }


	QPointF p = this->mapToScene(event->pos());
	QPoint q = view->mapFromScene(p);
	QRect r = view->viewport()->rect();
	if (!r.contains(q)) {
		autoScrollX = autoScrollY = 0;
		return;
	}

	r.adjust(16,16,-16,-16);						// these should be set someplace
	bool autoScroll = !r.contains(q);
	if (!autoScroll) {
		autoScrollX = autoScrollY = 0;
	}

    // Move all selected items
    int i = 0;
    while (i < selectedItems.size()) {
        QGraphicsItem *item = selectedItems.at(i);
        if ((item->flags() & ItemIsMovable) && !_qt_movableAncestorIsSelected(item)) {
            QPointF currentParentPos;
            QPointF buttonDownParentPos;
            if (_qt_ancestorIgnoresTransformations(item)) {
                // Items whose ancestors ignore transformations need to
                // map screen coordinates to local coordinates, then map
                // those to the parent.
                QTransform viewToItemTransform = (item->deviceTransform(view->viewportTransform())).inverted();
                currentParentPos = mapToParent(viewToItemTransform.map(QPointF(view->mapFromGlobal(event->screenPos()))));
                buttonDownParentPos = mapToParent(viewToItemTransform.map(QPointF(view->mapFromGlobal(event->buttonDownScreenPos(Qt::LeftButton)))));
            } else if (item->flags() & ItemIgnoresTransformations) {
                // Root items that ignore transformations need to
                // calculate their diff by mapping viewport coordinates
                // directly to parent coordinates.
                QTransform viewToParentTransform = (item->transform().translate(item->pos().x(), item->pos().y()))
                                                   * (item->sceneTransform() * view->viewportTransform()).inverted();
                currentParentPos = viewToParentTransform.map(QPointF(view->mapFromGlobal(event->screenPos())));
                buttonDownParentPos = viewToParentTransform.map(QPointF(view->mapFromGlobal(event->buttonDownScreenPos(Qt::LeftButton))));
            } else {
                // All other items simply map from the scene.
                currentParentPos = item->mapToParent(item->mapFromScene(event->scenePos()));
                buttonDownParentPos = item->mapToParent(item->mapFromScene(event->buttonDownScenePos(Qt::LeftButton)));
            }

            item->setPos(movingItemsInitialPositions.value(item) + currentParentPos - buttonDownParentPos);
        }
        ++i;
    }

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
		autoScrollX = dx;
		autoScrollY = dy;
		if (!autoScrollTimer.isActive()) {
			autoScrollTimer.start(10);
		}
	}

}

void ItemBase::autoScrollTimeout()
{
	if ((autoScrollX == 0 && autoScrollY == 0) || autoScrollView == NULL) return;

	QScrollBar * h = autoScrollView->horizontalScrollBar();
	QScrollBar * v = autoScrollView->verticalScrollBar();

	//DebugDialog::debug(QString("scroll dx:%1 dy%2 hval%3 vval:%4").arg(autoScrollX).arg(autoScrollY).arg(h->value()).arg(v->value()) );

	if (autoScrollX != 0) {
		h->setValue(((autoScrollX > 0) ? 1 : -1) + h->value());
	}
	if (autoScrollY != 0) {
		v->setValue(((autoScrollY > 0) ? 1 : -1) + v->value());
	}
}

void ItemBase::setItemPos(QPointF & loc) {
	setPos(loc);
}

bool ItemBase::sticky() {
	return m_sticky;
}


void ItemBase::addSticky(ItemBase * sticky, bool stickem) {
	if (stickem) {
		m_stickyList.insert(sticky, QPointF());
	}
	else {
		m_stickyList.remove(sticky);
	}
}


ItemBase * ItemBase::stuckTo() {
	if (m_sticky) return NULL;

	if (m_stickyList.count() < 1) return NULL;

	return m_stickyList.keys()[0];
}

QHash<ItemBase *, QPointF> & ItemBase::sticking() {
	return m_stickyList;
}

bool ItemBase::alreadySticking(ItemBase * itemBase) {
	return m_stickyList.keys().contains(itemBase);
}

void ItemBase::setChained(ConnectorItem * item, bool chained) {
	item->setChained(chained);
}

void ItemBase::setChained(const QString & connectorItemName, bool chained) {
	ConnectorItem * item = findConnectorItemNamed(connectorItemName);
	if (item != NULL) {
		item->setChained(chained);
	}
}

ConnectorItem* ItemBase::newConnectorItem(Connector *connector) {
	return new ConnectorItem(connector,this);
}

void ItemBase::restoreConnections(QDomElement & instance, QHash<long, ItemBase *> & newItems) {

	QDomElement connectorsElement = instance.firstChildElement("connectors");
	if (!connectorsElement.isNull()) {
		QDomElement connectorElement = connectorsElement.firstChildElement("connector");
		while (!connectorElement.isNull()) {
			ConnectorItem * connectorItem = findConnectorItemNamed(connectorElement.attribute("connectorId"));
			if (connectorItem != NULL) {
				connectorItem->restoreConnections(connectorElement, newItems);
			}
			connectorElement = connectorElement.nextSiblingElement("connector");
		}
	}

	// merge buses
	QDomElement busesElement = instance.firstChildElement("buses");
	if (!busesElement.isNull()) {
		QDomElement connectorElement = busesElement.firstChildElement("connector");
		while (!connectorElement.isNull()) {
			QString busID = connectorElement.attribute("busId");
			BusConnectorItem * bci = busConnectorItem(busID);
			if (bci != NULL) {
				QDomElement mergedElement = connectorElement.firstChildElement("merged");
				if (!mergedElement.isNull()) {
					QDomElement busElement = mergedElement.firstChildElement("bus");
					while (!busElement.isNull()) {
						bool ok;
						long modelIndex = busElement.attribute("modelIndex").toLong(&ok);
						if (ok) {
							QString otherBusID = busElement.attribute("busId");
							ItemBase * otherBase = newItems.value(modelIndex);
							if (otherBase != NULL) {
								BusConnectorItem* otherBusConnectorItem = otherBase->busConnectorItem(otherBusID);
								if (otherBusConnectorItem) {
									otherBusConnectorItem->merge(bci);
									bci->merge(otherBusConnectorItem);
								}
							}
						}

						busElement = busElement.nextSiblingElement("bus");
					}
				}
			}


			connectorElement = connectorElement.nextSiblingElement("connector");
		}
	}
}

ConnectorItem * ItemBase::anyConnectorItem() {
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (connectorItem != NULL) return connectorItem;
	}

	return NULL;
}

////////////////////////////////////////

bool _qt_movableAncestorIsSelected(const QGraphicsItem *item)
{
    const QGraphicsItem *parent = item->parentItem();
    return parent && (((parent->flags() & QGraphicsItem::ItemIsMovable) && parent->isSelected()) || _qt_movableAncestorIsSelected(parent));
}


bool _qt_ancestorIgnoresTransformations(const QGraphicsItem *item)
{
    const QGraphicsItem *parent = item->parentItem();
	return parent && ((parent->flags() & QGraphicsItem::ItemIgnoresTransformations) || _qt_ancestorIgnoresTransformations(parent));
}

QString ItemBase::instanceTitle() {
	if(m_modelPart->partInstanceStuff()) {
		return m_modelPart->partInstanceStuff()->title();
	}
	return ___emptyString___;
}

void ItemBase::setInstanceTitle(const QString &title) {
	InfoGraphicsView *infographics = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
	if (infographics != NULL) {
		infographics->setItemTooltip(id(),title);
	}
}

QString ItemBase::label() {
	if(m_modelPart->modelPartStuff()) {
		return m_modelPart->modelPartStuff()->label();
	}
	return ___emptyString___;
}

void ItemBase::setInstanceTitleAndTooltip(const QString &title) {
	m_modelPart->partInstanceStuff()->setTitle(title);
	setInstanceTitleTooltip(title);
}

void ItemBase::updateTooltip() {
	setInstanceTitleTooltip(instanceTitle());
}

void ItemBase::setInstanceTitleTooltip(const QString &text) {
	setToolTip("<b>"+text+"</b><br></br><font size='2'>"+modelPartStuff()->title()+"</font>");
}

void ItemBase::setDefaultTooltip() {
	QString base = "<font size='2'>%1</font>";
	if(m_modelPart->itemType() != ModelPart::Wire) {
		this->setToolTip(base.arg(m_modelPart->title()));
	} else {
		this->setToolTip(base.arg(m_modelPart->modelPartStuff()->title() + " (" + m_modelPart->modelPartStuff()->moduleID() + ")"));
	}
}

bool ItemBase::isConnectedTo(ItemBase * other) {
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * fromConnectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (fromConnectorItem == NULL) continue;

		foreach (ConnectorItem * toConnectorItem, fromConnectorItem->connectedToItems()) {
			if (toConnectorItem->attachedTo() == other) return true;
		}
	}

	return false;
}


bool ItemBase::stickyEnabled(ItemBase * stickTo) {
	Q_UNUSED(stickTo);

	return true;
}

void ItemBase::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    if (m_itemMenu != NULL) {
     	m_itemMenu->exec(event->screenPos());
	}
}

ViewLayer::ViewLayerID ItemBase::defaultConnectorLayer(ItemBase::ViewIdentifier viewId) {
	switch(viewId) {
		case ItemBase::BreadboardView: return ViewLayer::Breadboard;
		case ItemBase::SchematicView: return ViewLayer::Schematic;
		case ItemBase::PCBView: return ViewLayer::Copper0;
		default: return ViewLayer::UnknownLayer;
	}
}

bool ItemBase::hasConnectors() {
	foreach (QGraphicsItem * childItem, childItems()) {
		if (dynamic_cast<ConnectorItem *>(childItem) != NULL) return true;
	}

	return false;
}


bool ItemBase::canFlipHorizontal() {
	return m_canFlipHorizontal;
}

void ItemBase::setCanFlipHorizontal(bool cf) {
	m_canFlipHorizontal = cf;
}

bool ItemBase::canFlipVertical() {
	return m_canFlipVertical;
}

void ItemBase::setCanFlipVertical(bool cf) {
	m_canFlipVertical = cf;
}
