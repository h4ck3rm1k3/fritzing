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

$Revision: 3454 $:
$Author: cohen@irascible.com $:
$Date: 2009-09-11 19:39:43 +0200 (Fri, 11 Sep 2009) $

********************************************************************/

#include "gedaelement2svg.h"
#include "gedaelementparser.h"
#include "gedaelementlexer.h"
#include "../utils/textutils.h"

#include <QFile>
#include <QTextStream>
#include <QObject>
#include <limits>
#include <QDomDocument>
#include <QDomElement>

static const int MAX_INT = std::numeric_limits<int>::max();
static const int MIN_INT = std::numeric_limits<int>::min();

GedaElement2Svg::GedaElement2Svg() {
}

QString GedaElement2Svg::convert(QString filename) 
{
	QFile file(filename);
	if (!file.open(QFile::ReadOnly)) {
		throw QObject::tr("unable to open %1").arg(filename);
	}

	QString text;
	QTextStream textStream(&file);
	text = textStream.readAll();

	GedaElementLexer lexer(text);
	GedaElementParser parser;

	if (!parser.parse(&lexer)) {
		throw QObject::tr("unable to parse %1").arg(filename);
	}

	// TODO: other layers
	QString copper;
	QString silkscreen;

	m_maxX = MIN_INT;
	m_maxY = MIN_INT;
	m_minX = MAX_INT;
	m_minY = MAX_INT;
	QVector<QVariant> stack = parser.symStack();

	for (int ix = 0; ix < stack.size(); ) { 
		QVariant var = stack[ix];
		if (var.type() == QVariant::String) {
			QString thing = var.toString();
			int argCount = countArgs(stack, ix);
			bool mils = stack[ix + argCount + 1].toChar() == ')';
			if (thing.compare("element", Qt::CaseInsensitive) == 0) {
			}
			else if (thing.compare("pad", Qt::CaseInsensitive) == 0) {
				//copper += convertPad(stack, ix, argCount, mils);
			}
			else if (thing.compare("pin", Qt::CaseInsensitive) == 0) {
				copper += convertPin(stack, ix, argCount, mils);
			}
			else if (thing.compare("elementline", Qt::CaseInsensitive) == 0) {
			}
			else if (thing.compare("elementarc", Qt::CaseInsensitive) == 0) {
			}
			else if (thing.compare("mark", Qt::CaseInsensitive) == 0) {
			}
			ix += argCount + 2;
		}
		else if (var.type() == QVariant::Char) {
			// will arrive here at the end of the element
			// TODO: shouldn't happen otherwise
			ix++;
		}
		else {
			throw QObject::tr("parse failure in %1").arg(filename);
		}
	}

	// TODO: offset everything if minx or miny < 0
	copper = offsetMin("<g id='copper0'>" + copper + "</g>");
	silkscreen = offsetMin("<g id='silkscreen'>" + silkscreen + "</g>");

	QString svg = TextUtils::makeSVGHeader(100000, 100000, m_maxX - m_minX, m_maxY - m_minY) + copper +  silkscreen + "</svg>";

	return svg;
}

QString GedaElement2Svg::offsetMin(const QString & svg) {
	if (m_minX == 0 && m_minY == 0) return svg;

	QString errorStr;
	int errorLine;
	int errorColumn;
	QDomDocument domDocument;
	if (!domDocument.setContent(svg, true, &errorStr, &errorLine, &errorColumn)) {
		throw QObject::tr("failure in svg conversion 1: %1 %2 %3").arg(errorStr).arg(errorLine).arg(errorColumn);
	}

	QDomElement root = domDocument.documentElement();
	if (root.isNull()) {
		throw QObject::tr("failure in svg conversion 2: %1 %2 %3").arg(errorStr).arg(errorLine).arg(errorColumn);
	}

	QDomElement child = root.firstChildElement();
	while (!child.isNull()) {
		QString tagName = child.nodeName();
		if (tagName.compare("circle") == 0) {
			TextUtils::shiftAttribute(child, "cx", -m_minX);
			TextUtils::shiftAttribute(child, "cy", -m_minY);
		}
		else if (tagName.compare("line") == 0) {
			TextUtils::shiftAttribute(child, "x1", -m_minX);
			TextUtils::shiftAttribute(child, "y1", -m_minY);
			TextUtils::shiftAttribute(child, "x2", -m_minX);
			TextUtils::shiftAttribute(child, "y2", -m_minY);
		}
		child = child.nextSiblingElement();
	}

	return TextUtils::removeXMLEntities(domDocument.toString());

}

int GedaElement2Svg::countArgs(QVector<QVariant> & stack, int ix) {
	int argCount = 0;
	for (int i = ix + 1; i < stack.size(); i++) {
		QVariant var = stack[i];
		if (var.type() == QVariant::Char) {
			QChar ch = var.toChar();
			if (ch == ']' || ch == ')') {
				break;
			}
		}

		argCount++;
	}

	return argCount;
}

QString GedaElement2Svg::convertPin(QVector<QVariant> & stack, int ix, int argCount, bool mils)
{
	qreal drill = 0;
	QString name;

	int flags = stack[ix + argCount].toInt();
	bool useNumber = (flags & 1) != 0;

	if (argCount == 9) {
		drill = stack[ix + 6].toInt();
		name = stack[ix + (useNumber ? 7 : 8)].toString();
	}
	else if (argCount == 7) {
		drill = stack[ix + 4].toInt();
		name = stack[ix + (useNumber ? 5 : 6)].toString();
	}
	else if (argCount == 6) {
		drill = stack[ix + 4].toInt();
		name = stack[ix + 5].toString();
	}
	else if (argCount == 5) {
		name = stack[ix + 4].toString();
	}
	else {
		throw QObject::tr("bad pin argument count");
	}

	int cx = stack[ix + 1].toInt();
	int cy = stack[ix + 2].toInt();
	qreal r = stack[ix + 3].toInt() / 2.0;
	drill /= 2.0;

	if (mils) {
		// lo res
		cx *= 100;
		cy *= 100;
		r *= 100;
		drill *= 100;
	}

	if (cx - r < m_minX) m_minX = cx - r;
	if (cx + r > m_maxX) m_maxX = cx + r;
	if (cy - r < m_minY) m_minY = cy - r;
	if (cy + r > m_maxY) m_maxY = cy + r;

	qreal w = r - drill;

	QString circle = QString("<circle fill='none' cx='%1' cy='%2' stroke='rgb(255, 191, 0)' r='%3' id='%4' stroke-width='%5' />")
					.arg(cx)
					.arg(cy)
					.arg(r - (w / 2))
					.arg(name)
					.arg(w);
	return circle;
}

QString GedaElement2Svg::convertPad(QVector<QVariant> & stack, int ix, int argCount, bool mils)
{
	QString name;

	int flags = stack[ix + argCount].toInt();
	bool square = (flags & 0x0100) != 0;
	int x1 = stack[ix + 1].toInt();
	int y1 = stack[ix + 2].toInt();
	int x2 = stack[ix + 3].toInt();
	int y2 = stack[ix + 4].toInt();
	int thickness = stack[ix + 5].toInt();

	if (argCount == 10) {
		name = stack[ix + 8].toString();
		QString sflags = stack[ix + argCount].toString();
		if (sflags.contains("square", Qt::CaseInsensitive)) {
			square = true;
		}
	}
	else if (argCount == 8) {
		name = stack[ix + 6].toString();
	}
	else if (argCount == 7) {
		name = stack[ix + 6].toString();
	}
	else {
		throw QObject::tr("bad pad argument count");
	}

	if (mils) {
		// lo res
		x1 *= 100;
		y1 *= 100;
		x2 *= 100;
		y2 *= 100;
		thickness *= 100;
	}

	qreal halft = thickness / 2.0;

	if (x1 - halft < m_minX) m_minX = x1 - halft;
	if (x2 + halft > m_maxX) m_maxX = x2 + halft;
	if (y1 - halft < m_minY) m_minY = y1 - halft;
	if (y1 + halft > m_maxY) m_maxY = y1 + halft;
	  
	QString line = QString("<line fill='none' id='%8' x1='%1' y1='%2' x2='%3' y2='%4' stroke='rgb(255, 191, 0)' stroke-width='%5' stroke-linecap='%6' stroke-linejoin='%7' />")
					.arg(x1)
					.arg(y1)
					.arg(x2)
					.arg(y2)
					.arg(thickness)
					.arg(square ? "square" : "round")
					.arg(square ? "miter" : "round")
					.arg(name);
	return line;
}
