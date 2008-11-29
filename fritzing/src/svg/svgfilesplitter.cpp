/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-08 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "svgfilesplitter.h"

#include "../misc.h"
#include "../debugdialog.h"
#include "svgpathparser.h"
#include "svgpathlexer.h"

#include <QDomDocument>
#include <QFile>
#include <QtDebug>

SvgFileSplitter::SvgFileSplitter()
{
}

bool SvgFileSplitter::split(const QString & filename, const QString & elementID)
{
	m_byteArray.clear();

	QFile file(filename);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		return false;
	}

	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!m_domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		return false;
	}

	QDomElement root = m_domDocument.documentElement();
	if (root.isNull()) {
		return false;
	}

	if (root.tagName() != "svg") {
		return false;
	}

	QDomElement element = findElementWithAttribute(root, "id", elementID);
	if (element.isNull()) return false;

	while (!root.firstChild().isNull()) {
		root.removeChild(root.firstChild());
	}

	root.appendChild(element);
	m_byteArray = m_domDocument.toByteArray();

	return true;
}

QDomElement SvgFileSplitter::findElementWithAttribute(QDomElement element, const QString & attributeName, const QString & attributeValue) {
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

const QByteArray & SvgFileSplitter::byteArray() {
	return m_byteArray;
}

const QDomDocument & SvgFileSplitter::domDocument() {
	return m_domDocument;
}

bool SvgFileSplitter::normalize(qreal dpi, const QString & elementID) 
{
	// get the viewbox and the width and height
	// then normalize them
	// then normalize all the internal stuff
	// if there are transitions, we're fucked
	
	QDomElement root = m_domDocument.documentElement();
	QString swidthStr = root.attribute("width");
	if (swidthStr.isEmpty()) return false;

	QString sheightStr = root.attribute("height");
	if (sheightStr.isEmpty()) return false;

	bool ok;
	qreal sWidth = convertToInches(swidthStr, &ok);
	if (!ok) return false;

	qreal sHeight = convertToInches(sheightStr, &ok);
	if (!ok) return false;

	root.setAttribute("width", QString::number(sWidth));
	root.setAttribute("height", QString::number(sHeight));

	// assume that if there's no viewBox, the viewbox is at the right dpi?
	// or should the assumption be 90 or 100?
	qreal vbWidth = sWidth * dpi;
	qreal vbHeight = sHeight * dpi;

	QString sviewboxStr = root.attribute("viewBox");
	if (!sviewboxStr.isEmpty()) {
		QStringList strings = sviewboxStr.split(" ");
		if (strings.size() == 4) {
			qreal tempWidth = strings[2].toDouble(&ok);
			if (ok) {
				vbWidth = tempWidth;
			}

			qreal tempHeight= strings[3].toDouble(&ok);
			if (ok) {
				vbHeight = tempHeight;
			}
		}
	}

	root.setAttribute("viewBox", QString("%1 %2 %3 %4").arg(0).arg(0).arg(vbWidth).arg(vbHeight) );

	QDomElement mainElement = findElementWithAttribute(root, "id", elementID);
	if (mainElement.isNull()) return false;

	QDomElement childElement = mainElement.firstChildElement();
	while (!childElement.isNull()) {
		normalizeChild(childElement, sWidth * dpi, sHeight * dpi, vbWidth, vbHeight);
		childElement = childElement.nextSiblingElement();
	}

	return true;
}

void SvgFileSplitter::normalizeChild(QDomElement & element, 
									 qreal sNewWidth, qreal sNewHeight,
									 qreal vbWidth, qreal vbHeight)
{
	
	if (element.nodeName().compare("circle") == 0) {
		normalizeAttribute(element, "cx", sNewWidth, vbWidth);
		normalizeAttribute(element, "cy", sNewHeight, vbHeight);
		normalizeAttribute(element, "r", sNewWidth, vbWidth);
		normalizeAttribute(element, "stroke-width", sNewWidth, vbWidth);
		element.setAttribute("stroke", "black");
	}
	else if (element.nodeName().compare("line") == 0) {
		normalizeAttribute(element, "x1", sNewWidth, vbWidth);
		normalizeAttribute(element, "y1", sNewHeight, vbHeight);
		normalizeAttribute(element, "x2", sNewWidth, vbWidth);
		normalizeAttribute(element, "y2", sNewHeight, vbHeight);
		normalizeAttribute(element, "stroke-width", sNewWidth, vbWidth);
		element.setAttribute("stroke", "black");
	}
	else if (element.nodeName().compare("rect") == 0) {
		normalizeAttribute(element, "width", sNewWidth, vbWidth);
		normalizeAttribute(element, "height", sNewHeight, vbHeight);
		normalizeAttribute(element, "x", sNewWidth, vbWidth);
		normalizeAttribute(element, "y", sNewHeight, vbHeight);
		normalizeAttribute(element, "stroke-width", sNewWidth, vbWidth);
		element.setAttribute("stroke", "black");
	}
	else if (element.nodeName().compare("polygon") == 0) {
		DebugDialog::debug("svg polygon not yet implemented");
	}
	else if (element.nodeName().compare("path") == 0) {
		// if stroke attribute is not empty make it black
		// if fill attribute is not empty and not "none" make it black

		QString data = element.attribute("d");
		if (!data.isEmpty()) {
			SVGPathLexer lexer(data);
			SVGPathParser parser;
			if (!parser.parse(&lexer)) {
				DebugDialog::debug("svg path parse failed");
			}
			else {
				foreach (QVariant variant, parser.symStack()) {
					if (variant.type() == QVariant::Char) {
						qDebug() << variant.toChar();
					}
					else if (variant.type() == QVariant::Double) {
						qDebug() << variant.toDouble();
					}
				}
			}
		}
	}
	else {
		QDomElement childElement = element.firstChildElement();
		while (!childElement.isNull()) {
			normalizeChild(childElement, sNewWidth, sNewHeight, vbWidth, vbHeight);
			childElement = childElement.nextSiblingElement();
		}
	}
}

bool SvgFileSplitter::normalizeAttribute(QDomElement & element, const char * attributeName, qreal num, qreal denom) 
{
	qreal n = element.attribute(attributeName).toDouble() * num / denom;
	element.setAttribute(attributeName, QString::number(n)); 
	return true;
}

QString SvgFileSplitter::shift(qreal x, qreal y, const QString & elementID) 
{
	QDomElement root = m_domDocument.documentElement();

	QDomElement mainElement = findElementWithAttribute(root, "id", elementID);
	if (mainElement.isNull()) return false;

	QDomElement childElement = mainElement.firstChildElement();
	while (!childElement.isNull()) {
		shiftChild(childElement, x, y);
		childElement = childElement.nextSiblingElement();
	}

	QDomDocument document;
	QDomNode node = document.importNode(mainElement, true);
	document.appendChild(node);

	return document.toString();

}

void SvgFileSplitter::shiftChild(QDomElement & element, qreal x, qreal y)
{	
	if (element.nodeName().compare("circle") == 0) {
		shiftAttribute(element, "cx", x);
		shiftAttribute(element, "cy", y);
	}
	else if (element.nodeName().compare("line") == 0) {
		shiftAttribute(element, "x1", x);
		shiftAttribute(element, "y1", y);
		shiftAttribute(element, "x2", x);
		shiftAttribute(element, "y2", y);
	}
	else if (element.nodeName().compare("rect") == 0) {
		shiftAttribute(element, "x", x);
		shiftAttribute(element, "y", y);
	}
	else {
		QDomElement childElement = element.firstChildElement();
		while (!childElement.isNull()) {
			shiftChild(childElement, x, y);
			childElement = childElement.nextSiblingElement();
		}
	}
}

bool SvgFileSplitter::shiftAttribute(QDomElement & element, const char * attributeName, qreal d) 
{
	qreal n = element.attribute(attributeName).toDouble() + d;
	element.setAttribute(attributeName, QString::number(n)); 
	return true;
}