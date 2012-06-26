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
#include "../commands.h"
#include "../utils/textutils.h"
#include "../layerattributes.h"
#include "partlabel.h"
#include "partfactory.h"
#include "../sketch/infographicsview.h"
#include "../connectors/connectoritem.h"
#include "../connectors/connector.h"
#include "../utils/familypropertycombobox.h"

#include <QDomNodeList>
#include <QDomDocument>
#include <QDomElement>
#include <QLineEdit>
#include <QRegExp>

//////////////////////////////////////////////////

static QStringList Forms;

QString PinHeader::FemaleFormString;
QString PinHeader::FemaleRoundedFormString;
QString PinHeader::MaleFormString;
QString PinHeader::ShroudedFormString;

static int MinPins = 1;
static int MinShroudedPins = 6;
static int MaxPins = 64;
static QHash<QString, QString> Spacings;
static QString ShroudedSpacing;

static HoleClassThing TheHoleThing;

PinHeader::PinHeader( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{

    setUpHoleSizes("pinheader", TheHoleThing);

    m_form = modelPart->localProp("form").toString();
	if (m_form.isEmpty()) {
		m_form = modelPart->properties().value("form", FemaleFormString);
		modelPart->setLocalProp("form", m_form);
	}
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

QStringList PinHeader::collectValues(const QString & family, const QString & prop, QString & value) {
	if (prop.compare("form", Qt::CaseInsensitive) == 0) {
		QStringList values;
		foreach (QString f, forms()) {
			values.append(f);
		}

		value = m_form;
		return values;
	}

	if (prop.compare("rows", Qt::CaseInsensitive) == 0) {
		QStringList values;
		values.append("single");
        values.append("double");

		value = (moduleID().contains("double") || moduleID().contains("shrouded")) ? "double" : "single";
		return values;
	}

	if (prop.compare("package", Qt::CaseInsensitive) == 0) {
		QStringList values;
		values.append("through-hole");
        values.append("SMD");

		value = moduleID().contains("smd") ? "SMD" : "through-hole";
		return values;
	}

	if (prop.compare("pins", Qt::CaseInsensitive) == 0) {
		QStringList values;
		value = modelPart()->properties().value("pins");

		int step = 1;
		int minP = MinPins;
		if (m_form.contains("shrouded")) {
			minP = MinShroudedPins;
			step = 2;
		}
		if (m_form.contains("double")) {
			step = 2;
			minP = 2;
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
		if (m_form.contains("shrouded")) {
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
		//setForm(m_form, true);
	}

    return PaletteItem::addedToScene(temporary);
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

ItemBase::PluralType PinHeader::isPlural() {
	return Plural;
}

QString PinHeader::genFZP(const QString & moduleID)
{
	initSpacings();

    QString useModuleID = moduleID;
    int hsix = useModuleID.lastIndexOf(HoleSizePrefix);
    if (hsix >= 0) useModuleID.truncate(hsix);

	QStringList pieces = useModuleID.split("_");
	QString spacing = pieces.at(pieces.count() - 1);

	QString result = PaletteItem::genFZP(useModuleID, "generic_female_pin_header_fzpTemplate", MinPins, MaxPins, 1, useModuleID.contains("smd")); 
	result.replace(".percent.", "%");
	QString form = MaleFormString;
	QString formBread = "male";
	QString formText = formBread;
	QString formSchematic = formBread;
	QString formModule = formBread;
    QString formPackage = useModuleID.contains("smd", Qt::CaseInsensitive) ? "smd" : "tht";
    bool isDouble = useModuleID.contains("shrouded") || useModuleID.contains("double");
    QString formRow = isDouble ? "double" : "single";
	if (useModuleID.contains("rounded")) {
		form = FemaleRoundedFormString;
		if (useModuleID.contains("smd")) {
            formText = formRow + " row SMD rounded female";
            formModule = formText.toLower();
            formModule.replace(' ', '_');
            formBread = "rounded_female";
		}
		else {
            if (isDouble) {
			    formBread = formModule = "double_row_rounded_female";
                formText = "double row rounded female";
            }
            else {
			    formBread = formModule = "rounded_female";
                formText = "rounded female";
            }
		}
		formSchematic = "female";
	}
	else if (useModuleID.contains("female")) {
	    form = FemaleFormString;
		if (useModuleID.contains("smd")) {
            formText = formRow + " row SMD female";
            formModule = formText.toLower();
            formModule.replace(' ', '_');
            formBread = "female";
		}
		else {
            if (isDouble) {
			    formBread = formModule = "double_row_female";
                formText = "double row female";
            }
            else {
			    formBread = formModule = formText = "female";
            }
		}
        formSchematic = "female";
	}
	else if (useModuleID.contains("shrouded")) {
		form = ShroudedFormString;
		formText = formBread = formModule = "shrouded";
	}
	else if (useModuleID.contains("smd")) {
        formText = formRow + " row SMD male";
        formModule = formText.toLower();
        formModule.replace(' ', '_');
	}
    else if (isDouble) {
	    formBread = formModule = "double_row_male";
        formText = "double row male";
    }

	result = result
        .arg(Spacings.value(spacing, ""))
        .arg(spacing)
        .arg(form)
        .arg(formBread)
        .arg(formText)
        .arg(formSchematic)
        .arg(formModule)
        .arg(formRow)
        .arg(formPackage)
        ; 
	if (useModuleID.contains("smd")) {
		result.replace("nsjumper", QString("smd_%1_row_pin_header").arg(formRow));
		result.replace("jumper", QString("smd_%1_row_pin_header").arg(formRow));
	}
	else if (useModuleID.contains("shrouded")) {
		result.replace("nsjumper", "shrouded");
		result.replace("jumper", "shrouded");
	}

    if (isDouble) {
        result.replace("jumper", "jumper_double");
    }

    if (hsix >= 0) {
        return hackFzpHoleSize(result, moduleID, hsix);
    }

	return result;
}

QString PinHeader::genModuleID(QMap<QString, QString> & currPropsMap)
{
	initSpacings();
	QString pins = currPropsMap.value("pins");
	int p = pins.toInt();
	QString spacing = currPropsMap.value("pin spacing");
	if (spacing.isEmpty()) spacing = ShroudedSpacing;
	QString form = currPropsMap.value("form").toLower();
    QString package = currPropsMap.value("package").toLower();
    QString row = currPropsMap.value("row").toLower();
	QString formWord = "male";
	bool isDouble = false;

	if (form.contains("shrouded")) {
		if (p < MinShroudedPins) {
			pins = QString::number(p = MinShroudedPins);
		}
		isDouble = true; 
		spacing = ShroudedSpacing;
		formWord = "shrouded";
	}
	else if (form.contains("female")) {
        QString ff = form.contains("rounded") ? "rounded_female" : "female";
		if (package.contains("smd")) {
			if (row.contains("single", Qt::CaseInsensitive)) {
				formWord = "single_row_smd_" + ff;
			}
			else {
				isDouble = true;
				formWord = "double_row_smd_" + ff;
			}
		}
		else {
            if (row.contains("single", Qt::CaseInsensitive)) {
			    formWord = ff;
            }
            else {
				isDouble = true;
                formWord = "double_row_" + ff;
            }
		}
	}
	else if (package.contains("smd")) {
		if (row.contains("single")) {
			formWord = "single_row_smd_male";
		}
		else {
			isDouble = true;
			formWord = "double_row_smd_male";
		}
	}
    else if (row.contains("double", Qt::CaseInsensitive)) {
		formWord = "double_row_male";
	    isDouble = true;
    }

	if (isDouble && (p % 2 == 1)) {
		pins = QString::number(p + 1);
	}

	foreach (QString key, Spacings.keys()) {
		if (Spacings.value(key).compare(spacing, Qt::CaseInsensitive) == 0) {
			return QString("generic_%1_pin_header_%2_%3").arg(formWord).arg(pins).arg(key);
		}
	}

	return "";
}

QString PinHeader::makePcbSvg(const QString & originalExpectedFileName, const QString & moduleID) 
{
    QString expectedFileName = originalExpectedFileName;
    int hsix = expectedFileName.indexOf(HoleSizePrefix);
    if (hsix >= 0) {
        expectedFileName.truncate(hsix);
    }
	initSpacings();

	if (expectedFileName.contains("smd")) {
		return makePcbSMDSvg(expectedFileName, moduleID);
	}

    QString spacingString;
    int pins = getPinsAndSpacing(expectedFileName, spacingString);
    if (pins == 0) return "";

    QString svg;

    bool shrouded = false;
	if (expectedFileName.contains("shrouded")) {
		svg = makePcbShroudedSvg(pins);
        shrouded = true;
	}
    else {
	    static QString pcbLayerTemplate = "";
	    static QString pcbLayerTemplate2 = "";

        if (pcbLayerTemplate.isEmpty()) {
	        QFile file(":/resources/templates/jumper_pcb_svg_template.txt");
	        file.open(QFile::ReadOnly);
	        pcbLayerTemplate = file.readAll();
	        file.close();
            QFile file2(":/resources/templates/jumper_pcb_svg_2nd_template.txt");
	        file2.open(QFile::ReadOnly);
	        pcbLayerTemplate2 = file2.readAll();
	        file2.close();
        }

        bool isDouble = expectedFileName.contains("double");
        QString useTemplate = isDouble ? pcbLayerTemplate2 : pcbLayerTemplate;

	    double outerBorder = 15;
	    double innerBorder = outerBorder / 2;
	    double silkStrokeWidth = 10;
	    double standardRadius = 27.5;
        double radius = 29;
	    double copperStrokeWidth = 20;
	    double totalWidth = (outerBorder * 2) + (silkStrokeWidth * 2) + (innerBorder * 2) + (standardRadius * 2) + copperStrokeWidth;
	    double center = totalWidth / 2;
	    double spacing = TextUtils::convertToInches(spacingString) * GraphicsUtils::StandardFritzingDPI; 


	    QString middle;

        bool addSquare = false;
        if (expectedFileName.contains("nsjumper")) {
        }
        else if (expectedFileName.contains("jumper")) {
            addSquare = true;
        }
        else {
            DebugDialog::debug(QString("square: expected filename is confusing %1 %2").arg(expectedFileName).arg(moduleID));
        }

	    if (addSquare) {
		    middle += QString( "<rect id='square' fill='none' height='%1' width='%1' stroke='rgb(255, 191, 0)' stroke-width='%2' x='%3' y='%3'/>\n")
					    .arg(radius * 2)
					    .arg(copperStrokeWidth)
					    .arg(center - radius);
	    }

        QString circle("<circle cx='%1' cy='%2' fill='none' id='connector%3pin' r='%4' stroke='rgb(255, 191, 0)' stroke-width='%5'/>\n");

        if (isDouble) {
	        for (int i = 0; i < pins / 2; i++) {
		        middle += circle
					        .arg(center)
					        .arg(center + (i * spacing)) 
					        .arg(i)
					        .arg(radius)
					        .arg(copperStrokeWidth);
	        }
	        for (int i = 0; i < pins / 2; i++) {
		        middle += circle
					        .arg(center)
					        .arg(center + (i * spacing)) 
					        .arg(i)
					        .arg(radius)
					        .arg(copperStrokeWidth);
	        }
	        for (int i = pins / 2; i < pins; i++) {
		        middle += circle
					        .arg(center + 100)
					        .arg(center + ((pins - i - 1) * spacing)) 
					        .arg(i)
					        .arg(radius)
					        .arg(copperStrokeWidth);
	        }

        }
        else {
	        for (int i = 0; i < pins; i++) {
		        middle += circle
					        .arg(center)
					        .arg(center + (i * spacing)) 
					        .arg(i)
					        .arg(radius)
					        .arg(copperStrokeWidth);
	        }
        }

	    double totalHeight = totalWidth + (pins * spacing) - spacing;
        double originalTotalWidth = totalWidth;
        if (isDouble) {
            totalHeight = totalWidth + (spacing * pins / 2) - spacing;
            totalWidth += 100;
        }

	    svg = useTemplate
					    .arg(totalWidth / GraphicsUtils::StandardFritzingDPI)
					    .arg(totalHeight / GraphicsUtils::StandardFritzingDPI)
					    .arg(totalWidth)
					    .arg(totalHeight)

					    .arg(totalWidth - outerBorder - (silkStrokeWidth / 2))
					    .arg(totalHeight - outerBorder - (silkStrokeWidth / 2))

					    .arg(originalTotalWidth - outerBorder - (silkStrokeWidth / 2))

					    .arg(silkStrokeWidth)
					    .arg(silkStrokeWidth / 2)
					    .arg(middle);
    }

    if (hsix >= 0) {
        return hackSvgHoleSizeAux(svg, originalExpectedFileName);
    }

	return svg;
}

void PinHeader::initSpacings() {
	if (Spacings.count() == 0) {
		ShroudedSpacing = "0.1in (2.54mm)";
		Spacings.insert("100mil", ShroudedSpacing);
		Spacings.insert("200mil", "0.2in (5.08mm)");
	}
}

QString PinHeader::makeSchematicSvg(const QString & expectedFileName, const QString & moduleID) 
{
    Q_UNUSED(moduleID);
	QStringList pieces = expectedFileName.split("_");
	if (pieces.count() < 7) return "";

	int pins = pieces.at(pieces.count() - 3).toInt();
	QString form = expectedFileName.contains("female") ? "female" :"male";
	double unitHeight = GraphicsUtils::StandardSchematicSeparationMils / 1000;  // inches
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

QString PinHeader::makeBreadboardSvg(const QString & expectedFileName, const QString & moduleID) 
{
    Q_UNUSED(moduleID);
	QStringList pieces = expectedFileName.split("_");
	if (pieces.count() < 7) return "";

	int pinIndex = pieces.count() - 3;

	int pins = pieces.at(pinIndex).toInt();
	if (expectedFileName.contains("shrouded")) {
		return makeBreadboardShroudedSvg(pins);
	}
    if (expectedFileName.contains("double") && !expectedFileName.contains("smd", Qt::CaseInsensitive)) {
        return makeBreadboardDoubleSvg(expectedFileName, pins);
    }

	double unitHeight = 0.1;  // inches
	double unitHeightPoints = unitHeight * 10000;

	QString header("<?xml version='1.0' encoding='utf-8'?>\n"
				"<svg version='1.2' baseProfile='tiny' "
				"xmlns='http://www.w3.org/2000/svg'  x='0in' y='0in' width='%1in' "
				"height='0.1in' viewBox='0 0 %2 1000'>\n"
				"<g id='breadboard' >\n");

	QString fileForm;
	if (expectedFileName.contains("round")) {
		fileForm = "rounded_female";
		header += "<rect fill='#404040' width='%2' height='1000'/>\n";
	}
	else if (expectedFileName.contains("female")) {
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

QString PinHeader::makeBreadboardDoubleSvg(const QString & expectedFileName, int pins) {
	QString header("<?xml version='1.0' encoding='utf-8'?>\n"
				"<svg version='1.2' baseProfile='tiny' "
				"xmlns='http://www.w3.org/2000/svg'  x='0in' y='0in' width='%1in' "
				"height='0.2in' viewBox='0 0 %2 2000'>\n"
				"<g id='breadboard' >\n");

	QString fileForm;
	if (expectedFileName.contains("round")) {
		fileForm = "rounded_female";
		header += "<rect fill='#404040' width='%2' height='2000'/>\n";
	}
	else if (expectedFileName.contains("female")) {
		fileForm = "female";
		header += "<rect fill='#404040' width='%2' height='2000'/>\n";
	}
	else {
		fileForm = "male";
	}

	double unitHeight = 0.1;  // inches
	double unitHeightPoints = unitHeight * 10000;

	int userData[2];
	userData[0] = pins;
	userData[1] = 1;	QString svg = header.arg(unitHeight * pins / 2).arg(unitHeightPoints * pins / 2);
	svg += TextUtils::incrementTemplate(QString(":/resources/templates/generic_%1_pin_header_bread_template.txt").arg(fileForm),
							 pins / 2, unitHeightPoints, TextUtils::standardMultiplyPinFunction, TextUtils::negIncCopyPinFunction, userData);

	svg += TextUtils::incrementTemplate(QString(":/resources/templates/generic_%1_pin_header_bread_2nd_template.txt").arg(fileForm),
							 pins / 2, unitHeightPoints, TextUtils::standardMultiplyPinFunction, TextUtils::standardCopyPinFunction, NULL);

	svg += "</g>\n</svg>";

	return svg;
}


QString PinHeader::makeBreadboardShroudedSvg(int pins) 
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

QString PinHeader::makePcbShroudedSvg(int pins) 
{
	QString header("<?xml version='1.0' encoding='utf-8'?>\n"
					"<svg version='1.2' baseProfile='tiny' xmlns='http://www.w3.org/2000/svg' \n"
					"x='0in' y='0in' width='0.3542in' height='%1in' viewBox='0 0 3542 [3952]'>"
					"<g id='copper0' >\n"					
					"<g id='copper1' >\n"
					"<rect id='square' x='936' y='1691' width='570' height='570' stroke='#ff9400' r='285' fill='none' stroke-width='170'/>\n"
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

QString PinHeader::makePcbSMDSvg(const QString & expectedFileName, const QString & moduleID) 
{
    Q_UNUSED(moduleID);
	QStringList pieces = expectedFileName.split("_");

	int pins = pieces.at(5).toInt();
	QString spacingString = pieces.at(6);

	bool singleRow = expectedFileName.contains("single");

	double spacing = TextUtils::convertToInches(spacingString) * GraphicsUtils::StandardFritzingDPI;   

	QString header("<?xml version='1.0' encoding='utf-8'?>\n"
				"<svg version='1.2' baseProfile='tiny' xmlns:svg='http://www.w3.org/2000/svg' "
				"xmlns='http://www.w3.org/2000/svg'  x='0in' y='0in' width='%1in' "
				"height='%4in' viewBox='0 0 %2 %5'>\n"
				"<g id='silkscreen' >\n"
				"<rect x='2' y='86.708672' width='%3' height='%6' fill='none' stroke='#ffffff' stroke-width='4' stroke-opacity='1'/>\n"
				"</g>\n"
				"<g id='copper1'>\n");

	
	double baseWidth = 152.7559;			// mils
	double totalHeight = 283.4646;
	double totalWidth = baseWidth + ((pins - 1) * spacing);
	double rectHeight = 102.047256;
	double y = 141.823;
	if (!singleRow) {
		totalHeight = 393.7;
		totalWidth = baseWidth + ((pins - 2) * spacing / 2);
		rectHeight = 200;
		y = 251.9685;
	}

	QString svg = header.arg(totalWidth / GraphicsUtils::StandardFritzingDPI)
						.arg(totalWidth)
						.arg(totalWidth - 4)
						.arg(totalHeight  / GraphicsUtils::StandardFritzingDPI)
						.arg(totalHeight)
						.arg(rectHeight);

	double x = 51.18110;
	if (singleRow) {
		for (int i = 0; i < pins; i++) {
			double ay = (i % 2 == 0) ? 0 : y;
			svg += QString("<rect id='connector%1pin' x='%2' y='%3' width='51.18110' height='141.823' fill='#f7bf13' fill-opacity='1' stroke='none' stroke-width='0'/>\n").arg(i).arg(x).arg(ay);
			x += spacing;
		}
	}
	else {
		double holdX = x;
		for (int i = 0; i < pins / 2; i++) {
			svg += QString("<rect id='connector%1pin' x='%2' y='0' width='51.18110' height='141.823' fill='#f7bf13' fill-opacity='1' stroke='none' stroke-width='0'/>\n").arg(pins - 1 - i).arg(x);
			x += spacing;
		}
		x = holdX;
		for (int i = 0; i < pins / 2; i++) {
			svg += QString("<rect id='connector%1pin' x='%2' y='%3' width='51.18110' height='141.823' fill='#f7bf13' fill-opacity='1' stroke='none' stroke-width='0'/>\n").arg(i).arg(x).arg(y);
			x += spacing;
		}
	}

	svg += "</g>\n</svg>";
	return svg;
}

bool PinHeader::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget) 
{
	if (prop.compare("hole size", Qt::CaseInsensitive) == 0) {
        if (moduleID().contains("smd", Qt::CaseInsensitive)) {
            // or call the ancestor function?
            return false;
        }
        else {
            return collectHoleSizeInfo(TheHoleThing.holeSizeValue, parent, swappingEnabled, returnProp, returnValue, returnWidget);
        }
	}

	return PaletteItem::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

void PinHeader::swapEntry(const QString & text) {
    generateSwap(text, genModuleID, genFZP, makeBreadboardSvg, makeSchematicSvg, makePcbSvg);
    PaletteItem::swapEntry(text);
}
