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

#include "layerkinpaletteitem.h"
#include "infographicsview.h"
#include "debugdialog.h"

LayerKinPaletteItem::LayerKinPaletteItem(PaletteItemBase * chief, ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu* itemMenu)
	: PaletteItemBase(modelPart, viewIdentifier, viewGeometry, id, itemMenu)

{
    m_layerKinChief = chief;
    setFlags(QGraphicsItem::ItemIsSelectable);
    m_modelPart->removeViewItem(this);  // we don't need to save layerkin
}

void LayerKinPaletteItem::init(ViewLayer::ViewLayerID viewLayerID, const LayerHash & viewLayers) {
	m_ok = setUpImage(m_modelPart, m_viewIdentifier, viewLayers, viewLayerID, true);
	//DebugDialog::debug(QString("lk accepts hover %1 %2 %3 %4 %5").arg(m_modelPart->title()).arg(m_viewIdentifier).arg(m_id).arg(viewLayerID).arg(this->acceptHoverEvents()));
}

QVariant LayerKinPaletteItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	//DebugDialog::debug(QString("lk item change %1 %2").arg(this->id()).arg(change));
	if (m_layerKinChief != NULL) {
	    if (change == ItemSelectedChange) {
	       	bool selected = value.toBool();
	    	if (m_blockItemSelectedChange && m_blockItemSelectedValue == selected) {
	    		m_blockItemSelectedChange = false;
	   		}
			else {
	        	m_layerKinChief->syncKinSelection(selected, this);
	       	}
	    }
	    //else if (change == ItemVisibleHasChanged && value.toBool()) {
	    	//this->setSelected(m_layerKinChief->syncSelected());
	    	//this->setPos(m_offset + m_layerKinChief->syncMoved());
	    //}
	    else if (change == ItemPositionHasChanged) {
	    	m_layerKinChief->syncKinMoved(this->m_offset, value.toPointF());
	   	}
   	}
    return PaletteItemBase::itemChange(change, value);
}

void LayerKinPaletteItem::setOffset(qreal x, qreal y) {
	m_offset.setX(x);
	m_offset.setY(y);
	this->setPos(this->pos() + m_offset);
}

ItemBase * LayerKinPaletteItem::layerKinChief() {
	return m_layerKinChief;
}

bool LayerKinPaletteItem::ok() {
	return m_ok;
}

void LayerKinPaletteItem::updateConnections() {
	m_layerKinChief->updateConnections();
}

void LayerKinPaletteItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	//DebugDialog::debug("layer kin mouse press event");
	if (m_layerKinChief->isLowerConnectorLayerVisible(this)) {
		event->ignore();
		return;
	}

	m_layerKinChief->mousePressEvent(this, event);
}

void LayerKinPaletteItem::setHidden(bool hide) {
	ItemBase::setHidden(hide);
	m_layerKinChief->figureHover();
}

void LayerKinPaletteItem::figureHover() {
}


void LayerKinPaletteItem::clearModelPart() {
	m_layerKinChief->clearModelPart();
}

bool LayerKinPaletteItem::isLowerConnectorLayerVisible(PaletteItemBase * paletteItemBase) {
	return m_layerKinChief->isLowerConnectorLayerVisible(paletteItemBase);
}

QString LayerKinPaletteItem::toolTip2() {
	return m_layerKinChief->toolTip2();
}

bool LayerKinPaletteItem::stickyEnabled(ItemBase * stickTo) {
	return m_layerKinChief->stickyEnabled(stickTo);
}

bool LayerKinPaletteItem::sticky() {
	return m_layerKinChief->sticky();
}

void LayerKinPaletteItem::setSticky(bool s) 
{
	m_layerKinChief->setSticky(s);
}

void LayerKinPaletteItem::addSticky(ItemBase * sticky, bool stickem) {
	m_layerKinChief->addSticky(sticky, stickem);
}

void LayerKinPaletteItem::saveStickyOffsets(QGraphicsSceneMouseEvent *event) {
	m_layerKinChief->saveStickyOffsets(event);
}

ItemBase * LayerKinPaletteItem::stuckTo() {
	return m_layerKinChief->stuckTo();
}

QHash<ItemBase *, QPointF> & LayerKinPaletteItem::sticking() {
	return m_layerKinChief->sticking();
}

bool LayerKinPaletteItem::alreadySticking(ItemBase * itemBase) {
	return m_layerKinChief->alreadySticking(itemBase);
}

void LayerKinPaletteItem::resetID() {
	long offset = m_id % ModelPart::indexMultiplier;
	ItemBase::resetID();
	m_id += offset;
}
