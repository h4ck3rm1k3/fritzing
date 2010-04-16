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

$Revision: 4116 $:
$Author: cohen@irascible.com $:
$Date: 2010-04-15 15:12:52 +0200 (Thu, 15 Apr 2010) $

********************************************************************/



#include "syntaxer.h"
#include "../debugdialog.h"

#include <QRegExp>
#include <QXmlStreamReader>

bool Syntaxer::loadSyntax(const QString &filename)
 {

    QFile file(filename);

	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument domDocument;
	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		return false;
	}

	return false;

 }

QString Syntaxer::parseForName(const QString & filename)
{
	QFile file(filename);
	QXmlStreamReader xml(&file);
    xml.setNamespaceProcessing(false);

	bool inName = false;
	while (!xml.atEnd()) {
        switch (xml.readNext()) {
			case QXmlStreamReader::StartElement:
				if (xml.name().toString().compare("name") == 0) {
					inName = true;
					break;
				}
				if (inName && xml.isCharacters()) {
					return xml.text().toString();
				}

				break;
			case QXmlStreamReader::EndElement:
				if (xml.name().toString().compare("name") == 0) {
					// bail out
					return "";
				}
			default:
				break;
		}
	}

	return "";
}
