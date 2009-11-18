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

$Revision: 3406 $:
$Author: cohen@irascible.com $:
$Date: 2009-09-04 12:26:26 +0200 (Fri, 04 Sep 2009) $

********************************************************************/

#include "textutils.h"
#include "misc.h"
#include <QRegExp>

QDomElement TextUtils::findElementWithAttribute(QDomElement element, const QString & attributeName, const QString & attributeValue) {
	if (element.hasAttribute(attributeName)) {
		if (element.attribute(attributeName).compare(attributeValue) == 0) return element;
	}

     for(QDomElement e = element.firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
     {
		 QDomElement result = findElementWithAttribute(e, attributeName, attributeValue);
		 if (!result.isNull()) return result;
     }

	return ___emptyElement___;
}


QSet<QString> TextUtils::getRegexpCaptures(const QString &pattern, const QString &textToSearchIn) {
	QRegExp re(pattern);
	QSet<QString> captures;
	int pos = 0;

	while ((pos = re.indexIn(textToSearchIn, pos)) != -1) {
		captures << re.cap(1);
		pos += re.matchedLength();
	}

	return captures;
}

qreal TextUtils::convertToInches(const QString & s, bool * ok, bool isIllustrator) {
	QString string = s;
	qreal divisor = 1.0;
	if (string.endsWith("cm", Qt::CaseInsensitive)) {
		divisor = 2.54;
		string.chop(2);
	}
	else if (string.endsWith("mm", Qt::CaseInsensitive)) {
		divisor = 25.4;
		string.chop(2);
	}
	else if (string.endsWith("in", Qt::CaseInsensitive)) {
		divisor = 1.0;
		string.chop(2);
	}
	else if (string.endsWith("px", Qt::CaseInsensitive)) {
		divisor = isIllustrator? 72.0: 90.0;
		string.chop(2);
	}
	else {
		divisor = 90.0;			// default to Qt's standard internal units if all else fails
	}

	bool fine;
	qreal result = string.toDouble(&fine);
	if (!fine) {
		if (ok) *ok = false;
		return 0;
	}

	if (ok) *ok = true;
	return result / divisor;
}

QString TextUtils::replaceTextElement(QString svg, const QString & label) {
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument doc;
	if (!doc.setContent(svg, &errorStr, &errorLine, &errorColumn)) return svg;

	QDomElement root = doc.documentElement();
	QDomNodeList domNodeList = root.elementsByTagName("text");
	for (int i = 0; i < domNodeList.count(); i++) {
		QDomElement node = domNodeList.item(i).toElement();
		if (node.isNull()) continue;

		if (node.attribute("id").compare("label") != 0) continue;

		QDomNodeList childList = node.childNodes();
		for (int j = 0; j < childList.count(); j++) {
			QDomNode child = childList.item(i);
			if (child.isText()) {
				child.setNodeValue(label);
				return doc.toString();
			}
		}
	}
		
	return svg;
}

QString TextUtils::mergeSvg(const QString & svg1, const QString & svg2) {
	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument doc1;
	if (!doc1.setContent(svg1, &errorStr, &errorLine, &errorColumn)) return ___emptyString___;

	QDomDocument doc2;
	if (!doc2.setContent(svg2, &errorStr, &errorLine, &errorColumn)) return ___emptyString___;

	QDomElement root1 = doc1.documentElement();
	if (root1.tagName() != "svg") return ___emptyString___;

	QDomElement root2 = doc2.documentElement();
	if (root2.tagName() != "svg") ___emptyString___;

	QDomNode node = root2.firstChild();
	while (!node.isNull()) {
		root1.appendChild(node);
		node = node.nextSibling();
	}

	return doc1.toString();
}


