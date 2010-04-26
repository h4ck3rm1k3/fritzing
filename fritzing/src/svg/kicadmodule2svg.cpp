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

#include "kicadmodule2svg.h"
#include "../utils/textutils.h"
#include "svgfilesplitter.h"
#include "../version/version.h"
#include "../debugdialog.h"
#include "../viewlayer.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <QDateTime>
#include <qmath.h>
#include <limits>

#define KicadSilkscreenTop 21
#define KicadSilkscreenBottom 20


KicadModule2Svg::KicadModule2Svg() : X2Svg() {
}

QStringList KicadModule2Svg::listModules(const QString & filename) {
	QStringList modules;

	QFile file(filename);
	if (!file.open(QFile::ReadOnly)) return modules;

	QTextStream textStream(&file);
	bool gotIndex = false;
	while (true) {
		QString line = textStream.readLine();
		if (line.isNull()) break;

		if (line.compare("$INDEX") == 0) {
			gotIndex = true;
			break;
		}
	}

	if (!gotIndex) return modules;
	while (true) {
		QString line = textStream.readLine();
		if (line.isNull()) break;

		if (line.compare("$EndINDEX") == 0) {
			return modules;
		}

		modules.append(line);
	}

	modules.clear();
	return modules;
}

QString KicadModule2Svg::convert(const QString & filename, const QString & moduleName, bool allowPadsAndPins) 
{
	initLimits();

	QFile file(filename);
	if (!file.open(QFile::ReadOnly)) {
		throw QObject::tr("unable to open %1").arg(filename);
	}

	QString text;
	QTextStream textStream(&file);

	QFileInfo fileInfo(filename);

	QDateTime now = QDateTime::currentDateTime();
	QString dt = now.toString("dd/MM/yyyy hh:mm:ss");

	QString title = QString("<title>%1</title>").arg(fileInfo.fileName());
	QString description = QString("<desc>Kicad module '%2' from file '%1' converted by Fritzing</desc>")
			.arg(fileInfo.fileName()).arg(moduleName);

	QString attribute("<fz:attr name='%1'>%2</fz:attr>");
	QString comment("<fz:comment>%2</fz:comment>");

	QString metadata("<metadata xmlns:fz='http://fritzing.org/kicadmetadata/1.0/' xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'>");
	metadata += "<rdf:RDF>";
	metadata += "<rdf:Description rdf:about=''>";
	metadata += attribute.arg("kicad filename").arg(fileInfo.fileName());
	metadata += attribute.arg("kicad module").arg(moduleName);
	metadata += attribute.arg("fritzing version").arg(Version::versionString());
	metadata += attribute.arg("conversion date").arg(dt);

	bool gotModule = false;
	while (true) {
		QString line = textStream.readLine();
		if (line.isNull()) {
			break;
		}

		if (line.contains("$MODULE") && line.contains(moduleName)) {
			gotModule = true;
			break;
		}
	}

	if (!gotModule) {
		throw QObject::tr("footprint %1 not found in %2").arg(moduleName).arg(filename);
	}

	bool gotT0;
	while (true) {
		QString line = textStream.readLine();
		if (line.isNull()) {
			throw QObject::tr("unexpected end of file in footprint %1 in file %2").arg(moduleName).arg(filename);
		}

		if (line.startsWith("T0")) {
			gotT0 = true;
			break;
		}
		else if (line.startsWith("Cd")) {
			metadata += comment.arg(line.remove(0,3));
		}
		else if (line.startsWith("Kw")) {
			QStringList keywords = line.split(" ");
			for (int i = 1; i < keywords.count(); i++) {
				metadata += attribute.arg("keyword").arg(keywords[i]);
			}
		}
	}

	metadata += "</rdf:Description>";
	metadata += "</rdf:RDF>";
	metadata += "</metadata>";

	if (!gotT0) {
		throw QObject::tr("unexpected format (1) in %1 from %2").arg(moduleName).arg(filename);
	}

	QString line;
	while (true) {
		line = textStream.readLine();
		if (line.isNull()) {
			throw QObject::tr("unexpected end of file in footprint %1 in file %2").arg(moduleName).arg(filename);
		}

		if (!line.startsWith("T")) break;
	}

	QString copper0;
	QString copper1;
	QString silkscreen0;
	QString silkscreen1;

	while (true) {
		if (line.startsWith("$PAD")) break;

		int layer = 0;
		QString svgElement;
		if (line.startsWith("DS")) {
			layer = drawSegment(line, svgElement);
		}
		else if (line.startsWith("DA")) {
			layer = drawArc(line, svgElement);
		}
		else if (line.startsWith("DC")) {
			layer = drawCircle(line, svgElement);
		}
		switch (layer) {
			case KicadSilkscreenTop:
				silkscreen1 += svgElement;
				break;
			case KicadSilkscreenBottom:
				silkscreen0 += svgElement;
				break;
			default:
				break;
		}
	
		line = textStream.readLine();
		if (line.isNull()) {
			throw QObject::tr("unexpected end of file in footprint %1 in file %2").arg(moduleName).arg(filename);
		}
	}

	int pads = 0;
	int pins = 0;
	bool done = false;
	while (!done) {
		try {
			QString pad;
			PadLayer padLayer = convertPad(textStream, pad);
			switch (padLayer) {
				case ToCopper0:
					copper0 += pad;
					pins++;
					break;
				case ToCopper1:
					copper1 += pad;
					pads++;
					break;
				default:
					break;
			}
		}
		catch (const QString & msg) {
			DebugDialog::debug(QString("kicad pad %1 conversion failed in %2: %3").arg(moduleName).arg(filename).arg(msg));
		}

		while (true) {
			line = textStream.readLine();
			if (line.isNull()) {
				throw QObject::tr("unexpected end of file in footprint %1 in file %2").arg(moduleName).arg(filename);
			}

			if (line.contains("$SHAPE3D")) {
				done = true;
				break;
			}
			if (line.contains("$EndMODULE")) {
				done = true;
				break;
			}
			if (line.contains("$PAD")) {
				break;
			}
		}
	}

	if (!allowPadsAndPins && pins > 0 && pads > 0) {
		throw QObject::tr("Sorry, Fritzing can't yet handle both pins and pads together (in %1)").arg(filename);
	}

	if (!copper0.isEmpty()) {
		copper0 = offsetMin("<g id='copper0'>" + copper0 + "</g>");
	}
	if (!copper1.isEmpty()) {
		copper1 = offsetMin("<g id='copper1'>" + copper1 + "</g>");
	}
	if (!silkscreen1.isEmpty()) {
		silkscreen1 = offsetMin("<g id='silkscreen'>" + silkscreen1 + "</g>");
	}
	if (!silkscreen0.isEmpty()) {
		silkscreen0 = offsetMin("<g id='silkscreen0'>" + silkscreen0 + "</g>");
	}


	QString svg = TextUtils::makeSVGHeader(10000, 10000, m_maxX - m_minX, m_maxY - m_minY) 
					+ title + description + metadata + copper0 + copper1 + silkscreen0 + silkscreen1 + "</svg>";

	return svg;
}

int KicadModule2Svg::drawCircle(const QString & ds, QString & circle) {
	// DC Xcentre Ycentre Xend Yend width layer
	QStringList params = ds.split(" ");
	if (params.count() < 7) return -1;

	int cx = params.at(1).toInt();
	int cy = params.at(2).toInt();
	int x2 = params.at(3).toInt();
	int y2 = params.at(4).toInt();
	qreal radius = qSqrt((cx - x2) * (cx - x2) + (cy - y2) * (cy - y2));

	checkXLimit(cx + radius);
	checkXLimit(cx - radius);
	checkYLimit(cy + radius);
	checkYLimit(cy - radius);

	int layer = params.at(6).toInt();

	circle = QString("<circle cx='%1' cy='%2' r='%3' stroke-width='%4' stroke='white' fill='none' />")
		.arg(cx)
		.arg(cy)
		.arg(radius)
		.arg(params.at(5));

	return layer;
}

int KicadModule2Svg::drawSegment(const QString & ds, QString & line) {
	// DS Xstart Ystart Xend Yend Width Layer
	QStringList params = ds.split(" ");
	if (params.count() < 7) return -1;

	int x1 = params.at(1).toInt();
	int y1 = params.at(2).toInt();
	int x2 = params.at(3).toInt();
	int y2 = params.at(4).toInt();
	checkXLimit(x1);
	checkXLimit(x2);
	checkYLimit(y1);
	checkYLimit(y2);

	int layer = params.at(6).toInt();

	line = QString("<line x1='%1' y1='%2' x2='%3' y2='%4' stroke-width='%5' stroke='white' fill='none' />")
				.arg(x1)
				.arg(y1)
				.arg(x2)
				.arg(y2)
				.arg(params.at(5));

	return layer;
}

int KicadModule2Svg::drawArc(const QString & ds, QString & arc) {
	//DA x0 y0 x1 y1 angle width layer

	QStringList params = ds.split(" ");
	if (params.count() < 8) return -1;

	int cx = params.at(1).toInt();
	int cy = params.at(2).toInt();
	int x2 = params.at(3).toInt();
	int y2 = params.at(4).toInt();
	int width = params.at(6).toInt();
	qreal diffAngle = (params.at(5).toInt() % 3600) / 10.0;
	qreal radius = qSqrt((cx - x2) * (cx - x2) + (cy - y2) * (cy - y2));
	qreal endAngle = asin((y2 - cy) / radius);
	if (x2 < cx) {
		endAngle += M_PI;
	}
	qreal startAngle = endAngle + (diffAngle * M_PI / 180.0);
	qreal x1 = (radius * cos(startAngle)) + cx;
	qreal y1 = (radius * sin(startAngle)) + cy;

	// TODO: figure out bounding box for circular arc and set min and max accordingly
	checkXLimit(cx + radius);
	checkXLimit(cx - radius);
	checkYLimit(cy + radius);
	checkYLimit(cy - radius);

	int layer = params.at(7).toInt();

	arc = QString("<path stroke-width='%1' stroke='white' d='M%2,%3a%4,%5 0 %6,%7 %8,%9' fill='none' />")
			.arg(width / 2.0)
			.arg(x1)
			.arg(y1)
			.arg(radius)
			.arg(radius)
			.arg(qAbs(diffAngle) >= 180 ? 1 : 0)
			.arg(diffAngle > 0 ? 0 : 1)
			.arg(x2 - x1)
			.arg(y2 - y1);

	return layer;
}

KicadModule2Svg::PadLayer KicadModule2Svg::convertPad(QTextStream & stream, QString & pad) {
	PadLayer padLayer = UnableToTranslate;

	QStringList padStrings;
	while (true) {
		QString line = stream.readLine();
		if (line.isNull()) {
			throw QObject::tr("unexpected end of file");
		}
		if (line.contains("$EndPAD")) {
			break;
		}

		padStrings.append(line);
	}

	QString shape;
	QString drill;
	QString attributes;
	QString position;

	foreach (QString string, padStrings) {
		if (string.startsWith("Sh")) {
			shape = string;
		}
		else if (string.startsWith("Po")) {
			position = string;
		}
		else if (string.startsWith("At")) {
			attributes = string;
		}
		else if (string.startsWith("Dr")) {
			drill = string;
		}
	}

	if (drill.isEmpty()) {
		throw QObject::tr("pad missing drill");
	}
	if (attributes.isEmpty()) {
		throw QObject::tr("pad missing attributes");
	}
	if (position.isEmpty()) {
		throw QObject::tr("pad missing position");
	}
	if (shape.isEmpty()) {
		throw QObject::tr("pad missing shape");
	}

	QStringList positionStrings = position.split(" ");
	if (positionStrings.count() < 3) {
		throw QObject::tr("position missing params");
	}

	int posX = positionStrings.at(1).toInt();
	int posY = positionStrings.at(2).toInt();

	QStringList drillStrings = drill.split(" ");
	if (drillStrings.count() < 4) {
		throw QObject::tr("drill missing params");
	}

	int drillX = drillStrings.at(1).toInt();
	int drillXOffset = drillStrings.at(2).toInt();
	int drillYOffset = drillStrings.at(3).toInt();
	int drillY = drillX;

	if (drillXOffset != 0 || drillYOffset != 0) {
		throw QObject::tr("drill offset not implemented");
	}

	if (drillStrings.count() > 4) {
		if (drillStrings.at(4) == "O") {
			if (drillStrings.count() < 7) {
				throw QObject::tr("drill missing ellipse params");
			}
			drillY = drillStrings.at(6).toInt();

			throw QObject::tr("ellipsoidal drill holes not implemented");
		}
	}

	QStringList attributeStrings = attributes.split(" ");
	if (attributeStrings.count() < 4) {
		throw QObject::tr("attributes missing params");
	}

	QString padType = attributeStrings.at(1);
	if (padType == "STD") {
		padLayer = ToCopper0;
	}
	else if (padType == "SMD") {
		padLayer = ToCopper1;
	}
	else {
		throw QObject::tr("Sorry, can't handle pad type %1").arg(padType);
	}

	QStringList shapeStrings = shape.split(" ");
	if (shapeStrings.count() < 8) {
		throw QObject::tr("pad shape missing params");
	}

	QString padName = unquote(shapeStrings.at(1));
	bool ok;
	int padNumber = padName.toInt(&ok);
	if (ok) {
		padName = QString("%1").arg(padNumber - 1);
	}
	QString shapeIdentifier = shapeStrings.at(2);
	int xSize = shapeStrings.at(3).toInt();
	int ySize = shapeStrings.at(4).toInt();
	int xDelta = shapeStrings.at(5).toInt();
	int yDelta = shapeStrings.at(6).toInt();
	int orientation = shapeStrings.at(7).toInt();

	if (xDelta != 0 || yDelta != 0) {
		// note: so far, all cases of non-zero delta go with shape "T"
		throw QObject::tr("shape delta not implemented");
	}

	if (shapeIdentifier == "C") {
		checkXLimit(posX - (xSize / 2.0));
		checkXLimit(posX + (xSize / 2.0));
		checkYLimit(posY - (ySize / 2.0));
		checkYLimit(posY + (ySize / 2.0));
		if (padType == "SMD") {
			pad = QString("<circle cx='%1' cy='%2' r='%3' id='connector%4pin' fill='%5' stroke-width='0' />")
							.arg(posX)
							.arg(posY)
							.arg(xSize / 2.0)
							.arg(padName)
							.arg(ViewLayer::Copper1Color);
		}
		else {
			pad = QString("<circle cx='%1' cy='%2' r='%3' id='connector%4pad' stroke-width='%5' stroke='%6' fill='none' />")
							.arg(posX)
							.arg(posY)
							.arg((xSize / 2.0) - (drillX / 4.0))
							.arg(padName)
							.arg(drillX / 2.0)
							.arg(ViewLayer::Copper0Color);
		}
	}
	else if (shapeIdentifier == "R") {
		checkXLimit(posX - (xSize / 2.0));
		checkXLimit(posX + (xSize / 2.0));
		checkYLimit(posY - (ySize / 2.0));
		checkYLimit(posY + (ySize / 2.0));
		if (padType == "SMD") {
			pad = QString("<rect x='%1' y='%2' width='%3' height='%4' id='connector%5pin' stroke-width='0' fill='%6' />")
							.arg(posX - (xSize / 2.0))
							.arg(posY - (ySize / 2.0))
							.arg(xSize)
							.arg(ySize)
							.arg(padName)
							.arg(ViewLayer::Copper1Color);
		}
		else {
			pad += QString("<circle fill='none' cx='%1' cy='%2' r='%3' id='connector%4pin' stroke-width='%5' stroke='%6' />")
							.arg(posX)
							.arg(posY)
							.arg((qMin(xSize, ySize) / 2.0) - (drillX / 4.0))
							.arg(padName)
							.arg(drillX / 2.0)
							.arg(ViewLayer::Copper0Color);
			pad += QString("<rect x='%1' y='%2' width='%3' height='%4' fill='none' stroke-width='%5' stroke='%6' />")
							.arg(posX - (xSize / 2.0) + (drillX / 4.0))
							.arg(posY - (ySize / 2.0) + (drillX / 4.0))
							.arg(xSize - (drillX / 2.0))
							.arg(ySize - (drillX / 2.0))
							.arg(drillX / 2.0)
							.arg(ViewLayer::Copper1Color);
		}
	}
	else if (shapeIdentifier == "O") {
		checkXLimit(posX - (xSize / 2.0));
		checkXLimit(posX + (xSize / 2.0));
		checkYLimit(posY - (ySize / 2.0));
		checkYLimit(posY + (ySize / 2.0));
		if (xSize <= ySize) {
			pad += drawVerticalLosenge(posX, posY, xSize, ySize, drillX, drillY, padName, padType);
		}
		else {
			pad += drawHorizontalLosenge(posX, posY, xSize, ySize, drillX, drillY, padName, padType);
		}
	}
	else if (shapeIdentifier == "T") {
		throw QObject::tr("trapezoidal pads not implemented");

		// eventually polyline?
	}
	else {
		throw QObject::tr("unable to handle pad shape %1").arg(shapeIdentifier);
	}

		
	if (orientation != 0) {
		//throw QObject::tr("unable to handle orientation %1").arg(orientation);
	
		
		QTransform t = QTransform().translate(-posX, -posY) * 
						QTransform().rotate(orientation / 10.0) * 
						QTransform().translate(posX, posY);
		pad = TextUtils::svgTransform(pad, t, true, QString("_x='%1' _y='%2' _r='%3'").arg(posX).arg(posY).arg(orientation / 10.0));
	}


	return padLayer;
}

QString KicadModule2Svg::drawVerticalLosenge(int posX, int posY, int xSize, int ySize, int drillX, int drillY, const QString & padName, const QString & padType) {
	if (drillX == 0 && padType == "SMD") {
		drillY = drillX = qMin(xSize, ySize) / 2;
	}

	qreal r = (xSize - drillX) / 2.0;
	qreal cx = posX;
	qreal cy = posY - (drillX / 2.0);


	QString top = QString("<path d='M%1,%2a%3,%3 0 0 1 %4,0' fill='%5' stroke-width='0' />")
					.arg(cx - r - r)
					.arg(cy)
					.arg(r * 2)
					.arg(r * 4)
					.arg(ViewLayer::Copper1Color);
	QString bot = QString("<path d='M%1,%2a%3,%3 0 1 1 %4,0' fill='%5' stroke-width='0' />")
					.arg(cx + r + r)
					.arg(cy + drillY)
					.arg(r * 2)
					.arg(-r * 4)
					.arg(ViewLayer::Copper1Color);

	QString middle;
	QString g;
	QString afterg;

	if (padType == "SMD") {
		g = QString("<g id='connector%1pin' >").arg(padName);
		afterg = "</g>";
		middle = QString("<rect x='%1' y='%2' width='%3' height='%4' stroke-width='0' fill='%5' />")
							.arg(posX - (xSize / 2.0))
							.arg(posY - (drillY / 2.0))
							.arg(xSize)
							.arg(drillY)
							.arg(ViewLayer::Copper1Color);
	}
	else {
		middle = QString("<circle fill='none' cx='%1' cy='%2' r='%3' id='connector%4pin' stroke-width='%5' stroke='%6' />")
							.arg(posX)
							.arg(posY)
							.arg((qMin(xSize, ySize) / 2.0) - (drillX / 4.0))
							.arg(padName)
							.arg(drillX / 2.0)
							.arg(ViewLayer::Copper0Color);

		middle += QString("<line x1='%1' y1='%2' x2='%1' y2='%3' fill='none' stroke-width='%4' stroke='%5' />")
							.arg(posX - (xSize / 2.0) + (drillX / 4.0))
							.arg(posY - (drillY / 2.0))
							.arg(posY + (drillY / 2.0))
							.arg(drillX / 2.0)
							.arg(ViewLayer::Copper1Color);
		middle += QString("<line x1='%1' y1='%2' x2='%1' y2='%3' fill='none' stroke-width='%4' stroke='%5' />")
							.arg(posX + (xSize / 2.0) - (drillX / 4.0))
							.arg(posY - (drillY / 2.0))
							.arg(posY + (drillY / 2.0))
							.arg(drillX / 2.0)
							.arg(ViewLayer::Copper1Color);
	}

	QString pad = g;
	pad += middle;
	pad += top;
	pad += bot;
	pad += afterg;
	return pad;
}

QString KicadModule2Svg::drawHorizontalLosenge(int posX, int posY, int xSize, int ySize, int drillX, int drillY, const QString & padName, const QString & padType) {
	if (drillX == 0 && padType == "SMD") {
		drillY = drillX = qMin(xSize, ySize) / 2;
	}

	qreal r = (ySize - drillY) / 2.0;
	qreal cx = posX - (drillX / 2.0);
	qreal cy = posY;
	
	QString top = QString("<path d='M%1,%2a%3,%3 0 0 0 0,%4' fill='%5' stroke-width='0' />")
					.arg(cx)
					.arg(cy - r - r)
					.arg(r * 2)
					.arg(r * 4)
					.arg(ViewLayer::Copper1Color);
					
	
	QString bot = QString("<path d='M%1,%2a%3,%3 0 1 0 0,%4' fill='%5' stroke-width='0' />")
					.arg(cx + drillX)
					.arg(cy + r + r)
					.arg(r * 2)
					.arg(-r * 4)
					.arg(ViewLayer::Copper1Color);


	QString middle;
	QString g;
	QString afterg;

	if (padType == "SMD") {
		g = QString("<g id='connector%1pin' >").arg(padName);
		afterg = "</g>";
		middle = QString("<rect x='%1' y='%2' width='%3' height='%4' stroke-width='0' fill='%5' />")
							.arg(posX - (drillY / 2.0))
							.arg(posY - (ySize / 2.0))
							.arg(drillX)
							.arg(ySize)
							.arg(ViewLayer::Copper1Color);
	}
	else {
		middle = QString("<circle fill='none' cx='%1' cy='%2' r='%3' id='connector%4pin' stroke-width='%5' stroke='%6' />")
							.arg(posX)
							.arg(posY)
							.arg((qMin(xSize, ySize) / 2.0) - (drillY / 4.0))
							.arg(padName)
							.arg(drillY / 2.0)
							.arg(ViewLayer::Copper0Color);
		middle += QString("<line x1='%1' y1='%2' x2='%3' y2='%2' fill='none' stroke-width='%4' stroke='%5' />")
							.arg(posX - (drillX / 2.0))
							.arg(posY - (ySize / 2.0) + (drillY / 4.0))
							.arg(posX + (drillX / 2.0))
							.arg(drillY / 2.0)
							.arg(ViewLayer::Copper1Color);
		middle += QString("<line x1='%1' y1='%2' x2='%3' y2='%2' fill='none' stroke-width='%4' stroke='%5' />")
							.arg(posX - (drillX / 2.0))
							.arg(posY + (ySize / 2.0) - (drillY / 4.0))
							.arg(posX + (drillX / 2.0))
							.arg(drillY / 2.0)
							.arg(ViewLayer::Copper1Color);

	}

	QString pad = g;
	pad += middle;
	pad += top;
	pad += bot;
	pad += afterg;
	return pad;
}
