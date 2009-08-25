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

#include "symbolpaletteitem.h"
#include "../debugdialog.h"
#include "../connectoritem.h"

#include <QMultiHash>

#define VOLTAGE_HASH_CONVERSION 1000000
#define FROMVOLTAGE(v) ((long) (v * VOLTAGE_HASH_CONVERSION))

static QMultiHash<long, ConnectorItem *> SchemagicBus;			// Qt doesn't do Hash keys with qreal
static QList<qreal> Voltages;

SymbolPaletteItem::SymbolPaletteItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	if (Voltages.count() == 0) {
		Voltages.append(0.0);
		Voltages.append(3.3);
		Voltages.append(5.0);
		Voltages.append(12.0);
	}

	bool ok;
	qreal temp = modelPart->prop("voltage").toDouble(&ok);
	if (ok) {
		m_voltage = temp;
	}
	else {
		if (modelPart->properties().value("symbol").compare("ground") == 0) {
			m_voltage = 0;
		}
		else {
			m_voltage = 5;
		}
		modelPart->setProp("voltage", m_voltage);
	}
	if (!Voltages.contains(m_voltage)) {
		Voltages.append(m_voltage);
	}
}

SymbolPaletteItem::~SymbolPaletteItem() {
	removeMeFromBus();
}

void SymbolPaletteItem::removeMeFromBus() {
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (connectorItem == NULL) continue;

		SchemagicBus.remove(FROMVOLTAGE(m_voltage), connectorItem);
	}
}

ConnectorItem* SymbolPaletteItem::newConnectorItem(Connector *connector) 
{
	ConnectorItem * connectorItem = PaletteItemBase::newConnectorItem(connector);
	if (m_viewIdentifier != ViewIdentifierClass::SchematicView) return connectorItem;

	SchemagicBus.insert(FROMVOLTAGE(m_voltage), connectorItem);
	return connectorItem;
}

void SymbolPaletteItem::busConnectorItems(class Bus * bus, QList<class ConnectorItem *> & items) {
	PaletteItem::busConnectorItems(bus, items);

	if (m_viewIdentifier != ViewIdentifierClass::SchematicView) return;

	QList<ConnectorItem *> mitems = SchemagicBus.values(FROMVOLTAGE(m_voltage));
	foreach (ConnectorItem * connectorItem, mitems) {
		items.append(connectorItem);
	}
}

qreal SymbolPaletteItem::voltage() {
	return m_voltage;
}

void SymbolPaletteItem::setVoltage(qreal v) {
	removeMeFromBus();

	m_voltage = v;
	m_modelPart->setProp("voltage", v);
	if (!Voltages.contains(v)) {
		Voltages.append(v);
	}

	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (connectorItem == NULL) continue;

		SchemagicBus.insert(FROMVOLTAGE(m_voltage), connectorItem);
	}
}

void SymbolPaletteItem::collectExtraInfoValues(const QString & prop, QString & value, QStringList & extraValues, bool & ignoreValues) {
	ignoreValues = false;

	if (prop.compare("voltage", Qt::CaseInsensitive) == 0) {
		ignoreValues = true;
		value = QString::number(m_voltage);
		foreach (qreal v, Voltages) {
			extraValues.append(QString::number(v));
		}
	}
}

QString SymbolPaletteItem::collectExtraInfoHtml(const QString & prop, const QString & value) {
	Q_UNUSED(value);

	if (prop.compare("voltage", Qt::CaseInsensitive) != 0) return ___emptyString___;

	qreal v = qRound(m_voltage * 100) / 100.0;	// truncate to 2 decimal places
	return QString("&nbsp;<input type='text' name='sVoltage' id='sVoltage' maxlength='8' value='%1' style='width:55px' onblur='setVoltage()' onkeypress='setVoltageEnter(event)' />"
				   "<script language='JavaScript'>lastGoodVoltage=%1;</script>"
				   ).arg(v);

	return ___emptyString___;
}

bool SymbolPaletteItem::canChangeVoltage() {
	foreach (ConnectorItem * connectorItem, SchemagicBus.values(FROMVOLTAGE(m_voltage))) {
		if (connectorItem->attachedTo() != this && connectorItem->attachedToItemType() == ModelPart::Symbol) {
			// at least for now, don't allow swapping if another symbol is hooked up
			return false;
		}
	}

	return true;
}
