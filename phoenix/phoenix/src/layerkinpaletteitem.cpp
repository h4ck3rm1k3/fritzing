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

void LayerKinPaletteItem::hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) {
	InfoGraphicsView * infoGraphicsView = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
	if (infoGraphicsView != NULL) {
		infoGraphicsView->hoverEnterItem(event, m_layerKinChief);
	}
}

void LayerKinPaletteItem::hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) {
	InfoGraphicsView * infoGraphicsView = dynamic_cast<InfoGraphicsView *>(this->scene()->parent());
	if (infoGraphicsView != NULL) {
		infoGraphicsView->hoverLeaveItem(event, m_layerKinChief);
	}
}
