/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2010 Fachhochschule Potsdam - http://fh-potsdam.de

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
#include "moduleidnames.h"
#include "../fsvgrenderer.h"
#include "../utils/textutils.h"
#include "../utils/focusoutcombobox.h"
#include "../sketch/infographicsview.h"
#include "partlabel.h"

#include <QLineEdit>
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
	m_renderer = NULL;

	m_voltageReference = (modelPart->properties().value("type").compare("voltage reference") == 0);

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

	if (!m_voltageReference) return;

	QString svg = makeSvg();
	if (!svg.isEmpty()) {
		if (m_renderer == NULL) {
			m_renderer = new FSvgRenderer(this);
		}
		//DebugDialog::debug(svg);

		bool result = m_renderer->fastLoad(svg.toUtf8());
		if (result) {
			setSharedRendererEx(m_renderer);
		}
	}

    updateTooltip();
    if (m_partLabel) m_partLabel->displayTexts();
}

QString SymbolPaletteItem::makeSvg() {
	QString path = filename();
	QFile file(filename());
	QString svg;
	if (file.open(QFile::ReadOnly)) {
		svg = file.readAll();
		file.close();
		return replaceTextElement(svg);
	}

	return "";
}

QString SymbolPaletteItem::replaceTextElement(QString svg) {
	qreal v = ((int) (m_voltage * 1000)) / 1000.0;
	return TextUtils::replaceTextElement(svg, QString::number(v) + "V");
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

QVariant SymbolPaletteItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (m_voltageReference) {
		switch (change) {
			case ItemSceneHasChanged:
				if (this->scene()) {
					setVoltage(m_voltage);
				}
				break;
			default:
				break;
   		}
	}

    return PaletteItem::itemChange(change, value);
}

QString SymbolPaletteItem::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, SvgFileSplitter *> & svgHash, bool blackOnly, qreal dpi) 
{
	QString svg = PaletteItem::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
	if (m_voltageReference) {
		switch (viewLayerID) {
			case ViewLayer::Schematic:
				return replaceTextElement(svg);
			default:
				break;
		}
	}

	return svg; 
}

bool SymbolPaletteItem::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget)
{
	if ((prop.compare("voltage", Qt::CaseInsensitive) == 0) && 
		(modelPart()->moduleID().compare(ModuleIDNames::groundModuleIDName) != 0)) 
	{

		FocusOutComboBox * edit = new FocusOutComboBox(parent);
		edit->setEnabled(swappingEnabled);
		int ix = 0;
		foreach (qreal v, Voltages) {
			edit->addItem(QString::number(v));
			if (v == m_voltage) {
				edit->setCurrentIndex(ix);
			}
			ix++;
		}

		QDoubleValidator * validator = new QDoubleValidator(edit);
		validator->setRange(-9999.99, 9999.99, 2);
		validator->setNotation(QDoubleValidator::StandardNotation);
		edit->setValidator(validator);

		edit->setMaximumWidth(200);

		connect(edit, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(voltageEntry(const QString &)));
		returnWidget = edit;	

		returnProp = tr("voltage");
		return true;
	}

	return PaletteItem::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

void SymbolPaletteItem::voltageEntry(const QString & text) {
	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->setVoltage(text.toDouble(), true);
	}
}

ItemBase::PluralType SymbolPaletteItem::isPlural() {
	return Singular;
}

