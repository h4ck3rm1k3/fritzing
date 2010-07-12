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

#include "kicadschematic2svg.h"
#include "../utils/textutils.h"
#include "svgfilesplitter.h"
#include "../version/version.h"
#include "../debugdialog.h"
#include "../viewlayer.h"
#include "../fsvgrenderer.h"

#include <QFile>
#include <QTextStream>
#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <qmath.h>
#include <limits>



KicadSchematic2Svg::KicadSchematic2Svg() : Kicad2Svg() {
}

QStringList KicadSchematic2Svg::listDefs(const QString & filename) {
	QStringList defs;

	QFile file(filename);
	if (!file.open(QFile::ReadOnly)) return defs;

	QTextStream textStream(&file);
	while (true) {
		QString line = textStream.readLine();
		if (line.isNull()) break;

		if (line.startsWith("DEF")) {
			QStringList linedefs = line.split(" ");
			if (linedefs.count() > 1) {
				defs.append(linedefs[1]);
			}
		}
	}

	return defs;
}

QString KicadSchematic2Svg::convert(const QString & filename, const QString & defName) 
{
	initLimits();

	QFile file(filename);
	if (!file.open(QFile::ReadOnly)) {
		throw QObject::tr("unable to open %1").arg(filename);
	}

	QTextStream textStream(&file);

	QString metadata = makeMetadata(filename, "schematic part", defName);
	metadata += endMetadata();

	QString reference;
	bool gotDef = false;
	while (true) {
		QString line = textStream.readLine();
		if (line.isNull()) {
			break;
		}

		if (line.startsWith("DEF") && line.contains(defName, Qt::CaseInsensitive)) {
			QStringList defs = line.split(" ");
			if (defs.count() > 2) {
				reference = defs[2];
			}
			gotDef = true;
			break;
		}
	}

	if (!gotDef) {
		throw QObject::tr("schematic part %1 not found in %2").arg(defName).arg(filename);
	}

	QString contents = "<g id='schematic'>\n";
	while (true) {
		QString fline = textStream.readLine();
		if (fline.isNull()) {
			throw QObject::tr("schematic %1 unexpectedly ends (1) in %2").arg(defName).arg(filename);
		}

		if (fline.contains("ENDDEF")) {
			throw QObject::tr("schematic %1 unexpectedly ends (2) in %2").arg(defName).arg(filename);
		}

		if (fline.startsWith("ALIAS")) continue;

		if (!fline.startsWith("F")) {
			break;
		}

		QStringList fs = fline.split(" ");
		if (fs.count() < 9) continue;

		QString jib = fs[8];
		QString style;
		if (jib.contains("I")) {
			style += "font-style='italic' ";
		}
		if (jib.contains("B")) {
			style += "font-weight='bold' ";
		}
		QString anchor = "middle";
		if (style.contains("L") || style.contains("B")) {
			anchor = "start";
		}
		else if (style.contains("R") || style.contains("T")) {
			anchor = "end";
		}

		int x = fs[2].toInt();
		int y = -fs[3].toInt();						// KiCad flips y-axis w.r.t. svg

		// TODO: figure out actual length of string for limits
		checkXLimit(x);
		checkYLimit(y);

		contents += QString("<text x='%1' y='%2' font-size='%3' font-family='DroidSans' stroke='none' fill='#000000' text-anchor='%4' %5>%6</text>\n")
						.arg(x)		
						.arg(y)		
						.arg(fs[4])		// dimension = font size?
						.arg(anchor)
						.arg(style)
						.arg(unquote(fs[1]));		// the text

	}

	while (true) {
		QString line = textStream.readLine();
		if (line.isNull()) {
			throw QObject::tr("schematic %1 unexpectedly ends (3) in %2").arg(defName).arg(filename);
		}

		if (line.contains("ENDDEF")) {
			break;
		}

		if (line.startsWith("S")) {
			QStringList s = line.split(" ");
			if (s.count() < 9) {
				DebugDialog::debug(QString("bad rectangle %1").arg(line));
				continue;
			}

			int x = s[1].toInt();
			int y = -s[2].toInt();					// KiCad flips y-axis w.r.t. svg
			int x2 = s[3].toInt();
			int y2 = -s[4].toInt();					// KiCad flips y-axis w.r.t. svg

			checkXLimit(x);
			checkXLimit(x2);
			checkYLimit(y);
			checkYLimit(y2);

			int stroke = s[7].toInt();
			if (stroke <= 0) stroke = 1;

			contents += QString("<rect x='%1' y='%2' width='%3' height='%4' ")
					.arg(qMin(x, x2))
					.arg(qMin(y, y2))
					.arg(qAbs(x2 - x))
					.arg(qAbs(y2 - y));

			QString NF = s[8];
			if (NF == "F") {
				contents += "fill='#000000' ";
			}
			else if (NF == "N") {
				contents += QString("fill='none' stroke='#000000' stroke-width='%1' ").arg(stroke);
			}
			else {
				DebugDialog::debug(QString("bad NF param: %1").arg(line));
			}

			contents += " />\n";
		}
		else if (line.startsWith("X")) {
			QStringList l = line.split(" ");
			if (l.count() < 12) {
				DebugDialog::debug(QString("bad line %1").arg(line));
				continue;
			}

			if (l[6].length() != 1) {
				DebugDialog::debug(QString("bad orientation %1").arg(line));
				continue;
			}

			QChar orientation = l[6].at(0);
			QString name = l[1];
			int n = l[2].toInt();
			int x1 = l[3].toInt();
			int y1 = -l[4].toInt();							// KiCad flips y-axis w.r.t. svg
			int length = l[5].toInt();
			int x2 = x1;
			int y2 = y1;
			QString anchor;
			switch (orientation.toAscii()) {
				case 'D':
					y2 = y1 + length;
					anchor = "end";
					break;
				case 'U':
					y2 = y1 - length;
					anchor = "start";
					break;
				case 'L':
					x2 = x1 - length;
					anchor = "end";
					break;
				case 'R':
					x2 = x1 + length;
					anchor = "start";
					break;
				default:
					DebugDialog::debug(QString("bad orientation %1").arg(line));
					break;
			}

			checkXLimit(x1);
			checkXLimit(x2);
			checkYLimit(y1);
			checkYLimit(y2);

			int thickness = 1;
			
			QString ln = QString("<line fill='none' stroke='#000000' x1='%1' y1='%2' x2='%3' y2='%4' stroke-width='%5' id='connector%6pin' connectorname='%7' />\n")
					.arg(x1)
					.arg(y1)
					.arg(x2)
					.arg(y2)
					.arg(thickness)
					.arg(n)
					.arg(name);

			contents += ln;

			ln = QString("<rect fill='none' x='%1' y='%2' width='0' height='0' stroke-width='0' id='connector%3terminal'  />\n")
					.arg(x1)
					.arg(y1)
					.arg(n);


			QString style;
			contents += QString("<text x='%1' y='%2' font-size='%3' font-family='DroidSans' stroke='none' fill='#000000' text-anchor='%4' %5>%6</text>\n")
						.arg(x2)		
						.arg(y2)		
						.arg(l[8])		// dimension = font size?
						.arg(anchor)
						.arg(style)
						.arg(unquote(l[1]));		// the text


			contents += ln;
		}
	}

	contents += "</g>\n";

	QString svg = TextUtils::makeSVGHeader(1000, 1000, m_maxX - m_minX, m_maxY - m_minY) 
					+ m_title + m_description + metadata + offsetMin(contents) + "</svg>";

	return svg;
}


