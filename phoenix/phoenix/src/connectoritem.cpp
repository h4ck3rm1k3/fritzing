#include "connectoritem.h"

#include <QBrush>
#include <QPen>
#include <QColor>

#include "infographicsview.h"
#include "debugdialog.h"
#include "bus.h"
#include "wire.h"

QPen ConnectorItem::normalPen(QColor(255,0,0));
QPen ConnectorItem::hoverPen(QColor(0, 0, 255));
QPen ConnectorItem::connectedPen(QColor(0, 255, 0));
QBrush ConnectorItem::normalBrush(QColor(255,0,0));
QBrush ConnectorItem::hoverBrush(QColor(0,0,255));
QBrush ConnectorItem::connectedBrush(QColor(0,255,0));

ConnectorItem::ConnectorItem( Connector * connector, ItemBase * attachedTo )
	: QGraphicsRectItem(attachedTo)
{
	m_dirty = false;
	m_chained = false;
	m_opacity = 0.4;
	m_circular = false;
	m_overConnectorItem = NULL;
	m_hidden = false;
	m_attachedTo = attachedTo;
	m_connector = connector;
	if (connector != NULL) {
		connector->addViewItem(this);
	}
	restoreColor();
    setAcceptsHoverEvents(true);
    this->setCursor(Qt::CrossCursor);
	//DebugDialog::debug(QObject::tr("%1 attached to %2")
			//.arg(this->connector()->connectorStuff()->id())
			//.arg(attachedTo->modelPart()->modelPartStuff()->title()) );
}

ConnectorItem::~ConnectorItem() {
	for (int i = 0; i < m_connectedTo.count(); i++) {
		m_connectedTo[i]->tempRemove(this);
	}
	if (this->connector() != NULL) {
		this->connector()->removeViewItem(this);
	}
}

void ConnectorItem::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) {
	setHoverColor();
	InfoGraphicsView * infoGraphicsView = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
	if (infoGraphicsView != NULL) {
		infoGraphicsView->hoverEnterConnectorItem(event, this);
	}
	if (this->m_attachedTo != NULL) {
		m_attachedTo->hoverEnterConnectorItem(event, this);
	}
}

void ConnectorItem::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) {
	restoreColor();
	InfoGraphicsView * igv = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
	if (igv != NULL) {
		igv->hoverLeaveConnectorItem(event, this);
	}
}

Connector * ConnectorItem::connector() {
	return m_connector;
}

ItemBase * ConnectorItem::attachedTo() {
	return m_attachedTo;
}

void ConnectorItem::connectorHover(ItemBase * itemBase, bool hovering) {
	if (hovering) {
		setHoverColor();
	}
	else {
		restoreColor();
	}
	if (this->m_attachedTo != NULL) {
		m_attachedTo->connectorHover(this, itemBase, hovering);
	}
}

void ConnectorItem::connectTo(ConnectorItem * connected) {
	if (m_connectedTo.contains(connected)) return;

	m_connectedTo.append(connected);
	//DebugDialog::debug(QObject::tr("connect to cc:%4 this:%1 to:%2 %3").arg((long) this, 0, 16).arg((long) connected, 0, 16).arg(connected->attachedTo()->modelPart()->modelPartStuff()->title()).arg(m_connectedTo.count()) );
	restoreColor();
	if (m_attachedTo != NULL) {
		m_attachedTo->connectionChange(this);
	}
}

void ConnectorItem::tempConnectTo(ConnectorItem * item) {
	m_connectedTo.append(item);
}

void ConnectorItem::tempRemove(ConnectorItem * item) {
	m_connectedTo.removeOne(item);
}

void ConnectorItem::restoreColor() {
	if (m_connectedTo.count() <= 0) {
		setNormalColor();
		return;
	}

	setConnectedColor();
}

void ConnectorItem::setConnectedColor() {
	setColorAux(connectedBrush, connectedPen, true);
}

void ConnectorItem::setNormalColor() {
	setColorAux(normalBrush, normalPen, false);
}

void ConnectorItem::setHoverColor() {
	setColorAux(hoverBrush, hoverPen, true);
}

void ConnectorItem::setColorAux(QBrush brush, QPen pen, bool paint) {
	this->setBrush(brush);
	this->setPen(pen);
	m_paint = paint;
}

void ConnectorItem::setColorAux(const QColor &color, bool paint) {
	this->setBrush(QBrush(color));
	this->setPen(QPen(color));
	m_paint = paint;
}

void ConnectorItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	if (this->m_attachedTo != NULL) {
		m_attachedTo->mousePressConnectorEvent(this, event);
	}
}

int ConnectorItem::connectionsCount() {
	return m_connectedTo.count();
}


void ConnectorItem::attachedMoved() {
	//DebugDialog::debug("attached moved");
	for (int i = 0; i < m_connectedTo.count(); i++) {
		ItemBase * a = m_connectedTo[i]->attachedTo();
		if (a == NULL) {
			continue;
		}

		//DebugDialog::debug(QObject::tr("moved attached to %1 %2 %3").arg(this->connectorStuffID()).arg(this->attachedToID()).arg(this->attachedToTitle()) );
		a->connectedMoved(this, m_connectedTo[i]);
	}
}

ConnectorItem * ConnectorItem::removeConnection(ItemBase * itemBase) {
	for (int i = 0; i < m_connectedTo.count(); i++) {
		if (m_connectedTo[i]->attachedTo() == itemBase) {
			ConnectorItem * removed = m_connectedTo[i];
			m_connectedTo.removeAt(i);
			if (m_attachedTo != NULL) {
				m_attachedTo->connectionChange(this);
			}
			restoreColor();
			DebugDialog::debug(QObject::tr("remove from:%1 to:%2 count%3")
				.arg((long) this, 0, 16)
				.arg(itemBase->modelPart()->modelPartStuff()->title())
				.arg(m_connectedTo.count()) );
			return removed;
		}
	}

	return NULL;
}

void ConnectorItem::removeConnection(ConnectorItem * connectedItem, bool emitChange) {
	if (connectedItem == NULL) return;

	m_connectedTo.removeOne(connectedItem);
	restoreColor();
	if (emitChange) {
		m_attachedTo->connectionChange(this);
	}
}

ConnectorItem * ConnectorItem::firstConnectedToIsh() {
	if (m_connectedTo.count() <= 0) return NULL;

	foreach (ConnectorItem * connectorItem, m_connectedTo) {
		if (!connectorItem->attachedTo()->getVirtual()) return connectorItem;
	}

	return NULL;
}

void ConnectorItem::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) {
	if (m_hidden  || !m_paint) return;

	painter->setOpacity(m_opacity);
	if (m_circular) {
		painter->setPen(pen());
		painter->setBrush(brush());
		painter->drawEllipse(rect());
	}
	else {
		QGraphicsRectItem::paint(painter, option, widget);
	}

}

void ConnectorItem::setTerminalPoint(QPointF p) {
	m_terminalPoint = p;
}

QPointF ConnectorItem::terminalPoint() {
	return m_terminalPoint;
}

QPointF ConnectorItem::adjustedTerminalPoint() {
	return m_terminalPoint + this->rect().topLeft();
}

QPointF ConnectorItem::sceneAdjustedTerminalPoint() {
	return this->mapToScene(m_terminalPoint + this->rect().topLeft());
}

void ConnectorItem::restoreConnections(QDomElement & instance, QHash<long, ItemBase *> newItems) {
	setChained(instance.attribute("chained").compare("true") == 0);
		
	QDomElement connectsToElement = instance.firstChildElement("connects");
	if (connectsToElement.isNull()) return;
	
	QDomElement connectToElement = connectsToElement.firstChildElement("connect");
	while (!connectToElement.isNull()) {
		bool ok;
   		long modelIndex = connectToElement.attribute("modelIndex").toLong(&ok);
   		if (ok) {
			ItemBase * toBase = newItems.value(modelIndex);
			if (toBase != NULL) {	
				ConnectorItem * connectorItem = NULL;
				QString toConnectorID = connectToElement.attribute("connectorId");
				QString toBusID = connectToElement.attribute("busId");		
				if (toBusID.length() > 0) {
					connectorItem = toBase->busConnectorItemCast(toBusID);
				}
				else if (toConnectorID.length() > 0) {
					connectorItem = toBase->findConnectorItemNamed(toConnectorID);
				}
				if (connectorItem != NULL) {
					connectTo(connectorItem);
					connectorItem->connectTo(this);
					m_connector->connectTo(connectorItem->connector());
				}					
			}
		}
				
		connectToElement = connectToElement.nextSiblingElement("connect");
	}
}

bool ConnectorItem::connectedTo(ConnectorItem * connectorItem) {
	return this->m_connectedTo.contains(connectorItem);
}

const QList<ConnectorItem *> & ConnectorItem::connectedToItems() {
	return m_connectedTo;
}

void ConnectorItem::setHidden(bool hidden) {
	m_hidden = hidden;
	if (hidden) {
		this->setAcceptedMouseButtons(Qt::NoButton);
		this->unsetCursor();
	}
	else {
		this->setAcceptedMouseButtons(Qt::LeftButton | Qt::MidButton | Qt::RightButton | Qt::XButton1 | Qt::XButton2);
		this->setCursor(Qt::CrossCursor);
	}
	this->update();

}

ConnectorItem * ConnectorItem::overConnectorItem() {
	return m_overConnectorItem;
}

void ConnectorItem::setOverConnectorItem(ConnectorItem * connectorItem) {
	m_overConnectorItem = connectorItem;
}

long ConnectorItem::attachedToID() {
	if (attachedTo() == NULL) return -1;
	return attachedTo()->id();
}

const QString & ConnectorItem::attachedToTitle() {
	if (attachedTo() == NULL) return ___emptyString___;
	return attachedTo()->title();
}

const QString & ConnectorItem::connectorStuffID() {
	if (m_connector == NULL) return ___emptyString___;

	return m_connector->connectorStuffID();
}

const QString & ConnectorItem::busID() {
	if (m_connector == NULL) return ___emptyString___;

	return m_connector->busID();
}

ModelPartStuff * ConnectorItem::modelPartStuff() {
	if (m_attachedTo == NULL) return NULL;

	return m_attachedTo->modelPartStuff();
}


ModelPart * ConnectorItem::modelPart() {
	if (m_attachedTo == NULL) return NULL;

	return m_attachedTo->modelPart();
}

Bus * ConnectorItem::bus() {
	if (m_connector == NULL) return NULL;

	return m_connector->bus();
}

void ConnectorItem::adjustConnectedItems() {
	foreach (ConnectorItem * toConnector, m_connectedTo) {
		ItemBase * itemBase = toConnector->attachedTo();
		if (itemBase == NULL) continue;

		itemBase->connectedMoved(this, toConnector);
	}
}

void ConnectorItem::setCircular(bool circular) {
	m_circular = circular;
}

void ConnectorItem::setOpacity(qreal opacity) {
	m_opacity = opacity;
}

int ConnectorItem::attachedToItemType() {
	if (m_attachedTo == NULL) return ModelPart::Unknown;

	return m_attachedTo->itemType();
}


Connector::ConnectorType ConnectorItem::connectorType() {
	if (m_connector == NULL) return Connector::Unknown;

	return m_connector->connectorType();
}

void ConnectorItem::setChained(bool chained) {
	m_chained = chained;
}

bool ConnectorItem::chained() {
	return m_chained;
}

void ConnectorItem::writeTopLevelAttributes(QXmlStreamWriter & writer) {
	// do not write anything other than attributes in this routine.l
	writer.writeAttribute("layer", ViewLayer::viewLayerXmlNameFromID(attachedTo()->viewLayerID()));
}

void ConnectorItem::saveInstance(QXmlStreamWriter & writer) {
	if (m_connectedTo.count() <= 0) {
		// no need to save if there's no connection
		return;
	}
	
	writer.writeStartElement("connector");
	writer.writeAttribute("connectorId", connectorStuffID());
	if (m_chained) {
		writer.writeAttribute("chained", "true");
	}
	
	writeTopLevelAttributes(writer);
	writer.writeStartElement("geometry");	
	writer.writeAttribute("x", QString::number(this->pos().x()));
	writer.writeAttribute("y", QString::number(this->pos().y()));
	writer.writeEndElement();
	
	writer.writeStartElement("connects");
	foreach (ConnectorItem * connectorItem, this->m_connectedTo) {
		writer.writeStartElement("connect");
		writer.writeAttribute("connectorId", connectorItem->connectorStuffID());
		writer.writeAttribute("busId", connectorItem->busID());
		writer.writeAttribute("modelIndex", QString::number(connectorItem->connector()->modelIndex()));
		writer.writeEndElement();
	}
	writer.writeEndElement();
	
	writeOtherElements(writer);
	
	writer.writeEndElement();
}

void ConnectorItem::writeOtherElements(QXmlStreamWriter & writer) {
	Q_UNUSED(writer);
}

bool ConnectorItem::maleToFemale(ConnectorItem * other) {
	if (this->connectorType() == Connector::Male && other->connectorType() == Connector::Female) return true;
	if (this->connectorType() == Connector::Female && other->connectorType() == Connector::Male) return true;

	return false;
}

Wire * ConnectorItem::wiredTo(ConnectorItem * target, ViewGeometry::WireFlags flags) {
	foreach (ConnectorItem * toConnectorItem, m_connectedTo) {
		ItemBase * toItem = toConnectorItem->attachedTo();
		if (toItem == NULL) {
			continue;			// shouldn't happen
		}

		if (toItem->itemType() != ModelPart::Wire) continue;

		Wire * wire = dynamic_cast<Wire *>(toItem);
		if (!wire->hasAnyFlag(flags)) continue;

		ConnectorItem * otherEnd = wire->otherConnector(toConnectorItem);
		foreach (ConnectorItem * otherConnectorItem, otherEnd->m_connectedTo) {
			if (target == otherConnectorItem) {
				return wire;
			}
		}
	}

	return NULL;
}

bool ConnectorItem::wiredTo(ConnectorItem * target) 
{
	foreach (ConnectorItem * toConnectorItem, m_connectedTo) {
		if (target == toConnectorItem) 
		{
			return true;
		}
	}

	foreach (ConnectorItem * toConnectorItem, m_connectedTo) {
		if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
			ConnectorItem * otherConnector = dynamic_cast<Wire *>(toConnectorItem->attachedTo())->otherConnector(toConnectorItem);
			if (otherConnector->wiredTo(target)) {
				return true;
			}
		}
	}

	return false;
}


void ConnectorItem::setDirty(bool dirty) {
	m_dirty = dirty;
}

bool ConnectorItem::isDirty() {
	return m_dirty;
}

