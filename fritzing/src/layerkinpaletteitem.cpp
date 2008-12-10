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

#include "layerkinpaletteitem.h"
#include "infographicsview.h"
#include "debugdialog.h"

LayerKinPaletteItem::LayerKinPaletteItem(PaletteItemBase * chief, ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id,ViewLayer::ViewLayerID viewLayerID, QMenu* itemMenu, const LayerHash & viewLayers  )
	: PaletteItemBase(modelPart, viewIdentifier, viewGeometry, id, itemMenu, false)

{
	m_layerKinChief = chief;
    setFlags(QGraphicsItem::ItemIsSelectable  | QGraphicsItem::ItemIsMovable );
    m_ok = setUpImage(modelPart, viewIdentifier, viewLayers, viewLayerID, true);
    m_modelPart->removeViewItem(this);  // we don't need to save layerkin
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
	DebugDialog::debug("layer kin mouse press event");
	m_layerKinChief->mousePressEvent(event);
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
