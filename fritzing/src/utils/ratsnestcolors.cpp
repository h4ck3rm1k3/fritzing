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

#include "ratsnestcolors.h"
#include <QFile>

QHash<ViewIdentifierClass::ViewIdentifier, RatsnestColors *> RatsnestColors::m_viewList;
QHash<QString, class RatsnestColor *> RatsnestColors::m_allNames;

QColor ErrorColor(0, 0, 0);

//////////////////////////////////////////////////////

class RatsnestColor {
	RatsnestColor(const QDomElement &);
	~RatsnestColor();

	friend class RatsnestColors;

protected:
	QString m_name;
	QColor m_color;
	QStringList m_connectorNames;
};

//////////////////////////////////////////////////////

RatsnestColor::RatsnestColor(const QDomElement & color) {
	m_name = color.attribute("name");
	m_color.setNamedColor(color.attribute("color"));
	QDomElement connector = color.firstChildElement("connector");
	while (!connector.isNull()) {
		m_connectorNames.append(connector.attribute("name"));
		connector = connector.nextSiblingElement("connector");
	}
}

RatsnestColor::~RatsnestColor() {
}

//////////////////////////////////////////////////////

RatsnestColors::RatsnestColors(const QDomElement & view) 
{
	m_viewIdentifier = ViewIdentifierClass::idFromXmlName(view.attribute("name"));
	m_index = 0;
	QDomElement color = view.firstChildElement("color");
	while (!color.isNull()) {
		RatsnestColor * ratsnestColor = new RatsnestColor(color);
		m_ratsnestColors.append(ratsnestColor);
		foreach (QString name, ratsnestColor->m_connectorNames) {
			m_allNames.insert(name.toLower(), ratsnestColor);
		}
		color = color.nextSiblingElement("color");
	}
}

RatsnestColors::~RatsnestColors()
{
	foreach (RatsnestColor * ratsnestColor, m_ratsnestColors) {
		delete ratsnestColor;
	}
	m_ratsnestColors.clear();
}

void RatsnestColors::initNames() {
	QFile file(":/resources/ratsnestcolors.xml");

	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument domDocument;

	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		return;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull()) {
		return;
	}

	if (root.tagName() != "colors") {
		return;
	}

	QDomElement view = root.firstChildElement("view");
	while (!view.isNull()) {
		RatsnestColors * ratsnestColors = new RatsnestColors(view);
		m_viewList.insert(ratsnestColors->m_viewIdentifier, ratsnestColors);
		view = view.nextSiblingElement("view");
	}
}

void RatsnestColors::cleanup() {
	foreach (RatsnestColors * ratsnestColors, m_viewList.values()) {
		delete ratsnestColors;
	}
	m_viewList.clear();
}

const QColor & RatsnestColors::netColor(ViewIdentifierClass::ViewIdentifier viewIdentifier) {
	RatsnestColors * ratsnestColors = m_viewList.value(viewIdentifier, NULL);
	if (ratsnestColors == NULL) return ErrorColor;

	return ratsnestColors->getNextColor();
}

const QColor & RatsnestColors::getNextColor() {
	if (m_ratsnestColors.length() <= 0) return ErrorColor;

	int resetCount = 0;
	while (true) {
		if (m_index < 0 || m_index >= m_ratsnestColors.length()) {
			m_index = 0;
			if (++resetCount > 2) {
				// prevent infinite loops if all the colors in the list are for particular connectors
				return ErrorColor;
			}
		}
		RatsnestColor * ratsnestColor = m_ratsnestColors[m_index++];
		if (ratsnestColor->m_connectorNames.length() > 0) {
			// don't use colors designated for particular connectors
			m_index++;
		}
		else {
			return ratsnestColor->m_color;
		}
	}
}

	
bool RatsnestColors::findConnectorColor(const QStringList & names, QColor & color) {
	foreach (QString name, names) {
		RatsnestColor * ratsnestColor = m_allNames.value(name.toLower(), NULL);
		if (ratsnestColor == NULL) continue;

		color = ratsnestColor->m_color;
		return true;
	}

	return false;
}

bool RatsnestColors::isConnectorColor(ViewIdentifierClass::ViewIdentifier m_viewIdentifier, const QColor & color) {
	RatsnestColors * ratsnestColors = m_viewList.value(m_viewIdentifier, NULL);
	if (ratsnestColors == NULL) return false;

	foreach (RatsnestColor * ratsnestColor, ratsnestColors->m_ratsnestColors) {
		if (ratsnestColor->m_color == color) {
			return (ratsnestColor->m_connectorNames.length() > 0);
		}
	}

	return false;
}

