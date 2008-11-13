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

$Revision: 1485 $:
$Author: cohen@irascible.com $:
$Date: 2008-11-13 12:08:31 +0100 (Thu, 13 Nov 2008) $

********************************************************************/

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


