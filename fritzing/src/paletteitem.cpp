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



#include "paletteitem.h"
#include "debugdialog.h"
#include "viewgeometry.h"
#include "infographicsview.h"
#include "layerkinpaletteitem.h"
#include "fsvgrenderer.h"
#include "layerattributes.h"
#include "connectorshared.h"
#include "labels/partlabel.h"
#include "commands.h"
#include "connectoritem.h"

#include <math.h>

#include <QGraphicsSceneMouseEvent>
#include <QSvgRenderer>
#include <QtDebug>
#include <QPainter>
#include <QDomElement>
#include <QDir>
#include <QMessageBox>

PaletteItem::PaletteItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItemBase(modelPart, viewIdentifier, viewGeometry, id, itemMenu)
{
	if(doLabel) {
		m_partLabel = new PartLabel(this, "", NULL);
		m_partLabel->setVisible(false);
	} else {
		m_partLabel = NULL;
	}
}

PaletteItem::~PaletteItem() {
	if (m_partLabel) {
		delete m_partLabel;
	}
}

bool PaletteItem::renderImage(ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const LayerHash & viewLayers, ViewLayer::ViewLayerID viewLayerID, bool doConnectors) {
	Q_UNUSED(viewLayerID);

	bool result = setUpImage(modelPart, viewIdentifier, viewLayers, viewLayerID, doConnectors);

	m_syncMoved = this->pos();
	return result;
}

void PaletteItem::loadLayerKin( const LayerHash & viewLayers) {

	if (m_modelPart == NULL) return;

	ModelPartShared * modelPartShared = m_modelPart->modelPartShared();
	if (modelPartShared == NULL) return;
	if (modelPartShared->domDocument() == NULL) return;

	qint64 id = m_id + 1;
	ViewGeometry viewGeometry = m_viewGeometry;
	viewGeometry.setZ(-1);

	// the palette item already used the zeroth child "layer" element
	foreach (ViewLayer::ViewLayerID viewLayerID, viewLayers.keys()) {
		if (viewLayerID == m_viewLayerID) continue;

		LayerKinPaletteItem * lkpi = newLayerKinPaletteItem(this, m_modelPart, m_viewIdentifier, viewGeometry, id, viewLayerID, m_itemMenu, viewLayers);
		if (lkpi->ok()) {
			DebugDialog::debug(QString("add layer kin %1 %2").arg(id).arg(m_viewIdentifier) );
			addLayerKin(lkpi);
			id++;
		}
		else {
			delete lkpi;
		}
	}
}

void PaletteItem::addLayerKin(LayerKinPaletteItem * lkpi) {
	m_layerKin.append(lkpi);
}

void PaletteItem::removeLayerKin() {
	// assumes paletteitem is still in scene
	for (int i = 0; i < m_layerKin.size(); i++) {
		DebugDialog::debug(QString("removing kin %1 %2").arg(m_layerKin[i]->id()).arg(m_layerKin[i]->z()));
		this->scene()->removeItem(m_layerKin[i]);
		delete m_layerKin[i];
	}

	m_layerKin.clear();
}

void PaletteItem::syncKinSelection(bool selected, PaletteItemBase * originator) {
	PaletteItemBase::syncKinSelection(selected, originator);

	foreach (ItemBase * lkpi, m_layerKin) {
		if (lkpi != originator && lkpi->isSelected() != selected) {
				qobject_cast<LayerKinPaletteItem *>(lkpi)->blockItemSelectedChange(selected);
				lkpi->setSelected(selected);
		}
	}

	if (this != originator && this->isSelected() != selected) {
		this->blockItemSelectedChange(selected);
		this->setSelected(selected);
	}
}

QVariant PaletteItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	//DebugDialog::debug(QString("chief item change %1 %2").arg(this->id()).arg(change));
	if (m_layerKin.count() > 0) {
	    if (change == ItemSelectedChange) {
	       	bool selected = value.toBool();
	    	if (m_blockItemSelectedChange && m_blockItemSelectedValue == selected) {
	    		m_blockItemSelectedChange = false;
	   		}
			else {
	       		syncKinSelection(selected, this);
			}
	    }
	    //else if (change == ItemVisibleHasChanged && value.toBool()) {
	    	//this->setSelected(syncSelected());
	    	//this->setPos(m_offset + syncMoved());
	    //}
	    else if (change == ItemPositionHasChanged) {
	    	this->syncKinMoved(this->m_offset, value.toPointF());
	   	}
   	}

	if (m_partLabel && m_partLabel->initialized()) {
		if (change == ItemPositionHasChanged) {
	    	m_partLabel->ownerMoved(value.toPointF());
	   	}
		else if (change == ItemSelectedChange) {
			m_partLabel->update();
		}
	}

    return PaletteItemBase::itemChange(change, value);
}

const QList<class ItemBase *> & PaletteItem::layerKin()
{
	return m_layerKin;
}

void PaletteItem::rotateItem(qreal degrees) {
	PaletteItemBase::rotateItem(degrees);
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->rotateItem(degrees);
	}
}

void PaletteItem::flipItem(Qt::Orientations orientation) {
	PaletteItemBase::flipItem(orientation);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->flipItem(orientation);
	}
}

void PaletteItem::setTransforms() {
	setTransform(getViewGeometry().transform());
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->setTransform(m_layerKin[i]->getViewGeometry().transform());
	}
}

void PaletteItem::moveItem(ViewGeometry & viewGeometry) {
	PaletteItemBase::moveItem(viewGeometry);
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->moveItem(viewGeometry);
	}
}

void PaletteItem::setItemPos(QPointF & loc) {
	PaletteItemBase::setItemPos(loc);
	for (int i = 0; i < m_layerKin.count(); i++) {
		m_layerKin[i]->setItemPos(loc);
	}
}

void PaletteItem::updateConnections() {
	updateConnectionsAux();
	foreach (ItemBase * lkpi, m_layerKin) {
		qobject_cast<LayerKinPaletteItem *>(lkpi)->updateConnectionsAux();
	}
}

void PaletteItem::collectFemaleConnectees(QSet<ItemBase *> & items) {
	PaletteItemBase::collectFemaleConnectees(items);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->collectFemaleConnectees(items);
	}
}

void PaletteItem::collectWireConnectees(QSet<Wire *> & wires) {
	PaletteItemBase::collectWireConnectees(wires);
	foreach (ItemBase * lkpi, m_layerKin) {
		qobject_cast<LayerKinPaletteItem *>(lkpi)->collectWireConnectees(wires);
	}
}

void PaletteItem::mousePressEvent(PaletteItemBase * originalItem, QGraphicsSceneMouseEvent *event) {
	//DebugDialog::debug("layerkinchief got mouse press event");
	/*

	if (acceptsMousePressConnectorEvent(NULL, event) && isBuriedConnectorHit(event)  ) return;
	foreach(LayerKinPaletteItem * lkpi, m_layerKin) {
		if (lkpi->isBuriedConnectorHit(event)) return;
	}
	*/

	PaletteItemBase::mousePressEvent(originalItem, event);
}

void PaletteItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	InfoGraphicsView *infographics = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
	if (infographics != NULL && infographics->spaceBarIsPressed()) { 
		event->ignore();
		return;
	}

	if (isLowerConnectorLayerVisible(this)) {
		event->ignore();
		return;
	}

	mousePressEvent(this, event);
}


void PaletteItem::syncKinMoved(QPointF offset, QPointF newPos) {
	Q_UNUSED(offset);    // ignore offset--should all be zeros now

	//DebugDialog::debug(QString("sync kin moved %1 %2").arg(offset.x()).arg(offset.y()) );
	//m_syncMoved = pos - offset;
	//if (newPos != pos()) {
		setPos(newPos);
		foreach (ItemBase * lkpi, m_layerKin) {
			lkpi->setPos(newPos);
		}
	//}
}

void PaletteItem::setInstanceTitle(const QString& title) {
	ItemBase::setInstanceTitle(title);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->setInstanceTitle(title);
	}
}

void PaletteItem::updateTooltip() {
	ItemBase::updateTooltip();
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->updateTooltip();
	}
}

QString PaletteItem::family() {
	return modelPartShared()->family();
}

bool PaletteItem::swap(ModelPart* newModelPart, const LayerHash &layerHash, bool reinit, SwapCommand * swapCommand) {
	bool sameFamily = family() == newModelPart->modelPartShared()->family();
	if(sameFamily) {
		prepareGeometryChange();				// important to call this when QGraphicsItem boundary changes: seems to prevent crashes in the backingstore
		invalidateConnectors();
		clearBusConnectorItems();

		if (reinit) {
			m_modelPart->copy(newModelPart);
			m_modelPart->initConnectors(true);
		}

		QHash<ViewLayer::ViewLayerID,bool> layersVisibility = cleanupLayerKin();
		renderImage(m_modelPart,m_viewIdentifier,layerHash,m_viewLayerID,true);

		loadLayerKin(layerHash);
		updateLayerKinVisibility(layersVisibility);

		cleanupConnectors(swapCommand);
		updateTooltip();
		setConnectorTooltips();

		figureHover();


		scene()->update();
	}
	return sameFamily;
}

QHash<ViewLayer::ViewLayerID,bool> PaletteItem::cleanupLayerKin() {
	QHash<ViewLayer::ViewLayerID,bool> layersVisibility;

	foreach (ItemBase * lkpi, layerKin()) {
		layersVisibility[lkpi->viewLayerID()] = lkpi->isVisible();
		this->scene()->removeItem(lkpi);
		delete lkpi;
	}
	m_layerKin.clear();

	return layersVisibility;
}

void PaletteItem::updateLayerKinVisibility(QHash<ViewLayer::ViewLayerID,bool> layersVisibility) {
	foreach (ItemBase * lkpi, layerKin()) {
		this->scene()->addItem(lkpi);
		lkpi->setVisible(layersVisibility[lkpi->viewLayerID()]);
	}
}

void PaletteItem::invalidateConnectors() {
	foreach(QGraphicsItem* child, childItems()) {
		ConnectorItem *conn = dynamic_cast<ConnectorItem*>(child);
		if(conn) {
			conn->setDirty(true);
		}
	}
}

void PaletteItem::cleanupConnectors(SwapCommand * swapCommand) {
	QList<ConnectorItem*> newOnes;
	QHash<QString /*name*/, ConnectorItem*> oldOnes;

	foreach(QGraphicsItem* child, childItems()) {
		ConnectorItem *conn = dynamic_cast<ConnectorItem*>(child);
		if(conn) {
			if(conn->isDirty()) {
				oldOnes.insert(conn->connector()->connectorShared()->name(), conn);
			} else {
				newOnes << conn;
				conn->prepareGeometryChange();
			}
		}
	}

	foreach(ConnectorItem* newOne, newOnes) {
		QString name = newOne->connector()->connectorShared()->name();
		ConnectorItem *oldOne = oldOnes.value(name, NULL);
		if(oldOne) {
			foreach(ConnectorItem* oldConnectedTo, oldOne->connectedToItems()) {
				oldOne->tempRemove(oldConnectedTo, true);
				oldConnectedTo->tempRemove(oldOne, true);
				newOne->tempConnectTo(oldConnectedTo, true);
				oldConnectedTo->tempConnectTo(newOne, true);

				//newOne->attachedMoved();
			}
		} else {
			// nothing to do with this one
		}
	}

	// not working old ones
	bool removed = false;
	foreach(QString name, oldOnes.keys()) {
		ConnectorItem *toRemove = oldOnes.value(name, NULL);
		if(toRemove) {
			foreach(ConnectorItem* toDisconnect, toRemove->connectedToItems()) {
				if (swapCommand != NULL) {
					swapCommand->addDisconnect(toRemove, toDisconnect);
					removed = true;
				}
				else {
					//toRemove->tempRemove(toDisconnect, true);
					//toDisconnect->tempRemove(toRemove, true);
				}
			}
		}
	}

	foreach (ConnectorItem * oldOne, oldOnes.values()) {
		scene()->removeItem(oldOne);
		Connector * connector = oldOne->connector();
		delete oldOne;
		if (connector->connectorItemCount() == 0) {
			// delete the connector if all connectorItems being deleted are deleted
			// each connectorItem will remove itself from the connector's list of connectorItems
			delete connector;
		}
	}


	if (removed) {
		swapCommand->addAfterDisconnect();
	}

	updateConnections();
}

void PaletteItem::setHidden(bool hide) {
	ItemBase::setHidden(hide);
	figureHover();
}

void PaletteItem::figureHover() {
	// if a layer contains connectors, make it the one that accepts hover events
	// if you make all layers accept hover events, then the topmost layer will get the event
	// and lower layers won't

	QList<ItemBase *> allKin;
	allKin.append(this);
	foreach(ItemBase * lkpi, m_layerKin) {
		allKin.append(lkpi);
	}

	qSort(allKin.begin(), allKin.end(), ItemBase::zLessThan);
	foreach (ItemBase * base, allKin) {
		base->setAcceptHoverEvents(false);
	}

	int ix = 0;
	foreach (ItemBase * base, allKin) {
		if (!base->hidden() && base->hasConnectors()) {
			base->setAcceptHoverEvents(true);
			break;
		}
		ix++;
	}

	for (int i = 0; i < ix; i++) {
		ItemBase * base = allKin[i];
		if (!base->hidden()) {
			base->setAcceptHoverEvents(true);
			return;
		}
	}

	/* 
	foreach (PaletteItemBase * base, allKin) {
		base->setAcceptHoverEvents(false);
	}

	foreach (PaletteItemBase * base, allKin) {
		if (!base->hidden() && base->hasConnectors()) {
			base->setAcceptHoverEvents(true);
			return;
		}
	}

	foreach (PaletteItemBase * base, allKin) {
		if (!base->hidden()) {
			base->setAcceptHoverEvents(true);
			return;
		}
	}
	*/

}

void PaletteItem::clearModelPart() {
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->setModelPart(NULL);
	}
	ItemBase::clearModelPart();
}

bool PaletteItem::isLowerConnectorLayerVisible(PaletteItemBase * paletteItemBase) {
	if (m_layerKin.count() == 0) return false;

	if ((paletteItemBase != this) 
		&& this->isVisible() 
		&& (!this->hidden()) && (this->zValue() < paletteItemBase->zValue())
		&& this->hasConnectors()) 
	{
		return true;
	}

	foreach (ItemBase * lkpi, m_layerKin) {
		if (lkpi == paletteItemBase) continue;

		if (lkpi->isVisible() 
			&& (!lkpi->hidden()) 
			&& (lkpi->zValue() < paletteItemBase->zValue()) 
			&& lkpi->hasConnectors() ) 
		{
			return true;
		}
	}

	return false;
}

void PaletteItem::resetID() {
	ItemBase::resetID();
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->resetID();
	}
}
