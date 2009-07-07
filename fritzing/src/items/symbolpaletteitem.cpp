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

$Revision: 3228 $:
$Author: cohen@irascible.com $:
$Date: 2009-07-01 18:49:09 +0200 (Wed, 01 Jul 2009) $

********************************************************************/

#include "symbolpaletteitem.h"
#include "../debugdialog.h"
#include "../connectoritem.h"

#include <QMultiHash>

static QMultiHash<long, ConnectorItem *> SchemagicBus;

SymbolPaletteItem::SymbolPaletteItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_voltage = modelPart->properties().value("voltage").toDouble() * 100000;
}

SymbolPaletteItem::~SymbolPaletteItem() {
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (connectorItem == NULL) continue;

		SchemagicBus.remove(m_voltage, connectorItem);
	}
}

ConnectorItem* SymbolPaletteItem::newConnectorItem(Connector *connector) 
{
	ConnectorItem * connectorItem = PaletteItemBase::newConnectorItem(connector);
	if (m_viewIdentifier != ViewIdentifierClass::SchematicView) return connectorItem;

	SchemagicBus.insert(m_voltage, connectorItem);
	return connectorItem;
}

void SymbolPaletteItem::busConnectorItems(class Bus * bus, QList<class ConnectorItem *> & items) {
	PaletteItem::busConnectorItems(bus, items);

	if (m_viewIdentifier != ViewIdentifierClass::SchematicView) return;

	QList<ConnectorItem *> mitems = SchemagicBus.values(m_voltage);
	foreach (ConnectorItem * connectorItem, mitems) {
		items.append(connectorItem);
	}
}

