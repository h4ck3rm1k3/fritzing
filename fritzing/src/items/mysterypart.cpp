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

#include "mysterypart.h"
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

static QStringList Spacings;
static QRegExp Digits("(\\d)+");
static QRegExp DigitsMil("(\\d)+mil");

static int MinSipPins = 1;
static int MaxSipPins = 64;
static int MinDipPins = 4;
static int MaxDipPins = 64;



// TODO
//	save into parts bin

MysteryPart::MysteryPart( ModelPart * modelPart, ViewIdentifierClass::ViewIdentifier viewIdentifier, const ViewGeometry & viewGeometry, long id, QMenu * itemMenu, bool doLabel)
	: PaletteItem(modelPart, viewIdentifier, viewGeometry, id, itemMenu, doLabel)
{
	m_changingSpacing = false;
	m_chipLabel = modelPart->prop("chip label").toString();
	if (m_chipLabel.isEmpty()) {
		m_chipLabel = modelPart->properties().value("chip label", "?");
		modelPart->setProp("chip label", m_chipLabel);
	}
	m_spacing = modelPart->prop("spacing").toString();
	if (m_spacing.isEmpty()) {
		m_spacing = modelPart->properties().value("spacing", "300mil");
		modelPart->setProp("spacing", m_spacing);
	}

	m_renderer = NULL;
}

MysteryPart::~MysteryPart() {
}

void MysteryPart::setProp(const QString & prop, const QString & value) {
	if (prop.compare("chip label", Qt::CaseInsensitive) == 0) {
		setChipLabel(value, false);
		return;
	}

	if (prop.compare("spacing", Qt::CaseInsensitive) == 0) {
		setSpacing(value, false);
		return;
	}

	PaletteItem::setProp(prop, value);
}

void MysteryPart::setSpacing(QString spacing, bool force) {
	if (!force && m_spacing.compare(spacing) == 0) return;
	if (!isDIP()) return;

	switch (this->m_viewIdentifier) {
		case ViewIdentifierClass::BreadboardView:
		case ViewIdentifierClass::PCBView:
			{
				InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
				if (infoGraphicsView == NULL) break;

				// hack the dom element and call setUpImage
				FSvgRenderer::removeFromHash(moduleID(), "");
				QDomElement element = LayerAttributes::getSvgElementLayers(modelPart()->domDocument(), m_viewIdentifier);
				if (element.isNull()) break;

				QString filename = element.attribute("image");
				if (filename.isEmpty()) break;

				if (spacing.indexOf(Digits) < 0) break;

				QString newSpacing = Digits.cap(0);		
				filename.replace(DigitsMil, newSpacing + "mil");
				element.setAttribute("image", filename);

				m_changingSpacing = true;
				resetImage(infoGraphicsView);
				m_changingSpacing = false;

				if (m_viewIdentifier == ViewIdentifierClass::BreadboardView) {
					if (modelPart()->properties().value("chip label", "").compare(m_chipLabel) != 0) {
						setChipLabel(m_chipLabel, true);
					}
				}

				updateConnections();
			}
			break;
		default:
			break;
	}

	m_spacing = spacing;
	modelPart()->setProp("spacing", spacing);

	updateTooltip();
    if (m_partLabel) m_partLabel->displayTextsIf();

}

void MysteryPart::setChipLabel(QString chipLabel, bool force) {

	if (!force && m_chipLabel.compare(chipLabel) == 0) return;

	QString svg;
	switch (this->m_viewIdentifier) {
		case ViewIdentifierClass::BreadboardView:
		case ViewIdentifierClass::SchematicView:
			svg = makeSvg(chipLabel);
			break;
		default:
			break;
	}

	if (!svg.isEmpty()) {
		if (m_renderer == NULL) {
			m_renderer = new FSvgRenderer(this);
		}
		//DebugDialog::debug(svg);

		bool result = m_renderer->fastLoad(svg.toUtf8());
		if (result) {
			setSharedRendererEx(m_renderer);
		}
	}

	m_chipLabel = chipLabel;
	modelPart()->setProp("chip label", chipLabel);

	updateTooltip();
    if (m_partLabel) m_partLabel->displayTextsIf();

}

QString MysteryPart::retrieveSvg(ViewLayer::ViewLayerID viewLayerID, QHash<QString, QString> & svgHash, bool blackOnly, qreal dpi) 
{
	QString svg = PaletteItem::retrieveSvg(viewLayerID, svgHash, blackOnly, dpi);
	switch (viewLayerID) {
		case ViewLayer::Breadboard:
		case ViewLayer::Schematic:
		case ViewLayer::Icon:
			return TextUtils::replaceTextElement(svg, m_chipLabel);
		default:
			break;
	}

	return svg; 
}

QString MysteryPart::makeSvg(const QString & chipLabel) {
	QString path = filename();
	QFile file(filename());
	QString svg;
	if (file.open(QFile::ReadOnly)) {
		svg = file.readAll();
		file.close();
		return TextUtils::replaceTextElement(svg, chipLabel);
	}

	return "";
}

QStringList MysteryPart::collectValues(const QString & family, const QString & prop, QString & value) {
	if (prop.compare("spacing", Qt::CaseInsensitive) == 0) {
		QStringList values;
		if (isDIP()) {
			foreach (QString f, spacings()) {
				values.append(f);
			}
		}
		else {
			values.append(m_spacing);
		}

		value = m_spacing;
		return values;
	}

	if (prop.compare("pins", Qt::CaseInsensitive) == 0) {
		QStringList values;
		value = modelPart()->properties().value("pins");

		if (moduleID().contains("dip")) {
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


	return PaletteItem::collectValues(family, prop, value);
}

bool MysteryPart::collectExtraInfo(QWidget * parent, const QString & family, const QString & prop, const QString & value, bool swappingEnabled, QString & returnProp, QString & returnValue, QWidget * & returnWidget)
{
	if (prop.compare("chip label", Qt::CaseInsensitive) == 0) {
		returnProp = tr("label");
		returnValue = m_chipLabel;

		QLineEdit * e1 = new QLineEdit(parent);
		e1->setEnabled(swappingEnabled);
		e1->setText(m_chipLabel);
		connect(e1, SIGNAL(editingFinished()), this, SLOT(chipLabelEntry()));
		e1->setObjectName("infoViewLineEdit");


		returnWidget = e1;

		return true;
	}

	return PaletteItem::collectExtraInfo(parent, family, prop, value, swappingEnabled, returnProp, returnValue, returnWidget);
}

QString MysteryPart::getProperty(const QString & key) {
	if (key.compare("chip label", Qt::CaseInsensitive) == 0) {
		return m_chipLabel;
	}

	if (key.compare("spacing", Qt::CaseInsensitive) == 0) {
		return m_spacing;
	}

	return PaletteItem::getProperty(key);
}

QString MysteryPart::chipLabel() {
	return m_chipLabel;
}

void MysteryPart::addedToScene()
{
	if (this->scene()) {
		setChipLabel(m_chipLabel, true);
		setSpacing(m_spacing, true);
	}

    return PaletteItem::addedToScene();
}

const QString & MysteryPart::title() {
	return m_chipLabel;
}

bool MysteryPart::hasCustomSVG() {
	switch (m_viewIdentifier) {
		case ViewIdentifierClass::BreadboardView:
		case ViewIdentifierClass::SchematicView:
		case ViewIdentifierClass::IconView:
		case ViewIdentifierClass::PCBView:
			return true;
		default:
			return ItemBase::hasCustomSVG();
	}
}

void MysteryPart::chipLabelEntry() {
	QLineEdit * edit = dynamic_cast<QLineEdit *>(sender());
	if (edit == NULL) return;

	if (edit->text().compare(this->chipLabel()) == 0) return;

	InfoGraphicsView * infoGraphicsView = InfoGraphicsView::getInfoGraphicsView(this);
	if (infoGraphicsView != NULL) {
		infoGraphicsView->setProp(this, "chip label", tr("chip label"), this->chipLabel(), edit->text(), true);
	}
}

ConnectorItem* MysteryPart::newConnectorItem(Connector *connector) {
	if (m_changingSpacing) {
		return connector->connectorItemByViewLayerID(viewIdentifier(), viewLayerID());
	}

	return PaletteItem::newConnectorItem(connector);
}

ConnectorItem* MysteryPart::newConnectorItem(ItemBase * layerKin, Connector *connector) {
	if (m_changingSpacing) {
		return connector->connectorItemByViewLayerID(viewIdentifier(), layerKin->viewLayerID());
	}

	return PaletteItem::newConnectorItem(layerKin, connector);
}

const QString & MysteryPart::spacing() {
	return m_spacing;
}

bool MysteryPart::onlySpacingChanges(QMap<QString, QString> & propsMap) {
	if (propsMap.value("spacing", "").compare(m_spacing) == 0) return false;

	if (modelPart()->properties().value("pins", "").compare(propsMap.value("pins", "")) != 0) return false;

	if (otherPropsChange(propsMap)) return false;

	return true;
}

bool MysteryPart::isDIP() {
	QString layout = modelPart()->properties().value("layout", "");
	return (layout.indexOf("double", 0, Qt::CaseInsensitive) >= 0);
}

bool MysteryPart::otherPropsChange(const QMap<QString, QString> & propsMap) {
	QString layout = modelPart()->properties().value("layout", "");
	return (layout.compare(propsMap.value("layout", "")) != 0);
}

const QStringList & MysteryPart::spacings() {
	if (Spacings.count() == 0) {
		Spacings << "100mil" << "200mil" << "300mil" << "400mil" << "500mil" << "600mil" << "700mil" << "800mil" << "900mil";
	}
	return Spacings;
}

ItemBase::PluralType MysteryPart::isPlural() {
	return Plural;
}

QString MysteryPart::genSipFZP(const QString & moduleid)
{
	return PaletteItem::genFZP(moduleid, "mystery_part_sipFzpTemplate", MinSipPins, MaxSipPins, 1);
}

QString MysteryPart::genDipFZP(const QString & moduleid)
{
	return PaletteItem::genFZP(moduleid, "mystery_part_dipFzpTemplate", MinDipPins, MaxDipPins, 2);
}

QString MysteryPart::genModuleID(QMap<QString, QString> & currPropsMap)
{
	QString value = currPropsMap.value("layout");
	QString pins = currPropsMap.value("pins");
	if (value.contains("single", Qt::CaseInsensitive)) {
		return QString("mystery_part_%1").arg(pins);
	}
	else {
		int p = pins.toInt();
		if (p < 4) p = 4;
		if (p % 2 == 1) p--;
		return QString("mystery_part_%1_dip_300mil").arg(p);
	}
}

QString MysteryPart::makeSchematicSvg(const QString & expectedFileName) 
{
	bool sip = expectedFileName.contains("sip", Qt::CaseInsensitive);

	QStringList pieces = expectedFileName.split("_");
	if (sip) {
		if (pieces.count() != 5) return "";
	}
	else {
		if (pieces.count() != 4) return "";
	}

	int pins = pieces.at(2).toInt();
	int increment = 300;
	qreal totalHeight = (pins * increment) + 330;
	int border = 30;
	int textOffset = 50;
	int repeatTextOffset = 50;
	int fontSize = 255;
	QString labelText = "?";

	QString header("<?xml version='1.0' encoding='UTF-8' standalone='no'?>\n"
					"<svg\n"
					"xmlns:svg='http://www.w3.org/2000/svg'\n"
					"xmlns='http://www.w3.org/2000/svg'\n"
					"version='1.2' baseProfile='tiny'\n"
					"width='1.53in' height='%1in' viewBox='0 0 1530 %2'>\n"
					"<g id='schematic'>\n"
					"<rect x='315' y='15' fill='none' width='1200' height='%3' stroke='#000000' stroke-linejoin='round' stroke-linecap='round' stroke-width='30' />\n"
					"<text id='label' x='915' y='%4' fill='#000000' stroke='none' font-family='DroidSans' text-anchor='middle' font-size='%5' >%6</text>\n");

	if (!sip) {
		header +=	"<circle fill='#000000' cx='1330' cy='200' r='150' stroke-width='0' />\n"
					"<text x='1330' fill='#FFFFFF' y='305' font-family='DroidSans' text-anchor='middle' font-weight='bold' stroke-width='0' font-size='275' >?</text>\n";
	}
	else {
		labelText = "IC";
		fontSize = 235;
		textOffset = 0;
	}

	QString svg = header
		.arg(totalHeight / 1000)
		.arg(totalHeight)
		.arg(totalHeight - border)
		.arg((totalHeight / 2) + textOffset)
		.arg(fontSize)
		.arg(labelText);


	QString repeat("<line fill='none' stroke='#000000' stroke-linejoin='round' stroke-linecap='round' stroke-width='30' x1='15' y1='%1' x2='300' y2='%1'  />\n"
					"<rect x='0' y='%2' fill='none' width='300' height='30' id='connector%3pin' stroke-width='0' />\n"
					"<rect x='0' y='%2' fill='none' width='30' height='30' id='connector%3terminal' stroke-width='0' />\n"
					"<text id='label%3' x='390' y='%4' font-family='DroidSans' stroke='none' fill='#000000' text-anchor='start' font-size='130' >%5</text>\n");
  
	for (int i = 0; i < pins; i++) {
		svg += repeat
			.arg(315 + (i * increment))
			.arg(300 + (i * increment))
			.arg(i)
			.arg(300 + repeatTextOffset + (i * increment))
			.arg(i + 1);
	}

	svg += "</g>\n";
	svg += "</svg>\n";
	return svg;
}

QString MysteryPart::makeBreadboardSvg(const QString & expectedFileName) 
{
	if (expectedFileName.contains("_sip_")) return makeBreadboardSipSvg(expectedFileName);

	return "";
}

QString MysteryPart::makeBreadboardSipSvg(const QString & expectedFileName) 
{
	QStringList pieces = expectedFileName.split("_");
	int pins = pieces.at(2).toInt();
	int increment = 10;

	QString header("<?xml version='1.0' encoding='utf-8'?>\n"
					"<svg version='1.2' baseProfile='tiny' id='svg2' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink'\n"
					"width='%1in' height='0.27586in' viewBox='0 0 [6.0022] 27.586' xml:space='preserve'>\n"
					"<g id='breadboard'>\n"
					"<rect width='[6.0022]' x='0' y='0' height='24.17675' fill='#000000' id='upper' stroke-width='0' />\n"
					"<rect width='[6.0022]' x='0' y='22' fill='#404040' height='3.096' id='lower' stroke-width='0' />\n"
					"<text id='label' x='2.5894' y='13' fill='#e6e6e6' stroke='none' font-family='DroidSans' text-anchor='start' font-size='7.3' >?</text>\n"
					"<circle fill='#8C8C8C' cx='[1.0022]' cy='5' r='3' stroke-width='0' />\n"
					"<text x='[1.0022]' y='6.7' font-family='DroidSans' text-anchor='middle' font-weight='bold' stroke-width='0' font-size='5.5' >?</text>\n"      
					"%2\n"
					"</g>\n"
					"</svg>\n");

	QString svg = TextUtils::incrementTemplateString(header, 1, increment * (pins - 1), incMultiplyPinFunction, noCopyPinFunction);

	QString repeat("<rect id='connector%1terminal' stroke='none' stroke-width='0' x='[1.87]' y='25.586' fill='#8C8C8C' width='2.3' height='2.0'/>\n"
					"<rect id='connector%1pin' stroke='none' stroke-width='0' x='[1.87]' y='23.336' fill='#8C8C8C' width='2.3' height='4.25'/>\n");

	QString repeats = TextUtils::incrementTemplateString(repeat, pins, increment, TextUtils::standardMultiplyPinFunction, TextUtils::standardCopyPinFunction);

	int ix1 = svg.indexOf("viewBox='");
	int ix2 = svg.indexOf("'", ix1 + 9);
	QString vb = svg.mid(ix1 + 9, ix2 - ix1 - 1);
	QStringList coords = vb.split(" ");
	return svg.arg(coords.at(2).toDouble() / 100).arg(repeats);
}

QString MysteryPart::noCopyPinFunction(int, const QString & argString) 
{ 
	return argString; 
}

QString MysteryPart::incCopyPinFunction(int pin, const QString & argString) 
{ 
	return argString.arg(pin + 1); 
}

QString MysteryPart::incMultiplyPinFunction(int pin, qreal increment, qreal value) 
{
	return QString::number(value + ((pin + 1) * increment));
}