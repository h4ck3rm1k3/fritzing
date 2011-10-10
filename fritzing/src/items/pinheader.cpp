/*******************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2011 Fachhochschule Potsdam - http://fh-potsdam.de

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

#include "pinheader.h"
#include "../utils/graphicsutils.h"
#include "../fsvgrenderer.h"
#include "../sketch/infographicsview.h"
#include "../commands.h"
#include "../utils/textutils.h"
#include "../layerattributes.h"
#include "partlabel.h"

#include <QDomNodeList>
#include <QDomDocument>
#include <QDomElement>
#include <QLineEdit>

//////////////////////////////////////////////////

static QStringList Forms;
QString PinHeader::FemaleFormString;
QString PinHeader::FemaleRoundedFormString;
QString PinHeader::MaleFormString;
QString PinHeader::ShroudedFormString;
static int MinPins = 1;
static int MinShroudPins = 2;
static int MaxPins = 64;
static QHash<QString, QString> Spacings;


PinHeader::PinHeader( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_changingForm = false;
	m_form = modelPart->prop("form").toString();
	if (m_form.isEmpty()) {
		m_form = modelPart->properties().value("form", FemaleFormString);
		modelPart->setProp("form", m_form);
	}

	m_renderer = NULL;
}

PinHeader::~PinHeader() {
}


void PinHeader::initNames() {
	if (FemaleFormString.isEmpty()) {
		FemaleFormString = FemaleSymbolString + " (female)";
		FemaleRoundedFormString = FemaleSymbolString + " (female rounded)";
		MaleFormString = MaleSymbolString + " (male)";
		ShroudedFormString = MaleSymbolString + " (shrouded male)";
	}
}

void PinHeader::setProp(const QString & prop, const QString & value) {
	if (prop.compare("form", Qt::CaseInsensitive) == 0) {
		setForm(value, false);
		return;
	}

	PaletteItem::setProp(prop, value);
}

void PinHeader::setForm(QString form, bool force) {
	if (!force && m_form.compare(form) == 0) return;

	switch (this->m_viewIdentifier) {
		case ViewIdentifierClass::PCBView:
			if (form.contains("shroud")) {
				InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
				if (infoGraphicsView == NULL) break;

				// hack the dom element and call setUpImage
				FSvgRenderer::removeFromHash(moduleID(), "");
				QDomElement element = LayerAttributes::getSvgElementLayers(modelPart()->domDocument(), m_viewIdentifier);
				if (element.isNull()) break;

				QString filename = element.attribute("image");
				if (filename.isEmpty()) break;

				filename.replace("jumper", "shroud");
				element.setAttribute("image", filename);

				m_changingForm = true;
				resetImage(infoGraphicsView);
				m_changingForm = false;
				
				updateConnections();
			}
			break;
		case ViewIdentifierClass::BreadboardView:
		case ViewIdentifierClass::SchematicView:
			{
				InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
				if (infoGraphicsView == NULL) break;

				// hack the dom element and call setUpImage
				FSvgRenderer::removeFromHash(moduleID(), "");
				QDomElement element = LayerAttributes::getSvgElementLayers(modelPart()->domDocument(), m_viewIdentifier);
				if (element.isNull()) break;

				QString filename = element.attribute("image");
				if (filename.isEmpty()) break;

				QString genericString("generic_");
				int gix = filename.indexOf(genericString);
				if (gix < 0) break;

				gix += genericString.length();
				int pix = filename.indexOf("_pin_header");
				if (pix < 0) break;

				QString prefix = filename.left(gix);
				QString suffix = filename.remove(0, pix);

				if (form.contains("(male)", Qt::CaseInsensitive)) {
					filename = prefix + "male" + suffix;
				}
				else if (form.contains("rounded", Qt::CaseInsensitive)) {
					filename = prefix + 
						((this->m_viewIdentifier == ViewIdentifierClass::SchematicView) ? "female" : "rounded_female") + 
						suffix;
				}
				else if (form.contains("(female)", Qt::CaseInsensitive)) {
					filename = prefix + "female" + suffix;
				}
				else if (form.contains("shroud", Qt::CaseInsensitive)) {
					filename = prefix + "shrouded" + suffix;
				}
				else {
					break;
				}

				element.setAttribute("image", filename);

				m_changingForm = true;
				resetImage(infoGraphicsView);
				m_changingForm = false;
				
				updateConnections();
			}
			break;
		default:
			break;
	}

	m_form = form;
	modelPart()->setProp("form", form);

	updateTooltip();
    if (m_partLabel) m_partLabel->displayTextsIf();

}

QStringList PinHeader::collectValues(const QString & family, const QString & prop, QString & value) {
	if (prop.compare("form", Qt::CaseInsensitive) == 0) {
		QStringList values;
		foreach (QString f, forms()) {
			values.append(f);
		}

		value = m_form;
		return values;
	}

	if (prop.compare("pins", Qt::CaseInsensitive) == 0) {
		QStringList values;
		value = modelPart()->properties().value("pins");

		int step = 1;
		int minP = MinPins;
		if (m_form.contains("shroud")) {
			minP = MinShroudPins;
			step = 2;
		}

		for (int i = minP; i <= MaxPins; i += step) {
			values << QString::number(i);
		}
		
		return values;
	}

	if (prop.compare("pin spacing", Qt::CaseInsensitive) == 0) {
		initSpacings();
		QStringList values;

		value = modelPart()->properties().value("pin spacing");
		if (m_form.contains("shroud")) {
			values.clear();
			values.append(value);
		}
		else {
			values = Spacings.values();
		}
		
		return values;
	}


	return PaletteItem::collectValues(family, prop, value);
}

QString PinHeader::getProperty(const QString & key) {
	if (key.compare("form", Qt::CaseInsensitive) == 0) {
		return m_form;
	}

	return PaletteItem::getProperty(key);
}

void PinHeader::addedToScene(bool temporary)
{
	if (this->scene()) {
		setForm(m_form, true);
	}

    return PaletteItem::addedToScene(temporary);
}

ConnectorItem* PinHeader::newConnectorItem(Connector *connector) {
	if (m_changingForm) {
		return connector->connectorItemByViewLayerID(viewIdentifier(), viewLayerID());
	}

	return PaletteItem::newConnectorItem(connector);
}

ConnectorItem* PinHeader::newConnectorItem(ItemBase * layerKin, Connector *connector) {
	if (m_changingForm) {
		return connector->connectorItemByViewLayerID(viewIdentifier(), layerKin->viewLayerID());
	}

	return PaletteItem::newConnectorItem(layerKin, connector);
}

const QString & PinHeader::form() {
	return m_form;
}

const QStringList & PinHeader::forms() {
	if (Forms.count() == 0) {
		Forms << FemaleFormString << FemaleRoundedFormString << MaleFormString << ShroudedFormString;
	}
	return Forms;
}

bool PinHeader::onlyFormChanges(QMap<QString, QString> & propsMap) {
	if (propsMap.value("form", "").compare(m_form) == 0) return false;

	if (modelPart()->properties().value("pins", "").compare(propsMap.value("pins", "")) != 0) return false;

	return true;
}

ItemBase::PluralType PinHeader::isPlural() {
	return Plural;
}

QString PinHeader::genFZP(const QString & moduleid)
{
	initSpacings();
	QStringList pieces = moduleid.split("_");
	if (pieces.count() != 6) return "";

	QString spacing = pieces.at(5);

	QString result = PaletteItem::genFZP(moduleid, "generic_female_pin_header_fzpTemplate", MinPins, MaxPins, 1); 
	result.replace(".percent.", "%");
	return result.arg(Spacings.value(spacing, "")).arg(spacing).arg(forms().at(0)); 
}

QString PinHeader::genModuleID(QMap<QString, QString> & currPropsMap)
{
	initSpacings();
	QString pins = currPropsMap.value("pins");
	QString spacing = currPropsMap.value("pin spacing");
	QString form = currPropsMap.value("form");

	foreach (QString key, Spacings.keys()) {
		if (Spacings.value(key).compare(spacing, Qt::CaseInsensitive) == 0) {
			return QString("generic_female_pin_header_%1_%2").arg(pins).arg(key);
		}
	}

	return "";
}

QString PinHeader::makePcbSvg(const QString & expectedFileName) 
{
	initSpacings();

	QStringList pieces = expectedFileName.split("_");
	if (pieces.count() != 4) return "";

	int pins = pieces.at(1).toInt();
	QString spacingString = pieces.at(2);

	if (expectedFileName.contains("shroud")) {
		return makePcbShroudSvg(pins);
	}

	static QString pcbLayerTemplate = "";

	QFile file(":/resources/templates/jumper_pcb_svg_template.txt");
	file.open(QFile::ReadOnly);
	pcbLayerTemplate = file.readAll();
	file.close();

	double outerBorder = 15;
	double innerBorder = outerBorder / 2;
	double silkStrokeWidth = 10;
	double radius = 27.5;
	double copperStrokeWidth = 20;
	double totalWidth = (outerBorder * 2) + (silkStrokeWidth * 2) + (innerBorder * 2) + (radius * 2) + copperStrokeWidth;
	double center = totalWidth / 2;
	double spacing =TextUtils::convertToInches(spacingString) * GraphicsUtils::StandardFritzingDPI; 

	QString middle;

	middle += QString( "<rect fill='none' height='%1' width='%1' stroke='rgb(255, 191, 0)' stroke-width='%2' x='%3' y='%3'/>\n")
					.arg(radius * 2)
					.arg(copperStrokeWidth)
					.arg(center - radius);
	for (int i = 0; i < pins; i++) {
		middle += QString("<circle cx='%1' cy='%2' fill='none' id='connector%3pin' r='%4' stroke='rgb(255, 191, 0)' stroke-width='%5'/>\n")
					.arg(center)
					.arg(center + (i * spacing)) 
					.arg(i)
					.arg(radius)
					.arg(copperStrokeWidth);
	}

	double totalHeight = totalWidth + (pins * spacing) - spacing;

	QString svg = pcbLayerTemplate
					.arg(totalWidth / GraphicsUtils::StandardFritzingDPI)
					.arg(totalHeight / GraphicsUtils::StandardFritzingDPI)
					.arg(totalWidth)
					.arg(totalHeight)
					.arg(totalWidth - outerBorder - (silkStrokeWidth / 2))
					.arg(totalHeight - outerBorder - (silkStrokeWidth / 2))
					.arg(totalWidth - outerBorder - (silkStrokeWidth / 2))
					.arg(silkStrokeWidth)
					.arg(silkStrokeWidth / 2)
					.arg(middle);

	return svg;
}

void PinHeader::initSpacings() {
	if (Spacings.count() == 0) {
		Spacings.insert("100mil", "0.1in (2.54mm)");
		Spacings.insert("200mil", "0.2in (5.08mm)");
	}
}


QString PinHeader::makeSchematicSvg(const QString & expectedFileName) 
{
	QStringList pieces = expectedFileName.split("_");
	if (pieces.count() != 7) return "";

	int pins = pieces.at(4).toInt();
	QString form = pieces.at(1);
	double unitHeight = 0.27;  // inches
	double unitHeightPoints = unitHeight * 72;

	QString header("<?xml version='1.0' encoding='utf-8'?>\n"
				"<svg version='1.2' baseProfile='tiny' id='svg2' xmlns:svg='http://www.w3.org/2000/svg' "
				"xmlns='http://www.w3.org/2000/svg'  x='0in' y='0in' width='0.87in' "
				"height='%1in' viewBox='0 0 62.641 %2'>\n"
				"<g id='schematic' >\n");


	QString svg = header.arg(unitHeight * pins).arg(unitHeightPoints * pins);

	svg += TextUtils::incrementTemplate(QString(":/resources/templates/generic_%1_pin_header_schem_template.txt").arg(form.contains("female") ? "female" : "male"),
							 pins, unitHeightPoints, TextUtils::standardMultiplyPinFunction, TextUtils::standardCopyPinFunction, NULL);
		

	svg += "</g>\n</svg>";

	return svg;
}

QString PinHeader::makeBreadboardSvg(const QString & expectedFileName) 
{
	QStringList pieces = expectedFileName.split("_");
	if (pieces.count() < 7) return "";

	int pinIndex = 4;
	if (pieces.count() == 8) pinIndex++;
	QString form = pieces.at(1);

	int pins = pieces.at(pinIndex).toInt();
	if (form.contains("shroud")) {
		return makeBreadboardShroudSvg(pins);
	}

	double unitHeight = 0.1;  // inches
	double unitHeightPoints = unitHeight * 10000;

	QString header("<?xml version='1.0' encoding='utf-8'?>\n"
				"<svg version='1.2' baseProfile='tiny' "
				"xmlns='http://www.w3.org/2000/svg'  x='0in' y='0in' width='%1in' "
				"height='0.1in' viewBox='0 0 %2 1000'>\n"
				"<g id='breadboard' >\n");

	QString fileForm;
	if (form.contains("round")) {
		fileForm = "rounded_female";
		header += "<rect fill='#404040' width='%2' height='1000'/>\n";
	}
	else if (form.contains("female")) {
		fileForm = "female";
		header += "<rect fill='#404040' width='%2' height='1000'/>\n";
	}
	else {
		fileForm = "male";
	}

	QString svg = header.arg(unitHeight * pins).arg(unitHeightPoints * pins);
	svg += TextUtils::incrementTemplate(QString(":/resources/templates/generic_%1_pin_header_bread_template.txt").arg(fileForm),
							 pins, unitHeightPoints, TextUtils::standardMultiplyPinFunction, TextUtils::standardCopyPinFunction, NULL);

	svg += "</g>\n</svg>";

	return svg;
}


QString  PinHeader::findForm(const QString & filename)
{
	if (filename.contains("rounded")) return FemaleRoundedFormString;
	if (filename.contains("female")) return FemaleFormString;
	if (filename.contains("shroud")) return ShroudedFormString;
	return MaleFormString;
}

QString PinHeader::makeBreadboardShroudSvg(int pins) 
{
	QString header("<?xml version='1.0' encoding='utf-8'?>\n"
					"<svg version='1.2' baseProfile='tiny' "
					"xmlns='http://www.w3.org/2000/svg'  x='0in' y='0in' width='%1in' "
					"height='0.3484167in' viewBox='0 0 [400] 348.4167in'>\n"
					"<g id='breadboard' >\n"
					"<rect id='bgnd' x='0' y='0' width='[400]' height='348.4167' stroke='none' stroke-width='0' fill='#1a1a1a' />\n"
					"<rect id='top inset' x='0' y='0' width='[400]' height='38.861' stroke='none' stroke-width='0' fill='#2a2a29' />\n"
					"<rect id='bottom inset' x='0' y='309.5557' width='[400]' height='38.861' stroke='none' stroke-width='0' fill='#595959' />\n"
					"<path id='left inset'  d='M0,0 0,348.4167 38.861,309.5557 38.861,38.861z' stroke='none' stroke-width='0' fill='#373737' />\n"
					"<path id='right inset'  d='M[400],0 [400],348.4167 [361.139],309.5557 [361.139],38.861z' stroke='none' stroke-width='0' fill='#474747' />\n"
					"<rect id='top border' x='0' y='0' width='[400]' height='24.972' stroke='none' stroke-width='0' fill='#404040' />\n"
					"<rect id='bottom border' x='0' y='323.4447' width='[400]' height='24.972' stroke='none' stroke-width='0' fill='#404040' />\n"
					"<rect id='left border' x='0' y='0' width='24.972' height='348.4167' stroke='none' stroke-width='0' fill='#404040' />\n"
					"<rect id='right border' x='[375.028]' y='0' width='24.972' height='348.4167' stroke='none' stroke-width='0' fill='#404040' />\n"
					"%2\n"
					"%3\n"
					"<rect id='slot' x='{115.9514}' y='280.4973' width='168.0972' height='67.9194' stroke='none' stroke-width='0' fill='#1a1a1a' />\n"
					"</g>\n"
					"</svg>\n"
				);

	QString repeatT("<rect id='upper connector bgnd' x='[173.618055]' y='95.972535' width='52.76389' height='52.76389' stroke='none' stroke-width='0' fill='#141414' />\n"
					"<rect id='connector%1pin' x='[184.03472]' y='106.3892' width='31.93056' height='31.93056' stroke='none' stroke-width='0' fill='#8c8663' />\n"
					"<rect id='upper connector top inset' x='[184.03472]' y='106.3892' width='31.93056' height='7.75' stroke='none' stroke-width='0' fill='#B8AF82' />\n"
					"<rect id='upper connector bottom inset' x='[184.03472]' y='130.56976' width='31.93056' height='7.75' stroke='none' stroke-width='0' fill='#5E5B43' />\n"
					"<path id='upper connector left inset' d='M[184.03472],106.3892 [184.03472],138.31976 [191.7847],130.56976 [191.7847],114.1392z' stroke='none' stroke-width='0' fill='#9A916C' />\n"
					"<path id='upper connector right inset' d='M[215.96522],106.3892 [215.96522],138.31976 [208.21522],130.56976 [208.21522],114.1392z' stroke='none' stroke-width='0' fill='#9A916C' />\n"
				);
					
	QString repeatB("<rect id='lower connector bgnd' x='[173.618055]' y='195.972535' width='52.76389' height='52.76389' stroke='none' stroke-width='0' fill='#141414' />\n"
					"<rect id='connector%1pin' x='[184.03472]' y='206.3892' width='31.93056' height='31.93056' stroke='none' stroke-width='0' fill='#8c8663' />\n"
					"<rect id='lower connector top inset' x='[184.03472]' y='206.3892' width='31.93056' height='7.75' stroke='none' stroke-width='0' fill='#B8AF82' />\n"
					"<rect id='lower connector bottom inset' x='[184.03472]' y='230.56976' width='31.93056' height='7.75' stroke='none' stroke-width='0' fill='#5E5B43' />\n"
					"<path id='lower connector left inset' d='M[184.03472],206.3892 [184.03472],238.31976 [191.7847],230.56976 [191.7847],214.1392z' stroke='none' stroke-width='0' fill='#9A916C' />\n"
					"<path id='lower connector right inset' d='M[215.96522],206.3892 [215.96522],238.31976 [208.21522],230.56976 [208.21522],214.1392z' stroke='none' stroke-width='0' fill='#9A916C' />\n"
				);


	double increment = 100;  // 0.1in

	QString svg = TextUtils::incrementTemplateString(header, 1, increment * (pins - 2) / 2, TextUtils::incMultiplyPinFunction, TextUtils::noCopyPinFunction, NULL);
	svg.replace("{", "[");
	svg.replace("}", "]");
	svg = TextUtils::incrementTemplateString(svg, 1, increment * (pins - 2) / 4, TextUtils::incMultiplyPinFunction, TextUtils::noCopyPinFunction, NULL);

	int userData[2];
	userData[0] = pins;
	userData[1] = 1;
	QString repeatTs = TextUtils::incrementTemplateString(repeatT, pins / 2, increment, TextUtils::standardMultiplyPinFunction, TextUtils::negIncCopyPinFunction, userData);
	QString repeatBs = TextUtils::incrementTemplateString(repeatB, pins / 2, increment, TextUtils::standardMultiplyPinFunction, TextUtils::standardCopyPinFunction, NULL);

	return svg.arg(TextUtils::getViewBoxCoord(svg, 2) / 1000.0).arg(repeatTs).arg(repeatBs);
}

QString PinHeader::makePcbShroudSvg(int pins) 
{
	QString header("<?xml version='1.0' encoding='utf-8'?>\n"
					"<svg version='1.2' baseProfile='tiny' xmlns='http://www.w3.org/2000/svg' \n"
					"x='0in' y='0in' width='0.3542in' height='%1in' viewBox='0 0 3542 [3952]'>"
					"<g id='copper0' >\n"					
					"<g id='copper1' >\n"
					"%2\n"
					"%3\n"
					"</g>\n"
					"</g>\n"
					"<g id='silkscreen' >\n"					
					"<rect x='40' y='40' width='3462' height='[3872]' fill='none' stroke='#ffffff' stroke-width='80'/>\n"	
					"<path d='m473,{1150} 0,-{677} 2596,0 0,[3076] -2596,0 0,-{677}' fill='none' stroke='#ffffff' stroke-width='80'/>\n"	
					"<rect x='40' y='{1150}' width='550' height='1652' fill='none' stroke='#ffffff' stroke-width='80'/>\n"	
					"</g>\n"
					"</svg>\n"
				);


	QString repeatL = "<circle id='connector%1pin' cx='1221' cy='[1976]' stroke='#ff9400' r='285' fill='none' stroke-width='170'/>\n";
	QString repeatR = "<circle id='connector%1pin' cx='2221' cy='[1976]' stroke='#ff9400' r='285' fill='none' stroke-width='170'/>\n";


	double increment = 1000;  // 0.1in

	QString svg = TextUtils::incrementTemplateString(header, 1, increment * (pins - 2) / 2, TextUtils::incMultiplyPinFunction, TextUtils::noCopyPinFunction, NULL);
	svg.replace("{", "[");
	svg.replace("}", "]");
	svg = TextUtils::incrementTemplateString(svg, 1, increment * (pins - 2) / 4, TextUtils::incMultiplyPinFunction, TextUtils::noCopyPinFunction, NULL);

	int userData[2];
	userData[0] = pins;
	userData[1] = 1;
	QString repeatLs = TextUtils::incrementTemplateString(repeatR, pins / 2, increment, TextUtils::standardMultiplyPinFunction, TextUtils::negIncCopyPinFunction, userData);
	QString repeatRs = TextUtils::incrementTemplateString(repeatL, pins / 2, increment, TextUtils::standardMultiplyPinFunction, TextUtils::standardCopyPinFunction, NULL);

	return svg.arg(TextUtils::getViewBoxCoord(svg, 3) / 10000.0).arg(repeatLs).arg(repeatRs);
}

