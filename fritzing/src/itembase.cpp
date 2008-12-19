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
#include "connector.h"
#include "bus.h"

#include <QScrollBar>
#include <QTimer>

long ItemBase::nextID = 0;
QHash <ItemBase::ViewIdentifier, StringTriple * > ItemBase::names;
QString ItemBase::rulerModuleIDName = "RulerModuleID";
QString ItemBase::breadboardModuleIDName = "BreadboardModuleID";

bool wireLessThan(ConnectorItem * c1, ConnectorItem * c2)
{
	if (c1->connectorType() == c2->connectorType()) {
		// if they're the same type return the topmost
		return c1->zValue() > c2->zValue();
	}
	if (c1->connectorType() == Connector::Female) {
		// choose the female first
		return true;
	}
	if (c2->connectorType() == Connector::Female) {
		// choose the female first
		return false;
	}
	if (c1->connectorType() == Connector::Male) {
		// choose the male first
		return true;
	}
	if (c2->connectorType() == Connector::Male) {
		// choose the male first
		return false;
	}

	return c1->zValue() > c2->zValue();
	
}


ItemBase::ItemBase( ModelPart* modelPart, ItemBase::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, bool topLevel, QMenu * itemMenu )
	: GraphicsSvgLineItem()
{
	m_itemMenu = itemMenu;
	m_topLevel = topLevel;
	m_connectorHoverCount = 0;
	m_viewIdentifier = viewIdentifier;
	m_modelPart = modelPart;
	if (m_modelPart) {
		m_modelPart->addViewItem(this);
	}
	setTooltip();
	m_id = id;
	m_hidden = false;
	m_sticky = false;
	m_canFlipHorizontal = m_canFlipVertical = false;

	setCursor(Qt::ArrowCursor);

   	m_viewGeometry.set(viewGeometry);
	setAcceptHoverEvents ( true );
	m_zUninitialized = true;

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
	foreach (QList<ConnectorItem *> * list, m_busConnectorItems) {
		delete list;
	}

	m_busConnectorItems.clear();
}

void ItemBase::setTooltip() {
	if(m_modelPart && m_modelPart->partInstanceStuff()) {
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

void ItemBase::setConnectorTooltips() {
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (connectorItem == NULL) continue;

		QString tt = QString("%1<br />" + ITEMBASE_FONT_PREFIX + "%2" + ITEMBASE_FONT_SUFFIX).arg(toolTip()).arg(connectorItem->connectorStuffID());

		connectorItem->setBaseTooltip(tt);
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
	if (m_zUninitialized) {
   		ViewLayer * viewLayer = viewLayers.value(m_viewLayerID);
   		if (viewLayer != NULL) {
			m_zUninitialized = false;
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

void ItemBase::collectConnectors(ConnectorPairHash & connectorHash, QGraphicsScene * scene) {
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
	QList<ConnectorItem *> candidates;
	// for the moment, take the topmost ConnectorItem that doesn't belong to me
	foreach (QGraphicsItem * item, items) {
		ConnectorItem * connectorItemUnder = dynamic_cast<ConnectorItem *>(item);
		if (connectorItemUnder == NULL) continue;
		if (connectorItemUnder->connector() == NULL) continue;			// for now; this is probably a busConnectorItem
		if (childItems().contains(connectorItemUnder)) continue;
		if (!connectorItemOver->connector()->connectionIsAllowed(connectorItemUnder->connector())) {
			continue;
		}
		if (connectorItemUnder->connectedToItems().contains(connectorItemOver)) {
			continue;		// already connected
		}

		candidates.append(connectorItemUnder);
	}

	ConnectorItem * candidate = NULL;
	if (candidates.count() == 1) {
		candidate = candidates[0];
	}
	else if (candidates.count() > 0) {
		qSort(candidates.begin(), candidates.end(), wireLessThan);
		candidate = candidates[0];
	}

	if (lastUnderConnector != NULL && candidate != lastUnderConnector) {
		lastUnderConnector->connectorHover(this, false);
	}
	if (candidate != NULL && candidate != lastUnderConnector) {
		candidate->connectorHover(this, true);
	}

	lastUnderConnector = candidate;

	return lastUnderConnector;
}


void ItemBase::sendConnectionChangedSignal(ConnectorItem * from, ConnectorItem * to, bool connect) {
	Q_UNUSED(from);
	Q_UNUSED(connect);
	Q_UNUSED(to);
}

void ItemBase::updateConnections() {
}

void ItemBase::updateConnections(ConnectorItem * item) {
	item->attachedMoved();
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

void ItemBase::addBusConnectorItem(Bus * bus, ConnectorItem * item) {
	QList <ConnectorItem *> * busConnectorItems = m_busConnectorItems.value(bus);
	if (busConnectorItems == NULL) {
		busConnectorItems = new QList<ConnectorItem *>;
		m_busConnectorItems.insert(bus, busConnectorItems);
	}
	busConnectorItems->append(item);
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
	//setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	GraphicsSvgLineItem::mousePressEvent(event);
}

void ItemBase::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	// calling parent class so that multiple selection will work
	// haven't yet discovered any nasty side-effect
	GraphicsSvgLineItem::mouseReleaseEvent(event);
	//scene()->setItemIndexMethod(QGraphicsScene::BspTreeIndex);
	// setCacheMode(QGraphicsItem::NoCache);

}

void ItemBase::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
	return;
}

void ItemBase::setItemPos(QPointF & loc) {
	setPos(loc);
}

bool ItemBase::sticky() {
	return m_sticky;
}


void ItemBase::addSticky(ItemBase * sticky, bool stickem) {
	if (stickem) {
		if (!m_sticky) {
			foreach (ItemBase * oldStuckTo, m_stickyList.keys()) {
				oldStuckTo->addSticky(this, false);
			}
			m_stickyList.clear();
		}
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
}

ConnectorItem * ItemBase::anyConnectorItem() {
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (connectorItem != NULL) return connectorItem;
	}

	return NULL;
}


QString ItemBase::instanceTitle() {
	if(m_modelPart && m_modelPart->partInstanceStuff()) {
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
	if(m_modelPart && m_modelPart->modelPartStuff()) {
		return m_modelPart->modelPartStuff()->label();
	}
	return ___emptyString___;
}

void ItemBase::setInstanceTitleAndTooltip(const QString &title) {
	if (m_modelPart) {
		m_modelPart->partInstanceStuff()->setTitle(title);
	}
	setInstanceTitleTooltip(title);
}

void ItemBase::updateTooltip() {
	setInstanceTitleTooltip(instanceTitle());
}

void ItemBase::setInstanceTitleTooltip(const QString &text) {
	setToolTip("<b>"+text+"</b><br></br>" + ITEMBASE_FONT_PREFIX + modelPartStuff()->title()+ ITEMBASE_FONT_SUFFIX);
}

void ItemBase::setDefaultTooltip() {
	if (m_modelPart) {
		QString base = ITEMBASE_FONT_PREFIX + "%1" + ITEMBASE_FONT_SUFFIX;
		if(m_modelPart->itemType() != ModelPart::Wire) {
			this->setToolTip(base.arg(m_modelPart->title()));
		} else {
			this->setToolTip(base.arg(m_modelPart->modelPartStuff()->title() + " (" + m_modelPart->modelPartStuff()->moduleID() + ")"));
		}
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
	if (m_hidden) {
		event->ignore();
		return;
	}

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

void ItemBase::busConnectorItems(class Bus * bus, QList<class ConnectorItem *> & items) {
	QList<ConnectorItem *> * busConnectorItems = m_busConnectorItems.value(bus);
	if (busConnectorItems == NULL) return;
/*

	if (itemType() == ModelPart::Breadboard) {
		foreach (ConnectorItem * connectorItem, *busConnectorItems) {
			if (connectorItem->connectionsCount() > 0) {
				items.append(connectorItem);
			}
		}
	}
	else {
		foreach (ConnectorItem * connectorItem, *busConnectorItems) {
			items.append(connectorItem);
		}
	}

	*/

	foreach (ConnectorItem * connectorItem, *busConnectorItems) {
		items.append(connectorItem);
	}

}

void ItemBase::clearModelPart() {
	m_modelPart = NULL;
}
