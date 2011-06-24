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
#include "../utils/textutils.h"

static QStringList Spacings;

static int MinSipPins = 2;
static int MaxSipPins = 64;
static int MinDipPins = 4;
static int MaxDipPins = 64;

static int Pins;
QString negIncCopyPinFunction(int pin, const QString & argString) 
{ 
	return argString.arg(Pins - (pin + 3)); 
}

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
	return PaletteItem::genFZP(moduleid, "generic_sip_fzpTemplate", MinSipPins, MaxSipPins, 1);
}

QString Dip::genDipFZP(const QString & moduleid)
{
	return PaletteItem::genFZP(moduleid, "generic_dip_fzpTemplate", MinDipPins, MaxDipPins, 2);
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


QString Dip::genModuleID(QMap<QString, QString> & currPropsMap)
{
	QString value = currPropsMap.value("package");
	QString pins = currPropsMap.value("pins");
	if (pins.isEmpty()) pins = "16";		// pick something safe
	QString moduleID;
	if (value.contains("sip", Qt::CaseInsensitive)) {
		return QString("generic_sip_%1_300mil").arg(pins);
	}
	else {
		int p = pins.toInt();
		if (p < 4) p = 4;
		if (p % 2 == 1) p--;
		return QString("generic_ic_dip_%1_300mil").arg(p);
	}
}

QString Dip::makePcbSvg(const QString & expectedFileName) 
{
	QStringList pieces = expectedFileName.split("_");
	if (pieces.count() != 4) return "";

	int pins = pieces.at(1).toInt();
	QString spacingString = pieces.at(2);

	QString header("<?xml version='1.0' encoding='UTF-8'?>\n"
				    "<svg baseProfile='tiny' version='1.2' width='%1in' height='%2in' viewBox='0 0 %3 %4' xmlns='http://www.w3.org/2000/svg'>\n"
				    "<desc>Fritzing footprint SVG</desc>\n"
					"<g id='silkscreen'>\n"
					"<line stroke='white' stroke-width='10' x1='10' x2='10' y1='10' y2='%5'/>\n"
					"<line stroke='white' stroke-width='10' x1='10' x2='%6' y1='%5' y2='%5'/>\n"
					"<line stroke='white' stroke-width='10' x1='%6' x2='%6' y1='%5' y2='10'/>\n"
					"<line stroke='white' stroke-width='10' x1='10' x2='%7' y1='10' y2='10'/>\n"
					"<line stroke='white' stroke-width='10' x1='%8' x2='%6' y1='10' y2='10'/>\n"
					"</g>\n"
					"<g id='copper1'><g id='copper0'>\n"
					"<rect fill='none' height='55' stroke='rgb(255, 191, 0)' stroke-width='20' width='55' x='32.5' y='32.5'/>\n");

	qreal outerBorder = 10;
	qreal silkSplitTop = 100;
	qreal offsetX = 60;
	qreal offsetY = 60;
	qreal spacing = TextUtils::convertToInches(spacingString) * 1000; 
	qreal totalWidth = 120 + spacing;
	qreal totalHeight = (100 * pins / 2) + (outerBorder * 2);
	qreal center = totalWidth / 2;

	QString svg = header.arg(totalWidth / 1000).arg(totalHeight / 1000).arg(totalWidth).arg(totalHeight)
							.arg(totalHeight - outerBorder).arg(totalWidth - outerBorder)
							.arg(center - (silkSplitTop / 2)).arg(center + (silkSplitTop / 2));

	QString circle("<circle cx='%1' cy='%2' fill='none' id='connector%3pin' r='27.5' stroke='rgb(255, 191, 0)' stroke-width='20'/>\n");

	int y = offsetY;
	for (int i = 0; i < pins / 2; i++) {
		svg += circle.arg(offsetX).arg(y).arg(i);
		svg += circle.arg(totalWidth - offsetX).arg(y).arg(pins - 1 - i);
		y += 100;
	}

	svg += "</g></g>\n";
	svg += "</svg>\n";

	return svg;
}


QString Dip::makeSchematicSvg(const QString & expectedFileName) 
{
	QStringList pieces = expectedFileName.split("_");
	if (pieces.count() != 5) return "";

	int pins = pieces.at(3).toInt();
	int increment = 300;
	qreal totalHeight = (pins * increment / 2) + 330;
	int border = 30;

	QString header("<?xml version='1.0' encoding='UTF-8' standalone='no'?>\n"
					"<svg xmlns:svg='http://www.w3.org/2000/svg' xmlns='http://www.w3.org/2000/svg' version='1.2' baseProfile='tiny'\n"
					"width='1.83in' height='%1in' viewBox='0 0 1830 %2' >\n"
					"<g id='schematic' >\n"
					"<rect x='315' y='15' fill='none' width='1200' height='%3' stroke='#000000' stroke-linejoin='round' stroke-linecap='round' stroke-width='30' />\n"
					"<text id='label' x='915.0' y='465' font-family='DroidSans' stroke='none' fill='#000000' text-anchor='middle' font-size='235' >IC</text>\n");

	QString svg = header.arg(totalHeight / 1000).arg(totalHeight).arg(totalHeight - border);
  
	QString repeatL("<line fill='none' stroke='#000000' stroke-linejoin='round' stroke-linecap='round' stroke-width='30' x1='15' y1='%1' x2='300' y2='%1'  />\n"
					"<rect x='0' y='%2' fill='none' width='300' height='30' id='connector%3pin' stroke-width='0' />\n"
					"<rect x='0' y='%2' fill='none' width='30' height='30' id='connector%3terminal' stroke-width='0' />\n"
					"<text id='label%3' x='390' y='%4' font-family='DroidSans' stroke='none' fill='#000000' text-anchor='start' font-size='130' >%5</text>\n");

	QString repeatR("<line fill='none' stroke='#000000' stroke-linejoin='round' stroke-linecap='round' stroke-width='30' x1='1515' y1='%1' x2='1815' y2='%1'  />\n"
					"<rect x='1530' y='%2' fill='none' width='300' height='30' id='connector%3pin' stroke-width='0' />>\n"
					"<rect x='1830' y='%2' fill='none' width='30' height='30' id='connector%3terminal' stroke-width='0' />>\n"
					"<text id='label%3' x='1440' y='%4' font-family='DroidSans' stroke='none' fill='#000000' text-anchor='end' font-size='130' >%5</text>>\n");


	int y = 300;
	for (int i = 0; i < pins / 2; i++) {
		svg += repeatL.arg(15 + y).arg(y).arg(i).arg(y + 50).arg(i + 1);
		svg += repeatR.arg(15 + y).arg(y).arg(pins - i - 1).arg(y + 50).arg(pins - i);
		y += increment;
	}

	svg += "</g>\n";
	svg += "</svg>\n";

	return svg;
}
 
QString Dip::makeBreadboardSvg(const QString & expectedFileName) 
{
	if (expectedFileName.contains("_sip_")) return makeBreadboardSipSvg(expectedFileName);
	if (expectedFileName.contains("_dip_")) return makeBreadboardDipSvg(expectedFileName);
	return "";
}

QString Dip::makeBreadboardSipSvg(const QString & expectedFileName) 
{
	QStringList pieces = expectedFileName.split("_");
	if (pieces.count() != 5) return "";

	int pins = pieces.at(2).toInt();
	int increment = 10;
	qreal totalWidth = (pins * increment);

	QString svg = TextUtils::incrementTemplate(":/resources/templates/generic_sip_bread_template.txt", 1, increment * (pins - 2), incMultiplyPinFunction, noCopyPinFunction);

	QString repeat("<rect id='connector%1pin' x='[13.5]' y='25.66' fill='#8C8C8C' width='3' height='4.34'/>\n"
					"<rect id='connector%1terminal' x='[13.5]' y='27.0' fill='#8C8C8C' width='3' height='3'/>\n"
					"<polygon fill='#8C8C8C' points='[11.5],25.66 [11.5],26.74 [13.5],27.66 [16.5],27.66 [18.5],26.74 [18.5],25.66'/>\n"); 

	QString repeats;
	if (pins > 2) {
		repeats = TextUtils::incrementTemplateString(repeat, pins - 2, increment, TextUtils::standardMultiplyPinFunction, incCopyPinFunction);
	}

	return svg.arg(totalWidth / 100).arg(pins - 1).arg(repeats);

}

QString Dip::makeBreadboardDipSvg(const QString & expectedFileName) 
{
	QStringList pieces = expectedFileName.split("_");
	if (pieces.count() != 6) return "";

	int pins = pieces.at(3).toInt();
	int increment = 10;
	qreal spacing = TextUtils::convertToInches(pieces.at(4)) * 100;

	QString repeatB("<rect id='connector%1pin' x='{13.5}' y='[28.66]' fill='#8C8C8C' width='3' height='4.34'/>\n"
					"<rect id='connector%1terminal' x='{13.5}' y='[30.0]' fill='#8C8C8C' width='3' height='3'/>\n"
					"<polygon fill='#8C8C8C' points='{11.5},[28.66] {11.5},[29.74] {13.5},[30.66] {16.5},[30.66] {18.5},[29.74] {18.5},[28.66]'/> \n"); 

	QString repeatT("<rect id='connector%1pin' x='[13.5]' y='0' fill='#8C8C8C' width='3' height='4.34'/>\n"
					"<rect id='connector%1terminal' x='[13.5]' fill='#8C8C8C' width='3' height='3'/>\n"
					"<polygon fill='#8C8C8C' points='[18.5],4.34 [18.5],3.26 [16.5],2.34 [13.5],2.34 [11.5],3.26 [11.5],4.34'/>\n");

	QString header("<?xml version='1.0' encoding='utf-8'?>\n"
					"<svg version='1.2' baseProfile='tiny' id='svg2' xmlns='http://www.w3.org/2000/svg' \n"
					"width='.percent.1in' height='%1in' viewBox='0 0 {20.0} [33.0]' >\n"
					"<g id='breadboard'>\n"
					"<rect id='middle' x='0' y='3' fill='#303030' width='{20.0}' height='[27.0]' />\n"
					"<rect id='top' x='0' y='3.0' fill='#3D3D3D' width='{20.0}' height='2.46'/>\n"
					"<rect id='bottom' x='0' y='[27.54]' fill='#000000' width='{20.0}' height='2.46'/>\n"
					"<polygon id='right' fill='#141414' points='{20.0},3 {19.25},5.46 {19.25},[27.54] {20.0},[30.0]'/> \n"
					"<polygon id='left' fill='#1F1F1F' points='0,3 0.75,5.46 0.75,[27.54] 0,[30.0]'/>\n"
					"<polygon id='left-upper-rect' fill='#1C1C1C' points='5,{{11.5}} 0.75,{{11.46}} 0.56,{{13.58}} 0.56,{{16.5}} 5,{{16.5}}'/>\n"
					"<polygon id='left-lower-rect' fill='#383838' points='0.75,{{21.55}} 5,{{21.55}} 5,{{16.5}} 0.56,{{16.5}} 0.56,{{19.42}}'/>\n"
					"<path id='slot' fill='#262626' d='M0.56,{{13.58}}v5.83c1.47-0.17,2.62-1.4,2.62-2.92C3.18,{{14.97}},2.04,{{13.75}},0.56,{{13.58}}z'/>\n"
					"<path id='cover' fill='#303030' d='M0.75,5.46V{{11.45}}c2.38,0.45,4.19,2.53,4.19,5.04c0,2.51-1.8,4.6-4.19,5.05V{{21.55}}h5.0V5.46H0.75z'/>\n"
					"<circle fill='#212121' cx='6.5' cy='[23.47]' r='2.06'/>\n"
					"<text id='label' x='6.5' y='{{16.5}}' fill='#e6e6e6' stroke='none' font-family='OCRA' text-anchor='start' font-size='8' >IC</text>\n"

					"<rect id='connector0pin' x='3.5' y='[28.66]' fill='#8C8C8C' width='3' height='4.34'/>\n"
					"<rect id='connector0terminal' x='3.5' y='[30.0]' fill='#8C8C8C' width='3' height='3'/>\n"
					"<polygon fill='#8C8C8C' points='3.5,[28.66] 3.5,[30.66] 6.5,[30.66] 8.5,[29.74] 8.5,[28.66] '/>\n"

					"<rect id='connector%2pin' x='3.5' fill='#8C8C8C' width='3' height='4.34'/>\n"
					"<rect id='connector%2terminal' x='3.5' fill='#8C8C8C' width='3' height='3'/>\n"
					"<polygon fill='#8C8C8C' points='8.5,4.34 8.5,3.26 6.5,2.34 3.5,2.34 3.5,4.34'/>\n"

					"<rect id='connector%3pin' x='{13.5}' y='[28.66]' fill='#8C8C8C' width='3' height='4.34'/>\n"
					"<rect id='connector%3terminal' x='{13.5}' y='[30.0]' fill='#8C8C8C' width='3' height='3'/>\n"
					"<polygon fill='#8C8C8C' points='{11.5},[28.66] {11.5},[29.74] {13.5},[30.66] {16.5},[30.66] {16.5},[28.66]'/>\n"
					
					"<rect id='connector%4pin' x='{13.5}' y='0' fill='#8C8C8C' width='3' height='4.34'/>\n"
					"<rect id='connector%4terminal' x='{13.5}' y='0' fill='#8C8C8C' width='3' height='3'/>\n"
					"<polygon fill='#8C8C8C' points='{16.5},4.34 {16.5},2.34 {13.5},2.34 {11.5},3.26 {11.5},4.34 '/>\n"
 
					".percent.2\n"
					".percent.3\n"

					"</g>\n"
					"</svg>\n");

	// header came from a 300mil dip, so base case is spacing - (increment * 3)

	header = TextUtils::incrementTemplateString(header, 1, spacing - (increment * 3), incMultiplyPinFunction, noCopyPinFunction);
	header = header.arg(getViewBoxCoord(header, 3) / 100.0).arg(pins - 1).arg((pins / 2) - 1).arg(pins / 2);
	header.replace("{{", "[");
	header.replace("}}", "]");
	header = TextUtils::incrementTemplateString(header, 1, (spacing - (increment * 3)) / 2, incMultiplyPinFunction, noCopyPinFunction);
	header.replace("{", "[");
	header.replace("}", "]");
	header.replace(".percent.", "%");

	QString svg = TextUtils::incrementTemplateString(header, 1, increment * ((pins - 4) / 2), incMultiplyPinFunction, noCopyPinFunction);

	repeatB = TextUtils::incrementTemplateString(repeatB, 1, spacing - (increment * 3), incMultiplyPinFunction, noCopyPinFunction);
	repeatB.replace("{", "[");
	repeatB.replace("}", "]");

	NoExcusePins = pins - 1;
	QString repeatTs = TextUtils::incrementTemplateString(repeatT, (pins - 4) / 2, increment, TextUtils::standardMultiplyPinFunction, negCopyPinFunction);
	QString repeatBs = TextUtils::incrementTemplateString(repeatB, (pins - 4) / 2, increment, TextUtils::standardMultiplyPinFunction, incCopyPinFunction);


	return svg.arg(getViewBoxCoord(svg, 2) / 100.0).arg(repeatTs).arg(repeatBs);
}
