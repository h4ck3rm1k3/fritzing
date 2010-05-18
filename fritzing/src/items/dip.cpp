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

#include "dip.h"

static QStringList Spacings;

Dip::Dip( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: MysteryPart(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
}

Dip::~Dip() {
}

bool Dip::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget)
{
	bool result = MysteryPart::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
	if (prop.compare("chip label", Qt::CaseInsensitive) == 0) {
		returnProp = tr("chip label");
	}
	return result;
}

bool Dip::isDIP() {
	QString package = modelPart()->properties().value("package", "");
	return (package.indexOf("DIP", 0, Qt::CaseInsensitive) >= 0);
}

bool Dip::otherPropsChange(const QMap<QString, QString> & propsMap) {
	QString layout = modelPart()->properties().value("package", "");
	return (layout.compare(propsMap.value("package", "")) != 0);
}

const QStringList & Dip::spacings() {
	if (Spacings.count() == 0) {
		Spacings << "300mil" << "400mil" << "600mil" << "900mil";
	}

	return Spacings;
}
