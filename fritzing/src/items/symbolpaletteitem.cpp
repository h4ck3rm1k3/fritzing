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
#include "../connectors/connectoritem.h"
#include "../connectors/bus.h"

#include <QMultiHash>

#define VOLTAGE_HASH_CONVERSION 1000000
#define FROMVOLTAGE(v) ((long) (v * VOLTAGE_HASH_CONVERSION))

static QMultiHash<long, ConnectorItem *> localVoltages;			// Qt doesn't do Hash keys with qreal
static QList<qreal> Voltages;
qreal SymbolPaletteItem::DefaultVoltage = 5;

SymbolPaletteItem::SymbolPaletteItem( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_connector0 = m_connector1 = NULL;
	m_voltage = 0;

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
		temp = modelPart->properties().value("voltage").toDouble(&ok);
		if (ok) {
			m_voltage = SymbolPaletteItem::DefaultVoltage;
		}
		modelPart->setProp("voltage", m_voltage);
	}
	if (!Voltages.contains(m_voltage)) {
		Voltages.append(m_voltage);
	}
}

SymbolPaletteItem::~SymbolPaletteItem() {
	foreach (long key, localVoltages.uniqueKeys()) {
		if (m_connector0) {
			localVoltages.remove(key, m_connector0);
		}
		if (m_connector1) {
			localVoltages.remove(key, m_connector1);
		}
	}
}

void SymbolPaletteItem::removeMeFromBus(qreal v) {
	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (connectorItem == NULL) continue;

		qreal nv = useVoltage(connectorItem);
		if (nv == v) {
			int count = localVoltages.remove(FROMVOLTAGE(v), connectorItem);
			if (count == 0) {
				DebugDialog::debug(QString("removeMeFromBus failed %1 %2 %3 %4")
					.arg(this->id())
					.arg(connectorItem->connectorSharedID())
					.arg(v).arg(nv));
			}
		}
	}
}

ConnectorItem* SymbolPaletteItem::newConnectorItem(Connector *connector) 
{
	ConnectorItem * connectorItem = PaletteItemBase::newConnectorItem(connector);
	if (m_viewIdentifier != ViewIdentifierClass::SchematicView) return connectorItem;

	if (connector->connectorSharedID().compare("connector0") == 0) {
		m_connector0 = connectorItem;
	}
	else if (connector->connectorSharedID().compare("connector1") == 0) {
		m_connector1 = connectorItem;
	}
	else {
		return connectorItem;
	}

	localVoltages.insert(FROMVOLTAGE(useVoltage(connectorItem)), connectorItem);
	return connectorItem;
}

void SymbolPaletteItem::busConnectorItems(Bus * bus, QList<class ConnectorItem *> & items) {
	PaletteItem::busConnectorItems(bus, items);

	if (m_viewIdentifier != ViewIdentifierClass::SchematicView) return;

	qreal v = (bus->id().compare("groundbus", Qt::CaseInsensitive) == 0) ? 0 : m_voltage;
	QList<ConnectorItem *> mitems = localVoltages.values(FROMVOLTAGE(v));
	foreach (ConnectorItem * connectorItem, mitems) {
		if (connectorItem->scene() == this->scene()) {
			items.append(connectorItem);
		}
	}
}

qreal SymbolPaletteItem::voltage() {
	return m_voltage;
}

void SymbolPaletteItem::setProp(const QString & prop, const QString & value) {
	if (prop.compare("voltage", Qt::CaseInsensitive) == 0) {
		setVoltage(value.toDouble());
		return;
	}

	PaletteItem::setProp(prop, value);
}

void SymbolPaletteItem::setVoltage(qreal v) {
	removeMeFromBus(m_voltage);

	m_voltage = v;
	m_modelPart->setProp("voltage", v);
	if (!Voltages.contains(v)) {
		Voltages.append(v);
	}

	if (m_viewIdentifier != ViewIdentifierClass::SchematicView) return;

	foreach (QGraphicsItem * childItem, childItems()) {
		ConnectorItem * connectorItem = dynamic_cast<ConnectorItem *>(childItem);
		if (connectorItem == NULL) continue;

		if (connectorItem->connectorSharedName().compare("GND", Qt::CaseInsensitive) == 0) continue;

		localVoltages.insert(FROMVOLTAGE(v), connectorItem);
	}
}

void SymbolPaletteItem::collectExtraInfoValues(const QString & prop, QString & value, QStringList & extraValues, bool & ignoreValues) {
	ignoreValues = false;

	if (modelPart()->moduleID().compare(ItemBase::groundModuleIDName) == 0) return;

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
	if (modelPart()->moduleID().compare(ItemBase::groundModuleIDName) == 0) return ___emptyString___;

	qreal v = qRound(m_voltage * 100) / 100.0;	// truncate to 2 decimal places
	return QString("&nbsp;<input type='text' name='sVoltage' id='sVoltage' maxlength='8' value='%1' style='width:55px' onblur='setVoltage()' onkeypress='setVoltageEnter(event)' />"
				   "<script language='JavaScript'>lastGoodVoltage=%1;</script>"
				   ).arg(v);

	return ___emptyString___;
}

QString SymbolPaletteItem::getProperty(const QString & key) {
	if (key.compare("voltage", Qt::CaseInsensitive) == 0) {
		return QString::number(m_voltage);
	}

	return PaletteItem::getProperty(key);
}

qreal SymbolPaletteItem::useVoltage(ConnectorItem * connectorItem) {
	return (connectorItem->connectorSharedName().compare("GND", Qt::CaseInsensitive) == 0) ? 0 : m_voltage;
}

ConnectorItem * SymbolPaletteItem::connector0() {
	return m_connector0;
}

ConnectorItem * SymbolPaletteItem::connector1() {
	return m_connector1;
}
