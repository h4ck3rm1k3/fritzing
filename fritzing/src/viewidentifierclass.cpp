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


#include "viewidentifierclass.h"

/////////////////////////////////

class NameTriple {

public:
	NameTriple(const QString & _xmlName, const QString & _viewName, const QString & _naturalName) {
		m_xmlName = _xmlName;
		m_viewName = _viewName;
		m_naturalName = _naturalName;
	}

	QString & xmlName() {
		return m_xmlName;
	}

	QString & viewName() {
		return m_viewName;
	}

	QString & naturalName() {
		return m_naturalName;
	}

protected:
	QString m_xmlName;
	QString m_naturalName;
	QString m_viewName;
};


/////////////////////////////////

QHash <ViewIdentifierClass::ViewIdentifier, NameTriple * > ViewIdentifierClass::names;

QString & ViewIdentifierClass::viewIdentifierName(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
	Q_ASSERT(viewIdentifier >= 0);
	Q_ASSERT(viewIdentifier < ViewIdentifierClass::ViewCount);
	return names[viewIdentifier]->viewName();
}

QString & ViewIdentifierClass::viewIdentifierXmlName(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
	Q_ASSERT(viewIdentifier >= 0);
	Q_ASSERT(viewIdentifier < ViewIdentifierClass::ViewCount);
	return names[viewIdentifier]->xmlName();
}

QString & ViewIdentifierClass::viewIdentifierNaturalName(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
	Q_ASSERT(viewIdentifier >= 0);
	Q_ASSERT(viewIdentifier < ViewIdentifierClass::ViewCount);
	return names[viewIdentifier]->naturalName();
}

void ViewIdentifierClass::initNames() {
	if (names.count() == 0) {
		names.insert(ViewIdentifierClass::IconView, new NameTriple("iconView", QObject::tr("icon view"), "icon"));
		names.insert(ViewIdentifierClass::BreadboardView, new NameTriple("breadboardView", QObject::tr("breadboard view"), "breadboard"));
		names.insert(ViewIdentifierClass::SchematicView, new NameTriple("schematicView", QObject::tr("schematic view"), "schematic"));
		names.insert(ViewIdentifierClass::PCBView, new NameTriple("pcbView", QObject::tr("pcb view"), "pcb"));
	}
}

ViewIdentifierClass::ViewIdentifier ViewIdentifierClass::idFromXmlName(const QString & name) {
	foreach (ViewIdentifier id, names.keys()) {
		NameTriple * nameTriple = names.value(id);
		if (name.compare(nameTriple->xmlName()) == 0) return id;
	}

	return UnknownView;
}

void ViewIdentifierClass::cleanup() {
	foreach (NameTriple * nameTriple, names) {
		delete nameTriple;
	}
	names.clear();
}
