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

#include "paletteitembase.h"
#include "infographicsview.h"
#include "debugdialog.h"
#include "fsvgrenderer.h"
#include "svg/svgfilesplitter.h"
#include "connectorshared.h"
#include "layerattributes.h"
#include "layerkinpaletteitem.h"
#include "connectoritem.h"
#include "wire.h"

#include <math.h>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QDir>
#include <QMessageBox>


PaletteItemBase::PaletteItemBase(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu ) :
	ItemBase(modelPart, viewIdentifier, viewGeometry, id, itemMenu)
{
	m_syncSelected = false;
	m_offset.setX(0);
	m_offset.setY(0);
 	m_blockItemSelectedChange = false;
	this->setPos(viewGeometry.loc());
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setAcceptsHoverEvents(true);
}

QRectF PaletteItemBase::boundingRect() const
{
    return QRectF(0, 0, m_size.width(), m_size.height());
}

QPainterPath PaletteItemBase::shape() const
{
	// TODO: figure out real shape of svg
    QPainterPath path;

	if ((m_viewLayerID == ViewLayer::Copper0) && 
		((itemType() == ModelPart::Board) || (itemType() == ModelPart::ResizableBoard)) && 
		(m_modelPart->moduleID().compare("1234ABDE24_ST") == 0)) 
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! hack alert !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! eventually this should be replaced by someting in the actual fzp file
		// hack for testing click through on the arduino
		path.addRect(m_size.width() * 0.25, 0, 3 * m_size.width() / 4, m_size.height() / 10);
		path.addRect(m_size.width() * 0.5, 9 * m_size.height() / 10, m_size.width() * 0.5, m_size.height() / 10);
		return path;
	}

    path.addRect(0, 0, m_size.width(), m_size.height());
    return path;
}

QVariant PaletteItemBase::itemChange(GraphicsItemChange change, const QVariant &value)
{
     if (change == ItemPositionChange && scene()) {

		 emit posChangedSignal();

     	// snap to grid

        // QVariant argument is the new position.
        //QPointF newPos = value.toPointF();
        //int nx = abs((int) floor(newPos.x() + .5));
        //int ny = abs((int) floor(newPos.y() + .5));
        //nx -= nx % 20;
        //ny -= ny % 20;
        //if (newPos.x() < 0) nx = -nx;
        //if (newPos.y() < 0) ny = -ny;
        //newPos.setX(nx);
        //newPos.setY(ny);
        //return newPos;
     }
     return ItemBase::itemChange(change, value);
}

void PaletteItemBase::saveGeometry() {
	m_viewGeometry.setLoc(this->pos());
	m_viewGeometry.setSelected(this->isSelected());
	m_viewGeometry.setZ(this->zValue());
}

bool PaletteItemBase::itemMoved() {
	return (this->pos() != m_viewGeometry.loc());
}

void PaletteItemBase::moveItem(ViewGeometry & viewGeometry) {
	this->setPos(viewGeometry.loc());
	updateConnections();
}

void PaletteItemBase::saveInstanceLocation(QXmlStreamWriter & streamWriter)
{
	saveLocAndTransform(streamWriter);
}

void PaletteItemBase::syncKinSelection(bool selected, PaletteItemBase * /* originator */) {
	m_syncSelected = selected;
}

void PaletteItemBase::syncKinMoved(QPointF offset, QPointF newPos) {
	Q_UNUSED(offset);
	Q_UNUSED(newPos);
}

QPointF PaletteItemBase::syncMoved() {
	return m_syncMoved;
}

void PaletteItemBase::blockItemSelectedChange(bool selected) {
	m_blockItemSelectedChange = true;
	m_blockItemSelectedValue = selected;
}

bool PaletteItemBase::syncSelected() {
	return m_syncSelected;
}

void PaletteItemBase::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	if (m_hidden) return;

	ItemBase::paint(painter, option, widget);
}

void PaletteItemBase::mousePressConnectorEvent(ConnectorItem * connectorItem, QGraphicsSceneMouseEvent * event) {
	InfoGraphicsView * infoGraphicsView = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
	if (infoGraphicsView != NULL) {
		infoGraphicsView->mousePressConnectorEvent(connectorItem, event);
	}
}

bool PaletteItemBase::acceptsMousePressConnectorEvent(ConnectorItem *, QGraphicsSceneMouseEvent * event) {
	Q_UNUSED(event);
	
	if (m_viewIdentifier != ViewIdentifierClass::PCBView) {
		return true;
	}

	//if ((event->modifiers() & Qt::ShiftModifier) == 0) {
		// force a shift-click to drag out a wire in pcb view
		//return false;
	//}

	return true;
}


void PaletteItemBase::mousePressEvent(PaletteItemBase * originalItem, QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(originalItem);

	ItemBase::mousePressEvent(event);
	if (this->itemType() != ModelPart::Breadboard) {
		foreach (QGraphicsItem * childItem, childItems()) {
			ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
			if (connectorItem == NULL) continue;

			connectorItem->setOverConnectorItem(NULL);
		}
	}
}

void PaletteItemBase::findConnectorsUnder() {
	switch (itemType()) {
		case ModelPart::Breadboard:
		case ModelPart::Board:
		case ModelPart::ResizableBoard:
			// don't try to map connectors when we drag a breadboard: it's too damn slow
			return;
		default:
			break;
	}

	for (int i = 0; i < childItems().count(); i++) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItems()[i]);
		if (connectorItem == NULL) continue;

		if (connectorItem->connector()->connectorType() == Connector::Female) {
			// also helps speed things up
			continue;
		}

		connectorItem->setOverConnectorItem(
				findConnectorUnder(connectorItem,  connectorItem->overConnectorItem(), true));

	}
}

void PaletteItemBase::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	ItemBase::mouseReleaseEvent(event);
}

void PaletteItemBase::collectFemaleConnectees(QSet<ItemBase *> & items) {
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * item = dynamic_cast<ConnectorItem *>(childItem);
		if (item == NULL) continue;
		if (item->connectorType() != Connector::Female) continue;

		foreach (ConnectorItem * toConnectorItem, item->connectedToItems()) {
			if (toConnectorItem->attachedToItemType() == ModelPart::Wire) continue;
			if (!toConnectorItem->attachedTo()->isVisible()) continue;

			// walk up the tree in case it's a group
			ItemBase * parent = toConnectorItem->attachedTo();
			while (parent->parentItem() != NULL) {
				parent = dynamic_cast<ItemBase *>(parent->parentItem());
			}

			items.insert(parent);
		}
	}
}

void PaletteItemBase::collectWireConnectees(QSet<Wire *> & wires) {
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * item = dynamic_cast<ConnectorItem *>(childItem);
		if (item == NULL) continue;

		foreach (ConnectorItem * toConnectorItem, item->connectedToItems()) {
			if (toConnectorItem->attachedToItemType() == ModelPart::Wire) {
				if (toConnectorItem->attachedTo()->isVisible()) {
					wires.insert(dynamic_cast<Wire *>(toConnectorItem->attachedTo()));
				}
			}
		}
	}
}

bool PaletteItemBase::setUpImage(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID viewLayerID, bool doConnectors)
{
	LayerAttributes layerAttributes;
	FSvgRenderer * renderer = ItemBase::setUpImage(modelPart, viewIdentifier, viewLayerID, layerAttributes);
	if (renderer == NULL) {
		return false;
	}

	m_canFlipVertical = layerAttributes.canFlipVertical();
	m_canFlipHorizontal = layerAttributes.canFlipHorizontal();
	m_filename = layerAttributes.filename();
	//DebugDialog::debug(QString("filename %1").arg(m_filename) );
	setSticky(layerAttributes.sticky());
	QString elementID = layerAttributes.layerName();
	setViewLayerID(elementID, viewLayers);

	//DebugDialog::debug(QString("setting layer %1 view:%2 z:%3").arg(modelPart->title()).arg(viewIdentifier).arg(this->z()) );
	this->setZValue(this->z());
	this->setSharedRenderer(renderer);

	m_svg = true;

	if (doConnectors) {
		setUpConnectors(renderer, modelPart->ignoreTerminalPoints());
		setConnectorTooltips();
	}

	if (!m_viewGeometry.transform().isIdentity()) {
		setTransform(m_viewGeometry.transform());
		update();
	}

	return true;
}

void PaletteItemBase::setSharedRenderer(FSvgRenderer * renderer) {
	ItemBase::setSharedRenderer(renderer);
	m_size = renderer->defaultSizeF();
}

void PaletteItemBase::setUpConnectors(FSvgRenderer * renderer, bool ignoreTerminalPoints) {
	if (m_modelPart->connectors().count() <= 0) return;

	foreach (Connector * connector, m_modelPart->connectors().values()) {
		if (connector == NULL) continue;

		QRectF connectorRect;
		QPointF terminalPoint;
		bool result = connector->setUpConnector(renderer, m_modelPart->moduleID(), m_viewIdentifier, m_viewLayerID, connectorRect, terminalPoint, ignoreTerminalPoints);
		if (!result) continue;

		//DebugDialog::debug(QString("<rect view=\"%6\" id=\"%1pin\" x=\"%2\" y=\"%3\" width=\"%4\" height=\"%5\" />")
						   //.arg(connector->connectorSharedID())
						   //.arg(connectorRect.x()).arg(connectorRect.y())
						   //.arg(connectorRect.width()).arg(connectorRect.height())
						   //.arg(m_viewIdentifier) );
		//DebugDialog::debug(QString("<rect id=\"%1pterminal\" x=\"%2\" y=\"%3\" width=\"%4\" height=\"%5\" />")
						   //.arg(connector->connectorSharedID())
						   //.arg(connectorRect.x() + (connectorRect.width() / 2)).arg(connectorRect.y() + (connectorRect.height() /2))
						   //.arg(0).arg(0) );

		ConnectorItem * connectorItem = newConnectorItem(connector);

		//DebugDialog::debug(	QString("in layer %1 with z %2")
			//.arg(ViewLayer::viewLayerNameFromID(m_viewLayerID))
			//.arg(this->zValue()) );

		connectorItem->setRect(connectorRect);
		connectorItem->setTerminalPoint(terminalPoint);
		//DebugDialog::debug(QString("terminal point %1 %2").arg(terminalPoint.x()).arg(terminalPoint.y()) );

		Bus * bus = connectorItem->bus();
		if (bus != NULL) {
			addBusConnectorItem(bus, connectorItem);
		}
	}
}


const QString & PaletteItemBase::filename() {
	return m_filename;
}

void PaletteItemBase::connectedMoved(ConnectorItem * from, ConnectorItem * to) {
	if (from->connectorType() != Connector::Female) return;
	if (m_viewIdentifier != ViewIdentifierClass::BreadboardView) return;			// these only really exist in breadboard view

	// female connectors are equivalent to sticky

	QPointF fromTerminalPoint = from->sceneAdjustedTerminalPoint();
	QPointF toTerminalPoint = to->sceneAdjustedTerminalPoint();

	this->setPos(this->pos() + fromTerminalPoint - toTerminalPoint);
	updateConnections();
}

void PaletteItemBase::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) {
	if (lowerConnectorLayerVisible(this)) {
		DebugDialog::debug("PaletteItemBase::hoverEnterEvent isn't obsolete");
		event->ignore();
		return;
	}

	ItemBase::hoverEnterEvent(event);
}


void PaletteItemBase::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
	if (lowerConnectorLayerVisible(this)) {
		DebugDialog::debug("PaletteItemBase::contextMenuEvent isn't obsolete");
		event->ignore();
		return;
	}

	ItemBase::contextMenuEvent(event);
}


LayerKinPaletteItem *PaletteItemBase::newLayerKinPaletteItem(
	PaletteItemBase * chief, ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier,
	const ViewGeometry & viewGeometry, long id,ViewLayer::ViewLayerID viewLayerID, QMenu* itemMenu, const LayerHash & viewLayers)
{
	LayerKinPaletteItem *lk = new
                LayerKinPaletteItem(chief, modelPart, viewIdentifier, viewGeometry, id, itemMenu);
	lk->init(viewLayerID, viewLayers);
	return lk;
}

/*

void PaletteItemBase::setPos(const QPointF & pos) {
	ItemBase::setPos(pos);
	DebugDialog::debug(QString("set pos %1 %2, %3").arg(this->id()).arg(pos.x()).arg(pos.y()) );
}

void PaletteItemBase::setPos(qreal x, qreal y) {
	ItemBase::setPos(x, y);
}
*/

