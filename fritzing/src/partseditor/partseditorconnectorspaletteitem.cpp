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

$Revision: 2248 $:
$Author: merunga $:
$Date: 2009-01-22 19:47:17 +0100 (Thu, 22 Jan 2009) $

********************************************************************/


#include "partseditorconnectorspaletteitem.h"
#include "partseditorconnectorsconnectoritem.h"
#include "partseditorconnectorsview.h"
#include "partseditorconnectorslayerkinpaletteitem.h"


PartsEditorConnectorsPaletteItem::PartsEditorConnectorsPaletteItem(PartsEditorConnectorsView *owner, ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier)
	: PartsEditorPaletteItem(owner, modelPart, viewIdentifier)
{
	m_showingTerminalPoints = owner->showingTerminalPoints();
}

PartsEditorConnectorsPaletteItem::PartsEditorConnectorsPaletteItem(PartsEditorConnectorsView *owner, ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, SvgAndPartFilePath *path)
	: PartsEditorPaletteItem(owner, modelPart, viewIdentifier, path)
{
	m_showingTerminalPoints = owner->showingTerminalPoints();
}

void PartsEditorConnectorsPaletteItem::highlightConnectors(const QString &connId) {
	highlightConnsAux(this,connId);
	foreach(ItemBase* item, m_layerKin) {
		highlightConnsAux(item,connId);
	}
}

void PartsEditorConnectorsPaletteItem::highlightConnsAux(ItemBase* item, const QString &connId) {
	foreach(QGraphicsItem * child, item->childItems()) {
		PartsEditorConnectorsConnectorItem * connectorItem
			= dynamic_cast<PartsEditorConnectorsConnectorItem *>(child);
		if (connectorItem == NULL) continue;

		connectorItem->highlight(connId);
	}
}

bool PartsEditorConnectorsPaletteItem::showingTerminalPoints() {
	return dynamic_cast<PartsEditorConnectorsView*>(m_owner)->showingTerminalPoints();
}

ConnectorItem* PartsEditorConnectorsPaletteItem::newConnectorItem(Connector *connector) {
	return new PartsEditorConnectorsConnectorItem(connector,this,m_showingTerminalPoints);
}

LayerKinPaletteItem * PartsEditorConnectorsPaletteItem::newLayerKinPaletteItem(
		PaletteItemBase * chief, ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier,
		const ViewGeometry & viewGeometry, long id,ViewLayer::ViewLayerID viewLayerID, QMenu* itemMenu, const LayerHash & viewLayers)
{
	LayerKinPaletteItem *lk = new
                PartsEditorConnectorsLayerKinPaletteItem(chief, modelPart, viewIdentifier, viewGeometry, id, itemMenu, m_showingTerminalPoints);
	lk->init(viewLayerID, viewLayers);
	return lk;
}
