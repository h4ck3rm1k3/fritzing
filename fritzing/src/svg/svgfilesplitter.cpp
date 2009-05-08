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

#include "svgfilesplitter.h"

#include "../utils/misc.h"
#include "../debugdialog.h"
#include "svgpathparser.h"
#include "svgpathlexer.h"
#include "svgpathrunner.h"

#include <QDomDocument>
#include <QFile>
#include <QtDebug>

static QString findStyle("%1[\\s]*:[\\s]*([^;]*)[;]?");

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

	QString contents = file.readAll();
	file.close();

	// gets rid of some crap inserted by illustrator which can screw up polygons and paths
	contents.remove(QChar('\t'));
	contents.remove(QChar('\n'));
	contents.remove(QChar('\r'));

	QString errorStr;
	int errorLine;
	int errorColumn;

	if (!m_domDocument.setContent(contents, true, &errorStr, &errorLine, &errorColumn)) {
		return false;
	}

	QDomElement root = m_domDocument.documentElement();
	if (root.isNull()) {
		return false;
	}

	if (root.tagName() != "svg") {
		return false;
	}

	root.removeAttribute("space");

	QDomElement element = findElementWithAttribute(root, "id", elementID);
	if (element.isNull()) return false;

	while (!root.firstChild().isNull()) {
		root.removeChild(root.firstChild());
	}

	root.appendChild(element);
	m_byteArray = m_domDocument.toByteArray();

	QString s = m_domDocument.toString();
	//DebugDialog::debug(s);

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

QString SvgFileSplitter::elementString(const QString & elementID) {
	QDomElement root = m_domDocument.documentElement();

	QDomElement mainElement = findElementWithAttribute(root, "id", elementID);
	if (mainElement.isNull()) return ___emptyString___;

	QDomDocument document;
	QDomNode node = document.importNode(mainElement, true);
	document.appendChild(node);

	return document.toString();
}

bool SvgFileSplitter::normalize(qreal dpi, const QString & elementID, bool blackOnly)
{
	// get the viewbox and the width and height
	// then normalize them
	// then normalize all the internal stuff
	// if there are translations, we're fucked

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
	qreal vbWidth = sWidth * 90;
	qreal vbHeight = sHeight * 90;

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
		normalizeChild(childElement, sWidth * dpi, sHeight * dpi, vbWidth, vbHeight, blackOnly);
		childElement = childElement.nextSiblingElement();
	}

	return true;
}

void SvgFileSplitter::normalizeChild(QDomElement & element,
									 qreal sNewWidth, qreal sNewHeight,
									 qreal vbWidth, qreal vbHeight, bool blackOnly)
{

	if (element.nodeName().compare("circle") == 0) {
		fixStyleAttribute(element);
		normalizeAttribute(element, "cx", sNewWidth, vbWidth);
		normalizeAttribute(element, "cy", sNewHeight, vbHeight);
		normalizeAttribute(element, "r", sNewWidth, vbWidth);
		normalizeAttribute(element, "stroke-width", sNewWidth, vbWidth);
		setStrokeOrFill(element, blackOnly);
	}
	else if (element.nodeName().compare("line") == 0) {
		fixStyleAttribute(element);
		normalizeAttribute(element, "x1", sNewWidth, vbWidth);
		normalizeAttribute(element, "y1", sNewHeight, vbHeight);
		normalizeAttribute(element, "x2", sNewWidth, vbWidth);
		normalizeAttribute(element, "y2", sNewHeight, vbHeight);
		normalizeAttribute(element, "stroke-width", sNewWidth, vbWidth);
		setStrokeOrFill(element, blackOnly);
	}
	else if (element.nodeName().compare("rect") == 0) {
		fixStyleAttribute(element);
		normalizeAttribute(element, "width", sNewWidth, vbWidth);
		normalizeAttribute(element, "height", sNewHeight, vbHeight);
		normalizeAttribute(element, "x", sNewWidth, vbWidth);
		normalizeAttribute(element, "y", sNewHeight, vbHeight);
		normalizeAttribute(element, "stroke-width", sNewWidth, vbWidth);

		// rx, ry for rounded rects
		if (!element.attribute("rx").isEmpty()) {
			normalizeAttribute(element, "rx", sNewWidth, vbWidth);
		}
		if (!element.attribute("ry").isEmpty()) {
			normalizeAttribute(element, "ry", sNewHeight, vbHeight);
		}
		setStrokeOrFill(element, blackOnly);
	}
	else if (element.nodeName().compare("ellipse") == 0) {
		fixStyleAttribute(element);
		normalizeAttribute(element, "cx", sNewWidth, vbWidth);
		normalizeAttribute(element, "cy", sNewHeight, vbHeight);
		normalizeAttribute(element, "rx", sNewWidth, vbWidth);
		normalizeAttribute(element, "ry", sNewHeight, vbHeight);
		normalizeAttribute(element, "stroke-width", sNewWidth, vbWidth);
		setStrokeOrFill(element, blackOnly);
	}
	else if (element.nodeName().compare("polygon") == 0 || element.nodeName().compare("polyline") == 0) {
		fixStyleAttribute(element);
		QString data = element.attribute("points");
		if (!data.isEmpty()) {
			data.prepend("M");		// pretend it's a path so we can use the path parser
			data.append("Z");
			const char * slot = SLOT(normalizeCommandSlot(QChar, bool, QList<double> &, void *));
			PathUserData pathUserData;
			pathUserData.sNewHeight = sNewHeight;
			pathUserData.sNewWidth = sNewWidth;
			pathUserData.vbHeight = vbHeight;
			pathUserData.vbWidth = vbWidth;
                        if (parsePath(data, slot, pathUserData, this)) {
				pathUserData.string.remove(0, 1);			// get rid of the "M"
				pathUserData.string.remove(pathUserData.string.length() - 1, 1);
				element.setAttribute("points", pathUserData.string);
			}
		}
		setStrokeOrFill(element, blackOnly);
	}
	else if (element.nodeName().compare("path") == 0) {
		fixStyleAttribute(element);
		setStrokeOrFill(element, blackOnly);
		QString data = element.attribute("d");
		if (!data.isEmpty()) {
			if (!data.endsWith('z', Qt::CaseInsensitive)) {
				data.append("Z");
			}
			const char * slot = SLOT(normalizeCommandSlot(QChar, bool, QList<double> &, void *));
			PathUserData pathUserData;
			pathUserData.sNewHeight = sNewHeight;
			pathUserData.sNewWidth = sNewWidth;
			pathUserData.vbHeight = vbHeight;
			pathUserData.vbWidth = vbWidth;
                        if (parsePath(data, slot, pathUserData, this)) {
				element.setAttribute("d", pathUserData.string);
			}
		}
	}
	else {
		QDomElement childElement = element.firstChildElement();
		while (!childElement.isNull()) {
			normalizeChild(childElement, sNewWidth, sNewHeight, vbWidth, vbHeight, blackOnly);
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
	if (element.nodeName().compare("circle") == 0 || element.nodeName().compare("ellipse") == 0) {
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
	else if (element.nodeName().compare("polygon") == 0 || element.nodeName().compare("polyline") == 0) {
		QString data = element.attribute("points");
		if (!data.isEmpty()) {
			data.prepend("M");		// pretend it's a path so we can use the path parser
			data.append("Z");
			const char * slot = SLOT(shiftCommandSlot(QChar, bool, QList<double> &, void *));
			PathUserData pathUserData;
			pathUserData.x = x;
			pathUserData.y = y;
                        if (parsePath(data, slot, pathUserData, this)) {
				pathUserData.string.remove(0, 1);			// get rid of the "M"
				pathUserData.string.remove(pathUserData.string.length() - 1, 1);
				element.setAttribute("points", pathUserData.string);
			}
		}
	}
	else if (element.nodeName().compare("path") == 0) {
		QString data = element.attribute("d");
		if (!data.isEmpty()) {
			const char * slot = SLOT(shiftCommandSlot(QChar, bool, QList<double> &, void *));
			PathUserData pathUserData;
			pathUserData.x = x;
			pathUserData.y = y;
                        if (parsePath(data, slot, pathUserData, this)) {
				element.setAttribute("d", pathUserData.string);
			}
		}
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

void SvgFileSplitter::normalizeCommandSlot(QChar command, bool relative, QList<double> & args, void * userData) {

	Q_UNUSED(relative);			// just normalizing here, so relative is not used

	PathUserData * pathUserData = (PathUserData *) userData;

	double d;
	pathUserData->string.append(command);
	switch(command.toAscii()) {
		case 'v':
		case 'V':
			for (int i = 0; i < args.count(); i++) {
				d = args[i] * pathUserData->sNewHeight / pathUserData->vbHeight;
				pathUserData->string.append(QString::number(d));
				if (i < args.count() - 1) {
					pathUserData->string.append(',');
				}
			}
			break;
		case 'h':
		case 'H':
			for (int i = 0; i < args.count(); i++) {
				d = args[0] * pathUserData->sNewWidth / pathUserData->vbWidth;
				pathUserData->string.append(QString::number(d));
				if (i < args.count() - 1) {
					pathUserData->string.append(',');
				}
			}
			break;
		default:
			for (int i = 0; i < args.count(); i++) {
				if (i % 2 == 0) {
					d = args[i] * pathUserData->sNewWidth / pathUserData->vbWidth;
				}
				else {
					d = args[i] * pathUserData->sNewHeight / pathUserData->vbHeight;
				}
				pathUserData->string.append(QString::number(d));
				if (i < args.count() - 1) {
					pathUserData->string.append(',');
				}
			}
			break;
	}
}

void SvgFileSplitter::shiftCommandSlot(QChar command, bool relative, QList<double> & args, void * userData) {

	Q_UNUSED(relative);			// just normalizing here, so relative is not used

	PathUserData * pathUserData = (PathUserData *) userData;

	double d;
	pathUserData->string.append(command);
	switch(command.toAscii()) {
		case 'v':
		case 'V':
			d = args[0];
			if (!relative) {
				 d += pathUserData->y;
			}
			pathUserData->string.append(QString::number(d));
			break;
		case 'h':
		case 'H':
			d = args[0];
			if (!relative) {
				 d += pathUserData->x;
			}
			pathUserData->string.append(QString::number(d));
			break;
		default:
			for (int i = 0; i < args.count(); i++) {
				d = args[i];
				if (i % 2 == 0) {
					if (!relative) {
						d += pathUserData->x;
					}
				}
				else {
					if (!relative) {
						d += pathUserData->y;
					}
				}
				pathUserData->string.append(QString::number(d));
				if (i < args.count() - 1) {
					pathUserData->string.append(',');
				}
			}
			break;
	}
}

bool SvgFileSplitter::parsePath(const QString & data, const char * slot, PathUserData & pathUserData, QObject * slotTarget) {
	SVGPathLexer lexer(data);
	SVGPathParser parser;
	if (!parser.parse(&lexer)) {
		DebugDialog::debug(QString("svg path parse failed %1").arg(data));
		return false;
	}

	SVGPathRunner svgPathRunner;
        connect(&svgPathRunner, SIGNAL(commandSignal(QChar, bool, QList<double> &, void *)), slotTarget, slot, Qt::DirectConnection);
	return svgPathRunner.runPath(parser.symStack(), &pathUserData);
}

void SvgFileSplitter::setStrokeOrFill(QDomElement & element, bool blackOnly)
{
	if (!blackOnly) return;

	// if stroke attribute is not empty make it black
	// if fill attribute is not empty and not "none" make it black
	QString stroke = element.attribute("stroke");
	if (!stroke.isEmpty()) {
		element.setAttribute("stroke", "black");
	}
	QString fill = element.attribute("fill");
	if (!fill.isEmpty()) {
		if (fill.compare("none") != 0) {
			element.setAttribute("fill", "black");
		}
	}
}

void SvgFileSplitter::fixStyleAttribute(QDomElement & element)
{
	QString style = element.attribute("style");
	if (style.isEmpty()) return;

	fixStyleAttribute(element, style, "stroke-width");
	fixStyleAttribute(element, style, "stroke");
	fixStyleAttribute(element, style, "fill");

	element.setAttribute("style", style);
}

void SvgFileSplitter::fixStyleAttribute(QDomElement & element, QString & style, const QString & attributeName)
{
	QString str = findStyle.arg(attributeName);
	QRegExp sw(str);
	if (sw.indexIn(style) >= 0) {
		QString value = sw.cap(1);
		style.remove(sw);
		element.setAttribute(attributeName, value);
	}
}

bool SvgFileSplitter::getSvgSizeAttributes(const QString & path, QString & width, QString & height, QString & viewBox)
{

	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument domDocument;

	QFile file(path);
	if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
		return false;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull()) {
		return false;
	}

	if (root.tagName() != "svg") {
		return false;
	}

	width = root.attribute("width");
	if (width.isEmpty()) {
		return false;
	}

	height = root.attribute("height");
	if (height.isEmpty()) {
		return false;
	}

	viewBox = root.attribute("viewBox");
	if (viewBox.isEmpty()) {
		return false;
	}

	return true;
}
