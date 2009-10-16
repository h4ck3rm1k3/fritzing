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
#include "../utils/textutils.h"
#include "../debugdialog.h"
#include "svgpathparser.h"
#include "svgpathlexer.h"
#include "svgpathrunner.h"

#include <QDomDocument>
#include <QFile>
#include <QtDebug>

static QString findStyle("%1[\\s]*:[\\s]*([^;]*)[;]?");
const QRegExp SvgFileSplitter::sodipodiDetector("((inkscape)|(sodipodi)):[^=\\s]+=\"([^\"\\\\]*(\\\\.[^\"\\\\]*)*)\"");


struct HVConvertData {
	qreal x;
	qreal y;
	qreal subX;
	qreal subY;
	QString path;
};

void appendPair(QString & path, qreal a1, qreal a2) {
	path.append(QString::number(a1));
	path.append(',');
	path.append(QString::number(a2));
	path.append(',');
}

//////////////////////////////////////////////////

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

	return splitString(contents, elementID);
}

bool SvgFileSplitter::splitString(QString & contents, const QString & elementID)
{
	m_byteArray.clear();

	// gets rid of some crap inserted by illustrator which can screw up polygons and paths
	contents.remove(QChar('\t'));
	contents.remove(QChar('\n'));
	contents.remove(QChar('\r'));

	// get rid of inkscape stuff too
	contents.remove(sodipodiDetector);

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

	QDomElement element = TextUtils::findElementWithAttribute(root, "id", elementID);
	if (element.isNull()) {
		return false;
	}

	while (!root.firstChild().isNull()) {
		root.removeChild(root.firstChild());
	}

	root.appendChild(element);
	m_byteArray = m_domDocument.toByteArray();

	QString s = m_domDocument.toString();
	//DebugDialog::debug(s);

	return true;
}

const QByteArray & SvgFileSplitter::byteArray() {
	return m_byteArray;
}

const QDomDocument & SvgFileSplitter::domDocument() {
	return m_domDocument;
}

QString SvgFileSplitter::elementString(const QString & elementID) {
	QDomElement root = m_domDocument.documentElement();

	QDomElement mainElement = TextUtils::findElementWithAttribute(root, "id", elementID);
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
	if (root.isNull()) return false;

	QString swidthStr = root.attribute("width");
	if (swidthStr.isEmpty()) return false;

	QString sheightStr = root.attribute("height");
	if (sheightStr.isEmpty()) return false;

	bool ok;
	qreal sWidth = TextUtils::convertToInches(swidthStr, &ok);
	if (!ok) return false;

	qreal sHeight = TextUtils::convertToInches(sheightStr, &ok);
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

	QDomElement mainElement = TextUtils::findElementWithAttribute(root, "id", elementID);
	if (mainElement.isNull()) return false;

	QDomElement childElement = mainElement.firstChildElement();
	while (!childElement.isNull()) {
		normalizeChild(childElement, sWidth * dpi, sHeight * dpi, vbWidth, vbHeight, blackOnly);
		childElement = childElement.nextSiblingElement();
	}

	return true;
}

QPainterPath SvgFileSplitter::painterPath(qreal dpi, const QString & elementID)
{
	QPainterPath ppath;

	bool result = normalize(dpi, elementID, false);
	if (!result) return ppath;

	QDomElement root = m_domDocument.documentElement();
	if (root.isNull()) return ppath;

	QDomElement mainElement = TextUtils::findElementWithAttribute(root, "id", elementID);
	if (mainElement.isNull()) return ppath;

	QDomElement childElement = mainElement.firstChildElement();
	while (!childElement.isNull()) {
		painterPathChild(childElement, ppath);
		childElement = childElement.nextSiblingElement();
	}

	return ppath;
}

void SvgFileSplitter::painterPathChild(QDomElement & element, QPainterPath & ppath)
{
	// only partially implemented

	if (element.nodeName().compare("circle") == 0) {
		qreal cx = element.attribute("cx").toDouble();
		qreal cy = element.attribute("cy").toDouble();
		qreal r = element.attribute("r").toDouble();
		qreal stroke = element.attribute("stroke-width").toDouble();
		ppath.addEllipse(QRectF(cx - r - (stroke / 2), cy - r - (stroke / 2), (r * 2) + stroke, (r * 2) + stroke));
	}
	else if (element.nodeName().compare("line") == 0) {

		/*
		qreal x1 = element.attribute("x1").toDouble();
		qreal y1 = element.attribute("y1").toDouble();
		qreal x2 = element.attribute("x2").toDouble();
		qreal y2 = element.attribute("y2").toDouble();
		qreal stroke = element.attribute("stroke-width").toDouble();

		// treat as parallel lines stroke width apart?
		*/
	}
	else if (element.nodeName().compare("rect") == 0) {
		qreal width = element.attribute("width").toDouble();
		qreal height = element.attribute("height").toDouble();
		qreal x = element.attribute("x").toDouble();
		qreal y = element.attribute("y").toDouble();
		qreal stroke = element.attribute("stroke-width").toDouble();
		qreal rx = element.attribute("rx").toDouble();
		qreal ry = element.attribute("ry").toDouble();
		if (rx != 0 || ry != 0) { 
			ppath.addRoundedRect(x - (stroke / 2), y - (stroke / 2), width + stroke, height + stroke, rx, ry);
		}
		else {
			ppath.addRect(x - (stroke / 2), y - (stroke / 2), width + stroke, height + stroke);
		}
	}
	else if (element.nodeName().compare("ellipse") == 0) {
		qreal cx = element.attribute("cx").toDouble();
		qreal cy = element.attribute("cy").toDouble();
		qreal rx = element.attribute("rx").toDouble();
		qreal ry = element.attribute("ry").toDouble();
		qreal stroke = element.attribute("stroke-width").toDouble();
		ppath.addEllipse(QRectF(cx - rx - (stroke / 2), cy - ry - (stroke / 2), (rx * 2) + stroke, (ry * 2) + stroke));
	}
	else if (element.nodeName().compare("polygon") == 0 || element.nodeName().compare("polyline") == 0) {
		QString data = element.attribute("points");
		if (!data.isEmpty()) {
			const char * slot = SLOT(painterPathCommandSlot(QChar, bool, QList<double> &, void *));
			PathUserData pathUserData;
			pathUserData.painterPath = &ppath;
            if (parsePath(data, slot, pathUserData, this, false)) {
			}
		}
	}
	else if (element.nodeName().compare("path") == 0) {
		/*
		QString data = element.attribute("d");
		if (!data.isEmpty()) {
			const char * slot = SLOT(normalizeCommandSlot(QChar, bool, QList<double> &, void *));
			PathUserData pathUserData;
			pathUserData.sNewHeight = sNewHeight;
			pathUserData.sNewWidth = sNewWidth;
			pathUserData.vbHeight = vbHeight;
			pathUserData.vbWidth = vbWidth;
            if (parsePath(data, slot, pathUserData, this, true)) {
				element.setAttribute("d", pathUserData.string);
			}
		}
		*/
	}
	else {
		QDomElement childElement = element.firstChildElement();
		while (!childElement.isNull()) {
			painterPathChild(childElement, ppath);
			childElement = childElement.nextSiblingElement();
		}
	}
}

void SvgFileSplitter::normalizeTranslation(QDomElement & element,
											qreal sNewWidth, qreal sNewHeight,
											qreal vbWidth, qreal vbHeight)
{
	QString attr = element.attribute("transform");
	if (attr.isEmpty()) return;

	QMatrix matrix = elementToMatrix(element);
	qreal dx = matrix.dx() * sNewWidth / vbWidth;
	qreal dy = matrix.dy() * sNewHeight / vbHeight;
	if (dx == 0 && dy == 0) return;

	QString m = QString("matrix(%1, %2, %3, %4, %5, %6)").arg(matrix.m11()).arg(matrix.m12()).arg(matrix.m21()).arg(matrix.m22()).arg(dx).arg(dy);
	element.setAttribute("transform", m);
}


void SvgFileSplitter::normalizeChild(QDomElement & element,
									 qreal sNewWidth, qreal sNewHeight,
									 qreal vbWidth, qreal vbHeight, bool blackOnly)
{
	normalizeTranslation(element, sNewWidth, sNewHeight, vbWidth, vbHeight);

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
		normalizeAttribute(element, "stroke-width", sNewWidth, vbWidth);
		QString data = element.attribute("points");
		if (!data.isEmpty()) {
			const char * slot = SLOT(normalizeCommandSlot(QChar, bool, QList<double> &, void *));
			PathUserData pathUserData;
			pathUserData.sNewHeight = sNewHeight;
			pathUserData.sNewWidth = sNewWidth;
			pathUserData.vbHeight = vbHeight;
			pathUserData.vbWidth = vbWidth;
            if (parsePath(data, slot, pathUserData, this, false)) {
				pathUserData.string.remove(0, 1);			// get rid of the "M"
				element.setAttribute("points", pathUserData.string);
			}
		}
		setStrokeOrFill(element, blackOnly);
	}
	else if (element.nodeName().compare("path") == 0) {
		fixStyleAttribute(element);
		normalizeAttribute(element, "stroke-width", sNewWidth, vbWidth);
		setStrokeOrFill(element, blackOnly);
		QString data = element.attribute("d");
		if (!data.isEmpty()) {
			const char * slot = SLOT(normalizeCommandSlot(QChar, bool, QList<double> &, void *));
			PathUserData pathUserData;
			pathUserData.sNewHeight = sNewHeight;
			pathUserData.sNewWidth = sNewWidth;
			pathUserData.vbHeight = vbHeight;
			pathUserData.vbWidth = vbWidth;
            if (parsePath(data, slot, pathUserData, this, true)) {
				element.setAttribute("d", pathUserData.string);
			}
		}
	}
	else if (element.nodeName().compare("text") == 0) {
		fixStyleAttribute(element);
		normalizeAttribute(element, "x", sNewWidth, vbWidth);
		normalizeAttribute(element, "y", sNewHeight, vbHeight);
		normalizeAttribute(element, "stroke-width", sNewWidth, vbWidth);
		normalizeAttribute(element, "font-size", sNewWidth, vbWidth);
		setStrokeOrFill(element, blackOnly);
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

	QDomElement mainElement = TextUtils::findElementWithAttribute(root, "id", elementID);
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
	else if (element.nodeName().compare("text") == 0) {
		shiftAttribute(element, "x", x);
		shiftAttribute(element, "y", y);
	}
	else if (element.nodeName().compare("polygon") == 0 || element.nodeName().compare("polyline") == 0) {
		QString data = element.attribute("points");
		if (!data.isEmpty()) {
			const char * slot = SLOT(shiftCommandSlot(QChar, bool, QList<double> &, void *));
			PathUserData pathUserData;
			pathUserData.x = x;
			pathUserData.y = y;
            if (parsePath(data, slot, pathUserData, this, false)) {
				pathUserData.string.remove(0, 1);			// get rid of the "M"
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
            if (parsePath(data, slot, pathUserData, this, true)) {
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
			DebugDialog::debug("'v' and 'V' are now removed by preprocessing; shouldn't be here");
			/*
			for (int i = 0; i < args.count(); i++) {
				d = args[i] * pathUserData->sNewHeight / pathUserData->vbHeight;
				pathUserData->string.append(QString::number(d));
				if (i < args.count() - 1) {
					pathUserData->string.append(',');
				}
			}
			*/
			break;
		case 'h':
		case 'H':
			DebugDialog::debug("'h' and 'H' are now removed by preprocessing; shouldn't be here");
			/*
			for (int i = 0; i < args.count(); i++) {
				d = args[i] * pathUserData->sNewWidth / pathUserData->vbWidth;
				pathUserData->string.append(QString::number(d));
				if (i < args.count() - 1) {
					pathUserData->string.append(',');
				}
			}
			*/
			break;
		case 'a':
		case 'A':
			for (int i = 0; i < args.count(); i++) {
				switch (i % 7) {
					case 0:
					case 5:
						d = args[i] * pathUserData->sNewWidth / pathUserData->vbWidth;
						break;
					case 1:
					case 6:
						d = args[i] * pathUserData->sNewHeight / pathUserData->vbHeight;
						break;
					default:
						d = args[i];
						break;
				}
				pathUserData->string.append(QString::number(d));
				if (i < args.count() - 1) {
					pathUserData->string.append(',');
				}
			}
			break;
		case SVGPathLexer::FakeClosePathChar:
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

void SvgFileSplitter::painterPathCommandSlot(QChar command, bool relative, QList<double> & args, void * userData) {

	Q_UNUSED(relative);			// just normalizing here, so relative is not used
	Q_UNUSED(command)			// note: painterPathCommandSlot is only partially implemented

	PathUserData * pathUserData = (PathUserData *) userData;

	double dx, dy;
	for (int i = 0; i < args.count(); i += 2) {
		dx = args[i];
		dy = args[i + 1];
		if (i == 0) {
			pathUserData->painterPath->moveTo(dx, dy);
		}
		else {
			pathUserData->painterPath->lineTo(dx, dy);
		}
	}
	pathUserData->painterPath->closeSubpath();


}

void SvgFileSplitter::shiftCommandSlot(QChar command, bool relative, QList<double> & args, void * userData) {

	Q_UNUSED(relative);			// just normalizing here, so relative is not used

	PathUserData * pathUserData = (PathUserData *) userData;

	double d;
	pathUserData->string.append(command);
	switch(command.toAscii()) {
		case 'v':
		case 'V':
			DebugDialog::debug("'v' and 'V' are now removed by preprocessing; shouldn't be here");
			/*
			d = args[0];
			if (!relative) {
				 d += pathUserData->y;
			}
			pathUserData->string.append(QString::number(d));
			*/
			break;
		case 'h':
		case 'H':
			DebugDialog::debug("'h' and 'H' are now removed by preprocessing; shouldn't be here");
			/*
			d = args[0];
			if (!relative) {
				 d += pathUserData->x;
			}
			pathUserData->string.append(QString::number(d));
			*/
			break;
		case SVGPathLexer::FakeClosePathChar:
			break;
		case 'a':
		case 'A':
			for (int i = 0; i < args.count(); i++) {
				d = args[i];
				if (i % 7 == 5) {
					if (!relative) {
						d += pathUserData->x;
					}
				}
				else if (i % 7 == 6) {
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

bool SvgFileSplitter::parsePath(const QString & data, const char * slot, PathUserData & pathUserData, QObject * slotTarget, bool convertHV) {
	QString dataCopy(data);

	if (!dataCopy.startsWith('M', Qt::CaseInsensitive)) {
		dataCopy.prepend('M');
	}
	while (dataCopy.at(dataCopy.length() - 1).isSpace()) {
		dataCopy.remove(dataCopy.length() - 1, 1);
	}
	QChar last = dataCopy.at(dataCopy.length() - 1);
	if (last != 'z' && last != 'Z' && last != SVGPathLexer::FakeClosePathChar) {
		dataCopy.append(SVGPathLexer::FakeClosePathChar);
	}

	SVGPathLexer lexer(dataCopy);
	SVGPathParser parser;
	bool result = parser.parse(&lexer);
	if (!result) {
		DebugDialog::debug(QString("svg path parse failed %1").arg(dataCopy));
		return false;
	}

	if (convertHV) {
		SVGPathRunner svgPathRunner;
		HVConvertData data;
		data.x = data.y = data.subX = data.subY = 0;
		data.path = "";
		connect(&svgPathRunner, SIGNAL(commandSignal(QChar, bool, QList<double> &, void *)), 
				this, SLOT(convertHVSlot(QChar, bool, QList<double> &, void *)), 
				Qt::DirectConnection);
		svgPathRunner.runPath(parser.symStack(), &data);
		return parsePath(data.path, slot, pathUserData, slotTarget, false);
	}

	SVGPathRunner svgPathRunner;
    connect(&svgPathRunner, SIGNAL(commandSignal(QChar, bool, QList<double> &, void *)), slotTarget, slot, Qt::DirectConnection);
	return svgPathRunner.runPath(parser.symStack(), &pathUserData);
}

void SvgFileSplitter::convertHVSlot(QChar command, bool relative, QList<double> & args, void * userData) {
	Q_UNUSED(relative);
	HVConvertData * data = (HVConvertData *) userData;

	qreal x, y;
	switch(command.toAscii()) {
		case 'M':
			data->path.append(command);
			for (int i = 0; i < args.count(); i += 2) {
				data->x = data->subX = args[i];
				data->y = data->subY = args[i + 1];
				appendPair(data->path, args[i], args[i + 1]);
			}
			data->path.chop(1);
			break;
		case 'm':
			data->path.append(command);
			x = data->x;
			y = data->y;
			for (int i = 0; i < args.count(); i += 2) {
				data->subX = data->x = (x + args[i]);
				data->subY = data->y = (y + args[i + 1]);
				appendPair(data->path, args[i], args[i + 1]);
			}
			data->path.chop(1);
			break;
		case 'L':
		case 'T':
			data->path.append(command);
			for (int i = 0; i < args.count(); i += 2) {
				data->x = args[i];
				data->y = args[i + 1];
				appendPair(data->path, args[i], args[i + 1]);
			}
			data->path.chop(1);
			break;
		case 'l':
		case 't':
			data->path.append(command);
			x = data->x;
			y = data->y;
			for (int i = 0; i < args.count(); i += 2) {
				data->x = x + args[i];
				data->y = y + args[i + 1];
				appendPair(data->path, args[i], args[i + 1]);
			}
			data->path.chop(1);
			break;
		case 'C':
			data->path.append(command);
			for (int i = 0; i < args.count(); i += 6) {
				data->x = args[i + 4];
				data->y = args[i + 5];
				appendPair(data->path, args[i], args[i + 1]);
				appendPair(data->path, args[i + 2], args[i + 3]);
				appendPair(data->path, args[i + 4], args[i + 5]);
			}
			data->path.chop(1);
			break;
		case 'c':
			data->path.append(command);
			x = data->x;
			y = data->y;
			for (int i = 0; i < args.count(); i += 6) {
				data->x = x + args[i + 4];
				data->y = y + args[i + 5];
				appendPair(data->path, args[i], args[i + 1]);
				appendPair(data->path, args[i + 2], args[i + 3]);
				appendPair(data->path, args[i + 4], args[i + 5]);
			}
			data->path.chop(1);
			break;
		case 'S':
		case 'Q':
			data->path.append(command);
			for (int i = 0; i < args.count(); i += 4) {
				data->x = args[i + 2];
				data->y = args[i + 3];
				appendPair(data->path, args[i], args[i + 1]);
				appendPair(data->path, args[i + 2], args[i + 3]);
			}
			data->path.chop(1);
			break;
		case 's':
		case 'q':
			data->path.append(command);
			x = data->x;
			y = data->y;
			for (int i = 0; i < args.count(); i += 4) {
				data->x = x + args[i + 2];
				data->y = y + args[i + 3];
				appendPair(data->path, args[i], args[i + 1]);
				appendPair(data->path, args[i + 2], args[i + 3]);
			}
			data->path.chop(1);
			break;
		case 'Z':
		case 'z':
			data->path.append(command);
			data->x = data->subX;
			data->y = data->subY;
			break;
		case SVGPathLexer::FakeClosePathChar:
			data->path.append(command);
			break;
		case 'A':
			data->path.append(command);
			for (int i = 0; i < args.count(); i += 7) {
				data->x = args[i + 5];
				data->y = args[i + 6];
				appendPair(data->path, args[i], args[i + 1]);
				appendPair(data->path, args[i + 2], args[i + 3]);
				appendPair(data->path, args[i + 4], args[i + 5]);
				data->path.append(QString::number(args[i + 6]));
				data->path.append(',');
			}
			data->path.chop(1);
			break;
		case 'a':
			data->path.append(command);
			x = data->x;
			y = data->y;
			for (int i = 0; i < args.count(); i += 7) {
				data->x = x + args[i + 5];
				data->y = y + args[i + 6];
				appendPair(data->path, args[i], args[i + 1]);
				appendPair(data->path, args[i + 2], args[i + 3]);
				appendPair(data->path, args[i + 4], args[i + 5]);
				data->path.append(QString::number(args[i + 6]));
				data->path.append(',');
			}
			data->path.chop(1);
			break;
		case 'v':
			data->path.append('l');
			y = data->y;
			for (int i = 0; i < args.count(); i++) {
				data->y = y + args[i];
				appendPair(data->path, 0, args[i]);
			}
			data->path.chop(1);
			break;
		case 'h':
			data->path.append('l');
			x = data->x;
			for (int i = 0; i < args.count(); i++) {
				data->x = x + args[i];
				appendPair(data->path, args[i], 0);
			}
			data->path.chop(1);
			break;
		case 'H':
			data->path.append('L');
			for (int i = 0; i < args.count(); i++) {
				data->x = args[i];
				appendPair(data->path, args[i], data->y);
			}
			data->path.chop(1);
			break;
		case 'V':
			data->path.append('L');
			for (int i = 0; i < args.count(); i++) {
				data->y = args[i];
				appendPair(data->path, data->x, args[i]);
			}
			data->path.chop(1);
			break;
		default:
			DebugDialog::debug(QString("unknown path command %1").arg(command));
			data->path.append(command);
			for (int i = 0; i < args.count(); i++) {
				data->path.append(QString::number(args[i]));
				data->path.append(',');
			}
			data->path.chop(1);
			break;
	}
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

void SvgFileSplitter::fixStyleAttributeRecurse(QDomElement & element) {
	fixStyleAttribute(element);
	QDomElement childElement = element.firstChildElement();
	while (!childElement.isNull()) {
		fixStyleAttributeRecurse(childElement);
		childElement = childElement.nextSiblingElement();
	}
}

void SvgFileSplitter::fixStyleAttribute(QDomElement & element)
{
	QString style = element.attribute("style");
	if (style.isEmpty()) return;

	fixStyleAttribute(element, style, "stroke-width");
	fixStyleAttribute(element, style, "stroke");
	fixStyleAttribute(element, style, "fill");
	fixStyleAttribute(element, style, "fill-opacity");
	fixStyleAttribute(element, style, "font-size");

	if (style.trimmed().isEmpty()) {
		element.removeAttribute("style");
	}
	else {
		element.setAttribute("style", style);
	}

	//QString deleteMe;
	//QTextStream stream(&deleteMe);
	//stream << element;
	//DebugDialog::debug(deleteMe);
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

bool SvgFileSplitter::changeStrokeWidth(const QString & svg, qreal delta, QByteArray & byteArray) {
	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument domDocument;

	if (!domDocument.setContent(svg, true, &errorStr, &errorLine, &errorColumn)) {
		return false;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull()) {
		return false;
	}

	if (root.tagName() != "svg") {
		return false;
	}

	changeStrokeWidth(root, delta);
	byteArray = domDocument.toByteArray();
	return true;
}

void SvgFileSplitter::changeStrokeWidth(QDomElement & element, qreal delta) {
	bool ok;
	qreal sw = element.attribute("stroke-width").toDouble(&ok);
	if (ok) {
		element.setAttribute("stroke-width", QString::number(sw + delta));
	}
	QDomElement child = element.firstChildElement();
	while (!child.isNull()) {
		changeStrokeWidth(child, delta);
		child = child.nextSiblingElement();
	}
}

bool SvgFileSplitter::changeColors(const QString & svg, QString & toColor, QStringList & exceptions, QByteArray & byteArray) {
	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument domDocument;

	if (!domDocument.setContent(svg, true, &errorStr, &errorLine, &errorColumn)) {
		return false;
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull()) {
		return false;
	}

	if (root.tagName() != "svg") {
		return false;
	}

	changeColors(root, toColor, exceptions);
	byteArray = domDocument.toByteArray();
	return true;
}

void SvgFileSplitter::changeColors(QDomElement & element, QString & toColor, QStringList & exceptions) {
	QString c = element.attribute("stroke");
	if (!exceptions.contains(c)) {
		element.setAttribute("stroke", toColor);
	}
	c = element.attribute("fill");
	if (!exceptions.contains(c)) {
		element.setAttribute("fill", toColor);
	}

	QString fillo = element.attribute("fill-opacity");
	if (!fillo.isEmpty()) {
		element.setAttribute("fill-opacity", "1.0");
	}

	QDomElement child = element.firstChildElement();
	while (!child.isNull()) {
		changeColors(child, toColor, exceptions);
		child = child.nextSiblingElement();
	}
}

QList<qreal> SvgFileSplitter::getTransformFloats(QDomElement & element){
	return getTransformFloats(element.attribute("transform"));
}

QList<qreal> SvgFileSplitter::getTransformFloats(const QString & transform){
    QList<qreal> list;
    int pos = 0;

	while ((pos = SVGPathLexer::floatingPointMatcher.indexIn(transform, pos)) != -1) {
		list << transform.mid(pos, SVGPathLexer::floatingPointMatcher.matchedLength()).toDouble();
        pos += SVGPathLexer::floatingPointMatcher.matchedLength();
    }

#ifndef QT_NO_DEBUG
   // QString dbg = "got transform params: \n";
    //dbg += transform + "\n";
    //for(int i=0; i < list.size(); i++){
        //dbg += QString::number(list.at(i)) + " ";
    // }
    //DebugDialog::debug(dbg);
#endif

    return list;
}

QMatrix SvgFileSplitter::elementToMatrix(QDomElement & element) {
	QString transform = element.attribute("transform");
	if (transform.isEmpty()) return QMatrix();

	QList<qreal> floats = getTransformFloats(transform);

	if (transform.startsWith("translate")) {
		return QMatrix().translate(floats[0], (floats.length() > 1) ? floats[1] : 0);
	}
	else if (transform.startsWith("rotate")) {
		if (floats.length() == 1) {
			return QMatrix().rotate(floats[0]);
		}
		else if (floats.length() == 3) {
			return  QMatrix().translate(-floats[1], -floats[2]) * QMatrix().rotate(floats[0]) * QMatrix().translate(floats[1], floats[2]);
		}
	}
	else if (transform.startsWith("matrix")) {
        return QMatrix(floats[0], floats[1], floats[2], floats[3], floats[4], floats[5]);
	}
	else if (transform.startsWith("scale")) {
		return QMatrix().scale(floats[0], floats[1]);
	}
	else if (transform.startsWith("skewX")) {
		return QMatrix().shear(floats[0], 0);
	}
	else if (transform.startsWith("skewY")) {
		return QMatrix().shear(0, floats[0]);
	}

	return QMatrix();
}

