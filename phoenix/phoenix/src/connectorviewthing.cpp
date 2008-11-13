#include "connectorviewthing.h"


ConnectorViewThing::ConnectorViewThing(  ) 
	: ViewThing()
{
	for (int i = 0; i < ItemBase::ViewCount; i++) {
		for (int j = 0; j < ViewLayer::ViewLayerCount; j++) {
			m_processed[i][j] = 0;
		}
	}
}

void ConnectorViewThing::setVisibleInView(ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID layerID, bool visible) {
	m_visible[viewIdentifier][layerID] = visible;	
}

void ConnectorViewThing::setVisibleInView(ItemBase::ViewIdentifier viewIdentifier, bool visible) {
	for (int i = 0; i < ViewLayer::ViewLayerCount; i++) {		
		m_visible[viewIdentifier][i] = visible;
	}
}

bool ConnectorViewThing::visibleInView(ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID layerID) {
	return m_visible[viewIdentifier][layerID];
}

QRectF ConnectorViewThing::rectInView(ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID layerID) {
	return m_rect[viewIdentifier][layerID];
}

void ConnectorViewThing::setRectInView(ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID layerID, QRectF rect) {
	m_rect[viewIdentifier][layerID] = rect;
	m_visible[viewIdentifier][layerID] = true;	
}

void ConnectorViewThing::setProcessed(ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID layerID, bool processed) {
	m_processed[viewIdentifier][layerID] = processed;
	m_visible[viewIdentifier][layerID] = false;	
}

bool ConnectorViewThing::processed(ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID layerID) {
	return m_processed[viewIdentifier][layerID];
}

QPointF ConnectorViewThing::terminalPointInView(ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID layerID) {
	return m_point[viewIdentifier][layerID];
}

void ConnectorViewThing::setTerminalPointInView(ItemBase::ViewIdentifier viewIdentifier, ViewLayer::ViewLayerID layerID, QPointF point) {
	m_point[viewIdentifier][layerID] = point;
}


