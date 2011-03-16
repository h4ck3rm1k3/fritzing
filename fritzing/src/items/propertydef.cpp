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

#include "propertydef.h"
#include "../debugdialog.h"
#include "../model/modelpart.h"
#include "../utils/textutils.h"

#include <QDomNodeList>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>

QHash <QString, PropertyDef *> PropertyDefMaster::PropertyDefs;
QHash <QString, InstanceDef *> PropertyDefMaster::InstanceDefs;


void PropertyDefMaster::loadPropertyDefs() {
	QFile file(":/resources/properties.xml");

	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument domDocument;
	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		DebugDialog::debug(QString("failed loading properties %1 line:%2 col:%3").arg(errorStr).arg(errorLine).arg(errorColumn));
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
		propertyDef->editable = propertyElement.attribute("editable", "").compare("yes") == 0;
		propertyDef->numeric = propertyElement.attribute("numeric", "").compare("yes") == 0;
		QDomElement menuItem = propertyElement.firstChildElement("menuItem");
		while (!menuItem.isNull()) {
			QString val = menuItem.attribute("value");
			if (propertyDef->numeric) {
				propertyDef->menuItems.append(val.toDouble());
			}
			else {
				propertyDef->sMenuItems.append(val);
			}
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

void PropertyDefMaster::initPropertyDefs(ModelPart * modelPart, QHash<PropertyDef *, QString> & propertyDefs) 
{

	if (PropertyDefs.count() == 0) {
		loadPropertyDefs();
	}

	InstanceDef * instanceDef = InstanceDefs.value(modelPart->moduleID(), NULL);
	if (instanceDef == NULL) {
		DebugDialog::debug("instancedef missing; check properties.xml file");
		return;
	}

	foreach (PropertyDef * propertyDef, instanceDef->propertyDefs) {
		QString defaultValue = TextUtils::convertToPowerPrefix(propertyDef->defaultValue) + propertyDef->symbol;
		QString savedValue = modelPart->prop(propertyDef->name).toString();
		if (savedValue.isEmpty()) {
			savedValue = modelPart->properties().value(propertyDef->name.toLower(), defaultValue);
			modelPart->setProp(propertyDef->name, savedValue);
		}
		// caches the current value
		propertyDefs.insert(propertyDef, savedValue);
	}
}



