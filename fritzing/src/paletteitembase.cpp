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

#include "paletteitembase.h"
#include "infographicsview.h"
#include "busconnectoritem.h"
#include "debugdialog.h"
#include "rendererviewthing.h"
#include "connectorviewthing.h"
#include "svgfilesplitter.h"
#include "connectorstuff.h"
#include "layerattributes.h"

#include <math.h>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QDir>
#include <QMessageBox>


QString PaletteItemBase::SvgFilesDir = "svg";

PaletteItemBase::PaletteItemBase(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool topLevel ) :
	ItemBase(modelPart, viewIdentifier, viewGeometry, id, topLevel, itemMenu)
{
	m_syncSelected = false;
	m_offset.setX(0);
	m_offset.setY(0);
 	m_blockItemSelectedChange = false;
	this->setPos(viewGeometry.loc());
    setFlags(QGraphicsItem::ItemIsSelectable);
    setAcceptsHoverEvents(true);
}

QRectF PaletteItemBase::boundingRect() const
{
    return QRectF(0, 0, m_size.width(), m_size.height());
}

QPainterPath PaletteItemBase::shape() const
{
	// figure out real shape of svg
    QPainterPath path;
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
     return QGraphicsItem::itemChange(change, value);
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

void PaletteItemBase::rotateItem(qreal degrees) {
	transformItem(QTransform().rotate(degrees));
}

void PaletteItemBase::flipItem(Qt::Orientations orientation) {
	int xScale; int yScale;
	if(orientation == Qt::Vertical) {
		xScale = 1;
		yScale = -1;
	} else if(orientation == Qt::Horizontal) {
		xScale = -1;
		yScale = 1;
	}
	else {
		return;
	}

	transformItem(QTransform().scale(xScale,yScale));
}

void PaletteItemBase::transformItem(QTransform currTransf) {
	QRectF rect = this->boundingRect();
	qreal x = rect.width() / 2;
	qreal y = rect.height() / 2;
	QTransform transf = QTransform().translate(-x, -y) * currTransf * QTransform().translate(x, y);
	getViewGeometry().setTransform(getViewGeometry().transform()*transf);
	this->setTransform(getViewGeometry().transform());
	updateConnections();
	update();
}

void PaletteItemBase::saveInstanceLocation(QXmlStreamWriter & streamWriter)
{
	streamWriter.writeAttribute("x", QString::number(m_viewGeometry.loc().x()));
	streamWriter.writeAttribute("y", QString::number(m_viewGeometry.loc().y()));
	if (!m_viewGeometry.transform().isIdentity()) {
		streamWriter.writeStartElement("transform");
		streamWriter.writeAttribute("m11", QString::number(m_viewGeometry.transform().m11()));
		streamWriter.writeAttribute("m12", QString::number(m_viewGeometry.transform().m12()));
		streamWriter.writeAttribute("m13", QString::number(m_viewGeometry.transform().m13()));
		streamWriter.writeAttribute("m21", QString::number(m_viewGeometry.transform().m21()));
		streamWriter.writeAttribute("m22", QString::number(m_viewGeometry.transform().m22()));
		streamWriter.writeAttribute("m23", QString::number(m_viewGeometry.transform().m23()));
		streamWriter.writeAttribute("m31", QString::number(m_viewGeometry.transform().m31()));
		streamWriter.writeAttribute("m32", QString::number(m_viewGeometry.transform().m32()));
		streamWriter.writeAttribute("m33", QString::number(m_viewGeometry.transform().m33()));
		streamWriter.writeEndElement();
	}

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
	if (m_viewIdentifier != ItemBase::PCBView) {
		return true;
	}

	if ((event->modifiers() & Qt::ShiftModifier) == 0) {
		// force a shift-click to drag out a wire in pcb view
		return false;
	}

	return true;
}


void PaletteItemBase::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	ItemBase::mousePressEvent(event);
	if (this->itemType() != ModelPart::Breadboard) {
		foreach (QGraphicsItem * childItem, childItems()) {
			ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
			if (connectorItem == NULL) continue;

			connectorItem->setOverConnectorItem(NULL);
		}
	}

	if (m_sticky) {
		m_stickyPos = event->scenePos();
		foreach (ItemBase * itemBase, m_stickyList.keys()) {
			m_stickyList.insert(itemBase, itemBase->pos());
		}
	}
}

void PaletteItemBase::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	QPointF oldPos = pos();
	ItemBase::mouseMoveEvent(event);
	QPointF newPos = pos();


	if (newPos != oldPos) {
		// newPos == oldPos occurs when the mouse is out of the sketch view

		updateConnections();
		findConnectorsUnder();
		if (m_sticky) {
			QPointF d = event->scenePos() - m_stickyPos;
			foreach (ItemBase * itemBase, m_stickyList.keys()) {
				if (!itemBase->stickyEnabled(this)) continue;

				QPointF p = m_stickyList.value(itemBase) + d ;
				itemBase->setItemPos(p);
				itemBase->updateConnections();
			}
		}
	}
}

void PaletteItemBase::findConnectorsUnder() {
	if (itemType() == ModelPart::Breadboard || itemType() == ModelPart::Board) {
		// don't try to map connectors when we drag a breadboard: it's too damn slow
		return;
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
	updateConnections();
	for (int i = 0; i < childItems().count(); i++) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItems()[i]);
		if (connectorItem == NULL) continue;

		ConnectorItem * to = connectorItem->overConnectorItem();
		if (to != NULL) {
			to->connectorHover(this, false);
			connectorItem->setOverConnectorItem(NULL);   // clean up
			this->layerKinChief()->sendConnectionChangedSignal(connectorItem, to, true);
		}
	}
}


void PaletteItemBase::updateConnectionsAux() {
	//DebugDialog::debug("update connections");
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * item = dynamic_cast<ConnectorItem *>(childItem);
		if (item == NULL) continue;

		updateConnections(item);
	}
}

void PaletteItemBase::updateConnections(ConnectorItem * item) {
	item->attachedMoved();
}


bool PaletteItemBase::setUpImage(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID viewLayerID, bool doConnectors)
{
	LayerAttributes layerAttributes;
	QSvgRenderer * renderer = PaletteItemBase::setUpImage(modelPart, viewIdentifier, viewLayerID, layerAttributes);
	if (renderer == NULL) {
		return false;
	}

	m_canFlipVertical = layerAttributes.canFlipVertical();
	m_canFlipHorizontal = layerAttributes.canFlipHorizontal();
	m_filename = layerAttributes.filename();
	//DebugDialog::debug(QString("filename %1").arg(m_filename) );
	m_sticky = layerAttributes.sticky();
	QString elementID = layerAttributes.layerName();
	setViewLayerID(elementID, viewLayers);
	this->setZValue(this->z());
	this->setSharedRenderer(renderer);
	//this->setElementId(elementID);

	m_size = renderer->defaultSize();
	//DebugDialog::debug(QString("default size %1 %2 %3").arg(m_size.width()).arg(m_size.height()).arg(elementID) );

	m_svg = true;

	if (doConnectors) {
		setUpConnectors(renderer, modelPart->ignoreTerminalPoints());
	}

	if (!m_viewGeometry.transform().isIdentity()) {
		setTransform(m_viewGeometry.transform());
		update();
	}

	return true;
}

QSvgRenderer * PaletteItemBase::setUpImage(ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID viewLayerID, LayerAttributes & layerAttributes)
{
    ModelPartStuff * modelPartStuff = modelPart->modelPartStuff();

    if (modelPartStuff == NULL) return NULL;
    if (modelPartStuff->domDocument() == NULL) return NULL;

	bool result = layerAttributes.getSvgElementID(modelPartStuff->domDocument(), viewIdentifier, viewLayerID);
	if (!result) return NULL;

	//DebugDialog::debug(tr("setting z %1 %2")
		//.arg(this->z())
		//.arg(ViewLayer::viewLayerNameFromID(viewLayerID))  );

	if (modelPartStuff->viewThing() == NULL) {
		modelPartStuff->setViewThing(new RendererViewThing());
	}

	RendererViewThing * viewThing = dynamic_cast<RendererViewThing *>(modelPartStuff->viewThing());
	QSvgRenderer * renderer = viewThing->get(viewLayerID /* viewIdentifier */);			// all view layers now share the same renderer
	if (renderer == NULL) {

		QString tempPath;
		if(modelPartStuff->path() != ___emptyString___) {
			QDir dir(modelPartStuff->path());			// is a path to a filename
			dir.cdUp();									// lop off the filename
			dir.cdUp();									// parts root
			tempPath = dir.absolutePath() + "/" + PaletteItemBase::SvgFilesDir +"/%1/" + layerAttributes.filename();
		} else { // for fake models
			tempPath = getApplicationSubFolderPath("parts") +"/"+ PaletteItemBase::SvgFilesDir +"/%1/"+ layerAttributes.filename();
		}

    	renderer = new QSvgRenderer();
    	QStringList possibleFolders;
    	possibleFolders << "core" << "contrib" << "user";
		bool gotOne = false;
    	for(int i=0; i < possibleFolders.size(); i++) {
			QString filename = tempPath.arg(possibleFolders[i]);

			if (layerAttributes.multiLayer()) {
				// need to treat create "virtual" svg file for each layer
				SvgFileSplitter svgFileSplitter;
				if (svgFileSplitter.split(filename, layerAttributes.layerName())) {
					if (renderer->load(svgFileSplitter.byteArray())) {
						gotOne = true;
						break;
					}
				}
			}
			else {
				// only one layer, just load it directly
				if (renderer->load(filename)) {
					layerAttributes.setFilename(filename);
					gotOne = true;
					break;
				}
			}
    	}

		if(!gotOne) {
			delete renderer;
			return NULL;
		}


		viewThing->set(viewLayerID /* viewIdentifier */  , renderer);
	}

	return renderer;
}

void PaletteItemBase::setUpConnectors(QSvgRenderer * renderer, bool ignoreTerminalPoints) {
	if (m_modelPart->connectors().count() <= 0) return;

	foreach (Connector * connector, m_modelPart->connectors().values()) {
		if (connector == NULL) continue;

		QRectF connectorRect;
		QPointF terminalPoint;
		bool result = connector->setUpConnector(renderer, m_viewIdentifier, m_viewLayerID, connectorRect, terminalPoint, ignoreTerminalPoints);
		if (!result) continue;

		//DebugDialog::debug(QString("<rect view=\"%6\" id=\"%1pin\" x=\"%2\" y=\"%3\" width=\"%4\" height=\"%5\" />")
						   //.arg(connector->connectorStuffID())
						   //.arg(connectorRect.x()).arg(connectorRect.y())
						   //.arg(connectorRect.width()).arg(connectorRect.height())
						   //.arg(m_viewIdentifier) );
		//DebugDialog::debug(QString("<rect id=\"%1pterminal\" x=\"%2\" y=\"%3\" width=\"%4\" height=\"%5\" />")
						   //.arg(connector->connectorStuffID())
						   //.arg(connectorRect.x() + (connectorRect.width() / 2)).arg(connectorRect.y() + (connectorRect.height() /2))
						   //.arg(0).arg(0) );

		ConnectorItem * connectorItem = newConnectorItem(connector);

		//DebugDialog::debug(	tr("in layer %1 with z %2")
			//.arg(ViewLayer::viewLayerNameFromID(m_viewLayerID))
			//.arg(this->zValue()) );

		connectorItem->setRect(connectorRect);
		connectorItem->setTerminalPoint(terminalPoint);
		//DebugDialog::debug(tr("terminal point %1 %2").arg(terminalPoint.x()).arg(terminalPoint.y()) );

		Bus * bus = connectorItem->bus();
		if (bus != NULL) {
			BusConnectorItem * busConnectorItem = m_busConnectorItems.value(bus);
			if (busConnectorItem == NULL) {
				busConnectorItem = new BusConnectorItem(this, bus);
				addBusConnectorItem(bus, busConnectorItem);
				busConnectorItem->setVisible(false);
			}
		}
	}
}


const QString & PaletteItemBase::filename() {
	return m_filename;
}

void PaletteItemBase::connectedMoved(ConnectorItem * from, ConnectorItem * to) {
	if (from->connectorType() != Connector::Female) return;
	if (m_viewIdentifier != ItemBase::BreadboardView) return;			// these only really exist in breadboard view

	QPointF fromTerminalPoint = from->sceneAdjustedTerminalPoint();
	QPointF toTerminalPoint = to->sceneAdjustedTerminalPoint();

	this->setPos(this->pos() + fromTerminalPoint - toTerminalPoint);
	updateConnections();
}

bool PaletteItemBase::isBuriedConnectorHit(QGraphicsSceneMouseEvent *event) {
	if (itemType() == ModelPart::Breadboard) return false;

	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (connectorItem == NULL) continue;

		QPointF p = connectorItem->mapFromScene(event->scenePos());
		QRectF r = connectorItem->rect();
		if (r.contains(p)) {
			mousePressConnectorEvent(connectorItem, event);
			return true;
		}
	}

	return false;
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

