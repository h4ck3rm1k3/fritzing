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

PartsEditorConnectorsPaletteItem::PartsEditorConnectorsPaletteItem(PartsEditorConnectorsView *owner, ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier, StringPair *path, QString layer)
	: PartsEditorPaletteItem(owner, modelPart, viewIdentifier, path, layer )
{
	m_showsTerminalPoints = owner->showingTerminalPoints();
}

PartsEditorConnectorsPaletteItem::PartsEditorConnectorsPaletteItem(PartsEditorConnectorsView *owner, ModelPart * modelPart, ItemBase::ViewIdentifier viewIdentifier)
	: PartsEditorPaletteItem(owner, modelPart, viewIdentifier)
{
	m_showsTerminalPoints = owner->showingTerminalPoints();
}

void PartsEditorConnectorsPaletteItem::highlightConnectors(const QString &connId) {
	for (int i = 0; i < childItems().count(); i++) {
		PartsEditorConnectorsConnectorItem * connectorItem
			= dynamic_cast<PartsEditorConnectorsConnectorItem *>(childItems()[i]);
		if (connectorItem == NULL) continue;

		connectorItem->highlight(connId);
	}
}

bool PartsEditorConnectorsPaletteItem::showingTerminalPoints() {
	return dynamic_cast<PartsEditorConnectorsView*>(m_owner)->showingTerminalPoints();
}

ConnectorItem* PartsEditorConnectorsPaletteItem::newConnectorItem(Connector *connector) {
	return new PartsEditorConnectorsConnectorItem(connector,this,m_showsTerminalPoints);
}
