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

static int MinSipPins = 2;
static int MaxSipPins = 64;
static int MinDipPins = 4;
static int MaxDipPins = 64;

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

QString Dip::genSipFZP(const QString & moduleid)
{
	QString SipFzpTemplate = "";
	QString SipConnectorFzpTemplate = "";

	if (SipFzpTemplate.isEmpty()) {
		QFile file(":/resources/templates/generic_sip_fzpTemplate.txt");
		file.open(QFile::ReadOnly);
		SipFzpTemplate = file.readAll();
		file.close();
	}
	if (SipConnectorFzpTemplate.isEmpty()) {
		QFile file(":/resources/templates/generic_sip_connectorFzpTemplate.txt");
		file.open(QFile::ReadOnly);
		SipConnectorFzpTemplate = file.readAll();
		file.close();
	}

	QStringList ss = moduleid.split("_");
	int count = 0;
	foreach (QString s, ss) {
		bool ok;
		int c = s.toInt(&ok);
		if (ok) {
			count = c;
			break;
		}
	}

	if (count > MaxSipPins || count < MinSipPins) return "";

	QString middle;

	for (int i = 0; i < count; i++) {
		middle += SipConnectorFzpTemplate.arg(i).arg(i + 1);
	}

	return SipFzpTemplate.arg(count).arg(middle);
}

QString Dip::genDipFZP(const QString & moduleid)
{
	QString DipFzpTemplate = "";
	QString DipConnectorFzpTemplate = "";

	if (DipFzpTemplate.isEmpty()) {
		QFile file(":/resources/templates/generic_dip_fzpTemplate.txt");
		file.open(QFile::ReadOnly);
		DipFzpTemplate = file.readAll();
		file.close();
	}
	if (DipConnectorFzpTemplate.isEmpty()) {
		QFile file(":/resources/templates/generic_sip_connectorFzpTemplate.txt");
		file.open(QFile::ReadOnly);
		DipConnectorFzpTemplate = file.readAll();
		file.close();
	}

	QStringList ss = moduleid.split("_");
	int count = 0;
	foreach (QString s, ss) {
		bool ok;
		int c = s.toInt(&ok);
		if (ok) {
			count = c;
			break;
		}
	}

	if (count > MaxDipPins || count < MinDipPins) return "";

	QString middle;

	for (int i = 0; i < count; i++) {
		middle += DipConnectorFzpTemplate.arg(i).arg(i + 1);
	}

	return DipFzpTemplate.arg(count).arg(middle);
}

QStringList Dip::collectValues(const QString & family, const QString & prop, QString & value) {
	if (prop.compare("pins", Qt::CaseInsensitive) == 0) {
		QStringList values;
		value = modelPart()->properties().value("pins");

		if (isDIP()) {
			for (int i = MinDipPins; i <= MaxDipPins; i += 2) {
				values << QString::number(i);
			}
		}
		else {
			for (int i = MinSipPins; i <= MaxSipPins; i++) {
				values << QString::number(i);
			}
		}
		
		return values;
	}

	return MysteryPart::collectValues(family, prop, value);
}
