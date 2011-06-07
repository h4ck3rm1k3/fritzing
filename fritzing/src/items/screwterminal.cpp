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

#include "screwterminal.h"
#include "../utils/graphicsutils.h"
#include "../fsvgrenderer.h"
#include "../sketch/infographicsview.h"
#include "../commands.h"
#include "../utils/textutils.h"
#include "../layerattributes.h"
#include "partlabel.h"

#include <QDomNodeList>
#include <QDomDocument>
#include <QDomElement>
#include <QLineEdit>


static const int MinPins = 2;
static const int MaxPins = 12;
QHash<QString, QString> Spacings;

ScrewTerminal::ScrewTerminal( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
}

ScrewTerminal::~ScrewTerminal() {
}

QStringList ScrewTerminal::collectValues(const QString & family, const QString & prop, QString & value) {
	if (prop.compare("pins", Qt::CaseInsensitive) == 0) {
		QStringList values;
		value = modelPart()->properties().value("pins");

		for (int i = MinPins; i <= MaxPins; i++) {
			values << QString::number(i);
		}
		
		return values;
	}

	return PaletteItem::collectValues(family, prop, value);
}

ItemBase::PluralType ScrewTerminal::isPlural() {
	return Plural;
}

QString ScrewTerminal::genFZP(const QString & moduleid)
{
	initSpacings();

	QStringList pieces = moduleid.split("_");
	if (pieces.count() != 4) return "";

	QString result = PaletteItem::genFZP(moduleid, "screw_terminal_fzpTemplate", MinPins, MaxPins, 1);
	result.replace(".percent.", "%");
	QString spacing = pieces.at(3);
	return result.arg(spacing).arg(Spacings.value(spacing, "")); 
}

QString ScrewTerminal::genModuleID(QMap<QString, QString> & currPropsMap)
{
	initSpacings();

	QString spacing = currPropsMap.value("pin spacing");
	QString pins = currPropsMap.value("pins");

	foreach (QString key, Spacings.keys()) {
		if (Spacings.value(key).compare(spacing, Qt::CaseInsensitive) == 0) {
			return QString("screw_terminal_%1_%2").arg(pins).arg(key);
		}
	}

	return "";
}

void ScrewTerminal::initSpacings() {
	if (Spacings.count() == 0) {
		Spacings.insert("3.5mm", "0.137in (3.5mm)");
		Spacings.insert("100mil", "0.1in (2.54mm)");
		Spacings.insert("200mil", "0.2in (5.08mm)");
		Spacings.insert("300mil", "0.3in (7.62mm)");
	}
}
