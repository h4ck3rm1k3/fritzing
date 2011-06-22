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

static QStringList Forms;
QString PinHeader::FemaleFormString;
QString PinHeader::FemaleRoundedFormString;
QString PinHeader::MaleFormString;
static int MinPins = 1;
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

		for (int i = MinPins; i <= MaxPins; i++) {
			values << QString::number(i);
		}
		
		return values;
	}

	if (prop.compare("pin spacing", Qt::CaseInsensitive) == 0) {
		initSpacings();
		QStringList values;
		value = modelPart()->properties().value("pin spacing");

		values = Spacings.values();
		
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

void PinHeader::addedToScene()
{
	if (this->scene()) {
		setForm(m_form, true);
	}

    return PaletteItem::addedToScene();
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
		Forms << FemaleFormString << FemaleRoundedFormString << MaleFormString;
	}
	return Forms;
}

bool PinHeader::onlyFormChanges(QMap<QString, QString> & propsMap) {
	if (propsMap.value("form", "").compare(m_form) == 0) return false;

	if (modelPart()->properties().value("pins", "").compare(propsMap.value("pins", "")) != 0) return false;

	return true;
}

bool PinHeader::hasCustomSVG() {
	switch (m_viewIdentifier) {
		case ViewIdentifierClass::BreadboardView:
		case ViewIdentifierClass::SchematicView:
			return true;
		default:
			return ItemBase::hasCustomSVG();
	}
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
	return result.arg(Spacings.value(spacing, "")).arg(spacing); 
}

QString PinHeader::genModuleID(QMap<QString, QString> & currPropsMap)
{
	initSpacings();
	QString pins = currPropsMap.value("pins");
	QString spacing = currPropsMap.value("pin spacing");

	foreach (QString key, Spacings.keys()) {
		if (Spacings.value(key).compare(spacing, Qt::CaseInsensitive) == 0) {
			return QString("generic_female_pin_header_%1_%2").arg(pins).arg(key);
		}
	}

	return "";
}

QString PinHeader::makePcbSvg(const QString & moduleID) 
{
	initSpacings();

	QStringList pieces = moduleID.split("_");
	if (pieces.count() != 6) return "";

	int pins = pieces.at(4).toInt();
	QString spacingString = pieces.at(5);

	static QString pcbLayerTemplate = "";

	QFile file(":/resources/templates/jumper_pcb_svg_template.txt");
	file.open(QFile::ReadOnly);
	pcbLayerTemplate = file.readAll();
	file.close();

	qreal outerBorder = 15;
	qreal innerBorder = outerBorder / 2;
	qreal silkStrokeWidth = 10;
	qreal radius = 27.5;
	qreal copperStrokeWidth = 20;
	qreal totalWidth = (outerBorder * 2) + (silkStrokeWidth * 2) + (innerBorder * 2) + (radius * 2) + copperStrokeWidth;
	qreal center = totalWidth / 2;
	qreal spacing =TextUtils::convertToInches(spacingString) * 1000; 

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

	qreal totalHeight = totalWidth + (pins * spacing) - spacing;

	QString svg = pcbLayerTemplate
					.arg(totalWidth / 1000)
					.arg(totalHeight / 1000)
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