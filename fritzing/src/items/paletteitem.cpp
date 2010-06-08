/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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
#include "../debugdialog.h"
#include "../viewgeometry.h"
#include "../sketch/infographicsview.h"
#include "layerkinpaletteitem.h"
#include "../fsvgrenderer.h"
#include "partlabel.h"
#include "../commands.h"
#include "../connectors/connectoritem.h"


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
	m_blockSyncKinMoved = false;
	if(doLabel) {
		m_partLabel = new PartLabel(this, NULL);
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
	bool result = setUpImage(modelPart, viewIdentifier, viewLayers, viewLayerID, this->viewLayerSpec(), doConnectors);

	m_syncMoved = this->pos();
	return result;
}

void PaletteItem::loadLayerKin(const LayerHash & viewLayers, ViewLayer::ViewLayerSpec viewLayerSpec) {

	if (m_modelPart == NULL) return;

	ModelPartShared * modelPartShared = m_modelPart->modelPartShared();
	if (modelPartShared == NULL) return;
	if (modelPartShared->domDocument() == NULL) return;

	qint64 id = m_id + 1;
	ViewGeometry viewGeometry = m_viewGeometry;
	viewGeometry.setZ(-1);

	LayerList notLayers;
	switch (viewLayerSpec) {
		case ViewLayer::ThroughHoleThroughTop_OneLayer:
			if (m_modelPart->flippedSMD()) {
				notLayers << ViewLayer::Copper1 << ViewLayer::Copper1Trace << ViewLayer::Silkscreen1 << ViewLayer::Silkscreen1Label;
			}
			//notLayers << ViewLayer::Silkscreen0 << ViewLayer::Silkscreen0Label << ViewLayer::Copper1 << ViewLayer:: Copper1Trace;
			break;
		case ViewLayer::ThroughHoleThroughTop_TwoLayers:
			if (m_modelPart->flippedSMD()) {
				notLayers << ViewLayer::Copper0 << ViewLayer::Copper0Trace << ViewLayer::Silkscreen0 << ViewLayer::Silkscreen0Label;
			}
			//notLayers << ViewLayer::Silkscreen0 << ViewLayer::Silkscreen0Label;
			break;
		case ViewLayer::ThroughHoleThroughBottom_TwoLayers:
			//notLayers << ViewLayer::Silkscreen << ViewLayer::SilkscreenLabel;
			break;

		case ViewLayer::GroundPlane_Top:
			notLayers << ViewLayer::GroundPlane0;
			break;
		case ViewLayer::GroundPlane_Bottom:
			notLayers << ViewLayer::GroundPlane1;
			break;

		// not sure these ever get used...
		case ViewLayer::SMDOnTop_TwoLayers:
			notLayers << ViewLayer::Copper0 << ViewLayer::Copper0Trace << ViewLayer::Silkscreen0 << ViewLayer::Silkscreen0Label;
			break;
		case ViewLayer::SMDOnBottom_TwoLayers:
		case ViewLayer::SMDOnBottom_OneLayer:
			notLayers << ViewLayer::Copper1 << ViewLayer::Copper1Trace << ViewLayer::Silkscreen1 << ViewLayer::Silkscreen1Label;
			break;

		case ViewLayer::WireOnTop_TwoLayers:
		case ViewLayer::WireOnBottom_OneLayer:
		case ViewLayer::WireOnBottom_TwoLayers:
			DebugDialog::debug("bad view spec in LoadLayerKin");
			break;
	}

	foreach (ViewLayer::ViewLayerID viewLayerID, viewLayers.keys()) {
		if (viewLayerID == m_viewLayerID) continue;
		if (notLayers.contains(viewLayerID)) continue;
		if (!m_modelPart->hasViewFor(m_viewIdentifier, viewLayerID)) continue;

		LayerKinPaletteItem * lkpi = newLayerKinPaletteItem(this, m_modelPart, m_viewIdentifier, viewGeometry, id, viewLayerID, viewLayerSpec, m_itemMenu, viewLayers);
		if (lkpi->ok()) {
			DebugDialog::debug(QString("adding layer kin %1 %2 %3").arg(id).arg(m_viewIdentifier).arg(viewLayerID) );
			lkpi->setViewLayerSpec(viewLayerSpec);
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

void PaletteItem::transformItem2(const QMatrix & matrix) {
	PaletteItemBase::transformItem2(matrix);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->transformItem2(matrix);
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
		lkpi->updateConnectionsAux();
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
	InfoGraphicsView *infographics = InfoGraphicsView::getInfoGraphicsView(this);
	if (infographics != NULL && infographics->spaceBarIsPressed()) { 
		event->ignore();
		return;
	}

	if (lowerConnectorLayerVisible(this)) {
		DebugDialog::debug("PaletteItem::mousePressEvent isn't obsolete");
		event->ignore();
		return;
	}

	mousePressEvent(this, event);
}


void PaletteItem::syncKinMoved(QPointF offset, QPointF newPos) {
	Q_UNUSED(offset);    // ignore offset--should all be zeros now

	if (m_blockSyncKinMoved) return;

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

void PaletteItem::setHidden(bool hide) {
	ItemBase::setHidden(hide);
	figureHover();
}

void PaletteItem::figureHover() {
	// if a layer contains connectors, make it the one that accepts hover events
	// if you make all layers accept hover events, then the topmost layer will get the event
	// and lower layers won't

	// note: this affects which layer responds to tooltips: see FGraphicsScene::helpEvent()

	QList<ItemBase *> allKin;
	allKin.append(this);
	foreach(ItemBase * lkpi, m_layerKin) {
		allKin.append(lkpi);
	}

	qSort(allKin.begin(), allKin.end(), ItemBase::zLessThan);
	foreach (ItemBase * base, allKin) {
		base->setAcceptHoverEvents(false);
		base->setAcceptedMouseButtons(Qt::NoButton);
	}

	int ix = 0;
	foreach (ItemBase * base, allKin) {
		if (!base->hidden() && base->hasConnectors()) {
			base->setAcceptHoverEvents(true);
			base->setAcceptedMouseButtons(ALLMOUSEBUTTONS);
			break;
		}
		ix++;
	}

	for (int i = 0; i < ix; i++) {
		ItemBase * base = allKin[i];
		if (!base->hidden()) {
			base->setAcceptHoverEvents(true);
			base->setAcceptedMouseButtons(ALLMOUSEBUTTONS);
			return;
		}
	}
}

void PaletteItem::clearModelPart() {
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->setModelPart(NULL);
	}
	ItemBase::clearModelPart();
}

ItemBase * PaletteItem::lowerConnectorLayerVisible(ItemBase * itemBase) {
	if (m_layerKin.count() == 0) return NULL;

	if ((itemBase != this) 
		&& this->isVisible() 
		&& (!this->hidden()) && (this->zValue() < itemBase->zValue())
		&& this->hasConnectors()) 
	{
		return this;
	}

	foreach (ItemBase * lkpi, m_layerKin) {
		if (lkpi == itemBase) continue;

		if (lkpi->isVisible() 
			&& (!lkpi->hidden()) 
			&& (lkpi->zValue() < itemBase->zValue()) 
			&& lkpi->hasConnectors() ) 
		{
			return lkpi;
		}
	}

	return NULL;
}

void PaletteItem::resetID() {
	ItemBase::resetID();
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->resetID();
	}
}

void PaletteItem::blockSyncKinMoved(bool block) {
	m_blockSyncKinMoved = block;
}

void PaletteItem::slamZ(qreal z) {
	PaletteItemBase::slamZ(z);
	foreach (ItemBase * lkpi, m_layerKin) {
		lkpi->slamZ(z);
	}
}
