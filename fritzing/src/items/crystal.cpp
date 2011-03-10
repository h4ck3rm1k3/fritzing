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

#include "crystal.h"


#include "../utils/focusoutcombobox.h"
#include "../utils/boundedregexpvalidator.h"
#include "../sketch/infographicsview.h"
#include "../utils/textutils.h"

// TODO
//	save into parts bin

Crystal::Crystal( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	PropertyDefMaster::initPropertyDefs(modelPart, m_propertyDefs);
}

Crystal::~Crystal() {
}

ItemBase::PluralType Crystal::isPlural() {
	return Plural;
}

bool Crystal::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget)
{
	foreach (PropertyDef * propertyDef, m_propertyDefs.keys()) {
		if (prop.compare(propertyDef->name, Qt::CaseInsensitive) == 0) {
			returnProp = TranslatedPropertyNames.value(prop);
			if (returnProp.isEmpty()) {
				returnProp = propertyDef->name;
			}

			FocusOutComboBox * focusOutComboBox = new FocusOutComboBox();
			focusOutComboBox->setEnabled(swappingEnabled);
			focusOutComboBox->setEditable(false);
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
			connect(focusOutComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(propertyEntry(const QString &)));

			focusOutComboBox->setMaximumWidth(100);

			this->m_comboBoxes.insert(propertyDef, focusOutComboBox);
						
			returnWidget = focusOutComboBox;	

			return true;
		}
	}

	return PaletteItem::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

void Crystal::propertyEntry(const QString & text) {
	FocusOutComboBox * focusOutComboBox = qobject_cast<FocusOutComboBox *>(sender());
	if (focusOutComboBox == NULL) return;

	foreach (PropertyDef * propertyDef, m_comboBoxes.keys()) {
		if (m_comboBoxes.value(propertyDef) == focusOutComboBox) {
			InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
			if (infoGraphicsView != NULL) {
				infoGraphicsView->setProp(this, propertyDef->name, "", m_propertyDefs.value(propertyDef, ""), text, true);
			}
			break;
		}
	}
}

void Crystal::setProp(const QString & prop, const QString & value) {
	foreach (PropertyDef * propertyDef, m_propertyDefs.keys()) {
		if (prop.compare(propertyDef->name, Qt::CaseInsensitive) == 0) {
			m_propertyDefs.insert(propertyDef, value);
			modelPart()->setProp(propertyDef->name, value);
			return;
		}
	}

	PaletteItem::setProp(prop, value);
}

