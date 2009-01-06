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



#include "partinstancestuff.h"

PartInstanceStuff::PartInstanceStuff() {
	m_domDocument = NULL;
	m_title = QString::null;
}

PartInstanceStuff::PartInstanceStuff(QDomDocument * domDocument, const QString & path) {
	m_domDocument = domDocument;
	Q_UNUSED(path);
}

void PartInstanceStuff::loadText(QDomElement parent, QString tagName, QString &field) {
	QDomElement tagElement = parent.firstChildElement(tagName);
	if (!tagElement.isNull()) {
		field = tagElement.text();
	}
}

const QString & PartInstanceStuff::title() {
	return m_title;
}
void PartInstanceStuff::setTitle(QString title) {
	m_title = title;
}
