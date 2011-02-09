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

#include "capacitor.h"


#include "../utils/textutils.h"
#include "../utils/focusoutcombobox.h"
#include "../utils/boundedregexpvalidator.h"
#include "../sketch/infographicsview.h"

#include <QDomNodeList>
#include <QDomDocument>
#include <QDomElement>
#include <QMultiHash>

QHash <QString, PropertyDef *> PropertyDefs;
QHash <QString, InstanceDef *> InstanceDefs;

// TODO
//	save into parts bin

Capacitor::Capacitor( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	if (PropertyDefs.size() == 0) {
		loadPropertyDefs();
	}

	initPropertyDefs();
}

Capacitor::~Capacitor() {
}

void Capacitor::loadPropertyDefs() {
	QFile file(":/resources/properties.xml");

	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument domDocument;
	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		DebugDialog::debug(QString("failed loading capacitor properties %1 line:%2 col:%3").arg(errorStr).arg(errorLine).arg(errorColumn));
		return;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull()) return;
	if (root.tagName() != "properties") return;

	QDomElement propertyElement = root.firstChildElement("property");
	while (!propertyElement.isNull()) {
		PropertyDef * propertyDef = new PropertyDef;
		propertyDef->name = propertyElement.attribute("name");
		propertyDef->id = propertyElement.attribute("id");
		propertyDef->symbol = propertyElement.attribute("symbol");
		propertyDef->minValue = propertyElement.attribute("minValue").toDouble();
		propertyDef->maxValue = propertyElement.attribute("maxValue").toDouble();
		propertyDef->defaultValue = propertyElement.attribute("defaultValue").toDouble();
		QDomElement menuItem = propertyElement.firstChildElement("menuItem");
		while (!menuItem.isNull()) {
			propertyDef->menuItems.append(menuItem.attribute("value").toDouble());
			menuItem = menuItem.nextSiblingElement("menuItem");
		}
		PropertyDefs.insert(propertyDef->id, propertyDef);

		propertyElement = propertyElement.nextSiblingElement("property");
	}

	QDomElement instanceElement = root.firstChildElement("instance");
	while (!instanceElement.isNull()) {
		InstanceDef * instanceDef = new InstanceDef;
		instanceDef->moduleID = instanceElement.attribute("moduleID");
		InstanceDefs.insert(instanceDef->moduleID, instanceDef);
		QDomElement propertyElement = instanceElement.firstChildElement("property");
		while (!propertyElement.isNull()) {
			PropertyDef * propertyDef = PropertyDefs.value(propertyElement.attribute("id"), NULL);
			if (propertyDef) {
				instanceDef->propertyDefs.append(propertyDef);
			}
			propertyElement = propertyElement.nextSiblingElement("property");
		}

		instanceElement = instanceElement.nextSiblingElement("instance");
	}

}

void Capacitor::initPropertyDefs() {
	InstanceDef * instanceDef = InstanceDefs.value(moduleID(), NULL);
	if (instanceDef == NULL) return;

	foreach (PropertyDef * propertyDef, instanceDef->propertyDefs) {
		QString defaultValue = TextUtils::convertToPowerPrefix(propertyDef->defaultValue) + propertyDef->symbol;
		QString savedValue = modelPart()->prop(propertyDef->name).toString();
		if (savedValue.isEmpty()) {
			savedValue = modelPart()->properties().value(savedValue, defaultValue);
			modelPart()->setProp(propertyDef->name, savedValue);
		}
		m_propertyDefs.insert(propertyDef, savedValue);
	}
}

ItemBase::PluralType Capacitor::isPlural() {
	return Plural;
}

bool Capacitor::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget)
{
	foreach (PropertyDef * propertyDef, m_propertyDefs.keys()) {
		if (prop.compare(propertyDef->name, Qt::CaseInsensitive) == 0) {
			returnProp = TranslatedPropertyNames.value(prop);
			if (returnProp.isEmpty()) {
				returnProp = propertyDef->name;
			}

			FocusOutComboBox * focusOutComboBox = new FocusOutComboBox();
			focusOutComboBox->setEnabled(swappingEnabled);
			focusOutComboBox->setEditable(true);
			QString current = m_propertyDefs.value(propertyDef);
			qreal val = TextUtils::convertFromPowerPrefixU(current, propertyDef->symbol);
			if (!propertyDef->menuItems.contains(val)) {
				propertyDef->menuItems.append(val);
			}
			foreach(qreal q, propertyDef->menuItems) {
				QString s = TextUtils::convertToPowerPrefix(q) + propertyDef->symbol;
				focusOutComboBox->addItem(s);
			}
			int ix = focusOutComboBox->findText(current);
			if (ix < 0) {
				focusOutComboBox->addItem(current);
				ix = focusOutComboBox->findText(current);
			}
			focusOutComboBox->setCurrentIndex(ix);
			BoundedRegExpValidator * validator = new BoundedRegExpValidator(focusOutComboBox);
			validator->setSymbol(propertyDef->symbol);
			validator->setConverter(TextUtils::convertFromPowerPrefix);
			validator->setBounds(propertyDef->minValue, propertyDef->maxValue);
			QString pattern = QString("((\\d{1,3})|(\\d{1,3}\\.)|(\\d{1,3}\\.\\d{1,2}))[%1]{0,1}[%2]{0,1}")
											.arg(TextUtils::PowerPrefixesString)
											.arg(propertyDef->symbol);
			validator->setRegExp(QRegExp(pattern));
			focusOutComboBox->setValidator(validator);
			connect(focusOutComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(propertyEntry(const QString &)));

			focusOutComboBox->setMaximumWidth(100);

			this->m_comboBoxes.insert(propertyDef, focusOutComboBox);
						
			returnWidget = focusOutComboBox;	

			return true;
		}
	}

	return PaletteItem::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

void Capacitor::propertyEntry(const QString & text) {
	FocusOutComboBox * focusOutComboBox = qobject_cast<FocusOutComboBox *>(sender());
	if (focusOutComboBox == NULL) return;

	foreach (PropertyDef * propertyDef, m_comboBoxes.keys()) {
		if (m_comboBoxes.value(propertyDef) == focusOutComboBox) {
			QString utext = text;
			qreal val = TextUtils::convertFromPowerPrefixU(utext, propertyDef->symbol);
			if (!propertyDef->menuItems.contains(val)) {
				// info view is redrawn, so combobox is recreated, so the new item is added to the combo box menu
				propertyDef->menuItems.append(val);
			}

			InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
			if (infoGraphicsView != NULL) {
				infoGraphicsView->setProp(this, propertyDef->name, "", m_propertyDefs.value(propertyDef, ""), utext, true);
			}
			break;
		}
	}
}

void Capacitor::setProp(const QString & prop, const QString & value) {
	foreach (PropertyDef * propertyDef, m_propertyDefs.keys()) {
		if (prop.compare(propertyDef->name, Qt::CaseInsensitive) == 0) {
			m_propertyDefs.insert(propertyDef, value);
			modelPart()->setProp(propertyDef->name, value);
			return;
		}
	}

	PaletteItem::setProp(prop, value);
}

